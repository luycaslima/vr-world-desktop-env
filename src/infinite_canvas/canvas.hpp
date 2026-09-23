#ifndef CANVAS_HPP
#define CANVAS_HPP

// TODO manage SDL events on xray
#include "SDL3/SDL_events.h"
#include "infinite_canvas/shapes/circle.hpp"
#include "xray/xray.hpp"
class Canvas {

    private:
    Circle shape;
    XRay::Camera2D cam;
    
    SDL_Event event{};
    
    bool is_running = false;
    void Draw();
    void Update();
    
    public:
    Canvas() = default;
    ~Canvas() = default;

    void Init();
    void Run();
    void Destroy();

};


#endif