#include "xray.hpp"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include <openxr/openxr.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_openxr.h>

namespace {
    // Hidden internal global state -- exactly how Raylib manages its internals
    struct EngineState {

        float deltaTime;
        bool running = true;

        SDL_Window* window = nullptr;
        SDL_GPUDevice* device = nullptr;
        SDL_Event *event =nullptr;

        
        /* SDL GPU state */
        SDL_GPUGraphicsPipeline *pipeline = nullptr;
        SDL_GPUBuffer *vertexBuffer = nullptr;
        SDL_GPUBuffer *indexBuffer = nullptr;
        
        // Active frame structures
        SDL_GPUCommandBuffer* currentCmdBuffer = nullptr;
        SDL_GPURenderPass* currentRenderPass = nullptr;
        // get the swapchain texture
        SDL_GPUTexture* swapchainTexture= nullptr;
        

        // XR specific state managed by SDL3
        // (SDL 3.6+ tracking handles for OpenXR swapchains)
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
        // Tell SDL3 we want an OpenXR capable window & GPU backend
        state.window = SDL_CreateWindow(title, windowWidth, windowHeight, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE); 

        state.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
        SDL_ClaimWindowForGPUDevice(state.device, state.window);
        

        // Initialize OpenXR via SDL3 helper
        // state.xrSwapchain = SDL_CreateGPUXRSwapchain(...);
        
        return true;
    }

    //This doesnt work because the state isnt the one at main
    bool ShouldClose() {
        return state.event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
    }

    void BeginDrawing() {
        // Every frame starts by getting a fresh command buffer from SDL3 GPU
        state.currentCmdBuffer = SDL_AcquireGPUCommandBuffer(state.device);
        
       
        Uint32 width, height;
        SDL_WaitAndAcquireGPUSwapchainTexture(state.currentCmdBuffer, state.window, &state.swapchainTexture, &width, &height);

        // end the frame early if a swapchain texture is not available
        if (state.swapchainTexture == NULL)
        {
            // you must always submit the command buffer
            SDL_SubmitGPUCommandBuffer(state.currentCmdBuffer);
            return;
        }

    
    }

    void ClearBackground(Color color){
        SDL_GPUColorTargetInfo colorTargetInfo{};

        // discard previous content and clear to a color
        colorTargetInfo.clear_color = {color.r, color.g, color.b, color.a};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR; // or SDL_GPU_LOADOP_LOAD to 
                                                        // keep the previous content

        // store the content to the texture
        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

        // where are we going to store the result?
        colorTargetInfo.texture = state.swapchainTexture; // we will set this later to the 
                                   // window's swapchain texture

                                   // begin a render pass
        state.currentRenderPass = SDL_BeginGPURenderPass(state.currentCmdBuffer, &colorTargetInfo, 1, NULL);
    }


    void EndDrawing() {
        SDL_EndGPURenderPass(state.currentRenderPass);

        // Submit all our batched eye rendering work directly to the GPU hardware
        SDL_SubmitGPUCommandBuffer(state.currentCmdBuffer);
        state.currentCmdBuffer = nullptr;
        state.swapchainTexture = nullptr;
    }
    
    // void BeginEyeDrawing(int eye) {
    //     // 1. Get the target texture handle from your OpenXR swapchain for this eye
    //     SDL_GPUTexture* eyeTargetTexture = nullptr; 
    //     // SDL_GetGPUXRSwapchainTexture(state.xrSwapchain, eye, &eyeTargetTexture);

    //     // 2. Set up the modern SDL3 Render Pass Color Target
    //     SDL_GPUColorTargetInfo colorTarget = {};
    //     colorTarget.texture = eyeTargetTexture;
    //     colorTarget.load_op = SDL_GPU_LOADOP_CLEAR; // Clear the screen
    //     colorTarget.store_op = SDL_GPU_STOREOP_STORE;
        
    //     // 3. Kick off the render pass just like Raylib's BeginDrawing() setting up a camera
    //     state.currentRenderPass = SDL_BeginGPURenderPass(
    //         state.currentCmdBuffer, 
    //         &colorTarget, 1, 
    //         nullptr
    //     );
    // }

    // void EndEyeDrawing() {
    //     // End the draw commands for this eye
    //     SDL_EndGPURenderPass(state.currentRenderPass);
    //     state.currentRenderPass = nullptr;
    // }

    void Shutdown() {
        SDL_DestroyGPUDevice(state.device);
        SDL_DestroyWindow(state.window);
        SDL_Quit();
    }
}