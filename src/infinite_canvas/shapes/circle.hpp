#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include "xray/xray.hpp"

struct Circle {

    XRay::Color color{};
    XRay::Vector2 pos{};
    float radius{0.0f};

    Circle() = default; 
    Circle(XRay::Color color, XRay::Vector2 position, float r) 
        : color(color), pos(position), radius(r) {}

    void Draw() const {
        XRay::DrawCircleSDF(pos, radius, color);
    }
};


#endif