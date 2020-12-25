#include "rlvr.h"
#include <raylib.h>
#include <raymath.h>
#include <openvr.h>
#include <rlgl.h>

#if !defined(PLATFORM_DESKTOP)
#error OpenVR is only supported on desktop.
#endif

// OpenVrCoreData contains our OpenVR-related global state.
typedef struct OpenVrCoreData {
    // Hardware info
    vr::IVRSystem *hmd;
    vr::TrackedDevicePose_t trackedPoses[vr::k_unMaxTrackedDeviceCount];

    // Stereo rendering
    uint32_t renderTargetWidth, renderTargetHeight;
    RenderTexture renderTextureLeft, renderTextureRight;
    float clipZNear = 0.1f;
    float clipZFar = 40.0f;

    Matrix matrixProjLeft, matrixProjRight;     // Left & right eye projection matrices
    Matrix matrixEyeTfLeft, matrixEyeTfRight;   // Left & right eye transforms relative to head
    Matrix matrixHmdTf;                         // Transform of HMD
    Matrix matrixInvHmdTf;                      // Inverse of transform of HMD
} OpenVrCoreData;

static vr::IVRSystem *HMD;
static uint32_t RENDER_TEX_WIDTH, RENDER_TEX_HEIGHT;
RenderTexture RENDER_TEX_L;
RenderTexture RENDER_TEX_R;
static vr::TrackedDevicePose_t TRACKED_POSES[vr::k_unMaxTrackedDeviceCount];
static const float NEAR_Z = 0.1f;
static const float FAR_Z = 30.0f;

static Matrix MAT_PROJ_LEFT, MAT_EYE_LEFT;
static Matrix MAT_PROJ_RIGHT, MAT_EYE_RIGHT;
static Matrix HMD_POSE, HMD_POSE_INV;

// Converts a 4x4 OpenVR matrix (vr::HmdMatrix44_t) to a 4x4 Raylib matrix (Matrix)
inline Matrix rlvr::OpenVr44ToRaylib44Matrix(vr::HmdMatrix44_t mat) {
    return Matrix{
            mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
            mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
            mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
            mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3],
    };
}

// Converts a 3x4 OpenVR matrix (vr::HmdMatrix34_t) to a 4x4 Raylib matrix, filling the remaining column with
// <0, 0, 0, 1>.
inline Matrix rlvr::OpenVr34ToRaylib44Matrix(vr::HmdMatrix34_t mat) {
    return Matrix{
            mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
            mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
            mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
            0.0f, 0.0f, 0.0f, 1.0f,
    };
}

// Gets the projection matrix for a given eye.
// See: https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetProjectionMatrix
static inline Matrix GetProjectionMatrix(vr::Hmd_Eye eye) {
    return rlvr::OpenVr44ToRaylib44Matrix(HMD->GetProjectionMatrix(eye, NEAR_Z, FAR_Z));
}

// Gets the eye-to-head transformation matrix for a given eye.
// See: https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetEyeToHeadTransform
Matrix rlvr::GetEyeMatrix(vr::Hmd_Eye eye) {
    return rlvr::OpenVr34ToRaylib44Matrix(HMD->GetEyeToHeadTransform(eye));
}

// Gets the pose of the active HMD.
Matrix rlvr::GetPoseMatrix() {
    return HMD_POSE;
}

// Gets the vr::IVRSystem for the active HMD. If rlvr::InitOpenVr HAS NOT been called or rlvr::ShutdownOpenVr HAS been
// called, this will be nullptr.
vr::IVRSystem* rlvr::GetHmd() {
    return HMD;
}

std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop,
                                   vr::TrackedPropertyError *peError = nullptr) {
    uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
    if (unRequiredBufferLen == 0)
        return "";

    char *pchBuffer = new char[unRequiredBufferLen];
    unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen,
                                                                         peError);
    std::string sResult = pchBuffer;
    delete[] pchBuffer;
    return sResult;
}

// Initializes an OpenVR scene, allocates
void rlvr::InitOpenVr() {
    TraceLog(LOG_INFO, "OPENVR: Initializing OpenVR");

    vr::EVRInitError err;
    HMD = vr::VR_Init(&err, vr::VRApplication_Scene);
    if (err != vr::VRInitError_None) {
        HMD = nullptr;
        TraceLog(LOG_ERROR, "OPENVR: Failed to initialize runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(err));
        exit(EXIT_FAILURE);
    }

    auto trackingInfo = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
    TraceLog(LOG_INFO, "OPENVR: Tracking system is %s", trackingInfo.c_str());

    HMD->GetRecommendedRenderTargetSize(&RENDER_TEX_WIDTH, &RENDER_TEX_HEIGHT);

    RENDER_TEX_L = LoadRenderTexture(RENDER_TEX_WIDTH, RENDER_TEX_HEIGHT);
    RENDER_TEX_R = LoadRenderTexture(RENDER_TEX_WIDTH, RENDER_TEX_HEIGHT);

    MAT_PROJ_LEFT = GetProjectionMatrix(vr::Eye_Left);
    MAT_PROJ_RIGHT = GetProjectionMatrix(vr::Eye_Right);
    MAT_EYE_LEFT = GetEyeMatrix(vr::Eye_Left);
    MAT_EYE_RIGHT = GetEyeMatrix(vr::Eye_Right);
}

void rlvr::UpdateOpenVr() {
    vr::VRCompositor()->WaitGetPoses(TRACKED_POSES, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

    if (TRACKED_POSES[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid) {
        auto mat = TRACKED_POSES[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
        HMD_POSE = OpenVr34ToRaylib44Matrix(mat);
        HMD_POSE_INV = MatrixInvert(HMD_POSE);
    }
}

void rlvr::BeginOpenVrDrawing() {

}

void rlvr::BeginEyeDrawing(vr::Hmd_Eye eye) {
    BeginTextureMode(eye == vr::Eye_Left ? RENDER_TEX_L : RENDER_TEX_R);

    rlglDraw();

    // Sequence: Model * (View * Eye^-1) * Projection

    // Set up projection matrices
    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(GetProjectionMatrix(eye)));

    // Set up model-view matrices
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(MatrixMultiply(GetEyeMatrix(eye), MatrixInvert(HMD_POSE))));

    rlEnableDepthTest();
}

void rlvr::EndEyeDrawing() {
    EndTextureMode();

    rlglDraw();                         // Process internal buffers (update + draw)

    rlMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlPopMatrix();                      // Restore previous matrix (projection) from matrix stack

    rlMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlLoadIdentity();                   // Reset current matrix (modelview)

    rlDisableDepthTest();               // Disable DEPTH_TEST for 2D
}

void SubmitFrames() {
    vr::Texture_t left_texture = {(void *) (unsigned int) RENDER_TEX_L.texture.id, vr::TextureType_OpenGL,
                                  vr::ColorSpace_Gamma};
    vr::EVRCompositorError comp_err = vr::VRCompositor()->Submit(vr::Eye_Left, &left_texture);
    if (comp_err != vr::VRCompositorError_None) {
        TraceLog(LOG_ERROR, "OPENVR: Failed to submit frame (%d)", comp_err);
    }

    vr::Texture_t right_texture = {(void *) (unsigned int) RENDER_TEX_R.texture.id, vr::TextureType_OpenGL,
                                   vr::ColorSpace_Gamma};
    comp_err = vr::VRCompositor()->Submit(vr::Eye_Right, &right_texture);
    if (comp_err != vr::VRCompositorError_None) {
        TraceLog(LOG_ERROR, "OPENVR: Failed to submit frame (%d)", comp_err);
    }
}

void rlvr::EndOpenVrDrawing() {
    SubmitFrames();
}

void rlvr::ShutdownOpenVr() {
    vr::VR_Shutdown();
    HMD = nullptr;

    UnloadRenderTexture(RENDER_TEX_L);
    RENDER_TEX_L = {0};

    UnloadRenderTexture(RENDER_TEX_R);
    RENDER_TEX_R = {0};
}