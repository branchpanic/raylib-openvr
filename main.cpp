#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include "rayvr.h"

void DrawScene() {
    auto devices = GetTrackedDevices();
    for (int i = 0; i < 64; i++) {
        if (devices[i].bPoseIsValid && GetHmd()->GetTrackedDeviceClass(i) == ETrackedDeviceClass_TrackedDeviceClass_Controller) {
            rlPushMatrix();
            rlLoadIdentity();
            rlMultMatrixf(MatrixToFloat(OpenVr34ToRaylib44Matrix(devices[i].mDeviceToAbsoluteTracking)));
            DrawCube(Vector3Zero(), 0.08f, 0.08f, 0.08f, GREEN);
            rlPopMatrix();
        }
    }
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

    SetCameraMode(camera, CAMERA_CUSTOM);

    SetTargetFPS(120);

    while (!WindowShouldClose()) {
        UpdateOpenVr();
        UpdateCamera(&camera);

        auto hmdThisFrame = GetHmdTransform();
        camera.position = Vector3Transform(Vector3Zero(), hmdThisFrame);
        camera.up = Vector3Transform({0, 1, 0}, hmdThisFrame);
        camera.target = Vector3Transform({0, 0, -1}, hmdThisFrame);

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
