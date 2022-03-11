#if AUDIO_WORKLET && !WASM_WORKERS
#error "Building with -sAUDIO_WORKLET also requires enabling -sWASM_WORKERS!"
#endif
#if AUDIO_WORKLET && TEXTDECODER == 2
#error "-sAUDIO_WORKLET does not support -sTEXTDECODER=2 since TextDecoder is not available in AudioWorkletGlobalScope! Use e.g. -sTEXTDECODER=1 when building with -sAUDIO_WORKLET"
#endif
#if AUDIO_WORKLET && SINGLE_FILE
#error "-sAUDIO_WORKLET does not support -sSINGLE_FILE"
#endif

let LibraryWebAudio = {
  $EmAudio: {},
  $EmAudioCounter: 0,

  // Call this function from JavaScript to register a Wasm-side handle to an AudioContext that
  // you have already created manually without calling emscripten_create_audio_context().
  // Note: To let that AudioContext be garbage collected later, call the function
  // emscriptenDestroyAudioContext() to unbind it from Wasm.
  $emscriptenRegisterAudioObject__deps: ['$EmAudio', '$EmAudioCounter'],
  $emscriptenRegisterAudioObject: function(object) {
#if ASSERTIONS
    assert(object, 'Called emscriptenRegisterAudioObject() with a null object handle!');
#endif
    EmAudio[++EmAudioCounter] = object;
#if WEBAUDIO_DEBUG
    console.log(`Registered new WebAudio object ${object} with ID ${EmAudioCounter}`);
#endif
    return EmAudioCounter;
  },

  // Call this function from JavaScript to destroy a Wasm-side handle to an AudioContext.
  // After calling this function, it is no longer possible to reference this AudioContext
  // from Wasm code - and the GC can reclaim it after all references to it are cleared.
  $emscriptenDestroyAudioContext: 'emscripten_destroy_audio_context',

  // Call this function from JavaScript to get the Web Audio object corresponding to the given
  // Wasm handle ID.
  $emscriptenGetAudioObject: function(objectHandle) {
    return EmAudio[objectHandle];
  },

  emscripten_destroy_audio_context__sig: 'vi',
  emscripten_destroy_audio_context: function(contextHandle) {
#if ASSERTIONS
    assert(EmAudio[contextHandle], `Called emscripten_destroy_audio_context() on an already freed context handle ${contextHandle}`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_destroy_audio_context() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif
#if WEBAUDIO_DEBUG
    console.log(`Destroyed WebAudio context with ID ${contextHandle}`);
#endif
    delete EmAudio[contextHandle];
  },

  emscripten_destroy_web_audio_node: function(objectHandle) {
#if ASSERTIONS
    assert(EmAudio[objectHandle], `Called emscripten_destroy_web_audio_node() on a nonexisting/already freed object handle ${objectHandle}`);
    assert(EmAudio[objectHandle].disconnect, `Called emscripten_destroy_web_audio_node() on a handle ${objectHandle} that is not an Web Audio Node, but of type ${typeof EmAudio[objectHandle]}`);
#endif
#if WEBAUDIO_DEBUG
    console.log(`Destroyed Web Audio Node with ID ${objectHandle}`);
#endif
    // Explicitly disconnect the node from Web Audio graph before letting it GC,
    // to work around browser bugs such as https://bugs.webkit.org/show_bug.cgi?id=222098#c23
    EmAudio[objectHandle].disconnect();
    delete EmAudio[objectHandle];
  },

  emscripten_create_audio_context__deps: ['$emscriptenRegisterAudioObject'],
  emscripten_create_audio_context: function(options) {
    let ctx = window.AudioContext || window.webkitAudioContext;
#if ASSERTIONS
    if (!ctx) console.error('emscripten_create_audio_context failed! Web Audio is not supported.');
#endif
    options >>= 2;

    let opts = options ? {
      latencyHint: HEAPU32[options] ? UTF8ToString(HEAPU32[options]) : void 0,
      sampleRate: HEAP32[options+1] || void 0
    } : void 0;

#if WEBAUDIO_DEBUG
    console.log(`Creating new WebAudio context with parameters:`);
    console.dir(opts);
#endif

  	return ctx && emscriptenRegisterAudioObject(new ctx(opts));
  },

  emscripten_resume_audio_context_async: function(contextHandle, callback, userData) {
    function cb(state) {
#if WEBAUDIO_DEBUG
      console.log(`emscripten_resume_audio_context_async() callback: New audio state="${EmAudio[contextHandle].state}", ID=${state}`);
#endif
      {{{ makeDynCall('viii', 'callback') }}}(contextHandle, state, userData);
    }
#if WEBAUDIO_DEBUG
    console.log(`emscripten_resume_audio_context_async() resuming...`);
#endif
    EmAudio[contextHandle].resume().then(() => { cb(1/*running*/) }).catch(() => { cb(0/*suspended*/) });
  },

#if AUDIO_WORKLET
  emscripten_start_wasm_audio_worklet_thread_async__deps: [
#if WASM_WORKERS
    'wasm_workers_id',
#endif
    '$_EmAudioDispatchProcessorCallback'],
  emscripten_start_wasm_audio_worklet_thread_async: function(contextHandle, stackLowestAddress, stackSize, callback, userData) {
#if !AUDIO_WORKLET
    abort('emscripten_create_wasm_audio_worklet() requires building with -s AUDIO_WORKLET=1 enabled!');
#endif

#if ASSERTIONS
    assert(contextHandle, `Called emscripten_start_wasm_audio_worklet_thread_async() with a null Web Audio Context handle!`);
    assert(EmAudio[contextHandle], `Called emscripten_start_wasm_audio_worklet_thread_async() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_start_wasm_audio_worklet_thread_async() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif

    let audioContext = EmAudio[contextHandle],
      audioWorklet = audioContext.audioWorklet;

#if ASSERTIONS
    assert(stackLowestAddress != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
    assert(stackSize != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
    assert(!audioContext.audioWorkletInitialized, 'emscripten_create_wasm_audio_worklet() was already called for AudioContext ' + contextHandle + '! Only call this function once per AudioContext!');
    audioContext.audioWorkletInitialized = 1;
#endif

#if WEBAUDIO_DEBUG
    console.log(`emscripten_start_wasm_audio_worklet_thread_async() adding audioworklet.js...`);
#endif

    let audioWorkletCreationFailed = () => {
#if WEBAUDIO_DEBUG
      console.error(`emscripten_start_wasm_audio_worklet_thread_async() addModule() failed!`);
#endif
      {{{ makeDynCall('vii', 'callback') }}}(contextHandle, 0/*EM_FALSE*/, userData);
    };

    // Does browser not support AudioWorklets?
    if (!audioWorklet) {
#if WEBAUDIO_DEBUG
      if (location.protocol == 'http:') {
        console.error(`AudioWorklets are not supported. This is possibly due to running the page over unsecure http:// protocol. Try running over https://, or debug via a localhost-based server, which should also allow AudioWorklets to function.`);
      } else {
        console.error(`AudioWorklets are not supported by current browser.`);
      }
#endif
      return audioWorkletCreationFailed();
    }

    // TODO: In MINIMAL_RUNTIME builds, read this file off of a preloaded Blob, and/or embed from a string like with WASM_WORKERS==2 mode.
    audioWorklet.addModule('{{{ TARGET_BASENAME }}}.aw.js').then(() => {
#if WEBAUDIO_DEBUG
      console.log(`emscripten_start_wasm_audio_worklet_thread_async() addModule('audioworklet.js') completed`);
#endif
      audioWorklet.bootstrapMessage = new AudioWorkletNode(audioContext, 'message', {
        processorOptions: {
#if WASM_WORKERS
          '$ww': _wasm_workers_id++, // Assign the loaded AudioWorkletGlobalScope a Wasm Worker ID so that it can utilized its own TLS slots, and it is recognized to not be the main browser thread.
#endif
          'wasm': Module['wasm'],
          'mem': wasmMemory,
          'sb': stackLowestAddress,
          'sz': stackSize,
        }
      });
      audioWorklet.bootstrapMessage.port.onmessage = _EmAudioDispatchProcessorCallback;

      // AudioWorklets do not have a importScripts() function like Web Workers do (and AudioWorkletGlobalScope does not allow dynamic import() either),
      // but instead, the main thread must load all JS code into the worklet scope. Send the application main JS script to the audio worklet.
      return audioWorklet.addModule(Module['js']);
    }).then(() => {
#if WEBAUDIO_DEBUG
      console.log(`emscripten_start_wasm_audio_worklet_thread_async() addModule() of main application JS completed`);
#endif
      {{{ makeDynCall('vii', 'callback') }}}(contextHandle, 1/*EM_TRUE*/, userData);
    }).catch(audioWorkletCreationFailed);
  },

  // Store a queue of async callbacks we need to fire when AudioWorkletProcessors have completed
  // loading.
  $_EmAudioProcessorCreateRequests: [],

  $_EmAudioDispatchProcessorCallback__deps: ['$_EmAudioProcessorCreateRequests'],
  $_EmAudioDispatchProcessorCallback: function() {
    let e = _EmAudioProcessorCreateRequests.shift();
    {{{ makeDynCall('viii', 'e[1]') }}}(e[0], 1/*EM_TRUE*/, e[2]);
  },

  emscripten_create_wasm_audio_worklet_processor_async__deps: ['$_EmAudioProcessorCreateRequests'],
  emscripten_create_wasm_audio_worklet_processor_async: function(contextHandle, options, callback, userData) {
#if ASSERTIONS
    assert(contextHandle, `Called emscripten_create_wasm_audio_worklet_processor_async() with a null Web Audio Context handle!`);
    assert(EmAudio[contextHandle], `Called emscripten_create_wasm_audio_worklet_processor_async() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_create_wasm_audio_worklet_processor_async() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif

    _EmAudioProcessorCreateRequests.push([contextHandle, callback, userData]);

    options >>= 2;
    let audioParams = [],
      numAudioParams = HEAPU32[options+1],
      audioParamDescriptors = HEAPU32[options+2] >> 2,
      i = 0;

    while(numAudioParams--) {
      audioParams.push({
        name: i++,
        defaultValue: HEAPF32[audioParamDescriptors++],
        minValue: HEAPF32[audioParamDescriptors++],
        maxValue: HEAPF32[audioParamDescriptors++],
        automationRate: ['a','k'][HEAPU32[audioParamDescriptors++]] + '-rate',
      });
    }

#if WEBAUDIO_DEBUG
    console.log(`emscripten_create_wasm_audio_worklet_processor_async() creating a new AudioWorklet processor with name ${UTF8ToString(HEAPU32[options])}`);
#endif

    EmAudio[contextHandle].audioWorklet.bootstrapMessage.port.postMessage({
      name: UTF8ToString(HEAPU32[options]),
      audioParams: audioParams
    });
  },

  // emscripten_create_wasm_audio_worklet_node() function does not itself use emscriptenGetAudioObject() function, but mark it as a dependency, because
  // the user will not be able to utilize the node unless they call emscriptenGetAudioObject() on it on JS side to connect it to the graph, so record
  // it as a dependency here to avoid the user needing to manually do it on the command line.
  emscripten_create_wasm_audio_worklet_node__deps: ['$emscriptenGetAudioObject'],
  emscripten_create_wasm_audio_worklet_node: function(contextHandle, name, options, callback, userData) {
#if ASSERTIONS
    assert(contextHandle, `Called emscripten_create_wasm_audio_worklet_node() with a null Web Audio Context handle!`);
    assert(EmAudio[contextHandle], `Called emscripten_create_wasm_audio_worklet_node() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_create_wasm_audio_worklet_node() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif
    options >>= 2;

    function readChannelCountArray(heapIndex, numOutputs) {
      let channelCounts = [];
      while(numOutputs--) channelCounts.push(HEAPU32[heapIndex++]);
      return channelCounts;
    }

    let opts = options ? {
      numberOfInputs: HEAP32[options],
      numberOfOutputs: HEAP32[options+1],
      outputChannelCount: HEAPU32[options+2] ? readChannelCountArray(HEAPU32[options+2]>>2, HEAP32[options+1]) : void 0,
      processorOptions: { 'cb': callback, 'ud': userData }
    } : void 0;

#if WEBAUDIO_DEBUG
    console.log(`Creating AudioWorkletNode "${UTF8ToString(name)}" on context=${contextHandle} with options:`);
    console.dir(opts);
#endif
    return emscriptenRegisterAudioObject(new AudioWorkletNode(EmAudio[contextHandle], UTF8ToString(name), opts));
  },
#endif // ~AUDIO_WORKLET

  emscripten_current_thread_is_audio_worklet: function() {
    return typeof AudioWorkletGlobalScope !== 'undefined';
  }
};

mergeInto(LibraryManager.library, LibraryWebAudio);
