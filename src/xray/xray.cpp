#include "xray.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#ifdef ENABLE_OPENXR
    #include <openxr/openxr.h>
    #include <SDL3/SDL_openxr.h>
#endif

namespace {
    //Make separate for shapes?
    struct CirclePushConstants {
        float view_projection[16]; // Offset   0, Size 64
        float center[2];           // Offset  64, Size  8
        float padding1[2]{0, 0};   // Offset  72, Size  8 (Aligns color to offset 80)
        float color[4];            // Offset  80, Size 16
        float radius;              // Offset  96, Size  4
        float feather;             // Offset 100, Size  4
        float padding2[2]{0, 0};   // Offset 104, Size  8 (Total struct size: 112 bytes)
    };

    struct EngineState {
        float deltaTime = 0.0f;
        bool running = true;

        SDL_Window* window = nullptr;
        SDL_GPUDevice* device = nullptr;
        
        // Active frame structures
        SDL_GPUCommandBuffer* currentCmdBuffer = nullptr;
        SDL_GPURenderPass* currentRenderPass = nullptr;
        SDL_GPUTexture* swapchainTexture = nullptr;

        // Pipelines
        SDL_GPUGraphicsPipeline* sdfCirclePipeline = nullptr;

#ifdef ENABLE_OPENXR
        XrInstance xrInstance = XR_NULL_HANDLE;
        XrSystemId xrSystemId = XR_NULL_SYSTEM_ID;
        XrSession xrSession = XR_NULL_HANDLE;
        XrSpace xrLocalSpace = XR_NULL_HANDLE;
        XrSwapchain swapchain;
        SDL_GPUTexture **images;
        XrExtent2Di size;
        SDL_GPUTextureFormat format;
        uint32_t imageCount;
        bool xrSessionRunning = false;
        bool xrShouldQuit = false;
#endif
    };

    EngineState state;

    // TODO need a method to compile automatically or load according to the system
    
    // Helper to load compiled SPIR-V / MSL shaders
    SDL_GPUShader* LoadShader(
        SDL_GPUDevice* device, 
        const char* basePath,
        Uint32 samplerCount, 
        Uint32 uniformBufferCount, 
        Uint32 storageBufferCount, 
        Uint32 storageTextureCount, 
        SDL_GPUShaderStage stage
    ) {
        SDL_GPUShaderFormat supportedFormats = SDL_GetGPUShaderFormats(device);
        
        char fullFilename[256];
        SDL_GPUShaderFormat shaderFormat;
        const char* entrypoint;

        if (supportedFormats & SDL_GPU_SHADERFORMAT_MSL) {
            // macOS (Metal)
            SDL_snprintf(fullFilename, sizeof(fullFilename), "%s.msl", basePath);
            shaderFormat = SDL_GPU_SHADERFORMAT_MSL;
            entrypoint = "main0"; // spirv-cross defaults to main0 for MSL
        } else if (supportedFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
            // Linux / Windows (Vulkan)
            SDL_snprintf(fullFilename, sizeof(fullFilename), "%s.spv", basePath);
            shaderFormat = SDL_GPU_SHADERFORMAT_SPIRV;
            entrypoint = "main";
        } else {
            SDL_Log("Unsupported GPU backend format");
            return nullptr;
        }

        size_t codeSize;
        void* code = SDL_LoadFile(fullFilename, &codeSize);
        if (!code) {
            SDL_Log("Failed to load shader file: %s", fullFilename);
            return nullptr;
        }

        SDL_GPUShaderCreateInfo createInfo{};
        createInfo.code = (const Uint8*)code;
        createInfo.code_size = codeSize;
        createInfo.entrypoint = entrypoint;
        createInfo.format = shaderFormat;
        createInfo.stage = stage;
        createInfo.num_samplers = samplerCount;
        createInfo.num_uniform_buffers = uniformBufferCount;
        createInfo.num_storage_buffers = storageBufferCount;
        createInfo.num_storage_textures = storageTextureCount;

        SDL_GPUShader* shader = SDL_CreateGPUShader(device, &createInfo);
        SDL_free(code);

        if (!shader) {
            SDL_Log("Failed to create GPU shader for %s: %s", fullFilename, SDL_GetError());
        }

        return shader;
    }

    bool InitSDFPipelines() {
       SDL_GPUShader* vertShader = LoadShader(
        state.device, 
        "assets/shaders/glsl450/sdf/circle.vert", 
        0, 1, 0, 0, 
        SDL_GPU_SHADERSTAGE_VERTEX
        );
        
        SDL_GPUShader* fragShader = LoadShader(
            state.device, 
            "assets/shaders/glsl450/sdf/circle.frag", 
            0, 1, 0, 0, 
            SDL_GPU_SHADERSTAGE_FRAGMENT
        );

        if (!vertShader || !fragShader) return false;

        SDL_GPUColorTargetDescription colorTargetDesc{};
        colorTargetDesc.format = SDL_GetGPUSwapchainTextureFormat(state.device, state.window);

        // Enable Alpha Blending
        colorTargetDesc.blend_state.enable_blend = true;

        // Color blending: src * src_alpha + dst * (1 - src_alpha)
        colorTargetDesc.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        colorTargetDesc.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTargetDesc.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;

        // Alpha blending: src * 1 + dst * (1 - src_alpha)
        colorTargetDesc.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        colorTargetDesc.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        colorTargetDesc.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

        SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.vertex_shader = vertShader;
        pipelineInfo.fragment_shader = fragShader;
        pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        pipelineInfo.target_info.color_target_descriptions = &colorTargetDesc;
        pipelineInfo.target_info.num_color_targets = 1;

        state.sdfCirclePipeline = SDL_CreateGPUGraphicsPipeline(state.device, &pipelineInfo);

        SDL_ReleaseGPUShader(state.device, vertShader);
        SDL_ReleaseGPUShader(state.device, fragShader);

        return state.sdfCirclePipeline != nullptr;
    }
}

namespace XRay {

    bool Init(const char* title, int windowWidth, int windowHeight) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_HAPTIC)) {
            SDL_Log("SDL_Init failed: %s", SDL_GetError());
            return false;
        }
    
        SDL_Log("SDL initialized");
        
        // Removed SDL_WINDOW_VULKAN so SDL3 GPU can automatically manage Metal on macOS
        state.window = SDL_CreateWindow(title, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY); 
        if (!state.window) {
            SDL_Log("Failed to create window: %s", SDL_GetError());
            return false;
        }

        state.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
        if (!state.device) {
            SDL_Log("Failed to create GPU device: %s", SDL_GetError());
            return false;
        }

        if (!SDL_ClaimWindowForGPUDevice(state.device, state.window)) {
            SDL_Log("Failed to claim window for GPU device: %s", SDL_GetError());
            return false;
        }

        if (!InitSDFPipelines()) {
            SDL_Log("Failed to initialize SDF Pipelines");
            return false;
        }

        return true;
    }

    void BeginDrawing() {
        // Acquire fresh command buffer for this frame
        state.currentCmdBuffer = SDL_AcquireGPUCommandBuffer(state.device);
        if (!state.currentCmdBuffer) {
            return;
        }
        
        Uint32 width, height;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(state.currentCmdBuffer, state.window, &state.swapchainTexture, &width, &height) || !state.swapchainTexture) {
            // Swapchain texture not available (e.g., window minimized). 
            // Submit empty command buffer and NULL OUT state handles so draw calls safely skip this frame.
            SDL_SubmitGPUCommandBuffer(state.currentCmdBuffer);
            state.currentCmdBuffer = nullptr;
            state.swapchainTexture = nullptr;
            return;
        }
    }

    void ClearBackground(Color color) {
        // Guard: Skip if frame acquisition failed in BeginDrawing()
        if (!state.currentCmdBuffer || !state.swapchainTexture) {
            return;
        }

        // If a render pass is already active, end it before starting a new clear pass
        if (state.currentRenderPass) {
            SDL_EndGPURenderPass(state.currentRenderPass);
            state.currentRenderPass = nullptr;
        }

        SDL_GPUColorTargetInfo colorTargetInfo{};
        colorTargetInfo.clear_color = {color.r, color.g, color.b, color.a};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = state.swapchainTexture;

        state.currentRenderPass = SDL_BeginGPURenderPass(state.currentCmdBuffer, &colorTargetInfo, 1, NULL);
    }

    void DrawCircleSDF(Vector2 center, float radius, Color color, float feather) {
        if (!state.currentRenderPass) return;

        // Bind Pipeline
        SDL_BindGPUGraphicsPipeline(state.currentRenderPass, state.sdfCirclePipeline);

        // Get current window pixel dimensions
        int w, h;
        SDL_GetWindowSizeInPixels(state.window, &w, &h);
        float width = static_cast<float>(w);
        float height = static_cast<float>(h);

        // Build Orthographic Projection Matrix (Top-Left 0,0 origin)
        CirclePushConstants pushConstants{};
        pushConstants.view_projection[0]  =  2.0f / width;
        pushConstants.view_projection[5]  = -2.0f / height; // Negative to flip Y to top-left origin
        pushConstants.view_projection[10] =  1.0f;
        pushConstants.view_projection[15] =  1.0f;
        pushConstants.view_projection[12] = -1.0f;
        pushConstants.view_projection[13] =  1.0f;

        pushConstants.center[0] = center.x;
        pushConstants.center[1] = center.y;
        pushConstants.color[0]  = color.r; 
        pushConstants.color[1]  = color.g;
        pushConstants.color[2]  = color.b; 
        pushConstants.color[3]  = color.a;
        pushConstants.radius    = radius;
        pushConstants.feather   = feather;

       // Bind Pipeline & Push Data
        SDL_BindGPUGraphicsPipeline(state.currentRenderPass, state.sdfCirclePipeline);
        SDL_PushGPUVertexUniformData(state.currentCmdBuffer, 0, &pushConstants, sizeof(pushConstants));
        SDL_PushGPUFragmentUniformData(state.currentCmdBuffer, 0, &pushConstants, sizeof(pushConstants));

        // Draw 6 procedural vertices (1 quad)
        SDL_DrawGPUPrimitives(state.currentRenderPass, 6, 1, 0, 0);
    }

    void EndDrawing() {
        // Guard: Skip if no command buffer is active
        if (!state.currentCmdBuffer) {
            return;
        }

        if (state.currentRenderPass) {
            SDL_EndGPURenderPass(state.currentRenderPass);
            state.currentRenderPass = nullptr;
        }

        // Submit frame and reset pointers
        SDL_SubmitGPUCommandBuffer(state.currentCmdBuffer);
        state.currentCmdBuffer = nullptr;
        state.swapchainTexture = nullptr;
    }

    void Shutdown() {
        if (state.device) {
            SDL_DestroyGPUDevice(state.device);
            state.device = nullptr;
        }
        if (state.window) {
            SDL_DestroyWindow(state.window);
            state.window = nullptr;
        }
        SDL_Quit();
    }
}