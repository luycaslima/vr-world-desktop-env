#ifndef XRAY_HPP
#define XRAY_HPP

#include <array>

// Simple, clean Raylib-style data structures
// TODO read the Raylib heade functions to be adapted
namespace XRay {
    struct Vector3 { float x, y, z; };
    struct Color { unsigned char r, g, b, a; };

    namespace Colors {
        inline constexpr Color White{255, 255, 255, 255};
        inline constexpr Color Red{255, 0, 0, 255};
        inline constexpr Color Blue{0, 0, 255, 255};
    }

    // Idiomatic Lifecycles
    bool InitEngine(const char* title);
    void ShutdownEngine();
    bool ShouldClose();
    void PollEvents();

    // Idiomatic Frame Controls (Bridged to SDL3 GPU & OpenXR)
    void BeginDrawing();
    void EndDrawing();
    
    void BeginEyeDrawing(int eye); // 0 = Left, 1 = Right
    void EndEyeDrawing();

    // Idiomatic Drawing Primitives
    void ClearBackground(Color color);
    void DrawCube(Vector3 position, Vector3 size, Color color);
}

#endif