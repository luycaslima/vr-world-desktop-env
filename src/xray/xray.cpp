#include "xray.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_openxr.h>

namespace {
    // Hidden internal global state -- exactly how Raylib manages its internals
    struct EngineState {
        SDL_Window* window = nullptr;
        SDL_GPUDevice* device = nullptr;
        
        // Active frame structures
        SDL_GPUCommandBuffer* currentCmdBuffer = nullptr;
        SDL_GPURenderPass* currentRenderPass = nullptr;
        
        // XR specific state managed by SDL3
        // (SDL 3.6+ tracking handles for OpenXR swapchains)
        void* xrSwapchain = nullptr; 
        float deltaTime;
        bool running = true;
    };

    EngineState state;
}

namespace XRay {

    bool InitEngine(const char* title) {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_HAPTIC);
        
        // Tell SDL3 we want an OpenXR capable window & GPU backend
        state.window = SDL_CreateWindow(title, 1280, 720, SDL_WINDOW_OPENGL); 
        state.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
        SDL_ClaimWindowForGPUDevice(state.device, state.window);
        
        // Initialize OpenXR via SDL3 helper
        // state.xrSwapchain = SDL_CreateGPUXRSwapchain(...);
        
        return true;
    }

    void BeginDrawing() {
        // Every frame starts by getting a fresh command buffer from SDL3 GPU
        state.currentCmdBuffer = SDL_AcquireGPUCommandBuffer(state.device);
    }

    void BeginEyeDrawing(int eye) {
        // 1. Get the target texture handle from your OpenXR swapchain for this eye
        SDL_GPUTexture* eyeTargetTexture = nullptr; 
        // SDL_GetGPUXRSwapchainTexture(state.xrSwapchain, eye, &eyeTargetTexture);

        // 2. Set up the modern SDL3 Render Pass Color Target
        SDL_GPUColorTargetInfo colorTarget = {};
        colorTarget.texture = eyeTargetTexture;
        colorTarget.load_op = SDL_GPU_LOADOP_CLEAR; // Clear the screen
        colorTarget.store_op = SDL_GPU_STOREOP_STORE;
        
        // 3. Kick off the render pass just like Raylib's BeginDrawing() setting up a camera
        state.currentRenderPass = SDL_BeginGPURenderPass(
            state.currentCmdBuffer, 
            &colorTarget, 1, 
            nullptr
        );
    }

    void EndEyeDrawing() {
        // End the draw commands for this eye
        SDL_EndGPURenderPass(state.currentRenderPass);
        state.currentRenderPass = nullptr;
    }

    void EndDrawing() {
        // Submit all our batched eye rendering work directly to the GPU hardware
        SDL_SubmitGPUCommandBuffer(state.currentCmdBuffer);
        state.currentCmdBuffer = nullptr;
    }

    void ShutdownEngine() {
        SDL_DestroyGPUDevice(state.device);
        SDL_DestroyWindow(state.window);
        SDL_Quit();
    }
}