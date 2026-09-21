



#include "infinite_canvas/canvas.hpp"
int main(int argc, char *argv[])
{
    Canvas canvas;
    canvas.Init();

    canvas.Run();

    canvas.Destroy();

    // XRay::Init("Test", 1280, 720);
    // bool is_running = true;
    // //XRay::Camera3D camera = {};
    // SDL_Event event;
    // // Main application loop
    // while (is_running) {

    //     // Inner loop: Poll all currently pending events
    //     while (SDL_PollEvent(&event)) {
            
    //         // Check if the user requested to close the window
    //         if (event.type == SDL_EVENT_QUIT) {
    //             is_running = false;
    //         }
            
    //         // Optional: Close window when pressing the Escape key
    //         else if (event.type == SDL_EVENT_KEY_DOWN) {
    //             if (event.key.key == SDLK_ESCAPE) {
    //                 is_running = false;
    //             }
    //         }
    //     }
    //     // Add game logic updates and rendering here
    //     XRay::BeginDrawing();
    //     XRay::ClearBackground(XRay::Colors::White);

    //     XRay::EndDrawing();
    // }

    
    // XRay::Shutdown();
    

    return 0;
}