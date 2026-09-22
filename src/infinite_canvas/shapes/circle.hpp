#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "xray/xray.hpp"

struct Circle {
    //XRay::Matrix matrix;

    XRay::Color color{};
    XRay::Vector2 pos{};
    XRay::Vector2 scale{1.0f,1.0f};
    float radius{1.0f};
    float rotation = 0.0f;


    Circle() = default; 
    Circle(XRay::Color color, XRay::Vector2 position, float r) 
        : color(color), pos(position), radius(r) {}

    void Draw() const {
        XRay::DrawCircleSDF(pos, radius, color);
    }
};


#endif