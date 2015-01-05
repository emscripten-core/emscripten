var LibraryAudio = {
  $HTML5Audio: {
    audioContext: {},
    audioInstances: [null],
    enabled: false,
  },

  emscripten_audio_init: function() {
    try {
      if (!window.AudioContext) window.AudioContext = window.webkitAudioContext;
      HTML5Audio.audioContext = new AudioContext();
      HTML5Audio.enabled = true;
    }
    catch(e) {
      Module.printErr('Web Audio API is not supported in this browser');
      return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    }
    return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
  },

  emscripten_audio_load_pcm: function(channels, length, sampleRate, ptr) {
    if (!HTML5Audio.enabled) return 0;
    var sound = {
      buffer: HTML5Audio.audioContext.createBuffer(channels, length, sampleRate),
      error: false
    };
    for (var i = 0; i < channels; i++) {
      var offs = (ptr>>2) + length * i;
      sound.buffer.copyToChannel(HEAPF32.subarray(offs, offs + length), i, 0);
    }
    var instance = HTML5Audio.audioInstances.push(sound) - 1;
    return instance;
  },

  emscripten_audio_load: function(ptr, length) {
    if (!HTML5Audio.enabled) return 0;
    var sound = {
      buffer: null,
      error: false
    };
    var instance = HTML5Audio.audioInstances.push(sound) - 1;
    HTML5Audio.audioContext.decodeAudioData(
      HEAPU8.buffer.slice(ptr, ptr+length),
      function(buffer) {
        sound.buffer = buffer;
      },
      function() {
        sound.error = true;
        Module.printErr('Audio decode error.');
      }
    );
    return instance;
  },

  emscripten_audio_free: function(instance) {
    HTML5Audio.audioInstances[instance] = null;
  },

  emscripten_audio_create_channel: function(callback, userData) {
    if (!HTML5Audio.enabled) return;

    Module['noExitRuntime'] = true;

    var channel = {
      gain: HTML5Audio.audioContext.createGain(),
      panner: HTML5Audio.audioContext.createPanner(),
      threeD: false,
      playBuffer: function(buffer) {
        this.source.buffer = buffer;
        var chan = this;
        this.source.onended = function() {
          if (callback) Runtime.dynCall('vi', callback, [userData]);
          // recreate channel for future use.
          chan.setup();
        };
        this.source.start(0);
      },
      setup: function() {
        this.source = HTML5Audio.audioContext.createBufferSource();
        this.setupPanning();
      },
      setupPanning: function() {
        if (this.threeD) {
          this.source.disconnect();
          this.source.connect(this.panner);
          this.panner.connect(this.gain);
        } else {
          this.panner.disconnect();
          this.source.connect(this.gain);
        }
      }
    };
    //channel.panner.rolloffFactor = 0; // We calculate rolloff ourselves.
    channel.gain.connect(HTML5Audio.audioContext.destination);
    channel.setup();
    return HTML5Audio.audioInstances.push(channel) - 1;
  },

  emscripten_audio_play__deps: ['emscripten_audio_stop'],
  emscripten_audio_play: function(bufferInstance, channelInstance) {
    if (!HTML5Audio.enabled) return;

    // stop sound which is playing in the channel currently.
    _emscripten_audio_stop(channelInstance);

    var sound = HTML5Audio.audioInstances[bufferInstance];
    var channel = HTML5Audio.audioInstances[channelInstance];
    if (sound.buffer) {
      channel.playBuffer(sound.buffer);
    } else {
      Module.printErr('Trying to play sound which is not loaded.');
    }
  },

  emscripten_audio_set_paused: function(channelInstance, paused) {
    if (!HTML5Audio.enabled) return;
    // This is a bad hack to fake pausing by setting the pitch to 0.0000001 (as zero is not allowed).
    // The Web Audio API has no built-in support for pausing (which seems to be one hell of an oversight.),
    // and other commonly suggested workarounds based on remembering start and pause times of sounds, and seeking
    // have other issues with looping and pitch, so I chose this simpler hack instead.
    HTML5Audio.audioInstances[channelInstance].source.playbackRate.value = paused ? 0.0000001 : 1;
  },

  emscripten_audio_set_loop: function(channelInstance, loop) {
    if (!HTML5Audio.enabled) return;
    HTML5Audio.audioInstances[channelInstance].source.loop = loop;
  },

  emscripten_audio_set_3d: function(channelInstance, threeD) {
    var channel = HTML5Audio.audioInstances[channelInstance];
    if (channel.threeD !== threeD) {
      channel.threeD = threeD;
      channel.setupPanning();
    }
  },

  emscripten_audio_stop: function(channelInstance) {
    if (!HTML5Audio.enabled) return;

    var channel = HTML5Audio.audioInstances[channelInstance];

    // stop sound currently playing.
    if (channel.source.buffer) {
      channel.source.stop(0);

      // disable callback for this channel when manually stopped.
      channel.source.onended = function(){};

      // recreate channel for future use.
      channel.setup();
    }
  },

  emscripten_audio_set_position: function(channelInstance, x, y, z) {
    if (!HTML5Audio.enabled) return;
    HTML5Audio.audioInstances[channelInstance].panner.setPosition(x, y, z);
  },

  emscripten_audio_set_velocity: function(channelInstance, x, y, z) {
    if (!HTML5Audio.enabled) return;
    HTML5Audio.audioInstances[channelInstance].panner.setVelocity(x, y, z);
  },

  emscripten_audio_set_volume: function(channelInstance, v) {
    if (!HTML5Audio.enabled) return;
    HTML5Audio.audioInstances[channelInstance].gain.gain.value = v;
  },

  emscripten_audio_set_listener_position: function(x, y, z) {
    if (!HTML5Audio.enabled) return;

    HTML5Audio.audioContext.listener.setPosition(x, y, z);
  },

  emscripten_audio_set_listener_velocity: function(x, y, z) {
    if (!HTML5Audio.enabled) return;

    HTML5Audio.audioContext.listener.setVelocity(x, y, z);
  },

  emscripten_audio_set_listener_orientation: function(x, y, z, xUp, yUp, zUp) {
    if (!HTML5Audio.enabled) return;

    HTML5Audio.audioContext.listener.setPosition(x, y, z, xUp, yUp, zUp);
  },

  emscripten_audio_get_load_state: function(bufferInstance) {
    if (HTML5Audio.enabled == 0) return {{{ cDefine('EMSCRIPTEN_RESULT_NOT_SUPPORTED') }}};
    var sound = HTML5Audio.audioInstances[bufferInstance];
    if (sound.error) return {{{ cDefine('EMSCRIPTEN_RESULT_FAILED') }}};
    if (sound.buffer) return {{{ cDefine('EMSCRIPTEN_RESULT_SUCCESS') }}};
    return {{{ cDefine('EMSCRIPTEN_RESULT_DEFERRED') }}};
  }
};

autoAddDeps(LibraryAudio, '$HTML5Audio');
mergeInto(LibraryManager.library, LibraryAudio);

