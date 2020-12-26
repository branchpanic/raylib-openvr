#ifndef RAYLIB_OPENVR_H_
#define RAYLIB_OPENVR_H_

#include <openvr_capi.h>
#include <raymath.h>

#ifdef __cplusplus
extern "C" {
#endif

void InitOpenVr();

void UpdateOpenVr();

void ShutdownOpenVr();

void BeginOpenVrDrawing();

void EndOpenVrDrawing();

void BeginEyeDrawing(Hmd_Eye eye);

void EndEyeDrawing();

// Graphics
Matrix GetPoseMatrix();

Matrix GetEyeMatrix(Hmd_Eye eye);

// OpenVR Utility
Matrix OpenVr34ToRaylib44Matrix(HmdMatrix34_t mat);

Matrix OpenVr44ToRaylib44Matrix(HmdMatrix44_t mat);

#ifdef __cplusplus
}
#endif

#endif // RAYLIB_OPENVR_H_
