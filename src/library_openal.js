//"use strict";

var LibraryOpenAL = {
  $AL__deps: ['$Browser'],
  $AL: {
    contexts: [],
    currentContext: null,
    QUEUE_INTERVAL: 25,
    QUEUE_LOOKAHEAD: 100,

    updateSources: function updateSources(context) {
      for (var i = 0; i < context.src.length; i++) {
        AL.updateSource(context.src[i]);
      }
    },

    updateSource: function updateSource(src) {
#if OPENAL_DEBUG
      var idx = AL.currentContext.src.indexOf(src);
#endif
      if (src.state !== 0x1012 /* AL_PLAYING */) {
        return;
      }

      var currentTime = AL.currentContext.ctx.currentTime;
      var startTime = src.bufferPosition;

      for (var i = src.buffersPlayed; i < src.queue.length; i++) {
        var entry = src.queue[i];

        var startOffset = startTime - currentTime;
        var endTime = startTime + entry.buffer.duration;

        // Clean up old buffers.
        if (currentTime >= endTime) {
          // Update our location in the queue.
          src.bufferPosition = endTime;
          src.buffersPlayed = i + 1;

          // Stop / restart the source when we hit the end.
          if (src.buffersPlayed >= src.queue.length) {
            if (src.loop) {
              AL.setSourceState(src, 0x1012 /* AL_PLAYING */);
            } else {
              AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
            }
          }
        }
        // Process all buffers that'll be played before the next tick.
        else if (startOffset < (AL.QUEUE_LOOKAHEAD / 1000) && !entry.src) {
          // If the start offset is negative, we need to offset the actual buffer.
          var offset = Math.abs(Math.min(startOffset, 0));

          entry.src = AL.currentContext.ctx.createBufferSource();
          entry.src.buffer = entry.buffer;
          entry.src.connect(src.gain);
          entry.src.start(startTime, offset);
          
#if OPENAL_DEBUG
          console.log('updateSource queuing buffer ' + i + ' for source ' + idx + ' at ' + startTime + ' (offset by ' + offset + ')');
#endif
        }

        startTime = endTime;
      }
    },

    setSourceState: function setSourceState(src, state) {
#if OPENAL_DEBUG
      var idx = AL.currentContext.src.indexOf(src);
#endif
      if (state === 0x1012 /* AL_PLAYING */) {
        if (src.state !== 0x1013 /* AL_PAUSED */) {
          src.state = 0x1012 /* AL_PLAYING */;
          // Reset our position.
          src.bufferPosition = AL.currentContext.ctx.currentTime;
          src.buffersPlayed = 0;
#if OPENAL_DEBUG
          console.log('setSourceState resetting and playing source ' + idx);
#endif
        } else {
          src.state = 0x1012 /* AL_PLAYING */;
          // Use the current offset from src.bufferPosition to resume at the correct point.
          src.bufferPosition = AL.currentContext.ctx.currentTime - src.bufferPosition;
#if OPENAL_DEBUG
          console.log('setSourceState resuming source ' + idx + ' at ' + src.bufferPosition.toFixed(4));
#endif
        }
        AL.stopSourceQueue(src);
        AL.updateSource(src);
      } else if (state === 0x1013 /* AL_PAUSED */) {
        if (src.state === 0x1012 /* AL_PLAYING */) {
          src.state = 0x1013 /* AL_PAUSED */;
          // Store off the current offset to restore with on resume.
          src.bufferPosition = AL.currentContext.ctx.currentTime - src.bufferPosition;
          AL.stopSourceQueue(src);
#if OPENAL_DEBUG
          console.log('setSourceState pausing source ' + idx + ' at ' + src.bufferPosition.toFixed(4));
#endif
        }
      } else if (state === 0x1014 /* AL_STOPPED */) {
        if (src.state !== 0x1011 /* AL_INITIAL */) {
          src.state = 0x1014 /* AL_STOPPED */;
          src.buffersPlayed = src.queue.length;
          AL.stopSourceQueue(src);
#if OPENAL_DEBUG
          console.log('setSourceState stopping source ' + idx);
#endif
        }
      } else if (state == 0x1011 /* AL_INITIAL */) {
        if (src.state !== 0x1011 /* AL_INITIAL */) {
          src.state = 0x1011 /* AL_INITIAL */;
          src.bufferPosition = 0;
          src.buffersPlayed = 0;
#if OPENAL_DEBUG
          console.log('setSourceState initializing source ' + idx);
#endif
        }
      }
    },

    stopSourceQueue: function stopSourceQueue(src) {
      for (var i = 0; i < src.queue.length; i++) {
        var entry = src.queue[i];
        if (entry.src) {
          entry.src.stop(0);
          entry.src = null;
        }
      }
    }
  },

  alcProcessContext: function(context) {},
  alcSuspendContext: function(context) {},

  alcMakeContextCurrent: function(context) {
    if (context == 0) {
      AL.currentContext = null;
      return 0;
    } else {
      AL.currentContext = AL.contexts[context - 1];
      return 1;
    }
  },

  alcGetContextsDevice: function(context) {
    if (context <= AL.contexts.length && context > 0) {
      // Returns the only one audio device
      return 1;
    }
    return 0;
  },

  alcGetCurrentContext: function() {
    for (var i = 0; i < AL.contexts.length; ++i) {
      if (AL.contexts[i] == AL.currentContext) {
        return i + 1;
      }
    }
    return 0;
  },

  alcDestroyContext: function(context) {
    // Stop playback, etc
    clearInterval(AL.contexts[context - 1].interval);
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

  alcCreateContext__deps: ['updateSources'],
  alcCreateContext: function(device, attrList) {
    if (device != 1) {
      return 0;
    }

    if (attrList) {
#if OPENAL_DEBUG
      console.log("The attrList argument of alcCreateContext is not supported yet");
#endif
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
      var context = {
        ctx: ctx,
        err: 0,
        src: [],
        buf: [],
        interval: setInterval(function() { AL.updateSources(context); }, AL.QUEUE_INTERVAL)
      };
      AL.contexts.push(context);
      return AL.contexts.length;
    } else {
      return 0;
    }
  },

  alGetError: function() {
    if (!AL.currentContext) {
      return 0xA004 /* AL_INVALID_OPERATION */;
    } else {
      // Reset error on get.
      var err = AL.currentContext.err;
      AL.currentContext.err = 0 /* AL_NO_ERROR */;
      return err;
    }
  },

  alcGetError__deps: ['alGetError'],
  alcGetError: function(device) {
    // We have only one audio device, so just return alGetError.
    return _alGetError();
  },

  alDeleteSources: function(count, sources) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alDeleteSources called without a valid context");
#endif
      return;
    }
    for (var i = 0; i < count; ++i) {
      var sourceIdx = {{{ makeGetValue('sources', 'i*4', 'i32') }}} - 1;
      delete AL.currentContext.src[sourceIdx];
    }
  },

  alGenSources: function(count, sources) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGenSources called without a valid context");
#endif
      return;
    }
    for (var i = 0; i < count; ++i) {
      var gain = AL.currentContext.ctx.createGain();
      gain.connect(AL.currentContext.ctx.destination);
      AL.currentContext.src.push({
        state: 0x1011 /* AL_INITIAL */,
        queue: [],
        loop: false,
        get refDistance() {
          return this._refDistance || 1;
        },
        set refDistance(val) {
          this._refDistance = val;
          if (this.panner) this.panner.refDistance = val;
        },
        get maxDistance() {
          return this._maxDistance || 10000;
        },
        set maxDistance(val) {
          this._maxDistance = val;
          if (this.panner) this.panner.maxDistance = val;
        },
        get rolloffFactor() {
          return this._rolloffFactor || 1;
        },
        set rolloffFactor(val) {
          this._rolloffFactor = val;
          if (this.panner) this.panner.rolloffFactor = val;
        },
        get position() {
          return this._position || [0, 0, 0];
        },
        set position(val) {
          this._position = val;
          if (this.panner) this.panner.setPosition(val[0], val[1], val[2]);
        },
        get velocity() {
          return this._velocity || [0, 0, 0];
        },
        set velocity(val) {
          this._velocity = val;
          if (this.panner) this.panner.setVelocity(val[0], val[1], val[2]);
        },
        gain: gain,
        panner: null,
        buffersPlayed: 0,
        bufferPosition: 0
      });
      {{{ makeSetValue('sources', 'i*4', 'AL.currentContext.src.length', 'i32') }}};
    }
  },

  alSourcei__deps: ['updateSource'],
  alSourcei: function(source, param, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourcei called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcei called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x1007 /* AL_LOOPING */:
      src.loop = (value === 1 /* AL_TRUE */);
      break;
    case 0x1009 /* AL_BUFFER */:
      var buffer = AL.currentContext.buf[value - 1];
      if (value == 0) {
        src.queue = [];
      } else {
        src.queue = [{ buffer: buffer }];
      }
      AL.updateSource(src);
      break;
    case 0x202 /* AL_SOURCE_RELATIVE */:
      if (value === 1 /* AL_TRUE */) {
        if (src.panner) {
          src.panner = null;
            
          // Disconnect from the panner.
          src.gain.disconnect();

          src.gain.connect(AL.currentContext.ctx.destination);
        }
      } else if (value === 0 /* AL_FALSE */) {
        if (!src.panner) {
          var panner = src.panner = AL.currentContext.ctx.createPanner();
          panner.panningModel = "equalpower";
          panner.distanceModel = "linear";
          panner.refDistance = src.refDistance;
          panner.maxDistance = src.maxDistance;
          panner.rolloffFactor = src.rolloffFactor;
          panner.setPosition(src.position[0], src.position[1], src.position[2]);
          panner.setVelocity(src.velocity[0], src.velocity[1], src.velocity[2]);
          panner.connect(AL.currentContext.ctx.destination);

          // Disconnect from the default source.
          src.gain.disconnect();

          src.gain.connect(panner);
        }
      } else {
        AL.currentContext.err = 0xA003 /* AL_INVALID_VALUE */;
      }
      break;
    default:
#if OPENAL_DEBUG
      console.log("alSourcei with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alSourcef: function(source, param, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourcef called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcef called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x1003 /* AL_PITCH */:
#if OPENAL_DEBUG
      console.log("alSourcef was called with 0x1003 /* AL_PITCH */, but Web Audio does not support static pitch changes");
#endif
      break;
    case 0x100A /* AL_GAIN */:
      src.gain.gain.value = value;
      break;
    // case 0x100D /* AL_MIN_GAIN */:
    //   break;
    // case 0x100E /* AL_MAX_GAIN */:
    //   break;
    case 0x1023 /* AL_MAX_DISTANCE */:
      src.maxDistance = value;
      break;
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
      src.rolloffFactor = value;
      break;
    // case 0x1022 /* AL_CONE_OUTER_GAIN */:
    //   break;
    // case 0x1001 /* AL_CONE_INNER_ANGLE */:
    //   break;
    // case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    //   break;
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
      src.refDistance = value;
      break;
    default:
#if OPENAL_DEBUG
      console.log("alSourcef with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alSource3f: function(source, param, v1, v2, v3) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSource3f called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSource3f called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x1004 /* AL_POSITION */:
      src.position = [v1, v2, v3];
      break;
    case 0x1006 /* AL_VELOCITY */:
      src.velocity = [v1, v2, v3];
      break;
    default:
#if OPENAL_DEBUG
      console.log("alSource3f with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alSourcefv__deps: ['alSource3f'],
  alSourcefv: function(source, param, value) {
    _alSource3f(source, param,
      {{{ makeGetValue('value', '0', 'float') }}},
      {{{ makeGetValue('value', '4', 'float') }}},
      {{{ makeGetValue('value', '8', 'float') }}});
  },

  alSourceQueueBuffers__deps: ["updateSource"],
  alSourceQueueBuffers: function(source, count, buffers) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourceQueueBuffers called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourceQueueBuffers called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    for (var i = 0; i < count; ++i) {
      var bufferIdx = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
      if (bufferIdx > AL.currentContext.buf.length) {
#if OPENAL_DEBUG
        console.error("alSourceQueueBuffers called with an invalid buffer");
#endif
        AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }
    }

    for (var i = 0; i < count; ++i) {
      var bufferIdx = {{{ makeGetValue('buffers', 'i*4', 'i32') }}};
      var buffer = AL.currentContext.buf[bufferIdx - 1];
      src.queue.push({ buffer: buffer, src: null });
    }

    AL.updateSource(src);
  },

  alSourceUnqueueBuffers__deps: ["updateSource"],
  alSourceUnqueueBuffers: function(source, count, buffers) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourceUnqueueBuffers called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourceUnqueueBuffers called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }

    if (count > src.buffersPlayed) {
      AL.currentContext.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    for (var i = 0; i < count; i++) {
      var entry = src.queue.shift();
      // Write the buffers index out to the return list.
      for (var j = 0; j < AL.currentContext.buf.length; j++) {
        var b = AL.currentContext.buf[j];
        if (b && b == entry.buffer) {
          {{{ makeSetValue('buffers', 'i*4', 'j+1', 'i32') }}};
          break;
        }
      }
      src.buffersPlayed--;
    }

    AL.updateSource(src);
  },

  alDeleteBuffers: function(count, buffers)
  {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alDeleteBuffers called without a valid context");
#endif
      return;
    }
    if (count > AL.currentContext.buf.length) {
      AL.currentContext.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    for (var i = 0; i < count; ++i) {
      var bufferIdx = {{{ makeGetValue('buffers', 'i*4', 'i32') }}} - 1;

      // Make sure the buffer index is valid.
      if (bufferIdx >= AL.currentContext.buf.length || !AL.currentContext.buf[bufferIdx]) {
        AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
        return;
      }

      // Make sure the buffer is no longer in use.
      var buffer = AL.currentContext.buf[bufferIdx];
      for (var j = 0; j < AL.currentContext.src.length; ++j) {
        var src = AL.currentContext.src[j];
        if (!src) {
          continue;
        }
        for (var k = 0; k < src.queue.length; k++) {
          if (buffer === src.queue[k].buffer) {
            AL.currentContext.err = 0xA004 /* AL_INVALID_OPERATION */;
            return;
          }
        }
      }
    }

    for (var i = 0; i < count; ++i) {
      var bufferIdx = {{{ makeGetValue('buffers', 'i*4', 'i32') }}} - 1;
      delete AL.currentContext.buf[bufferIdx];
    }
  },

  alGenBuffers: function(count, buffers) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGenBuffers called without a valid context");
#endif
      return;
    }
    for (var i = 0; i < count; ++i) {
      AL.currentContext.buf.push(null);
      {{{ makeSetValue('buffers', 'i*4', 'AL.currentContext.buf.length', 'i32') }}};
    }
  },

  alBufferData: function(buffer, format, data, size, freq) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alBufferData called without a valid context");
#endif
      return;
    }
    if (buffer > AL.currentContext.buf.length) {
#if OPENAL_DEBUG
      console.error("alBufferData called with an invalid buffer");
#endif
      return;
    }
    var channels, bytes;
    switch (format) {
    case 0x1100 /* AL_FORMAT_MONO8 */:
      bytes = 1;
      channels = 1;
      break;
    case 0x1101 /* AL_FORMAT_MONO16 */:
      bytes = 2;
      channels = 1;
      break;
    case 0x1102 /* AL_FORMAT_STEREO8 */:
      bytes = 1;
      channels = 2;
      break;
    case 0x1103 /* AL_FORMAT_STEREO16 */:
      bytes = 2;
      channels = 2;
      break;
    default:
#if OPENAL_DEBUG
      console.error("alBufferData called with invalid format " + format);
#endif
      return;
    }
    try {
      AL.currentContext.buf[buffer - 1] = AL.currentContext.ctx.createBuffer(channels, size / (bytes * channels), freq);
    } catch (e) {
      AL.currentContext.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }
    var buf = new Array(channels);
    for (var i = 0; i < channels; ++i) {
      buf[i] = AL.currentContext.buf[buffer - 1].getChannelData(i);
    }
    for (var i = 0; i < size / (bytes * channels); ++i) {
      for (var j = 0; j < channels; ++j) {
        switch (bytes) {
        case 1:
          var val = {{{ makeGetValue('data', 'i*channels+j', 'i8') }}} & 0xff;  // unsigned
          buf[j][i] = -1.0 + val * (2/256);
          break;
        case 2:
          var val = {{{ makeGetValue('data', '2*(i*channels+j)', 'i16') }}};
          buf[j][i] = val/32768;
          break;
        }
      }
    }
  },

  alSourcePlay__deps: ['setSourceState'],
  alSourcePlay: function(source) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourcePlay called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcePlay called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1012 /* AL_PLAYING */);
  },

  alSourceStop__deps: ['setSourceState'],
  alSourceStop: function(source) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourceStop called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourceStop called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
  },

  alSourcePause__deps: ['setSourceState'],
  alSourcePause: function(source) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourcePause called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcePause called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1013 /* AL_PAUSED */);
  },

  alGetSourcei__deps: ['updateSource'],
  alGetSourcei: function(source, param, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetSourcei called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alGetSourcei called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }

    // Being that we have no way to receive end events from buffer nodes,
    // we currently proccess and update a source's buffer queue every
    // ~QUEUE_INTERVAL milliseconds. However, this interval is not precise,
    // so we also forcefully update the source when alGetSourcei is queried
    // to aid in the common scenario of application calling alGetSourcei(AL_BUFFERS_PROCESSED)
    // to recycle buffers.
    AL.updateSource(src);

    switch (param) {
    case 0x202 /* AL_SOURCE_RELATIVE */:
      {{{ makeSetValue('value', '0', 'src.panner ? 1 : 0', 'i32') }}};
      break;
    case 0x1009 /* AL_BUFFER */:
      if (!src.queue.length) {
        {{{ makeSetValue('value', '0', '0', 'i32') }}};
      } else {
        // Find the first unprocessed buffer.
        var buffer = src.queue[src.buffersPlayed].buffer;
        // Return its index.
        for (var i = 0; i < AL.currentContext.buf.length; ++i) {
          if (buffer == AL.currentContext.buf[i]) {
            {{{ makeSetValue('value', '0', 'i+1', 'i32') }}};
            return;
          }
        }
        {{{ makeSetValue('value', '0', '0', 'i32') }}};
      }
      break;
    case 0x1010 /* AL_SOURCE_STATE */:
      {{{ makeSetValue('value', '0', 'src.state', 'i32') }}};
      break;
    case 0x1015 /* AL_BUFFERS_QUEUED */:
      {{{ makeSetValue('value', '0', 'src.queue.length', 'i32') }}}
      break;
    case 0x1016 /* AL_BUFFERS_PROCESSED */:
      if (src.loop) {
        {{{ makeSetValue('value', '0', '0', 'i32') }}}
      } else {
        {{{ makeSetValue('value', '0', 'src.buffersPlayed', 'i32') }}}
      }
      break;
    default:
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alGetSourcef: function(source, param, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetSourcef called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source - 1];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alGetSourcef called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    // case 0x1003 /* AL_PITCH */:
    //   break;
    case 0x100A /* AL_GAIN */:
      {{{ makeSetValue('value', '0', 'src.gain.gain.value', 'float') }}}
      break;
    // case 0x100D /* AL_MIN_GAIN */:
    //   break;
    // case 0x100E /* AL_MAX_GAIN */:
    //   break;
    case 0x1023 /* AL_MAX_DISTANCE */:
      {{{ makeSetValue('value', '0', 'src.maxDistance', 'float') }}}
      break;
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
      {{{ makeSetValue('value', '0', 'src.rolloffFactor', 'float') }}}
      break;
    // case 0x1022 /* AL_CONE_OUTER_GAIN */:
    //   break;
    // case 0x1001 /* AL_CONE_INNER_ANGLE */:
    //   break;
    // case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    //   break;
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
      {{{ makeSetValue('value', '0', 'src.refDistance', 'float') }}}
      break;
    // case 0x1024 /* AL_SEC_OFFSET */:
    //   break;
    // case 0x1025 /* AL_SAMPLE_OFFSET */:
    //   break;
    // case 0x1026 /* AL_BYTE_OFFSET */:
    //   break;
    default:
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alDistanceModel: function(model) {
    if (model !== 0 /* AL_NONE */) {
#if OPENAL_DEBUG
      console.log("Only alDistanceModel(AL_NONE) is currently supported");
#endif
    }
  },

  alListenerfv: function(param, values) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alListenerfv called without a valid context");
#endif
      return;
    }
    switch (param) {
    case 0x1004 /* AL_POSITION */:
      AL.currentContext.ctx.listener.setPosition(
          {{{ makeGetValue('values', '0', 'float') }}},
          {{{ makeGetValue('values', '4', 'float') }}},
          {{{ makeGetValue('values', '8', 'float') }}}
        );
      break;
    case 0x1006 /* AL_VELOCITY */:
      AL.currentContext.ctx.listener.setVelocity(
          {{{ makeGetValue('values', '0', 'float') }}},
          {{{ makeGetValue('values', '4', 'float') }}},
          {{{ makeGetValue('values', '8', 'float') }}}
        );
      break;
    case 0x100F /* AL_ORIENTATION */:
      AL.currentContext.ctx.listener.setOrientation(
          {{{ makeGetValue('values', '0', 'float') }}},
          {{{ makeGetValue('values', '4', 'float') }}},
          {{{ makeGetValue('values', '8', 'float') }}},
          {{{ makeGetValue('values', '12', 'float') }}},
          {{{ makeGetValue('values', '16', 'float') }}},
          {{{ makeGetValue('values', '20', 'float') }}}
        );
      break;
    default:
#if OPENAL_DEBUG
      console.log("alListenerfv with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alIsExtensionPresent: function(extName) {
    return 0;
  },

  alcIsExtensionPresent: function(device, extName) {
    return 0;
  },

  alGetString: function(param) {
    return allocate(intArrayFromString('NA'), 'i8', ALLOC_NORMAL);
  },

  alGetProcAddress: function(fname) {
    return 0;
  },

  alcGetString: function(param) {
    return allocate(intArrayFromString('NA'), 'i8', ALLOC_NORMAL);
  },

  alcGetProcAddress: function(device, fname) {
    return 0;
  },

  alDopplerFactor: function(value) {
  },

  alDopplerVelocity: function(value) {
  }
};

autoAddDeps(LibraryOpenAL, '$AL');
mergeInto(LibraryManager.library, LibraryOpenAL);

