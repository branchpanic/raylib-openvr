#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include "rayvr.h"

void DrawScene() {
    auto poseTf = GetHmdTransform();

    DrawGrid(10, 1.0f);
    DrawCube({3, 0, 0}, 0.5f, 0.5f, 0.5f, RED);
    DrawCubeWires({-3, 0, 0}, 0.5f, 0.5f, 0.5f, RED);
    DrawCube({0, 0, 3}, 0.5f, 0.5f, 0.5f, BLUE);
    DrawCubeWires({0, 0, -3}, 0.5f, 0.5f, 0.5f, BLUE);

    rlPushMatrix();
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(poseTf));
    DrawCubeWires(Vector3Zero(), 0.5f, 0.5f, 0.5f, RED);
    DrawGizmo(Vector3Zero());
    rlPopMatrix();
}

int main() {
    InitWindow(800, 600, "rlvr");
    InitOpenVr();

    Camera3D camera = {0};
    camera.position = {10.0f, 10.0f, 10.0f}; // Camera position
    camera.target = {0.0f, 0.0f, 0.0f};      // Camera looking at point
    camera.up = {0.0f, 1.0f, 0.0f};          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                     // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_FREE);

    SetTargetFPS(120);

    while (!WindowShouldClose()) {
        UpdateOpenVr();
        UpdateCamera(&camera);

        BeginDrawing();

        // TODO: This approach is temporary
        BeginOpenVrDrawing();
        {
            BeginEyeDrawing(EVREye_Eye_Left);
            ClearBackground(BLACK);
            DrawScene();
            EndEyeDrawing();

            BeginEyeDrawing(EVREye_Eye_Right);
            ClearBackground(BLACK);
            DrawScene();
            EndEyeDrawing();
        }
        EndOpenVrDrawing();

        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        DrawScene();
        EndMode3D();

        EndDrawing();
    }

    ShutdownOpenVr();
    CloseWindow();

    return 0;
}
