# hello_vr
This heavily-commented example introduces `raylib-openvr`'s core functionality in lieu of proper documentation.
It may be a helpful starting point for creating a VR raylib project.
```c
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
```

## Raylib Setup
```c
int main() {
    InitWindow(800, 600, "Hello OpenVR");
    SetTargetFPS(120);
```

InitVr() starts OpenVR and reads information about the current device. If no SteamVR runtime is present or no
HMD is connected, InitVr() will log an error message and terminate the program.
```c
    InitVr();
```

A VrRig is the main structure used for pose information. Its definition is as follows.
```c
// typedef struct VrRig {
//     Vector3 trackingOrigin;         // Position of rig
//     VrTrackedDevice hmd;            // HMD relative to trackingOrigin
//     VrTrackedDevice controllers[2]; // Left and right controllers relative to trackingOrigin
// } VrRig;
```
```c
```

Each VrTrackedDevice (see OpenVR: TrackedDevicePose_t) stores device and tracking information.
```c
// typedef struct VrTrackedDevice {
//    unsigned long id;    // OpenVR device ID
//    bool valid;          // Pose validity as reported by OpenVR
//    Matrix transform;    // Tracking origin -> device transform
//    Vector3 position;    // Device position relative to origin
//    Quaternion rotation; // Device rotation
// } VrTrackedDevice;
```
```c
    VrRig rig = { 0 };
```

## Main Loop
```c
    while (!WindowShouldClose()) {
```

UpdateVrTracking(&rig) must be called every frame. It reads VR input, and in doing so, focuses the
application so we can submit frames to the runtime. It updates the `hmd` and `controllers` fields of the
given `VrRig` with new information.
```c
        UpdateVrTracking(&rig);
        BeginDrawing();
```

BeginVrDrawing() signals that we are rendering in stereo to both eyes of the HMD. When EndVrDrawing is
called, both frames will be sent to the runtime and rendered to the desktop window.
```c
            BeginVrDrawing();
```

BeginMode3DVr() is an analog to BeginMode3D() that takes a VrRig instead of a Camera3D. It is
finalized with EndMode3D().
```c
                BeginMode3DVr(rig);
                    ClearBackground(RAYWHITE);
                    DrawGrid(10, 5);
```

This example draws gizmos at the left and right controller positions. It uses RLGL functions to
correctly (? #10) position and orient them.
```c
                    rlPushMatrix();
                    rlMultMatrixf(MatrixToFloat(rig.controllers[0].transform));
                    rlScalef(0.25f, 0.25f, 0.25f);
                    DrawCube(Vector3Zero(), 0.25f, 0.25f, 0.25f, RED);
                    DrawGizmo(Vector3Zero());
                    rlPopMatrix();
                    rlPushMatrix();
                    rlMultMatrixf(MatrixToFloat(rig.controllers[1].transform));
                    rlScalef(0.25f, 0.25f, 0.25f);
                    DrawCube(Vector3Zero(), 0.25f, 0.25f, 0.25f, RED);
                    DrawGizmo(Vector3Zero());
                    rlPopMatrix();
                EndMode3D();
            EndVrDrawing();
```

After rendering to the headset, we can continue drawing to the desktop window.
```c
            DrawFPS(2, 2);
        EndDrawing();
    } // main loop
```

## Cleanup
CloseVr() gracefully shuts down OpenVR and frees resources.
```c
    CloseVr();
    CloseWindow();
    return 0;
} // main
```

###### Automatically generated from [hello_vr.c](hello_vr.c)
