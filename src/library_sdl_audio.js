
  // SDL audio common code used in browser and proxy-worker contexts

  allocateChannels_core: function (SDL, num) { // called from Mix_AllocateChannels and init
    if (SDL.numChannels && SDL.numChannels >= num && num != 0) return;
    SDL.numChannels = num;
    SDL.channels = [];
    for (var i = 0; i < num; i++) {
      SDL.channels[i] = {
        audio: null,
        volume: 1.0
      };
    }
  },

  SDL_OpenAudio_core: function (SDL, Browser, copyMozillaAudioData, copyAudioData) {

    SDL.fillWebAudioBufferFromHeap = function(heapPtrOrBuffer, sizeSamplesPerChannel, dstAudioBuffer) {
      // The input audio data is interleaved across the channels, i.e. [L, R, L, R, L, R, ...] and is either 8-bit or 16-bit as
      // supported by the SDL API. The output audio wave data for Web Audio API must be in planar buffers of [-1,1]-normalized Float32 data,
      // so perform a buffer conversion for the data.
      var numChannels = SDL.audio.channels;
      for(var c = 0; c < numChannels; ++c) {
        var channelData = dstAudioBuffer['getChannelData'](c);
        if (channelData.length != sizeSamplesPerChannel) {
          throw 'Web Audio output buffer length mismatch! Destination size: ' + channelData.length + ' samples vs expected ' + sizeSamplesPerChannel + ' samples!';
        }
        copyAudioData(heapPtrOrBuffer, channelData, sizeSamplesPerChannel, SDL.audio.format, numChannels, c);
      }
    };

    var totalSamples = SDL.audio.samples*SDL.audio.channels;
    
    SDL.audio.audioOutput = new Audio();
    // As a workaround use Mozilla Audio Data API on Firefox until it ships with Web Audio and sound quality issues are fixed.
    if (typeof(SDL.audio.audioOutput['mozSetup'])==='function') {
      SDL.audio.audioOutput['mozSetup'](SDL.audio.channels, SDL.audio.freq); // use string attributes on mozOutput for closure compiler
      SDL.audio.mozBuffer = new Float32Array(totalSamples);
      SDL.audio.nextPlayTime = 0;
      // Heap pointer used in browser context, buffer used in proxy-worker context.
      SDL.audio.pushAudio = function SDL_audio_pushAudio(ptrOrBuffer, size) {
        --SDL.audio.numAudioTimersPending;
        var mozBuffer = SDL.audio.mozBuffer;
  
        copyMozillaAudioData(ptrOrBuffer, mozBuffer, SDL.audio.format, totalSamples);
  
        // Submit the audio data to audio device.
        SDL.audio.audioOutput['mozWriteAudio'](mozBuffer);
        
        // Compute when the next audio callback should be called.
        var curtime = Date.now() / 1000.0 - SDL.audio.startTime;
    	  if ({{{ ASSERTIONS }}}) {
    	    if (curtime > SDL.audio.nextPlayTime && SDL.audio.nextPlayTime != 0) {
    	      console.log('warning: Audio callback had starved sending audio by ' + (curtime - SDL.audio.nextPlayTime) + ' seconds.');
    	    }
    	  }
        var playtime = Math.max(curtime, SDL.audio.nextPlayTime);
        var buffer_duration = SDL.audio.samples / SDL.audio.freq;
        SDL.audio.nextPlayTime = playtime + buffer_duration;
        // Schedule the next audio callback call to occur when the current one finishes.
        SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, 1000.0 * (playtime-curtime));
        ++SDL.audio.numAudioTimersPending;
        // And also schedule extra buffers _now_ if we have too few in queue.
        if (SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
          ++SDL.audio.numAudioTimersPending;
          Browser.safeSetTimeout(SDL.audio.caller, 1.0);
        }
      }
    } else {
      // Initialize Web Audio API if we haven't done so yet. Note: Only initialize Web Audio context ever once on the web page,
      // since initializing multiple times fails on Chrome saying 'audio resources have been exhausted'.
      if (!SDL.audioContext) {
        if (typeof(AudioContext) !== 'undefined') {
          SDL.audioContext = new AudioContext();
        } else if (typeof(webkitAudioContext) !== 'undefined') {
          SDL.audioContext = new webkitAudioContext();
        } else {
          throw 'Web Audio API is not available!';
        }
      }
      SDL.audio.soundSource = new Array(); // Use an array of sound sources as a ring buffer to queue blocks of synthesized audio to Web Audio API.
      SDL.audio.nextSoundSource = 0; // Index of the next sound buffer in the ring buffer queue to play.
      SDL.audio.nextPlayTime = 0; // Time in seconds when the next audio block is due to start.
      
      // The pushAudio function with a new audio buffer whenever there is new audio data to schedule to be played back on the device.
      // Heap pointer used in browser context, buffer used in proxy-worker context.
      SDL.audio.pushAudio=function(ptrOrBuffer, sizeBytes) {
        try {
          --SDL.audio.numAudioTimersPending;
          if (SDL.audio.paused) return;
    
          var sizeSamples = sizeBytes / SDL.audio.bytesPerSample; // How many samples fit in the callback buffer?
          var sizeSamplesPerChannel = sizeSamples / SDL.audio.channels; // How many samples per a single channel fit in the cb buffer?
          if (sizeSamplesPerChannel != SDL.audio.samples) {
            throw 'Received mismatching audio buffer size!';
          }
          // Allocate new sound buffer to be played.
          var source = SDL.audioContext['createBufferSource']();
          if (SDL.audio.soundSource[SDL.audio.nextSoundSource]) {
            SDL.audio.soundSource[SDL.audio.nextSoundSource]['disconnect'](); // Explicitly disconnect old source, since we know it shouldn't be running anymore.
          }
          SDL.audio.soundSource[SDL.audio.nextSoundSource] = source;
          var soundBuffer = SDL.audioContext['createBuffer'](SDL.audio.channels,sizeSamplesPerChannel,SDL.audio.freq);
          SDL.audio.soundSource[SDL.audio.nextSoundSource]['connect'](SDL.audioContext['destination']);
    
          SDL.fillWebAudioBufferFromHeap(ptrOrBuffer, sizeSamplesPerChannel, soundBuffer);
          // Workaround https://bugzilla.mozilla.org/show_bug.cgi?id=883675 by setting the buffer only after filling. The order is important here!
          source['buffer'] = soundBuffer;
          
          // Schedule the generated sample buffer to be played out at the correct time right after the previously scheduled
          // sample buffer has finished.
          var curtime = SDL.audioContext['currentTime'];
          if ({{{ ASSERTIONS }}}) {
            if (curtime > SDL.audio.nextPlayTime && SDL.audio.nextPlayTime != 0) {
              console.log('warning: Audio callback had starved sending audio by ' + (curtime - SDL.audio.nextPlayTime) + ' seconds.');
            }
    		  }
          var playtime = Math.max(curtime, SDL.audio.nextPlayTime);
            var ss = SDL.audio.soundSource[SDL.audio.nextSoundSource];
            if (typeof ss['start'] !== 'undefined') {
              ss['start'](playtime);
            } else if (typeof ss['noteOn'] !== 'undefined') {
              ss['noteOn'](playtime);
            }
          var buffer_duration = sizeSamplesPerChannel / SDL.audio.freq;
          SDL.audio.nextPlayTime = playtime + buffer_duration;
          // Timer will be scheduled before the buffer completed playing.
          // Extra buffers are needed to avoid disturbing playing buffer.
          SDL.audio.nextSoundSource = (SDL.audio.nextSoundSource + 1) % (SDL.audio.numSimultaneouslyQueuedBuffers + 2);
          var secsUntilNextCall = playtime-curtime;
          
          // Queue the next audio frame push to be performed when the previously queued buffer has finished playing.
          var preemptBufferFeedMSecs = 1000*buffer_duration/2.0;
          SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, Math.max(0.0, 1000.0*secsUntilNextCall-preemptBufferFeedMSecs));
          ++SDL.audio.numAudioTimersPending;

    
          // If we are risking starving, immediately queue extra buffers.
          if (SDL.audio.numAudioTimersPending < SDL.audio.numSimultaneouslyQueuedBuffers) {
            ++SDL.audio.numAudioTimersPending;
            Browser.safeSetTimeout(SDL.audio.caller, 1.0);
          }
        } catch(e) {
          console.log('Web Audio API error playing back audio: ' + e.toString());
        }
      }
    }
  },

  SDL_PauseAudio_core: function (SDL, Browser, pauseOn, scriptProcessorNodeOnAudioProcessFn) {
    if (!SDL.audio) {
      return;
    }
    if (pauseOn) {
      if (SDL.audio.timer !== undefined) {
        clearTimeout(SDL.audio.timer);
        SDL.audio.numAudioTimersPending = 0;
        SDL.audio.timer = undefined;
      }
      if (SDL.audio.scriptProcessorNode !== undefined) {
        SDL.audio.scriptProcessorNode['disconnect']();
        SDL.audio.scriptProcessorNode = undefined;
      }
    } else if (!SDL.audio.timer && !SDL.audio.scriptProcessorNode) {
      // If we are using the same sampling frequency as the native sampling
      // rate of the Web Audio graph is using, we can feed our buffers via Web
      // Audio ScriptProcessorNode, which is a pull-mode API that calls back
      // to our code to get audio data.
      if (SDL.audioContext !== undefined && SDL.audio.freq == SDL.audioContext['sampleRate'] && typeof SDL.audioContext['createScriptProcessor'] !== 'undefined') {
        var sizeSamplesPerChannel = SDL.audio.bufferSize / SDL.audio.bytesPerSample / SDL.audio.channels; // How many samples per a single channel fit in the cb buffer?
        SDL.audio.scriptProcessorNode = SDL.audioContext['createScriptProcessor'](sizeSamplesPerChannel, 0, SDL.audio.channels);
        SDL.audio.scriptProcessorNode['onaudioprocess'] = scriptProcessorNodeOnAudioProcessFn(SDL, sizeSamplesPerChannel);
        SDL.audio.scriptProcessorNode['connect'](SDL.audioContext['destination']);
      } else { 
        // If we are using a different sampling rate, must manually queue
        // audio data to the graph via timers. Start the audio playback timer
        // callback loop.
        SDL.audio.numAudioTimersPending = 1;
        SDL.audio.timer = Browser.safeSetTimeout(SDL.audio.caller, 1);
        SDL.audio.startTime = Date.now() / 1000.0; // Only used for Mozilla Audio Data API. Not needed for Web Audio API.
      }
    }
    SDL.audio.paused = pauseOn;
  },

  SDL_CloseAudio_core: function (SDL, Browser, pauseAudioCore, allocateChannelsCore) {
    if (SDL.audio) {
      try{
        if (SDL.audio.soundSource) {
          for(var i = 0; i < SDL.audio.soundSource.length; ++i) {
            if (typeof(SDL.audio.soundSource[i]) !== 'undefined') {
              SDL.audio.soundSource[i].stop(0);
            }
          }
        }
      } catch(e) {
        console.log(e.toString());
      }
      SDL.audio.soundSource = null;
      pauseAudioCore(SDL, Browser, 1);
      allocateChannelsCore(SDL, 0);
    }
  },

