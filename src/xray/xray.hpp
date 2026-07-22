#ifndef XRAY_HPP
#define XRAY_HPP

namespace XRay {

    enum  CameraProjection {
        CAMERA_PERSPECTIVE = 0 ,
        CAMERA_ORTHOGRAPHIC
    };
    // Vector2, 2 components
    struct Vector2 {
        float x;
        float y;

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
        Vector3 target;
        Vector3 up;
        float fovy;
        CameraProjection projection;
    };

    using Camera = Camera3D;

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


    // Color, 4 components, R8G8B8A8 (32bit)
    struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };

    namespace Colors {
        inline constexpr Color White{255, 255, 255, 255};
        inline constexpr Color Red{255, 0, 0, 255};
        inline constexpr Color Blue{0, 0, 255, 255};
    }

    // Idiomatic Lifecycles
    bool InitEngine(const char* title, int windowWidth, int windowHeight);
    void Shutdown();
    bool ShouldClose();
    
    void PollEvents();
    
    void ClearBackground(Color color);
    
    void BeginDrawing(Camera3D camera);
    void EndDrawing();

    // Lifecycles & Shaders
    Shader LoadShader(const char* vsFileName, const char* fsFileName);
    void UnloadShader(Shader shader);

    // Idiomatic State Changers
    void BeginShaderMode(Shader shader);
    void EndShaderMode();

    // Idiomatic Drawing Primitives
    void DrawCube(Vector3 position, Vector3 size, Color color);

    //VR functions
    void BeginVRMode();
    void EndVRMode();

    void BeginEyeDrawing(int eye); // 0 = Left, 1 = Right
    void EndEyeDrawing();
}

#endif