/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef __emscripten_vr_h__
#define __emscripten_vr_h__

#include <stdint.h>
#include <stdbool.h>

/*
 * This file provides some basic interfaces for interacting with WebVR from Emscripten.
 *
 * Documentation for the public APIs defined in this file must be updated in:
 *    site/source/docs/api_reference/vr.h.rst
 * A prebuilt local version of the documentation is available at:
 *    site/build/text/docs/api_reference/emscripten.h.txt
 * You can also build docs locally as HTML or other formats in site/
 * An online HTML version (which may be of a different version of Emscripten)
 *    is up at http://kripken.github.io/emscripten-site/docs/api_reference/vr.h.html
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Version of the emscripten vr API, assembled as follows:
 * WebVR Version Major * 10000 + WebVR Version Minor * 100 + API version [0-99]
 *
 * The first two can be used to determine the for which WebVR version the API
 * has been built while the last can be used to determine which functions are
 * available with the current build.
 *
 * E.g. emscripten vr API version 1.1.0 implements some initial features for
 * WebVR 1.1 and its version define will have the value 10100
 */
#define EMSCRIPTEN_VR_API_VERSION 10101

typedef int32_t VRDisplayHandle;

typedef void (*em_vr_callback_func)(void);
typedef void (*em_vr_arg_callback_func)(void*);

#define VR_EYE_LEFT 0
#define VR_EYE_RIGHT 1

typedef enum {
    VREyeLeft = VR_EYE_LEFT,
    VREyeRight = VR_EYE_RIGHT
} VREye;

typedef struct VRVector3 {
    float x, y, z;
} VRVector3;

typedef struct VRQuaternion {
    float x, y, z, w;
} VRQuaternion;

typedef struct VRDisplayCapabilities {
    int32_t hasPosition;
    int32_t hasExternalDisplay;
    int32_t canPresent;
    unsigned long maxLayers;
} VRDisplayCapabilities;

#define VR_LAYER_DEFAULT_LEFT_BOUNDS {0.0f, 0.0f, 0.5f, 1.0f}
#define VR_LAYER_DEFAULT_RIGHT_BOUNDS {0.5f, 0.0f, 0.5f, 1.0f}
typedef struct VRLayerInit {
    const char* source;

    float leftBounds[4];
    float rightBounds[4];
} VRLayerInit;

/* Defines for VRPose::poseFlags */
#define VR_POSE_POSITION 0x1
#define VR_POSE_LINEAR_VELOCITY 0x2
#define VR_POSE_LINEAR_ACCELERATION 0x4
#define VR_POSE_ORIENTATION 0x8
#define VR_POSE_ANGULAR_VELOCITY 0x10
#define VR_POSE_ANGULAR_ACCELERATION 0x20

typedef struct VRPose {
    /** Position, valid only if `poseFlags & VR_POSE_POSITION == 0` */
    VRVector3 position;
    /** Linear velocity, valid only if `poseFlags & VR_POSE_LINEAR_VELOCITY == 0` */
    VRVector3 linearVelocity;
    /** Linear acceleration, valid only if `poseFlags & VR_POSE_LINEAR_ACCELERATION == 0` */
    VRVector3 linearAcceleration;

    /** Orientation quaternion, valid only if `poseFlags & VR_POSE_ORIENTATION == 0` */
    VRQuaternion orientation;
    /** Angular velocity, valid only if `poseFlags & VR_POSE_ANGULAR_VELOCITY == 0` */
    VRVector3 angularVelocity;
    /** Angular acceleration, valid only if `poseFlags & VR_POSE_ANGULAR_ACCELERATION == 0` */
    VRVector3 angularAcceleration;

    /** Bitmask of VR_POSE_* which determines whether the corresponding pose
     * attributes are valid */
    int poseFlags;
} VRPose;

typedef struct VREyeParameters {
    VRVector3 offset;

    unsigned long renderWidth;
    unsigned long renderHeight;
} VREyeParameters;

typedef struct VRFrameData {
    double timestamp;

    float leftProjectionMatrix[16];
    float leftViewMatrix[16];

    float rightProjectionMatrix[16];
    float rightViewMatrix[16];

    VRPose pose;
} VRFrameData;

extern int emscripten_vr_ready(void);
extern int emscripten_vr_init(em_vr_arg_callback_func callback, void* userData);
extern int emscripten_vr_deinit(void);

extern int emscripten_vr_version_major(void);
extern int emscripten_vr_version_minor(void);

extern int emscripten_vr_count_displays(void);
extern VRDisplayHandle emscripten_vr_get_display_handle(int displayIndex);

extern int emscripten_vr_set_display_render_loop(VRDisplayHandle handle, em_vr_callback_func callback);
extern int emscripten_vr_set_display_render_loop_arg(VRDisplayHandle handle, em_vr_arg_callback_func callback, void* arg);
extern int emscripten_vr_cancel_display_render_loop(VRDisplayHandle handle);

extern int emscripten_vr_request_present(VRDisplayHandle handle, VRLayerInit* layerInit, int layerCount, em_vr_arg_callback_func callback, void* userData);
extern int emscripten_vr_get_frame_data(VRDisplayHandle handle, VRFrameData* frameData);
extern int emscripten_vr_submit_frame(VRDisplayHandle handle);
extern int emscripten_vr_exit_present(VRDisplayHandle handle);

extern const char *emscripten_vr_get_display_name(VRDisplayHandle handle);
extern int emscripten_vr_get_eye_parameters(VRDisplayHandle handle, VREye whichEye, VREyeParameters* eyeParams);
extern int emscripten_vr_get_display_capabilities(VRDisplayHandle handle, VRDisplayCapabilities* displayCaps);
extern bool emscripten_vr_display_connected(VRDisplayHandle handle);
extern bool emscripten_vr_display_presenting(VRDisplayHandle handle);

#ifdef __cplusplus
} // ~extern "C"
#endif

#endif
