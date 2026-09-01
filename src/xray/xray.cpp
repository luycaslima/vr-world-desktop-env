#include "xray.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#ifdef ENABLE_OPENXR
    #include <openxr/openxr.h>
    #include <SDL3/SDL_openxr.h>
#endif

namespace {
    struct EngineState {
        float deltaTime = 0.0f;
        bool running = true;

        SDL_Window* window = nullptr;
        SDL_GPUDevice* device = nullptr;
        
        // Active frame structures
        SDL_GPUCommandBuffer* currentCmdBuffer = nullptr;
        SDL_GPURenderPass* currentRenderPass = nullptr;
        SDL_GPUTexture* swapchainTexture = nullptr;

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