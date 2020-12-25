#ifndef RAYLIB_CONTRIB_VR_RLVR_H
#define RAYLIB_CONTRIB_VR_RLVR_H

#include <openvr.h>
#include "cmake-build-debug-vs-vcpkg/_deps/raylib-src/src/raylib.h"

namespace rlvr {
    void InitOpenVr();
    void ShutdownOpenVr();

    void UpdateOpenVr();

    void BeginOpenVrDrawing();
    void EndOpenVrDrawing();

    void BeginEyeDrawing(vr::Hmd_Eye eye);
    void EndEyeDrawing();

    Matrix GetPoseMatrix();

    Matrix GetEyeMatrix(vr::Hmd_Eye eye);

    vr::IVRSystem *GetHmd();

    Matrix OpenVr34ToRaylib44Matrix(vr::HmdMatrix34_t mat);

    Matrix OpenVr44ToRaylib44Matrix(vr::HmdMatrix44_t mat);
}

#endif //RAYLIB_CONTRIB_VR_RLVR_H
