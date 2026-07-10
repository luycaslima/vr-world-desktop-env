#include "raylib.h"
#include "raymath.h"

#define RLXR_APP_NAME "Rlxr example - actions"
#define RLXR_IMPLEMENTATION
#include "../include/rlxr/src/rlxr.h"

typedef struct {
    Mesh mesh;
    Material mat;
} Panel;

typedef struct {
    rlPose hands[2];
    Panel textPanel;
} WorldState;

static void drawScene(WorldState *state);

/* clang-format off */

int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Rlxr example - actions");

    // Initialize the XR runtime, exit if no XR runtime found
    bool success = InitXr();
    if (!success)
    {
        return -1;
    }

    // Position the XR play space and the player in the scene
    SetXrPosition((Vector3){0.0f, 0.0f, 0.0f});

    // Define a camera to mirror the XR view for the flatscreen window
    Camera camera = {0};
    camera.position = (Vector3){0.0f, 1.5f, 1.5f}; // Camera position
    camera.target = (Vector3){0.0f, 1.0f, 0.0f};   // Camera looking at point
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};       // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                           // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;        // Camera projection type

    // Setup Actions
    //--------------------------------------------------------------------------------------

    // In OpenXR, an input is made from two parts: an action and its bindings. First an app creates an
    // action which describes an input controlling a specific part of the app, an example might be
    // an "aim" pose action, a "walk" Vector2 action and a "fire" boolean action together controlling a fps player.

    // create a boolean action valid only for the left controller
    int menu = rlLoadAction("menu-example", RLXR_TYPE_BOOLEAN, RLXR_HAND_LEFT);

    // then to bind the action to a specific hardware component, a binding must be suggested. This binding
    // may be ignored / remapped by the Xr runtime. (eg. remapped in the SteamVR Controller Bindings UI)
    // Each action must have at least one suggested binding but multiple bindings can also be suggested.

    // bind the "menu" action to the menu component (maps to the menu or home button on most controllers)
    rlSuggestBinding(menu, RLXR_COMPONENT_MENU);

    // create another boolean action and bind it to the select component (for both controllers)
    int select = rlLoadAction("select-example", RLXR_TYPE_BOOLEAN, RLXR_HAND_BOTH);
    rlSuggestBinding(select, RLXR_COMPONENT_SELECT); // (maps to the trigger on most controllers)

    // create a pose action, this action will be used for fetching the position and rotation of both controllers
    int pose = rlLoadAction("controller-pose", RLXR_TYPE_POSE, RLXR_HAND_BOTH);
    rlSuggestBinding(pose, RLXR_COMPONENT_GRIP_POSE);

    // note: we used the grip pose, but other pose types are also available (see https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html#semantic-paths-standard-pose-identifiers)
    //       Most often you'll encounter either the grip pose or the aim pose. The grip pose
    //       is ideal for visually holding items, while the aim pose is ideal when the pose is
    //       used for aiming an object (eg. a gun) by the user.
    //
    //       Both poses can be bound to two isolated actions (eg. hold-pose + aim-pose) and then
    //       which between them in app code as needed.

    // WARNING: all actions and bindings must be created and suggested *before* the first UpdateXr() call

    // Setup text panel resources
    //--------------------------------------------------------------------------------------

    RenderTexture2D panelTarget = LoadRenderTexture(800, 450);
    WorldState world = {};

    world.textPanel.mesh = GenMeshPlane(2.f, -1.125f, 1, 1);
    world.textPanel.mat = LoadMaterialDefault();
    SetMaterialTexture(&world.textPanel.mat, MATERIAL_MAP_DIFFUSE, panelTarget.texture);

    while (!WindowShouldClose() && IsXrConnected())
    {
        // Update
        //----------------------------------------------------------------------------------

        // Update all action states and internal XR event loop, this needs to be done every frame
        UpdateXr();

        // Update the flatscreen camera from the Hmd view pose

        rlPose viewPose = GetXrViewPose();

        if (viewPose.isPositionValid)
        {
            camera.position = viewPose.position;
        }

        if (viewPose.isOrientationValid)
        {
            // camera conversion snippet from https://github.com/FireFlyForLife/rlOpenXR/blob/2fd2433eec8a096dd67c26c94671c4976f9b7dd8/src/rlOpenXR.cpp#L869

            camera.target = Vector3Add(Vector3RotateByQuaternion((Vector3){0.0f, 0.0f, -1.0f}, viewPose.orientation), viewPose.position);
            camera.up = Vector3RotateByQuaternion((Vector3){0.0f, 1.0f, 0.0f}, viewPose.orientation);
        }

        // fetch action values from a source device, if the source device is inactive, a zero-like value is returned
        // note: a rlGet* call can only fetch from a single device at once so a source device must be passed in
        bool leftSelectPressed = rlGetBool(select, RLXR_HAND_LEFT);
        bool rightSelectPressed = rlGetBool(select, RLXR_HAND_RIGHT);

        // fetch a full action state from a source device, this is identical to the simpler version above but it adds a few bits about the source device
        rlBoolState menuState = rlGetBoolState(menu, RLXR_HAND_LEFT);

        // menuState.value - the current input value
        // menuState.active - is there a component / binding that is awake and active for this action (eg. false if the source controller is sleeping)
        // menuState.changed - did .value change since the last UpdateXr() call

        // fetch controller poses for rendering in-app models
        world.hands[0] = rlGetPose(pose, RLXR_HAND_LEFT);
        world.hands[1] = rlGetPose(pose, RLXR_HAND_RIGHT);

        // Draw 2D text panel into a texture

        BeginTextureMode(panelTarget);

            ClearBackground(BLANK); // (fully transparent)

            DrawText(TextFormat("Select - left: %d right: %d", leftSelectPressed, rightSelectPressed), 32, 128, 26, BLACK);
            DrawText(TextFormat("Menu state - value: %d active: %d changed: %d", menuState.value, menuState.active, menuState.changed), 32, 154, 26, BLACK);

            DrawText("Left Hand:", 32, 200, 26, BLACK);
            DrawText(TextFormat("    valid: %d; position: %.04f %.04f %.04f", world.hands[0].isPositionValid, world.hands[0].position.x, world.hands[0].position.y, world.hands[0].position.z), 32, 226, 26, BLACK);
            DrawText(TextFormat("    valid: %d; orientation: %.04f %.04f %.04f %.04f", world.hands[0].isOrientationValid, world.hands[0].orientation.x, world.hands[0].orientation.y, world.hands[0].orientation.z, world.hands[0].orientation.w), 32, 252, 26, BLACK);

            DrawText("Right Hand:", 32, 300, 26, BLACK);
            DrawText(TextFormat("    valid: %d; position: %.04f %.04f %.04f", world.hands[1].isPositionValid, world.hands[1].position.x, world.hands[1].position.y, world.hands[1].position.z), 32, 326, 26, BLACK);
            DrawText(TextFormat("    valid: %d; orientation: %.04f %.04f %.04f %.04f", world.hands[1].isOrientationValid, world.hands[1].orientation.x, world.hands[1].orientation.y, world.hands[1].orientation.z, world.hands[1].orientation.w), 32, 352, 26, BLACK);

            DrawRectangleLines(16, 112, 784, 282, BLACK);

        EndTextureMode();

        // Draw to XR
        //----------------------------------------------------------------------------------

        // Begin new XR frame, the number of views (cameras) to render requested by the XR runtime is returned.
        int views = BeginXrMode();

        for (int i = 0; i < views; i++)
        {
            // Begin a XR view, this will setup 3D rendering from the perspective of the view
            BeginView(i);

                ClearBackground(RAYWHITE);
                drawScene(&world);

            // Release the XR view to the runtime and disable 3D rendering
            EndView();
        }

        // Release the frame to the runtime and show it on device
        EndXrMode();

        // Draw to screen
        //----------------------------------------------------------------------------------

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                drawScene(&world);
            EndMode3D();

            DrawFPS(10, 10);

        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseXr();
    CloseWindow();
}

/* clang-format on */

static void drawScene(WorldState *world) {
    // draw controller cubes

    for (int i = 0; i < 2; i++)
    {
        rlPose hand = world->hands[i];

        if (hand.isPositionValid && hand.isOrientationValid)
        {
            rlPushMatrix();
            rlTranslatef(hand.position.x, hand.position.y, hand.position.z);
            rlMultMatrixf(MatrixToFloat(QuaternionToMatrix(hand.orientation)));

            DrawCube((Vector3){0.0f, 0.0f, 0.0f}, .08f, .1f, .12f, i == 1 ? ORANGE : BLUE);
            DrawCubeWires((Vector3){0.0f, 0.0f, 0.0f}, .08f, .1f, .12f, i == 1 ? RED : DARKBLUE);
            rlPopMatrix();
        }
    }

    // draw 3D scene

    rlPushMatrix();

    rlTranslatef(.0f, 1.2f, -1.5f);
    rlRotatef(90.f, 1.f, .0f, .0f);

    rlSetCullFace(RL_CULL_FACE_FRONT);
    DrawMesh(world->textPanel.mesh, world->textPanel.mat, MatrixIdentity());
    rlSetCullFace(RL_CULL_FACE_BACK);

    rlPopMatrix();

    DrawGrid(10, 0.25f);
}



// /*******************************************************************************************
// *
// *   raylib [core] example - vr simulator
// *
// *   Example complexity rating: [★★★☆] 3/4
// *
// *   Example originally created with raylib 2.5, last time updated with raylib 4.0
// *
// *   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
// *   BSD-like license that allows static linking with closed source software
// *
// *   Copyright (c) 2017-2025 Ramon Santamaria (@raysan5)
// *
// ********************************************************************************************/

// #include <raylib.h>


// #if defined(PLATFORM_DESKTOP)
//     #define GLSL_VERSION        330
// #else   // PLATFORM_ANDROID, PLATFORM_WEB
//     #define GLSL_VERSION        100
// #endif

// //------------------------------------------------------------------------------------
// // Program main entry point
// //------------------------------------------------------------------------------------
// int main(void)
// {
//     // Initialization
//     //--------------------------------------------------------------------------------------
//     const int screenWidth = 800;
//     const int screenHeight = 450;

//     // NOTE: screenWidth/screenHeight should match VR device aspect ratio
//     InitWindow(screenWidth, screenHeight, "raylib [core] example - vr simulator");

//     // VR device parameters definition
//     VrDeviceInfo device = {
//         // Oculus Rift CV1 parameters for simulator
//         .hResolution = 2160,                 // Horizontal resolution in pixels
//         .vResolution = 1200,                 // Vertical resolution in pixels
//         .hScreenSize = 0.133793f,            // Horizontal size in meters
//         .vScreenSize = 0.0669f,              // Vertical size in meters
//         .eyeToScreenDistance = 0.041f,       // Distance between eye and display in meters
//         .lensSeparationDistance = 0.07f,     // Lens separation distance in meters
//         .interpupillaryDistance = 0.07f,     // IPD (distance between pupils) in meters

//         // NOTE: CV1 uses fresnel-hybrid-asymmetric lenses with specific compute shaders
//         // Following parameters are just an approximation to CV1 distortion stereo rendering
//         .lensDistortionValues[0] = 1.0f,     // Lens distortion constant parameter 0
//         .lensDistortionValues[1] = 0.22f,    // Lens distortion constant parameter 1
//         .lensDistortionValues[2] = 0.24f,    // Lens distortion constant parameter 2
//         .lensDistortionValues[3] = 0.0f,     // Lens distortion constant parameter 3
//         .chromaAbCorrection[0] = 0.996f,     // Chromatic aberration correction parameter 0
//         .chromaAbCorrection[1] = -0.004f,    // Chromatic aberration correction parameter 1
//         .chromaAbCorrection[2] = 1.014f,     // Chromatic aberration correction parameter 2
//         .chromaAbCorrection[3] = 0.0f,       // Chromatic aberration correction parameter 3
//     };

//     // Load VR stereo config for VR device parameteres (Oculus Rift CV1 parameters)
//     VrStereoConfig config = LoadVrStereoConfig(device);

//     // Distortion shader (uses device lens distortion and chroma)
//     Shader distortion = LoadShader(0, TextFormat("assets/shaders/glsl%i/distortion.fs", GLSL_VERSION));

//     // Update distortion shader with lens and distortion-scale parameters
//     SetShaderValue(distortion, GetShaderLocation(distortion, "leftLensCenter"),
//                    config.leftLensCenter, SHADER_UNIFORM_VEC2);
//     SetShaderValue(distortion, GetShaderLocation(distortion, "rightLensCenter"),
//                    config.rightLensCenter, SHADER_UNIFORM_VEC2);
//     SetShaderValue(distortion, GetShaderLocation(distortion, "leftScreenCenter"),
//                    config.leftScreenCenter, SHADER_UNIFORM_VEC2);
//     SetShaderValue(distortion, GetShaderLocation(distortion, "rightScreenCenter"),
//                    config.rightScreenCenter, SHADER_UNIFORM_VEC2);

//     SetShaderValue(distortion, GetShaderLocation(distortion, "scale"),
//                    config.scale, SHADER_UNIFORM_VEC2);
//     SetShaderValue(distortion, GetShaderLocation(distortion, "scaleIn"),
//                    config.scaleIn, SHADER_UNIFORM_VEC2);
//     SetShaderValue(distortion, GetShaderLocation(distortion, "deviceWarpParam"),
//                    device.lensDistortionValues, SHADER_UNIFORM_VEC4);
//     SetShaderValue(distortion, GetShaderLocation(distortion, "chromaAbParam"),
//                    device.chromaAbCorrection, SHADER_UNIFORM_VEC4);

//     // Initialize framebuffer for stereo rendering
//     // NOTE: Screen size should match HMD aspect ratio
//     RenderTexture2D target = LoadRenderTexture(device.hResolution, device.vResolution);

//     // The target's height is flipped (in the source Rectangle), due to OpenGL reasons
//     Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
//     Rectangle destRec = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };

//     // Define the camera to look into our 3d world
//     Camera camera = { 0 };
//     camera.position = (Vector3){ 5.0f, 2.0f, 5.0f };    // Camera position
//     camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
//     camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector
//     camera.fovy = 60.0f;                                // Camera field-of-view Y
//     camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

//     Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

//     DisableCursor();                    // Limit cursor to relative movement inside the window

//     SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
//     //--------------------------------------------------------------------------------------

//     // Main game loop
//     while (!WindowShouldClose())        // Detect window close button or ESC key
//     {
//         // Update
//         //----------------------------------------------------------------------------------
//         UpdateCamera(&camera, CAMERA_FIRST_PERSON);
//         //----------------------------------------------------------------------------------

//         // Draw
//         //----------------------------------------------------------------------------------
//         BeginTextureMode(target);
//             ClearBackground(RAYWHITE);
//             BeginVrStereoMode(config);
//                 BeginMode3D(camera);

//                     DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
//                     DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
//                     DrawGrid(40, 1.0f);

//                 EndMode3D();
//             EndVrStereoMode();
//         EndTextureMode();

//         BeginDrawing();
//             ClearBackground(RAYWHITE);
//             BeginShaderMode(distortion);
//                 DrawTexturePro(target.texture, sourceRec, destRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
//             EndShaderMode();
//             DrawFPS(10, 10);
//         EndDrawing();
//         //----------------------------------------------------------------------------------
//     }

//     // De-Initialization
//     //--------------------------------------------------------------------------------------
//     UnloadVrStereoConfig(config);   // Unload stereo config

//     UnloadRenderTexture(target);    // Unload stereo render fbo
//     UnloadShader(distortion);       // Unload distortion shader

//     CloseWindow();                  // Close window and OpenGL context
//     //--------------------------------------------------------------------------------------

//     return 0;
// }