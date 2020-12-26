#include "rayvr.h"
#include <raylib.h>
#include <raymath.h>
#include <openvr_capi.h>
#include <rlgl.h>
#include <stdio.h>

#if !defined(PLATFORM_DESKTOP)
#error OpenVR is only supported on desktop.
#endif

extern intptr_t VR_InitInternal(EVRInitError *peError, EVRApplicationType eType);

extern void VR_ShutdownInternal();

extern bool VR_IsHmdPresent();

extern intptr_t VR_GetGenericInterface(const char *pchInterfaceVersion, EVRInitError *peError);

extern bool VR_IsRuntimeInstalled();

extern const char *VR_GetVRInitErrorAsSymbol(EVRInitError error);

extern const char *VR_GetVRInitErrorAsEnglishDescription(EVRInitError error);

// OpenVrCoreData contains our OpenVR-related global state.
typedef struct OpenVrCoreData {
    // Hardware info
    struct VR_IVRSystem_FnTable *hmd;
    struct VR_IVRCompositor_FnTable *compositor;

    TrackedDevicePose_t trackedPoses[64]; // TODO: k_unMaxTrackedDeviceCount

    // Stereo rendering
    uint32_t renderTargetWidth, renderTargetHeight;         // Width & height of eye render target (for convenience)
    RenderTexture renderTextureLeft, renderTextureRight;    // Render textures for left and right eye, later sent to OpenVR

    float clipZNear;     // Clipping plane, near
    float clipZFar;     // Clipping plane, far

    Matrix matrixHmdTf;                         // Transform of HMD
    Matrix matrixInvHmdTf;                      // Inverse of transform of HMD
} OpenVrCoreData;

static OpenVrCoreData VRCORE = {0};

// Converts a 4x4 OpenVR matrix (HmdMatrix44_t) to a 4x4 Raylib matrix (Matrix)
inline Matrix OpenVr44ToRaylib44Matrix(HmdMatrix44_t mat) {
    return (Matrix) {
            mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
            mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
            mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
            mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3],
    };
}

// Converts a 3x4 OpenVR matrix (HmdMatrix34_t) to a 4x4 Raylib matrix, filling the remaining column with
// <0, 0, 0, 1>.
inline Matrix OpenVr34ToRaylib44Matrix(HmdMatrix34_t mat) {
    return (Matrix) {
            mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
            mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
            mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
            0.0f, 0.0f, 0.0f, 1.0f,
    };
}

// Gets the projection matrix for a given eye.
// See: https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetProjectionMatrix
inline Matrix GetProjectionMatrix(Hmd_Eye eye) {
    return OpenVr44ToRaylib44Matrix(VRCORE.hmd->GetProjectionMatrix(eye, VRCORE.clipZNear, VRCORE.clipZFar));
}

// Gets the eye-to-head transformation matrix for a given eye.
// See: https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetEyeToHeadTransform
Matrix GetEyeMatrix(Hmd_Eye eye) {
    return OpenVr34ToRaylib44Matrix(VRCORE.hmd->GetEyeToHeadTransform(eye));
}

Matrix GetPoseMatrix() {
    return VRCORE.matrixHmdTf;
}

void InitOpenVr() {
    TraceLog(LOG_INFO, "OPENVR: Initializing OpenVR");

    if (!VR_IsRuntimeInstalled()) {
        TraceLog(LOG_ERROR, "OPENVR: No OpenVR runtime installed");
    }

    if (!VR_IsHmdPresent()) {
        TraceLog(LOG_WARNING, "OPENVR: No HMD present");
    }

    EVRInitError err;
    VRCORE.hmd = (struct VR_IVRSystem_FnTable *) VR_InitInternal(&err, EVRApplicationType_VRApplication_Scene);
    if (err != EVRInitError_VRInitError_None) {
        VRCORE.hmd = NULL;
        TraceLog(LOG_FATAL, "OPENVR: Failed to initialize runtime: %s", VR_GetVRInitErrorAsEnglishDescription(err));
    }

    char tableName[128];

    sprintf(tableName, "FnTable:%s", IVRSystem_Version);
    VRCORE.hmd = (struct VR_IVRSystem_FnTable *) VR_GetGenericInterface(tableName, &err);
    if (err != EVRInitError_VRInitError_None) {
        TraceLog(LOG_FATAL, "Failed to get table %s (%s): %s", IVRSystem_Version, VR_GetVRInitErrorAsSymbol(err),
                 VR_GetVRInitErrorAsEnglishDescription(err));
    }

    VRCORE.hmd->GetRecommendedRenderTargetSize(&VRCORE.renderTargetWidth, &VRCORE.renderTargetHeight);

    VRCORE.renderTextureLeft = LoadRenderTexture(VRCORE.renderTargetWidth, VRCORE.renderTargetHeight);
    VRCORE.renderTextureRight = LoadRenderTexture(VRCORE.renderTargetWidth, VRCORE.renderTargetHeight);

    VRCORE.clipZNear = 0.1f;
    VRCORE.clipZFar = 30.0f;

    sprintf(tableName, "FnTable:%s", IVRCompositor_Version);
    VRCORE.compositor = (struct VR_IVRCompositor_FnTable *) VR_GetGenericInterface(tableName, &err);
    if (err != EVRInitError_VRInitError_None) {
        TraceLog(LOG_FATAL, "Failed to get table %s (%s): %s", IVRCompositor_Version, VR_GetVRInitErrorAsSymbol(err),
                 VR_GetVRInitErrorAsEnglishDescription(err));
    }
}

void UpdateOpenVr() {
    VRCORE.compositor->WaitGetPoses(VRCORE.trackedPoses, k_unMaxTrackedDeviceCount, NULL, 0);

    if (VRCORE.trackedPoses[k_unTrackedDeviceIndex_Hmd].bPoseIsValid) {
        HmdMatrix34_t mat = VRCORE.trackedPoses[k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
        VRCORE.matrixHmdTf = OpenVr34ToRaylib44Matrix(mat);
        VRCORE.matrixInvHmdTf = MatrixInvert(VRCORE.matrixHmdTf);
    }
}

void BeginOpenVrDrawing() {

}

void BeginEyeDrawing(Hmd_Eye eye) {
    BeginTextureMode(eye == EVREye_Eye_Left ? VRCORE.renderTextureLeft : VRCORE.renderTextureRight);
    rlglDraw();

    // Sequence: [ Model * (View * Eye^-1) ] * [ Projection ]
    //           ^- RL_MODELVIEW               ^- RL_PROJECTION

    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();

    rlMultMatrixf(MatrixToFloat(GetProjectionMatrix(eye)));

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(MatrixMultiply(MatrixInvert(GetEyeMatrix(eye)), VRCORE.matrixInvHmdTf)));

    rlEnableDepthTest();
}

void EndEyeDrawing() {
    EndTextureMode();

    rlglDraw();

    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();

    rlDisableDepthTest();
}

void SubmitFrames() {
    Texture_t leftOvrTexture = {(void *) (unsigned int) VRCORE.renderTextureLeft.texture.id,
                                ETextureType_TextureType_OpenGL,
                                EColorSpace_ColorSpace_Auto};
    EVRCompositorError err = VRCORE.compositor->Submit(EVREye_Eye_Left, &leftOvrTexture, NULL, 0);
    if (err != EVRCompositorError_VRCompositorError_None) {
        TraceLog(LOG_ERROR, "OPENVR: Failed to submit frame (%d)", err);
    }

    Texture_t rightOvrTexture = {(void *) (unsigned int) VRCORE.renderTextureRight.texture.id,
                                 ETextureType_TextureType_OpenGL,
                                 EColorSpace_ColorSpace_Auto};
    err = VRCORE.compositor->Submit(EVREye_Eye_Right, &rightOvrTexture, NULL, 0);
    if (err != EVRCompositorError_VRCompositorError_None) {
        TraceLog(LOG_ERROR, "OPENVR: Failed to submit frame (%d)", err);
    }
}

void EndOpenVrDrawing() {
    SubmitFrames();
}

void ShutdownOpenVr() {
    VR_ShutdownInternal();
    VRCORE.hmd = NULL;

    UnloadRenderTexture(VRCORE.renderTextureLeft);
    UnloadRenderTexture(VRCORE.renderTextureRight);

    VRCORE.renderTextureLeft = (RenderTexture) {0};
    VRCORE.renderTextureRight = (RenderTexture) {0};
}