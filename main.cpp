#include <openvr.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include "rlvr.h"

extern RenderTexture RENDER_TEX_L;
extern RenderTexture RENDER_TEX_R;

void DrawScene() {
    auto poseTf = rlvr::GetPoseMatrix();

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
    rlvr::InitOpenVr();

    Camera3D camera = {0};
    camera.position = {10.0f, 10.0f, 10.0f}; // Camera position
    camera.target = {0.0f, 0.0f, 0.0f};      // Camera looking at point
    camera.up = {0.0f, 1.0f, 0.0f};          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_FREE);

    SetTargetFPS(120);

    while (!WindowShouldClose()) {
        rlvr::UpdateOpenVr();
        UpdateCamera(&camera);

        BeginDrawing();

        rlvr::BeginOpenVrDrawing();
        {
            rlvr::BeginEyeDrawing(vr::Eye_Left);
            ClearBackground(BLACK);
            DrawScene();
            rlvr::EndEyeDrawing();

            rlvr::BeginEyeDrawing(vr::Eye_Right);
            ClearBackground(BLACK);
            DrawScene();
            rlvr::EndEyeDrawing();
        }
        rlvr::EndOpenVrDrawing();

        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        DrawScene();
        EndMode3D();

        DrawTextureEx(RENDER_TEX_L.texture, Vector2Zero(), 0.0f, 0.25f, WHITE);
        DrawTextureEx(RENDER_TEX_R.texture, {0.25f * RENDER_TEX_L.texture.width + 2, 0}, 0.0f, 0.25f, WHITE);

        EndDrawing();
    }

    rlvr::ShutdownOpenVr();
    CloseWindow();

    return 0;
}
