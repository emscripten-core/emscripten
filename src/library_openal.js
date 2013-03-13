//"use strict";

var LibraryOpenAL = {
  $AL__deps: ['$Browser'],
  $AL: {
    contexts: [],
    currentContext: null,

    structs: {
      Rect: Runtime.generateStructInfo([
        ['i32', 'x'], ['i32', 'y'], ['i32', 'w'], ['i32', 'h'], 
      ])
    },
  },

  alcProcessContext: function(context) {},
  alcSuspendContext: function(context) {},

  alcMakeContextCurrent: function(context) {
    if (context == 0) {
      AL.currentContext = null;
    } else {
      AL.currentContext = AL.contexts[context - 1];
    }
  },

  alcDestroyContext: function(context) {
    // Stop playback, etc
  },

  alcCloseDevice: function(device) {
    // Stop playback, etc
  },

  alcOpenDevice: function(deviceName) {
    if (typeof(AudioContext) == "function" ||
        typeof(webkitAudioContext) == "function") {
      return 1; // non-null pointer -- we just simulate one device
    } else {
      return 0;
    }
  },

  alcCreateContext: function(device, attrList) {
    if (device != 1) {
      return 0;
    }

    if (attrList) {
      console.log("The attrList argument of alcCreateContext is not supported yet");
      return 0;
    }

    var ctx;
    try {
      ctx = new AudioContext();
    } catch (e) {
      try {
        ctx = new webkitAudioContext();
      } catch (e) {}
    }

    if (ctx) {
      AL.contexts.push({ctx: ctx, err: 0, src: [], buf: []});
      return AL.contexts.length;
    } else {
      return 0;
    }
  },

  alGetError: function() {
    if (!AL.currentContext) {
      return 0xA004 /* AL_INVALID_OPERATION */;
    } else {
      return AL.currentContext.err;
    }
  },

  alGenSources: function(count, sources) {
    if (!AL.currentContext) {
      console.error("alGenSources called without a valid context");
      return;
    }
    for (var i = 0; i < count; ++i) {
      var src = AL.currentContext.ctx.createBufferSource();
      var gain = AL.currentContext.ctx.createGain();
      var panner = AL.currentContext.ctx.createPanner();
      src.connect(gain);
      gain.connect(panner);
      panner.connect(AL.currentContext.ctx.destination);
      AL.currentContext.src.push({src: src, gain: gain, panner: panner});
      {{{ makeSetValue('sources', 'i', 'AL.currentContext.src.length', 'i32') }}};
    }
  },

  alSourcei: function(source, param, value) {
    if (!AL.currentContext) {
      console.error("alSourcei called without a valid context");
      return;
    }
    if (source > AL.currentContext.src.length) {
      console.error("alSourcei called with an invalid source");
      return;
    }
    switch (param) {
    case 0x1007 /* AL_LOOPING */:
      AL.currentContext.src[source - 1].src.loop = (value != 0 /* AL_FALSE */);
      break;
    case 0x1009 /* AL_BUFFER */:
      if (value == 0) {
        AL.currentContext.src[source - 1].src.buffer = null;
      } else {
        AL.currentContext.src[source - 1].src.buffer = AL.currentContext.buf[value - 1].buf;
      }
      break;
    default:
      console.log("alSourcei with param " + param + " not implemented yet");
      break;
    }
  },

  alSourcef: function(source, param, value) {
    if (!AL.currentContext) {
      consoue.error("alSourcef called without a valid context");
      return;
    }
    if (source > AL.currentContext.src.length) {
      console.error("alSourcef called with an invalid source");
      return;
    }
    switch (param) {
    case 0x100A /* AL_GAIN */:
      AL.currentContext.src[source - 1].gain.gain.value = value;
      break;
    case 0x1003 /* AL_PITCH */:
      console.log("alSourcef was called with AL_PITCH, but Web Audio does not support static pitch changes");
      break;
    default:
      console.log("alSourcef with param " + param + " not implemented yet");
      break;
    }
  },

  alSourcefv: function(source, param, value) {
    if (!AL.currentContext) {
      consoue.error("alSourcefv called without a valid context");
      return;
    }
    if (source > AL.currentContext.src.length) {
      console.error("alSourcefv called with an invalid source");
      return;
    }
    switch (param) {
    case 0x1004 /* AL_POSITION */:
      AL.currentContext.src[source - 1].panner.setPosition(
          {{{ makeGetValue('value', '0', 'float') }}},
          {{{ makeGetValue('value', '1', 'float') }}},
          {{{ makeGetValue('value', '2', 'float') }}}
        );
      break;
    case 0x1006 /* AL_VELOCITY */:
      AL.currentContext.src[source - 1].panner.setVelocity(
          {{{ makeGetValue('value', '0', 'float') }}},
          {{{ makeGetValue('value', '1', 'float') }}},
          {{{ makeGetValue('value', '2', 'float') }}}
        );
      break;
    default:
      console.log("alSourcefv with param " + param + " not implemented yet");
      break;
    }
  },

  alSourceQueueBuffers: function(source, count, buffers) {
    if (!AL.currentContext) {
      console.error("alSourceQueueBuffers called without a valid context");
      return;
    }
    if (source > AL.currentContext.src.length) {
      console.error("alSourceQueueBuffers called with an invalid source");
      return;
    }
    if (count != 1) {
      console.error("Queuing multiple buffers using alSourceQueueBuffers is not supported yet");
      return;
    }
    for (var i = 0; i < count; ++i) {
      var buffer = {{{ makeGetValue('buffers', 'i', 'i32') }}};
      if (buffer >= AL.currentContext.buf.length) {
        console.error("alSourceQueueBuffers called with an invalid buffer");
        return;
      }
      AL.currentCOntext.src[source].src.buffer = AL.currentContext.buf[buffer].buf;
    }
  },

  alGenBuffers: function(count, buffers) {
    if (!AL.currentContext) {
      console.error("alGenBuffers called without a valid context");
      return;
    }
    for (var i = 0; i < count; ++i) {
      AL.currentContext.buf.push({buf: null});
      {{{ makeSetValue('buffers', 'i', 'AL.currentContext.buf.length - 1', 'i32') }}};
    }
  },

  alBufferData: function(buffer, format, data, size, freq) {
    if (!AL.currentContext) {
      console.error("alBufferData called without a valid context");
      return;
    }
    if (buffer >= AL.currentContext.buf.length) {
      console.error("alBufferData called with an invalid buffer");
      return;
    }
    var channels, bytes;
    switch (format) {
    case 0x1100 /* AL_FORMAT_MONO8 */:
      bytes = 1;
    case 0x1101 /* AL_FORMAT_MONO16 */:
      bytes = 2;
      channels = 1;
      break;
    case 0x1102 /* AL_FORMAT_STEREO8 */:
      bytes = 1;
    case 0x1103 /* AL_FORMAT_STEREO16 */:
      bytes = 2;
      channels = 2;
      break;
    default:
      console.error("alBufferData called with invalid format " + format);
      return;
    }
    AL.currentContext.buf[buffer].buf = AL.currentContext.ctx.createBuffer(channels, size / (bytes * channels), freq);
    var data = new Array(channels);
    for (var i = 0; i < channels; ++i) {
      data[i] = AL.currentContext.buf[buffer].buf.getChannelData(i);
    }
    for (var i = 0; i < size / (bytes * channels); ++i) {
      for (var j = 0; j < channels; ++j) {
        switch (bytes) {
        case 1:
          var val = {{{ makeGetValue('data', 'i+j', 'i8') }}};
          data[j][i] = -1.0 + val * (2/256);
          break;
        case 2:
          var val = {{{ makeGetValue('data', 'i+j', 'i16') }}};
          data[j][i] = -1.0 + (val + -32768) * (2/65536);
          break;
        }
      }
    }
  },

  alDistanceModel: function(model) {
    if (model != 0 /* AL_NONE */) {
      console.log("Only alDistanceModel(AL_NONE) is currently supported");
    }
  },

  alListenerfv: function(param, values) {
    console.log("alListenerfv is not supported yet");
  },

  alIsExtensionPresent: function(extName) {
    return 0;
  },

  alcIsExtensionPresent: function(device, extName) {
    return 0;
  },

  alGetProcAddress: function(fname) {
    return 0;
  },

  alcGetProcAddress: function(device, fname) {
    return 0;
  },

};

autoAddDeps(LibraryOpenAL, '$AL');
mergeInto(LibraryManager.library, LibraryOpenAL);

