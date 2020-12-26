#ifndef RAYLIB_OPENVR_H_
#define RAYLIB_OPENVR_H_

#include <openvr_capi.h>
#include <raymath.h>

#ifdef __cplusplus
extern "C" {
#endif

// OpenVR Lifecycle
void InitOpenVr();                          // Initialize an OpenVR scene.
void UpdateOpenVr();                        // Update OpenVR. Should be called every frame to get tracking data.
void ShutdownOpenVr();                      // Shut down the current OpenVR scene.

void BeginOpenVrDrawing();                  // Start drawing for OpenVR.
void EndOpenVrDrawing();                    // Finish drawing for OpenVR.

void BeginEyeDrawing(Hmd_Eye eye);          // Start drawing directly to the specified eye.
void EndEyeDrawing();                       // Finish drawing to the current eye.

// Graphics
Matrix GetHmdTransform();                   // Gets the transformation matrix of the HMD
Matrix GetHmdToEyeTransform(Hmd_Eye eye);   // Gets the transformation matrix from the HMD to an eye

// OpenVR Utility
Matrix OpenVr34ToRaylib44Matrix(HmdMatrix34_t mat);     // Convert from a 3x4 OpenVR matrix to a 4x4 Raylib matrix
Matrix OpenVr44ToRaylib44Matrix(HmdMatrix44_t mat);     // Convert from a 4x4 OpenVR matrix to a 4x4 Raylib matrix

struct VR_IVRSystem_FnTable* GetHmd();
TrackedDevicePose_t* GetTrackedDevices();

#ifdef __cplusplus
}
#endif

#endif // RAYLIB_OPENVR_H_
