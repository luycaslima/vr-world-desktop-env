#include "xray/xray.hpp"
#include "canvas.hpp"
#include "infinite_canvas/shapes/circle.hpp"



void Canvas::Init(){
    shape = Circle(XRay::Colors::Red, XRay::Vector2(200.0f, 200.0f), 100.0f);
    XRay::Init("Inifite Canvas", 1280, 720);
    is_running = true;

}


void Canvas::Update(){

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
}


void Canvas::Draw(){
    XRay::BeginDrawing();
    XRay::ClearBackground(XRay::Colors::White);
    
    shape.Draw();


    XRay::EndDrawing();
}

void Canvas::Destroy(){
    XRay::Shutdown();
}

void Canvas::Run(){
    while (is_running) {
        Update();
        Draw();
    }
}