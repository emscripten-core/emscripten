// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

var LibraryWebVR = {
  $WebVR: {
    EYE_LEFT: {{{ cDefine('VR_EYE_LEFT') }}},
    EYE_RIGHT: {{{ cDefine('VR_EYE_RIGHT') }}},

    POSE_POSITION: {{{ cDefine('VR_POSE_POSITION') }}},
    POSE_LINEAR_VELOCITY: {{{ cDefine('VR_POSE_LINEAR_VELOCITY') }}},
    POSE_LINEAR_ACCELERATION: {{{ cDefine('VR_POSE_LINEAR_ACCELERATION') }}},
    POSE_ORIENTATION: {{{ cDefine('VR_POSE_ORIENTATION') }}},
    POSE_ANGULAR_VELOCITY: {{{ cDefine('VR_POSE_ANGULAR_VELOCITY') }}},
    POSE_ANGULAR_ACCELERATION: {{{ cDefine('VR_POSE_ANGULAR_ACCELERATION') }}},

    initialized: false,
    ready: false,
    version: [-1, -1],
    displays: [],
    displayNames: [],

    init: function(callback) {
      if (WebVR.initialized) return;

      WebVR.initialized = true;

      if (!navigator.getVRDisplays) {
        /* WebVR 1.1 required, but not supported. */
        WebVR.ready = true;
        WebVR.displays = [];
        return 0;
      }

      WebVR.version = [1, 1];

      navigator.getVRDisplays().then(function(displays) {
        WebVR.ready = true;
        WebVR.displays = displays;
        WebVR.displayNames = new Array(displays.length);
        callback();
      });

      return 1;
    },

    deinit: function() {
      WebVR.displayNames.forEach(function(name) {
        _free(name);
      });
      return 1
    },

    dereferenceDisplayHandle: function(displayHandle) {
      /* Display handles start as 1 as 0 will be interpreted as false or null-handle
       * on errors */
      if (displayHandle < 1 || displayHandle > WebVR.displays.length) {
        console.log("library_vr dereferenceDisplayHandle invalid display handle at: " + stackTrace());
        return null;
      }

      return WebVR.displays[displayHandle-1];
    }
  },

  emscripten_vr_init: function(func, userData) {
    return WebVR.init(function() {
      Runtime.dynCall('vi', func, [userData]);
    });
  },

  emscripten_vr_deinit: function() {
    return WebVR.deinit();
  },

  emscripten_vr_version_major: function() {
    return WebVR.version[0];
  },

  emscripten_vr_version_minor: function() {
    return WebVR.version[1];
  },

  emscripten_vr_ready: function() {
    return WebVR.ready ? 1 : 0;
  },

  emscripten_vr_count_displays: function() {
    return WebVR.displays.length;
  },

  emscripten_vr_get_display_handle: function(displayIndex) {
    if (displayIndex < 0 || displayIndex >= WebVR.displays.length) {
      return -1;
    }

    /* As displayHandle == 0 will be interpreted as NULL handle for errors,
     * the handle is index + 1. */
    return displayIndex + 1;
  },

  emscripten_vr_get_display_name: function(displayHandle) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display) return 0;

    var name = WebVR.displayNames[displayHandle-1];
    if (name) {
      return name;
    }

    var buffer, displayName;
    displayName = display ? display.displayName : "";
    var len = lengthBytesUTF8(displayName);
    buffer = _malloc(len + 1);
    stringToUTF8(displayName, buffer, len + 1);

    WebVR.displayNames[displayHandle-1] = buffer;

    return buffer;
  },

  emscripten_vr_get_display_capabilities: function(displayHandle, capsPtr) {
    if (!capsPtr) return 0;

    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display) return 0;

    var caps = display.capabilities;

    {{{ makeSetValue('capsPtr', C_STRUCTS.VRDisplayCapabilities.hasPosition, 'caps.hasPosition ? 1 : 0', 'i32') }}};
    {{{ makeSetValue('capsPtr', C_STRUCTS.VRDisplayCapabilities.hasExternalDisplay, 'caps.hasExternalDisplay ? 1 : 0', 'i32') }}};
    {{{ makeSetValue('capsPtr', C_STRUCTS.VRDisplayCapabilities.canPresent, 'caps.canPresent ? 1 : 0', 'i32') }}};

    {{{ makeSetValue('capsPtr', C_STRUCTS.VRDisplayCapabilities.maxLayers, 'caps.maxLayers', 'i64') }}};

    return 1;
  },

  emscripten_vr_get_eye_parameters: function(displayHandle, whichEye, eyeParamsPtr) {
    if (!eyeParamsPtr) return 0;

    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display) return 0;

    var params = display.getEyeParameters(whichEye == WebVR.EYE_LEFT ? "left" : "right");

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.VREyeParameters.offset.x, 'params.offset[0]', 'float') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.VREyeParameters.offset.y, 'params.offset[1]', 'float') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.VREyeParameters.offset.z, 'params.offset[2]', 'float') }}};

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.VREyeParameters.renderWidth, 'params.renderWidth', 'i64') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.VREyeParameters.renderHeight, 'params.renderHeight', 'i64') }}};

    return 1;
  },

  emscripten_vr_display_connected: function(displayHandle) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display || !display.isConnected) return 0;
    return 1;
  },

  emscripten_vr_display_presenting: function(displayHandle) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display || !display.isPresenting) return 0;
    return 1;
  },

  emscripten_vr_set_display_render_loop: function(displayHandle, func, arg) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display) return 0;

    assert(!display.mainLoop || !display.mainLoop.scheduler, "emscripten_vr_set_device_main_loop: there can only be one render loop function per VRDisplay: call emscripten_vr_cancel_render_loop to cancel the previous one before setting a new one with different parameters.");

    var displayIterationFunc;
    if (typeof arg !== 'undefined') {
      displayIterationFunc = function() {
        dynCall('vi', func, [arg]);
      };
    } else {
      displayIterationFunc = function() {
        dynCall('v', func);
      };
    }

    display.mainLoop = {
      running: !display.mainLoop ? false : display.mainLoop.running,
      scheduler: function() {
        display.requestAnimationFrame(display.mainLoop.runner);
      },
      runner: function() {
        if (ABORT) return;

        /* Prevent scheduler being called twice when loop is changed */
        display.mainLoop.running = true;

#if USES_GL_EMULATION
        GL.newRenderingFrameStarted();
#endif

        try {
          displayIterationFunc();
        } catch (e) {
          if (e instanceof ExitStatus) {
            return;
          } else {
            if (e && typeof e === 'object' && e.stack) err('exception thrown in render loop of VR display ' + displayHandle.toString() + ': ' + [e, e.stack]);
            throw e;
          }
        }

#if STACK_OVERFLOW_CHECK
        checkStackCookie();
#endif

        if (!display.mainLoop.scheduler) {
          display.mainLoop.running = false;
        } else {
          display.mainLoop.scheduler();
        }
      },
      pause: function() {
        display.mainLoop.scheduler = null;
      }
    };

    if (!display.mainLoop.running) {
      display.mainLoop.scheduler();
    } // otherwise called by display.mainLoop.runner()
    return 1;
  },

  emscripten_vr_set_display_render_loop_arg__deps: ['emscripten_vr_set_display_render_loop'],
  emscripten_vr_set_display_render_loop_arg: function(displayHandle, func, arg) {
    return _emscripten_vr_set_display_render_loop(displayHandle, func, arg);
  },

  emscripten_vr_cancel_display_render_loop: function(displayHandle) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display || !display.mainLoop) return 0;

    display.mainLoop.pause();
    return 1;
  },

  emscripten_vr_request_present: function(displayHandle, layerInitPtr, layerCount, func, userData) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display) return 0;

    layerInit = new Array(layerCount);
    for (var i = 0; i < layerCount; ++i) {
      sourceStrPtr = {{{ makeGetValue('layerInitPtr', C_STRUCTS.VRLayerInit.source, 'void*') }}};

      var source = null;
      if (sourceStrPtr == 0) {
        source = Module['canvas'];
      } else {
        sourceStr = UTF8ToString(sourceStrPtr);

        if (sourceStr && sourceStr.length > 0) {
          source = document.getElementById(sourceStr);
        }

        if (!source) {
          return 0;
        }
      }

      leftBounds = new Float32Array(4);
      rightBounds = new Float32Array(4);
      var ptr = layerInitPtr;
      for (var j = 0; j < 4; ++j) {
        leftBounds[j] = {{{ makeGetValue('layerInitPtr', C_STRUCTS.VRLayerInit.leftBounds + '+ 4*j', 'float') }}};
        rightBounds[j] = {{{ makeGetValue('layerInitPtr', C_STRUCTS.VRLayerInit.rightBounds + '+ 4*j', 'float') }}};
        ptr += 4;
      }

      layerInit[i] = {
        source: source,
        leftBounds: leftBounds,
        rightBounds: rightBounds
      };
      layerInitPtr += {{{ C_STRUCTS.VRLayerInit.__size__ }}};
    }

    display.requestPresent(layerInit).then(function() {
      if (!func) return;
      dynCall('vi', func, [userData]);
    });

    return 1;
  },

  emscripten_vr_exit_present: function(displayHandle) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display) return 0;

    display.exitPresent();
    return 1;
  },

  emscripten_vr_get_frame_data: function(displayHandle, frameDataPtr) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display || !display.mainLoop || !frameDataPtr) return 0;

    if (!display.frameData) {
      display.frameData = new VRFrameData();
    }
    display.getFrameData(display.frameData);

    /* Pose */

    /* Used to expose to C which attributes are valid (!== null) */
    var poseFlags = 0;

    {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.timestamp, 'display.frameData.timestamp', 'double') }}};

    if (display.frameData.pose.position !== null) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.position.x, 'display.frameData.pose.position[0]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.position.y, 'display.frameData.pose.position[1]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.position.z, 'display.frameData.pose.position[2]', 'float') }}};

      poseFlags |= WebVR.POSE_POSITION;
    }

    if (display.frameData.pose.linearVelocity !== null) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.linearVelocity.x, 'display.frameData.pose.linearVelocity[0]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.linearVelocity.y, 'display.frameData.pose.linearVelocity[1]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.linearVelocity.z, 'display.frameData.pose.linearVelocity[2]', 'float') }}};

      poseFlags |= WebVR.POSE_LINEAR_VELOCITY;
    }

    if (display.frameData.pose.linearAcceleration !== null) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.linearAcceleration.x, 'display.frameData.pose.linearAcceleration[0]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.linearAcceleration.y, 'display.frameData.pose.linearAcceleration[1]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.linearAcceleration.z, 'display.frameData.pose.linearAcceleration[2]', 'float') }}};

      poseFlags |= WebVR.POSE_LINEAR_ACCELERATION;
    }

    if (display.frameData.pose.orientation !== null) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.orientation.x, 'display.frameData.pose.orientation[0]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.orientation.y, 'display.frameData.pose.orientation[1]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.orientation.z, 'display.frameData.pose.orientation[2]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.orientation.w, 'display.frameData.pose.orientation[3]', 'float') }}};

        poseFlags |= WebVR.POSE_ORIENTATION;
    }

    if (display.frameData.pose.angularVelocity !== null) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.angularVelocity.x, 'display.frameData.pose.angularVelocity[0]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.angularVelocity.y, 'display.frameData.pose.angularVelocity[1]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.angularVelocity.z, 'display.frameData.pose.angularVelocity[2]', 'float') }}};

      poseFlags |= WebVR.POSE_ANGULAR_VELOCITY;
    }

    if (display.frameData.pose.angularAcceleration !== null) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.angularAcceleration.x, 'display.frameData.pose.angularAcceleration[0]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.angularAcceleration.y, 'display.frameData.pose.angularAcceleration[1]', 'float') }}};
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.angularAcceleration.z, 'display.frameData.pose.angularAcceleration[0]', 'float') }}};

      poseFlags |= WebVR.POSE_ANGULAR_ACCELERATION;
    }

    {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.pose.poseFlags, 'poseFlags', 'i32') }}};

    /* Matrices */

    for (var i = 0; i < 16; ++i) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.leftProjectionMatrix + ' + i*4', 'display.frameData.leftProjectionMatrix[i]', 'float') }}};
    }

    for (var i = 0; i < 16; ++i) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.leftViewMatrix + ' + i*4', 'display.frameData.leftViewMatrix[i]', 'float') }}};
    }

    for (var i = 0; i < 16; ++i) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.rightProjectionMatrix + ' + i*4', 'display.frameData.rightProjectionMatrix[i]', 'float') }}};
    }

    for (var i = 0; i < 16; ++i) {
      {{{ makeSetValue('frameDataPtr', C_STRUCTS.VRFrameData.rightViewMatrix + ' + i*4', 'display.frameData.rightViewMatrix[i]', 'float') }}};
    }

    return 1;
  },

  emscripten_vr_submit_frame: function(displayHandle) {
    var display = WebVR.dereferenceDisplayHandle(displayHandle);
    if (!display || !display.mainLoop) return 0;

    display.submitFrame();

    return 1;
  }

};

autoAddDeps(LibraryWebVR, '$WebVR');
mergeInto(LibraryManager.library, LibraryWebVR);
