
#include "SDL3/SDL_events.h"
#include "xray/xray.hpp"


int main(int argc, char *argv[])
{
    XRay::InitEngine("Test", 1280, 720);

    bool is_running = true;
    SDL_Event event;

    // Main application loop
    while (is_running) {
        
        // Inner loop: Poll all currently pending events
        while (SDL_PollEvent(&event)) {
            
            // Check if the user requested to close the window
            if (event.type == SDL_EVENT_QUIT) {
                is_running = false;
            }
            
            // Optional: Close window when pressing the Escape key
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    is_running = false;
                }
            }
        }
        // Add game logic updates and rendering here
    }

    XRay::Shutdown();
    
    return 0;
}