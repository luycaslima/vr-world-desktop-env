#ifndef XRAY_HPP
#define XRAY_HPP

//#include "xray/base.h"

#include <SDL3/SDL.h>

namespace XRay {

    enum  CameraProjection {
        CAMERA_PERSPECTIVE = 0 ,
        CAMERA_ORTHOGRAPHIC
    };

    // Vector2, 2 components
    struct Vector2 {
        float x =0.0;
        float y =0.0;

        Vector2() = default;
        Vector2(float x, float y) : x(x) , y(y) {} 

        Vector2& operator+=(const Vector2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

    };
    
    // Vector3, 3 components
    struct Vector3 {
        float x;
        float y;
        float z;
    };

    // Vector4, 4 components
    struct Vector4 {
        float x;
        float y;
        float z;
        float w;
    };

    // Opaque handle for a custom shader
    struct Shader {
        unsigned int id;
        void* internal_ptr = nullptr; // Points to our internal PipelineResource
    };
    
    // Quaternion, 4 components (Vector4 alias)
    using Quaternion = Vector4;

    // Matrix, 4x4 components, column major, OpenGL style, right-handed
    struct Matrix {
        float m0, m4, m8, m12;  // Matrix first row
        float m1, m5, m9, m13;  // Matrix second row
        float m2, m6, m10, m14; // Matrix third row
        float m3, m7, m11, m15; // Matrix fourth row
    };

    struct Camera3D {
        Vector3 position;
        Vector3 target ;
        Vector3 up ;
        float fovy ;
        CameraProjection projection;
    };

    using Camera = Camera3D;

    struct Camera2D{
        Vector2 offset;         // Camera offset (screen space offset from window origin)
        Vector2 target;         // Camera target (world space target point that is mapped to screen space offset)
        float rotation;         // Camera rotation in degrees (pivots around target)
        float zoom;             // Camera zoom (scaling around target), must not be set to 0, set to 1.0f for no scale
    };

    // Mesh, vertex data and vao/vbo
    struct Mesh {
        int vertexCount;        // Number of vertices stored in arrays
        int triangleCount;      // Number of triangles stored (indexed or not)

        // Vertex attributes data
        float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
        float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
        float *texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
        float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
        float *tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
        unsigned char *colors;  // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
        unsigned short *indices; // Vertex indices (in case vertex data comes indexed)

        // Skin data for animation
        //int boneCount;          // Number of bones (MAX: 256 bones)
        //unsigned char *boneIndices; // Vertex bone indices, up to 4 bones influence by vertex (skinning) (shader-location = 6)
        //float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning) (shader-location = 7)

        // Runtime animation vertex data (CPU skinning)
        // NOTE: In case of GPU skinning, not used, pointers are NULL
        //float *animVertices;    // Animated vertex positions (after bones transformations)
        // float *animNormals;     // Animated normals (after bones transformations)

        // OpenGL identifiers
        // unsigned int vaoId;     // OpenGL Vertex Array Object id
        // unsigned int *vboId;    // OpenGL Vertex Buffer Objects id (default vertex data)
    };


    // Color, 4 components, 
    struct Color {
        float r;
        float g;
        float b;
        float a;
    };

    //Shaders
    struct PositionVertex {
        float x,y,z;
    };

    struct PositionColorVertex {
        float x,y,z;
        float r,g,b,a;
    };

    struct PositionTextureVertex
    {
        float x, y, z;
        float u, v;
    };

    namespace Vector3D {
        inline constexpr Vector3 UP{0.0,1.0,0.0};
        inline constexpr Vector3 FORWARD{0.0,0.0,1.0};
    }

    namespace Colors {
        inline constexpr Color White{255, 255, 255, 255};
        inline constexpr Color Red{255, 0, 0, 255};
        inline constexpr Color Blue{0, 0, 255, 255};
    }

    // Idiomatic Lifecycles
    bool Init(const char* title, int windowWidth, int windowHeight);
    void Shutdown();
    bool ShouldClose();
    
    //TODO make my event system here
    void PollEvents();
    void ClearBackground(Color color);
    
    void Begin3DMode(Camera3D camera);
    void End3DMode();
    
    void Begin2DMode(Camera2D camera);
    void End2DMode();

    void BeginDrawing();
    void EndDrawing();

    // Lifecycles & Shaders
    
    //Shader LoadShader(const char* vsFileName, const char* fsFileName);
    //void UnloadShader(Shader shader);

    // void BeginShaderMode(Shader shader);
    // void EndShaderMode();

    SDL_GPUShader* LoadShader( SDL_GPUDevice* device, 
        const char* basePath,
        Uint32 samplerCount, 
        Uint32 uniformBufferCount, 
        Uint32 storageBufferCount, 
        Uint32 storageTextureCount, 
        SDL_GPUShaderStage stage);

    void DrawCircleSDF(Vector2 center, float radius, Color color, float feather = 1.5f);

    // Idiomatic Drawing Primitives
    void DrawCube(Vector3 position, Vector3 size, Color color);

    //VR functions
    void BeginVRMode();
    void EndVRMode();

    void BeginEyeDrawing(int eye); // 0 = Left, 1 = Right
    void EndEyeDrawing();
    
}

#endif