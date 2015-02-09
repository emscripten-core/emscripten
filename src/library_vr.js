var LibraryWebVR = {
  $WebVR: {
    DEVICE_TYPE_UNKNOWN: 0,
    DEVICE_TYPE_HMD: 1,
    DEVICE_TYPE_SENSOR: 2,

    EYE_LEFT: 0,
    EYE_RIGHT: 1,

    initialized: false,
    ready: false,
    devices: [],
    deviceHardwareIds: {},
    nextHardwareDeviceId: 1,
    getDevicesPromise: null,

    selectedHMD: null,
    selectedHMDId: 0,

    init: function() {
      if (WebVR.initialized) return;

      WebVR.initialized = true;

      if (!navigator.getVRDevices) {
        WebVR.ready = true;
        WebVR.devices = [];
        return;
      }

      navigator.getVRDevices().then(
        function(devs) {
          WebVR.ready = true;
          WebVR.devices = devs;
        },
        function() {
          console.log("Emscripten WebVR getVRDevices() hit error callback");
          WebVR.ready = true;
          WebVR.devices = [];
        }
      );
    },

    getDeviceByID: function(deviceId) {
      if (deviceId < 1 || deviceId > WebVR.devices.length) {
        console.log("library_vr getDeviceByID invalid device id at: " + stackTrace());
        return null;
      }

      return WebVR.devices[deviceId-1];
    }
  },

  emscripten_vr_init: function() {
    WebVR.init();
  },

  emscripten_vr_ready: function() {
    return WebVR.ready ? 1 : 0;
  },

  emscripten_vr_count_devices: function() {
    return WebVR.devices.length;
  },

  emscripten_vr_get_device_id: function(deviceIndex) {
    if (deviceIndex < 0 || deviceIndex >= WebVR.devices.length) {
      return -1;
    }

    // we're doing to treat the device ID the same as the index + 1
    return deviceIndex + 1;
  },

  emscripten_vr_get_device_hwid: function(deviceId) {
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return -1;

    if (!WebVR.deviceHardwareIds[dev.hardwareUnitId]) {
      WebVR.deviceHardwareIds[dev.hardwareUnitId] = WebVR.nextHardwareDeviceId++;
    }
    return WebVR.deviceHardwareIds[dev.hardwareUnitId];
  },

  emscripten_vr_get_device_type: function(deviceId) {
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return -1;

    if (dev instanceof HMDVRDevice) {
      return WebVR.DEVICE_TYPE_HMD;
    }
    if (dev instanceof PositionSensorVRDevice) {
      return WebVR.DEVICE_TYPE_SENSOR;
    }
    return WebVR.DEVICE_TYPE_UNKNOWN;
  },

  emscripten_vr_select_hmd_device: function(deviceId) {
    if (deviceId == 0) {
      WebVR.selectedHMD = null;
      WebVR.selectedHMDId = 0;
      return 1;
    }

    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev || !(dev instanceof HMDVRDevice)) {
      console.log("Trying to call emscripten_vr_select_hmd_device on invalid or non-HMD device ID!");
      return 0;
    }
    WebVR.selectedHMD = dev;
    WebVR.selectedHMDId = deviceId;
    return 1;
  },

  emscripten_vr_get_selected_hmd_device: function() {
    return WebVR.selectedHMDId;
  },

  emscripten_vr_hmd_get_eye_translation: function(deviceId, whichEye, translationPtr) {
    if (!translationPtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var translation = dev.getEyeTranslation(whichEye == WebVR.EYE_LEFT ? "left" : "right");
    {{{ makeSetValue('translationPtr', C_STRUCTS.WebVRPoint.x, 'translation.x', 'double') }}};
    {{{ makeSetValue('translationPtr', C_STRUCTS.WebVRPoint.y, 'translation.y', 'double') }}};
    {{{ makeSetValue('translationPtr', C_STRUCTS.WebVRPoint.z, 'translation.z', 'double') }}};
    {{{ makeSetValue('translationPtr', C_STRUCTS.WebVRPoint.w, 'translation.w', 'double') }}};
    return 1;
  },

  emscripten_vr_hmd_get_current_fov: function(deviceId, whichEye, fovPtr) {
    if (!fovPtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var fov = dev.getCurrentEyeFieldOfView(whichEye == WebVR.EYE_LEFT ? "left" : "right");
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.upDegrees, 'fov.upDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.downDegrees, 'fov.downDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.leftDegrees, 'fov.leftDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.rightDegrees, 'fov.rightDegrees', 'double') }}};
    return 1;
  },

  emscripten_vr_hmd_get_recommended_fov: function(deviceId, whichEye, fovPtr) {
    if (!fovPtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var fov = dev.getRecommendedEyeFieldOfView(whichEye == WebVR.EYE_LEFT ? "left" : "right");
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.upDegrees, 'fov.upDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.downDegrees, 'fov.downDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.leftDegrees, 'fov.leftDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.rightDegrees, 'fov.rightDegrees', 'double') }}};
    return 1;
  },

  emscripten_vr_hmd_get_maximum_fov: function(deviceId, whichEye, fovPtr) {
    if (!fovPtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var fov = dev.getMaximumEyeFieldOfView(whichEye == WebVR.EYE_LEFT ? "left" : "right");
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.upDegrees, 'fov.upDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.downDegrees, 'fov.downDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.leftDegrees, 'fov.leftDegrees', 'double') }}};
    {{{ makeSetValue('fovPtr', C_STRUCTS.WebVRFieldOfView.rightDegrees, 'fov.rightDegrees', 'double') }}};
    return 1;
  },

  emscripten_vr_hmd_set_fov: function(deviceId, leftFovPtr, rightFovPtr, zNear, zFar) {
    if (!leftFovPtr || !rightFovPtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var leftFov = {
      upDegrees: {{{ makeGetValue('leftFovPtr', C_STRUCTS.WebVRFieldOfView.upDegrees, 'double') }}},
      downDegrees: {{{ makeGetValue('leftFovPtr', C_STRUCTS.WebVRFieldOfView.downDegrees, 'double') }}},
      leftDegrees: {{{ makeGetValue('leftFovPtr', C_STRUCTS.WebVRFieldOfView.leftDegrees, 'double') }}},
      rightDegrees: {{{ makeGetValue('leftFovPtr', C_STRUCTS.WebVRFieldOfView.rightDegrees, 'double') }}}
    };
    var rightFov = {
      upDegrees: {{{ makeGetValue('rightFovPtr', C_STRUCTS.WebVRFieldOfView.upDegrees, 'double') }}},
      downDegrees: {{{ makeGetValue('rightFovPtr', C_STRUCTS.WebVRFieldOfView.downDegrees, 'double') }}},
      leftDegrees: {{{ makeGetValue('rightFovPtr', C_STRUCTS.WebVRFieldOfView.leftDegrees, 'double') }}},
      rightDegrees: {{{ makeGetValue('rightFovPtr', C_STRUCTS.WebVRFieldOfView.rightDegrees, 'double') }}}
    };
    dev.setFieldOfView(leftFov, rightFov, zNear, zFar);
    return 1;
  },

  emscripten_vr_hmd_get_eye_render_rect: function(deviceId, whichEye, rectPtr) {
    if (!rectPtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var rect = dev.getRecommendedEyeRenderRect(whichEye == WebVR.EYE_LEFT ? "left" : "right");
    {{{ makeSetValue('rectPtr', C_STRUCTS.WebVRIntRect.x, 'rect.x', 'i32') }}};
    {{{ makeSetValue('rectPtr', C_STRUCTS.WebVRIntRect.y, 'rect.y', 'i32') }}};
    {{{ makeSetValue('rectPtr', C_STRUCTS.WebVRIntRect.width, 'rect.width', 'i32') }}};
    {{{ makeSetValue('rectPtr', C_STRUCTS.WebVRIntRect.height, 'rect.height', 'i32') }}};
    return 1;
  },

  emscripten_vr_sensor_get_state: function(deviceId, timeOffset, statePtr) {
    if (!statePtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var state = dev.getState(timeOffset);

    {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.timeStamp, 'state.timeStamp', 'double') }}};

    {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.hasPosition, 'state.hasPosition ? 1 : 0', 'i32') }}};
    if (state.hasPosition) {
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.position.x, 'state.position.x', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.position.y, 'state.position.y', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.position.z, 'state.position.z', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.linearVelocity.x, 'state.linearVelocity.x', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.linearVelocity.y, 'state.linearVelocity.y', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.linearVelocity.z, 'state.linearVelocity.z', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.linearAcceleration.x, 'state.linearAcceleration.x', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.linearAcceleration.y, 'state.linearAcceleration.y', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.linearAcceleration.z, 'state.linearAcceleration.z', 'double') }}};
    }

    {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.hasOrientation, 'state.hasOrientation ? 1 : 0', 'i32') }}};
    if (state.hasOrientation) {
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.orientation.x, 'state.orientation.x', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.orientation.y, 'state.orientation.y', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.orientation.z, 'state.orientation.z', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.orientation.w, 'state.orientation.w', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularVelocity.x, 'state.angularVelocity.x', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularVelocity.y, 'state.angularVelocity.y', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularVelocity.z, 'state.angularVelocity.z', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularVelocity.w, 'state.angularVelocity.w', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularAcceleration.x, 'state.angularAcceleration.x', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularAcceleration.y, 'state.angularAcceleration.y', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularAcceleration.z, 'state.angularAcceleration.z', 'double') }}};
      {{{ makeSetValue('statePtr', C_STRUCTS.WebVRPositionState.angularAcceleration.w, 'state.angularAcceleration.w', 'double') }}};
    }

    return 1;
  },

  emscripten_vr_sensor_zero: function(deviceId) {
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    dev.zeroSensor();
    return 1;
  }
};

autoAddDeps(LibraryWebVR, '$WebVR');
mergeInto(LibraryManager.library, LibraryWebVR);
