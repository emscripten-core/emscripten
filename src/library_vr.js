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

  emscripten_vr_get_device_name: function(deviceId) {
    var dev = WebVR.getDeviceByID(deviceId);
    var buffer, devName;
    devName = dev ? dev.deviceName : "";
    buf = _malloc(devName.length + 1);
    writeStringToMemory(devName, buf);
    return buf;
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

  emscripten_vr_hmd_get_eye_parameters: function(deviceId, whichEye, eyeParamsPtr) {
    if (!eyeParamsPtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var params = dev.getEyeParameters(whichEye == WebVR.EYE_LEFT ? "left" : "right");

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.minimumFieldOfView.upDegrees, 'params.minimumFieldOfView.upDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.minimumFieldOfView.downDegrees, 'params.minimumFieldOfView.downDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.minimumFieldOfView.leftDegrees, 'params.minimumFieldOfView.leftDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.minimumFieldOfView.rightDegrees, 'params.minimumFieldOfView.rightDegrees', 'double') }}};

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.maximumFieldOfView.upDegrees, 'params.maximumFieldOfView.upDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.maximumFieldOfView.downDegrees, 'params.maximumFieldOfView.downDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.maximumFieldOfView.leftDegrees, 'params.maximumFieldOfView.leftDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.maximumFieldOfView.rightDegrees, 'params.maximumFieldOfView.rightDegrees', 'double') }}};

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.recommendedFieldOfView.upDegrees, 'params.recommendedFieldOfView.upDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.recommendedFieldOfView.downDegrees, 'params.recommendedFieldOfView.downDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.recommendedFieldOfView.leftDegrees, 'params.recommendedFieldOfView.leftDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.recommendedFieldOfView.rightDegrees, 'params.recommendedFieldOfView.rightDegrees', 'double') }}};

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.eyeTranslation.x, 'params.eyeTranslation.x', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.eyeTranslation.y, 'params.eyeTranslation.y', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.eyeTranslation.z, 'params.eyeTranslation.z', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.eyeTranslation.w, 'params.eyeTranslation.w', 'double') }}};

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.currentFieldOfView.upDegrees, 'params.currentFieldOfView.upDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.currentFieldOfView.downDegrees, 'params.currentFieldOfView.downDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.currentFieldOfView.leftDegrees, 'params.currentFieldOfView.leftDegrees', 'double') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.currentFieldOfView.rightDegrees, 'params.currentFieldOfView.rightDegrees', 'double') }}};

    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.renderRect.x, 'params.renderRect.x', 'i32') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.renderRect.y, 'params.renderRect.y', 'i32') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.renderRect.width, 'params.renderRect.width', 'i32') }}};
    {{{ makeSetValue('eyeParamsPtr', C_STRUCTS.WebVREyeParameters.renderRect.height, 'params.renderRect.height', 'i32') }}};

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

  emscripten_vr_sensor_get_state: function(deviceId, immediate, statePtr) {
    if (!statePtr) return 0;
    var dev = WebVR.getDeviceByID(deviceId);
    if (!dev) return 0;
    var state = immediate ? dev.getImmediateState : dev.getState();

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
    dev.resetSensor();
    return 1;
  }
};

autoAddDeps(LibraryWebVR, '$WebVR');
mergeInto(LibraryManager.library, LibraryWebVR);
