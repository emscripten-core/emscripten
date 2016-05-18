//"use strict";

var LibraryOpenAL = {
  $AL__deps: ['$Browser'],
  $AL: {
    contexts: [],
    currentContext: null,

    alcErr: 0,

    stringCache: {},
    alcStringCache: {},

    QUEUE_INTERVAL: 25,
    QUEUE_LOOKAHEAD: 100,

    newSrcId: 1,

#if OPENAL_DEBUG
    //This function is slow and used only for debugging purposes
    srcIdBySrc: function srcIdBySrc(src) {
      var idx = 0;
      for (var srcId in AL.currentContext.src) {
        if (AL.currentContext.src[srcId] == src) {
          idx = srcId;
          break;
        }
      }
      return idx;
    },
#endif

    updateSources: function updateSources(context) {
      // If we are animating using the requestAnimationFrame method, then the main loop does not run when in the background.
      // To give a perfect glitch-free audio stop when switching from foreground to background, we need to avoid updating
      // audio altogether when in the background, so detect that case and kill audio buffer streaming if so.
      if (Browser.mainLoop.timingMode == 1/*EM_TIMING_RAF*/ && document['visibilityState'] != 'visible') return;

      for (var srcId in context.src) {
        AL.updateSource(context.src[srcId]);
      }
    },

    updateSource: function updateSource(src) {
      // See comment on updateSources above.
      if (Browser.mainLoop.timingMode == 1/*EM_TIMING_RAF*/ && document['visibilityState'] != 'visible') return;

#if OPENAL_DEBUG
      var idx = AL.srcIdBySrc(src);
#endif
      if (src.state !== 0x1012 /* AL_PLAYING */) {
        return;
      }

      var currentTime = AL.currentContext.ctx.currentTime;
      var startTime = src.bufferPosition;

      for (var i = src.buffersPlayed; i < src.queue.length; i++) {
        var entry = src.queue[i];

        var startOffset = (startTime - currentTime) / src.playbackRate;
        var endTime;
        if (entry.src) endTime = startTime + entry.src.duration; // n.b. entry.src.duration already factors in playbackRate, so no divide by src.playbackRate on it.
        else endTime = startTime + entry.buffer.duration / src.playbackRate;

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
          entry.src.playbackRate.value = src.playbackRate;
          entry.src.duration = entry.buffer.duration / src.playbackRate;
          if (typeof(entry.src.start) !== 'undefined') {
            entry.src.start(startTime, offset);
          } else if (typeof(entry.src.noteOn) !== 'undefined') {
            entry.src.noteOn(startTime);
#if OPENAL_DEBUG
            if (offset > 0) {
              Runtime.warnOnce('The current browser does not support AudioBufferSourceNode.start(when, offset); method, so cannot play back audio with an offset '+offset+' secs! Audio glitches will occur!');
            }
#endif
          }
#if OPENAL_DEBUG
          else {
            Runtime.warnOnce('Unable to start AudioBufferSourceNode playback! Not supported by the browser?');
          }

          console.log('updateSource queuing buffer ' + i + ' for source ' + idx + ' at ' + startTime + ' (offset by ' + offset + ')');
#endif
        }

        startTime = endTime;
      }
    },

    setSourceState: function setSourceState(src, state) {
#if OPENAL_DEBUG
      var idx = AL.srcIdBySrc(src);
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
    if (typeof(AudioContext) !== "undefined" ||
        typeof(webkitAudioContext) !== "undefined") {
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
      // Old Web Audio API (e.g. Safari 6.0.5) had an inconsistently named createGainNode function.
      if (typeof(ctx.createGain) === 'undefined') ctx.createGain = ctx.createGainNode;

      var gain = ctx.createGain();
      gain.connect(ctx.destination);
      // Extend the Web Audio API AudioListener object with a few tracking values of our own.
      ctx.listener._position = [0, 0, 0];
      ctx.listener._velocity = [0, 0, 0];
      ctx.listener._orientation = [0, 0, 0, 0, 0, 0];
      var context = {
        ctx: ctx,
        err: 0,
        src: {},
        buf: [],
        interval: setInterval(function() { AL.updateSources(context); }, AL.QUEUE_INTERVAL),
        gain: gain
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

  alcGetError: function(device) {
    var err = AL.alcErr;
    AL.alcErr = 0;
    return err;
  },

  alcGetIntegerv: function(device, param, size, data) {
    if (size == 0 || !data) {
      AL.currentContext.err = 0xA003 /* AL_INVALID_VALUE */;
      return;
    }

    switch(param) {
    case 0x1000 /* ALC_MAJOR_VERSION */:
      {{{ makeSetValue('data', '0', '1', 'i32') }}};
      break;
    case 0x1001 /* ALC_MINOR_VERSION */:
      {{{ makeSetValue('data', '0', '1', 'i32') }}};
      break;
    case 0x1002 /* ALC_ATTRIBUTES_SIZE */:
      if (!device) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return 0;
      }
      {{{ makeSetValue('data', '0', '1', 'i32') }}};
      break;
    case 0x1003 /* ALC_ALL_ATTRIBUTES */:
      if (!device) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return 0;
      }
      {{{ makeSetValue('data', '0', '0', 'i32') }}};
      break;
    case 0x20003 /* ALC_MAX_AUXILIARY_SENDS */:
      if (!device) {
        AL.currentContext.err = 0xA001 /* ALC_INVALID_DEVICE */;
        return 0;
      }
      {{{ makeSetValue('data', '0', '1', 'i32') }}};
    default:
#if OPENAL_DEBUG
      console.log("alcGetIntegerv with param " + param + " not implemented yet");
#endif
      AL.alcErr = 0xA003 /* ALC_INVALID_ENUM */;
      break;
    }
  },

  alDeleteSources: function(count, sources) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alDeleteSources called without a valid context");
#endif
      return;
    }
    for (var i = 0; i < count; ++i) {
      var sourceIdx = {{{ makeGetValue('sources', 'i*4', 'i32') }}};
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
      gain.connect(AL.currentContext.gain);
      AL.currentContext.src[AL.newSrcId] = {
        state: 0x1011 /* AL_INITIAL */,
        queue: [],
        loop: false,
        playbackRate: 1,
        _position: [0, 0, 0],
        _velocity: [0, 0, 0],
        _direction: [0, 0, 0],
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
          return this._position;
        },
        set position(val) {
          this._position[0] = val[0];
          this._position[1] = val[1];
          this._position[2] = val[2];
          if (this.panner) this.panner.setPosition(val[0], val[1], val[2]);
        },
        get velocity() {
          return this._velocity;
        },
        set velocity(val) {
          this._velocity[0] = val[0];
          this._velocity[1] = val[1];
          this._velocity[2] = val[2];
          if (this.panner) this.panner.setVelocity(val[0], val[1], val[2]);
        },
        get direction() {
          return this._direction;
        },
        set direction(val) {
          this._direction[0] = val[0];
          this._direction[1] = val[1];
          this._direction[2] = val[2];
          if (this.panner) this.panner.setOrientation(val[0], val[1], val[2]);
        },
        get coneOuterGain() {
          return this._coneOuterGain || 0.0;
        },
        set coneOuterGain(val) {
          this._coneOuterGain = val;
          if (this.panner) this.panner.coneOuterGain = val;
        },
        get coneInnerAngle() {
          return this._coneInnerAngle || 360.0;
        },
        set coneInnerAngle(val) {
          this._coneInnerAngle = val;
          if (this.panner) this.panner.coneInnerAngle = val;
        },
        get coneOuterAngle() {
          return this._coneOuterAngle || 360.0;
        },
        set coneOuterAngle(val) {
          this._coneOuterAngle = val;
          if (this.panner) this.panner.coneOuterAngle = val;
        },
        gain: gain,
        panner: null,
        buffersPlayed: 0,
        bufferPosition: 0
      };
      {{{ makeSetValue('sources', 'i*4', 'AL.newSrcId', 'i32') }}};
      AL.newSrcId++;
    }
  },

  alIsSource: function(sourceId) {
    if (!AL.currentContext) {
      return false;
    }

    if (!AL.currentContext.src[sourceId]) {
      return false;
    } else {
      return true;
    }
  },

  alSourcei: function(source, param, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourcei called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcei called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
      src.coneInnerAngle = value;
      break;
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      src.coneOuterAngle = value;
      break;
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

          src.gain.connect(AL.currentContext.gain);
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
          panner.connect(AL.currentContext.gain);

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
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcef called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x1003 /* AL_PITCH */:
      if (value <= 0) {
        AL.currentContext.err = 0xA003 /* AL_INVALID_VALUE */;
        return;
      }
      src.playbackRate = value;

      if (src.state === 0x1012 /* AL_PLAYING */) {
        // update currently playing entry
        var entry = src.queue[src.buffersPlayed];
        var currentTime = AL.currentContext.ctx.currentTime;
        var oldrate = entry.src.playbackRate.value;
        var offset = currentTime - src.bufferPosition;
        // entry.src.duration is expressed after factoring in playbackRate, so when changing playback rate, need
        // to recompute/rescale the rate to the new playback speed.
        entry.src.duration = (entry.src.duration - offset) * oldrate / src.playbackRate;
        entry.src.playbackRate.value = src.playbackRate;
        src.bufferPosition = currentTime;

        // stop other buffers
        for (var k = src.buffersPlayed + 1; k < src.queue.length; k++) {
          var entry = src.queue[k];
          if (entry.src) {
            entry.src.stop();
            entry.src = null;
          }
        }
        // update the source to reschedule buffers with the new playbackRate
        AL.updateSource(src);
      }
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
    case 0x1022 /* AL_CONE_OUTER_GAIN */:
      src.coneOuterGain = value;
      break;
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
      src.coneInnerAngle = value;
      break;
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      src.coneOuterAngle = value;
      break;
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

  alSource3i: ['alSource3f'],
  alSource3i: function(source, param, v1, v2, v3) {
    _alSource3f(source, param, v1, v2, v3);
  },

  alSource3f: function(source, param, v1, v2, v3) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSource3f called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSource3f called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x1004 /* AL_POSITION */:
      src.position[0] = v1;
      src.position[1] = v2;
      src.position[2] = v3;
      break;
    case 0x1005 /* AL_DIRECTION */:
      src.direction[0] = v1;
      src.direction[1] = v2;
      src.direction[2] = v3;
      break;
    case 0x1006 /* AL_VELOCITY */:
      src.velocity[0] = v1;
      src.velocity[1] = v2;
      src.velocity[2] = v3;
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

  alSourceQueueBuffers: function(source, count, buffers) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourceQueueBuffers called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
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

  alSourceUnqueueBuffers: function(source, count, buffers) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourceUnqueueBuffers called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
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
      for (var srcId in AL.currentContext.src) {
        var src = AL.currentContext.src[srcId];
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

  alIsBuffer: function(bufferId) {
    if (!AL.currentContext) {
      return false;
    }
    if (bufferId > AL.currentContext.buf.length) {
      return false;
    }

    if (!AL.currentContext.buf[bufferId - 1]) {
      return false;
    } else {
      return true;
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
    case 0x10010 /* AL_FORMAT_MONO_FLOAT32 */:
      bytes = 4;
      channels = 1;
      break;
    case 0x10011 /* AL_FORMAT_STEREO_FLOAT32 */:
      bytes = 4;
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
      AL.currentContext.buf[buffer - 1].bytesPerSample =  bytes;
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
        case 4:
          buf[j][i] = {{{ makeGetValue('data', '4*(i*channels+j)', 'float') }}};
          break;
        }
      }
    }
  },

  alGetBufferi: function(buffer, param, value)
  {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetBufferi called without a valid context");
#endif
      return;
    }
    var buf = AL.currentContext.buf[buffer - 1];
    if (!buf) {
#if OPENAL_DEBUG
      console.error("alGetBufferi called with an invalid buffer");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x2001 /* AL_FREQUENCY */:
      {{{ makeSetValue('value', '0', 'buf.sampleRate', 'i32') }}};
      break;
    case 0x2002 /* AL_BITS */:
      {{{ makeSetValue('value', '0', 'buf.bytesPerSample * 8', 'i32') }}};
      break;
    case 0x2003 /* AL_CHANNELS */:
      {{{ makeSetValue('value', '0', 'buf.numberOfChannels', 'i32') }}};
      break;
    case 0x2004 /* AL_SIZE */:
      {{{ makeSetValue('value', '0', 'buf.length * buf.bytesPerSample * buf.numberOfChannels', 'i32') }}};
      break;
    default:
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alSourcePlay: function(source) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourcePlay called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcePlay called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1012 /* AL_PLAYING */);
  },

  alSourceStop: function(source) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourceStop called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourceStop called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
  },

  alSourceRewind: function(source) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourceRewind called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourceRewind called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    // Stop the source first to clear the source queue
    AL.setSourceState(src, 0x1014 /* AL_STOPPED */);
    // Now set the state of AL_INITIAL according to the specification
    AL.setSourceState(src, 0x1011 /* AL_INITIAL */);
  },

  alSourcePause: function(source) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alSourcePause called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alSourcePause called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    AL.setSourceState(src, 0x1013 /* AL_PAUSED */);
  },

  alGetSourcei: function(source, param, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetSourcei called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
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
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
      {{{ makeSetValue('value', '0', 'src.coneInnerAngle', 'i32') }}};
      break;
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      {{{ makeSetValue('value', '0', 'src.coneOuterAngle', 'i32') }}};
      break;
    case 0x1007 /* AL_LOOPING */:
      {{{ makeSetValue('value', '0', 'src.loop', 'i32') }}};
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

  alGetSourceiv__deps: ['alGetSourcei'],
  alGetSourceiv: function(source, param, values) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetSourceiv called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alGetSourceiv called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x202 /* AL_SOURCE_RELATIVE */:
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1007 /* AL_LOOPING */:
    case 0x1009 /* AL_BUFFER */:
    case 0x1010 /* AL_SOURCE_STATE */:
    case 0x1015 /* AL_BUFFERS_QUEUED */:
    case 0x1016 /* AL_BUFFERS_PROCESSED */:
      _alGetSourcei(source, param, values);
      break;
    default:
#if OPENAL_DEBUG
      console.error("alGetSourceiv with param " + param + " not implemented yet");
#endif
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
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alGetSourcef called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
     case 0x1003 /* AL_PITCH */:
      {{{ makeSetValue('value', '0', 'src.playbackRate', 'float') }}}
      break;
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
    case 0x1022 /* AL_CONE_OUTER_GAIN */:
      {{{ makeSetValue('value', '0', 'src.coneOuterGain', 'float') }}}
      break;
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
      {{{ makeSetValue('value', '0', 'src.coneInnerAngle', 'float') }}}
      break;
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
      {{{ makeSetValue('value', '0', 'src.coneOuterAngle', 'float') }}}
      break;
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

  alGetSourcefv__deps: ['alGetSourcef'],
  alGetSourcefv: function(source, param, values) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetSourcefv called without a valid context");
#endif
      return;
    }
    var src = AL.currentContext.src[source];
    if (!src) {
#if OPENAL_DEBUG
      console.error("alGetSourcefv called with an invalid source");
#endif
      AL.currentContext.err = 0xA001 /* AL_INVALID_NAME */;
      return;
    }
    switch (param) {
    case 0x1003 /* AL_PITCH */:
    case 0x100A /* AL_GAIN */:
    case 0x100D /* AL_MIN_GAIN */:
    case 0x100E /* AL_MAX_GAIN */:
    case 0x1023 /* AL_MAX_DISTANCE */:
    case 0x1021 /* AL_ROLLOFF_FACTOR */:
    case 0x1022 /* AL_CONE_OUTER_GAIN */:
    case 0x1001 /* AL_CONE_INNER_ANGLE */:
    case 0x1002 /* AL_CONE_OUTER_ANGLE */:
    case 0x1020 /* AL_REFERENCE_DISTANCE */:
    case 0x1024 /* AL_SEC_OFFSET */:
    case 0x1025 /* AL_SAMPLE_OFFSET */:
    case 0x1026 /* AL_BYTE_OFFSET */:
      _alGetSourcef(source, param, values);
      break;
    case 0x1004 /* AL_POSITION */:
      var position = src.position;
      {{{ makeSetValue('values', '0', 'position[0]', 'float') }}}
      {{{ makeSetValue('values', '4', 'position[1]', 'float') }}}
      {{{ makeSetValue('values', '8', 'position[2]', 'float') }}}
      break;
    case 0x1005 /* AL_DIRECTION */:
      var direction = src.direction;
      {{{ makeSetValue('values', '0', 'direction[0]', 'float') }}}
      {{{ makeSetValue('values', '4', 'direction[1]', 'float') }}}
      {{{ makeSetValue('values', '8', 'direction[2]', 'float') }}}
      break;
    case 0x1006 /* AL_VELOCITY */:
      var velocity = src.velocity;
      {{{ makeSetValue('values', '0', 'velocity[0]', 'float') }}}
      {{{ makeSetValue('values', '4', 'velocity[1]', 'float') }}}
      {{{ makeSetValue('values', '8', 'velocity[2]', 'float') }}}
      break;
    default:
#if OPENAL_DEBUG
      console.error("alGetSourcefv with param " + param + " not implemented yet");
#endif
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

  alGetListenerf: function(pname, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetListenerf called without a valid context");
#endif
      return;
    }
    switch (pname) {
    case 0x100A /* AL_GAIN */:
      {{{ makeSetValue('value', '0', 'AL.currentContext.gain.gain.value', 'float') }}}
      break;
    default:
#if OPENAL_DEBUG
      console.error("alGetListenerf with param " + pname + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }

  },

  alGetListenerfv: function(pname, values) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetListenerfv called without a valid context");
#endif
      return;
    }
    switch (pname) {
    case 0x1004 /* AL_POSITION */:
      var position = AL.currentContext.ctx.listener._position;
      {{{ makeSetValue('values', '0', 'position[0]', 'float') }}}
      {{{ makeSetValue('values', '4', 'position[1]', 'float') }}}
      {{{ makeSetValue('values', '8', 'position[2]', 'float') }}}
      break;
    case 0x1006 /* AL_VELOCITY */:
      var velocity = AL.currentContext.ctx.listener._velocity;
      {{{ makeSetValue('values', '0', 'velocity[0]', 'float') }}}
      {{{ makeSetValue('values', '4', 'velocity[1]', 'float') }}}
      {{{ makeSetValue('values', '8', 'velocity[2]', 'float') }}}
      break;
    case 0x100F /* AL_ORIENTATION */:
      var orientation = AL.currentContext.ctx.listener._orientation;
      {{{ makeSetValue('values', '0', 'orientation[0]', 'float') }}}
      {{{ makeSetValue('values', '4', 'orientation[1]', 'float') }}}
      {{{ makeSetValue('values', '8', 'orientation[2]', 'float') }}}
      {{{ makeSetValue('values', '12', 'orientation[3]', 'float') }}}
      {{{ makeSetValue('values', '16', 'orientation[4]', 'float') }}}
      {{{ makeSetValue('values', '20', 'orientation[5]', 'float') }}}
      break;
    default:
#if OPENAL_DEBUG
      console.error("alGetListenerfv with param " + pname + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alGetListeneri: function(pname, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alGetListeneri called without a valid context");
#endif
      return;
    }
    switch (pname) {
    default:
#if OPENAL_DEBUG
      console.error("alGetListeneri with param " + pname + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alListenerf: function(param, value) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alListenerf called without a valid context");
#endif
      return;
    }
    switch (param) {
    case 0x100A /* AL_GAIN */:
      AL.currentContext.gain.gain.value = value;
      break;
    default:
#if OPENAL_DEBUG
      console.error("alListenerf with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alEnable: function(param) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alEnable called without a valid context");
#endif
      return;
    }
    switch (param) {
    default:
#if OPENAL_DEBUG
      console.error("alEnable with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alDisable: function(param) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alDisable called without a valid context");
#endif
      return;
    }
    switch (pname) {
    default:
#if OPENAL_DEBUG
      console.error("alDisable with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alListener3f: function(param, v1, v2, v3) {
    if (!AL.currentContext) {
#if OPENAL_DEBUG
      console.error("alListenerfv called without a valid context");
#endif
      return;
    }
    switch (param) {
    case 0x1004 /* AL_POSITION */:
      AL.currentContext.ctx.listener._position[0] = v1;
      AL.currentContext.ctx.listener._position[1] = v2;
      AL.currentContext.ctx.listener._position[2] = v3;
      AL.currentContext.ctx.listener.setPosition(v1, v2, v3);
      break;
    case 0x1006 /* AL_VELOCITY */:
      AL.currentContext.ctx.listener._velocity[0] = v1;
      AL.currentContext.ctx.listener._velocity[1] = v2;
      AL.currentContext.ctx.listener._velocity[2] = v3;
      AL.currentContext.ctx.listener.setVelocity(v1, v2, v3);
      break;
    default:
#if OPENAL_DEBUG
      console.error("alListener3f with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
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
      var x = {{{ makeGetValue('values', '0', 'float') }}};
      var y = {{{ makeGetValue('values', '4', 'float') }}};
      var z = {{{ makeGetValue('values', '8', 'float') }}};
      AL.currentContext.ctx.listener._position[0] = x;
      AL.currentContext.ctx.listener._position[1] = y;
      AL.currentContext.ctx.listener._position[2] = z;
      AL.currentContext.ctx.listener.setPosition(x, y, z);
      break;
    case 0x1006 /* AL_VELOCITY */:
      var x = {{{ makeGetValue('values', '0', 'float') }}};
      var y = {{{ makeGetValue('values', '4', 'float') }}};
      var z = {{{ makeGetValue('values', '8', 'float') }}};
      AL.currentContext.ctx.listener._velocity[0] = x;
      AL.currentContext.ctx.listener._velocity[1] = y;
      AL.currentContext.ctx.listener._velocity[2] = z;
      AL.currentContext.ctx.listener.setVelocity(x, y, z);
      break;
    case 0x100F /* AL_ORIENTATION */:
      var x = {{{ makeGetValue('values', '0', 'float') }}};
      var y = {{{ makeGetValue('values', '4', 'float') }}};
      var z = {{{ makeGetValue('values', '8', 'float') }}};
      var x2 = {{{ makeGetValue('values', '12', 'float') }}};
      var y2 = {{{ makeGetValue('values', '16', 'float') }}};
      var z2 = {{{ makeGetValue('values', '20', 'float') }}};
      AL.currentContext.ctx.listener._orientation[0] = x;
      AL.currentContext.ctx.listener._orientation[1] = y;
      AL.currentContext.ctx.listener._orientation[2] = z;
      AL.currentContext.ctx.listener._orientation[3] = x2;
      AL.currentContext.ctx.listener._orientation[4] = y2;
      AL.currentContext.ctx.listener._orientation[5] = z2;
      AL.currentContext.ctx.listener.setOrientation(x, y, z, x2, y2, z2);
      break;
    default:
#if OPENAL_DEBUG
      console.error("alListenerfv with param " + param + " not implemented yet");
#endif
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      break;
    }
  },

  alIsExtensionPresent: function(extName) {
    extName = Pointer_stringify(extName);

    if (extName == "AL_EXT_float32") return 1;

    return 0;
  },

  alcIsExtensionPresent: function(device, extName) {
    return 0;
  },

  alGetString: function(param) {
    if (AL.stringCache[param]) return AL.stringCache[param];
    var ret;
    switch (param) {
    case 0 /* AL_NO_ERROR */:
      ret = 'No Error';
      break;
    case 0xA001 /* AL_INVALID_NAME */:
      ret = 'Invalid Name';
      break;
    case 0xA002 /* AL_INVALID_ENUM */:
      ret = 'Invalid Enum';
      break;
    case 0xA003 /* AL_INVALID_VALUE */:
      ret = 'Invalid Value';
      break;
    case 0xA004 /* AL_INVALID_OPERATION */:
      ret = 'Invalid Operation';
      break;
    case 0xA005 /* AL_OUT_OF_MEMORY */:
      ret = 'Out of Memory';
      break;
    case 0xB001 /* AL_VENDOR */:
      ret = 'Emscripten';
      break;
    case 0xB002 /* AL_VERSION */:
      ret = '1.1';
      break;
    case 0xB003 /* AL_RENDERER */:
      ret = 'WebAudio';
      break;
    case 0xB004 /* AL_EXTENSIONS */:
      ret = 'AL_EXT_float32';
      break;
    default:
      AL.currentContext.err = 0xA002 /* AL_INVALID_ENUM */;
      return 0;
    }

    ret = allocate(intArrayFromString(ret), 'i8', ALLOC_NORMAL);

    AL.stringCache[param] = ret;

    return ret;
  },

  alGetProcAddress: function(fname) {
    return 0;
  },

  alcGetString: function(device, param) {
    if (AL.alcStringCache[param]) return AL.alcStringCache[param];
    var ret;
    switch (param) {
    case 0 /* ALC_NO_ERROR */:
      ret = 'No Error';
      break;
    case 0xA001 /* ALC_INVALID_DEVICE */:
      ret = 'Invalid Device';
      break;
    case 0xA002 /* ALC_INVALID_CONTEXT */:
      ret = 'Invalid Context';
      break;
    case 0xA003 /* ALC_INVALID_ENUM */:
      ret = 'Invalid Enum';
      break;
    case 0xA004 /* ALC_INVALID_VALUE */:
      ret = 'Invalid Value';
      break;
    case 0xA005 /* ALC_OUT_OF_MEMORY */:
      ret = 'Out of Memory';
      break;
    case 0x1004 /* ALC_DEFAULT_DEVICE_SPECIFIER */:
      if (typeof(AudioContext) !== "undefined" ||
          typeof(webkitAudioContext) !== "undefined") {
        ret = 'Device';
      } else {
        return 0;
      }
      break;
    case 0x1005 /* ALC_DEVICE_SPECIFIER */:
      if (typeof(AudioContext) !== "undefined" ||
          typeof(webkitAudioContext) !== "undefined") {
        ret = 'Device\0';
      } else {
        ret = '\0';
      }
      break;
    case 0x311 /* ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER */:
      return 0;
      break;
    case 0x310 /* ALC_CAPTURE_DEVICE_SPECIFIER */:
      ret = '\0'
      break;
    case 0x1006 /* ALC_EXTENSIONS */:
      if (!device) {
        AL.alcErr = 0xA001 /* ALC_INVALID_DEVICE */;
        return 0;
      }
      ret = '';
      break;
    default:
      AL.alcErr = 0xA003 /* ALC_INVALID_ENUM */;
      return 0;
    }

    ret = allocate(intArrayFromString(ret), 'i8', ALLOC_NORMAL);

    AL.alcStringCache[param] = ret;

    return ret;
  },

  alcGetProcAddress: function(device, fname) {
    return 0;
  },

  alGetEnumValue: function(name) {
    name = Pointer_stringify(name);

    if (name == "AL_FORMAT_MONO_FLOAT32") return 0x10010;
    if (name == "AL_FORMAT_STEREO_FLOAT32") return 0x10011;

    AL.currentContext.err = 0xA003 /* AL_INVALID_VALUE */;
    return 0;
  },

  alSpeedOfSound: function(value) {
    Runtime.warnOnce('alSpeedOfSound() is not yet implemented! Ignoring all calls to it.');
  },

  alDopplerFactor: function(value) {
    Runtime.warnOnce('alDopplerFactor() is not yet implemented! Ignoring all calls to it.');
  },

  alDopplerVelocity: function(value) {
    Runtime.warnOnce('alDopplerVelocity() is not yet implemented! Ignoring all calls to it.');
  }
};

autoAddDeps(LibraryOpenAL, '$AL');
mergeInto(LibraryManager.library, LibraryOpenAL);

