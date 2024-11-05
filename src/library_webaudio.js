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
  $emscriptenRegisterAudioObject: (object) => {
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
  $emscriptenGetAudioObject: (objectHandle) => EmAudio[objectHandle],

  // Performs the work of getting the AudioContext's render quantum size.
  $emscriptenGetContextQuantumSize: (contextHandle) => {
    // TODO: in a future release this will be something like:
    //   return EmAudio[contextHandle].renderQuantumSize || 128;
    // It comes two caveats: it needs the hint when generating the context adding to
    // emscripten_create_audio_context(), and altering the quantum requires a secure
    // context and fallback implementing. Until then we simply use the 1.0 API value:
    return 128;
  },

  // emscripten_create_audio_context() does not itself use the
  // emscriptenGetAudioObject() function, but mark it as a dependency, because
  // the user will not be able to utilize the node unless they call
  // emscriptenGetAudioObject() on it on JS side to connect it to the graph, so
  // this avoids the user needing to manually add the dependency on the command line.
  emscripten_create_audio_context__deps: ['$emscriptenRegisterAudioObject', '$emscriptenGetAudioObject'],
  emscripten_create_audio_context: (options) => {
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

  emscripten_resume_audio_context_async: (contextHandle, callback, userData) => {
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

  emscripten_resume_audio_context_sync: (contextHandle) => {
#if ASSERTIONS
    assert(EmAudio[contextHandle], `Called emscripten_resume_audio_context_sync() on a nonexisting context handle ${contextHandle}`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_resume_audio_context_sync() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif
#if WEBAUDIO_DEBUG
    console.log(`AudioContext.resume() on WebAudio context with ID ${contextHandle}`);
#endif
    EmAudio[contextHandle].resume();
  },

  emscripten_audio_context_state: (contextHandle) => {
#if ASSERTIONS
    assert(EmAudio[contextHandle], `Called emscripten_audio_context_state() on a nonexisting context handle ${contextHandle}`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_audio_context_state() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif
    return ['suspended', 'running', 'closed', 'interrupted'].indexOf(EmAudio[contextHandle].state);
  },

  emscripten_destroy_audio_context: (contextHandle) => {
#if ASSERTIONS
    assert(EmAudio[contextHandle], `Called emscripten_destroy_audio_context() on an already freed context handle ${contextHandle}`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_destroy_audio_context() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif
#if WEBAUDIO_DEBUG
    console.log(`Destroyed WebAudio context with ID ${contextHandle}`);
#endif
    EmAudio[contextHandle].suspend();
    delete EmAudio[contextHandle];
  },

  emscripten_destroy_web_audio_node: (objectHandle) => {
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

#if AUDIO_WORKLET
  // emscripten_start_wasm_audio_worklet_thread_async() doesn't use stackAlloc,
  // etc., but the created worklet does.
  emscripten_start_wasm_audio_worklet_thread_async__deps: [
    '$_wasmWorkersID',
    '$_EmAudioDispatchProcessorCallback',
    '$stackAlloc', '$stackRestore', '$stackSave'],
  emscripten_start_wasm_audio_worklet_thread_async: (contextHandle, stackLowestAddress, stackSize, callback, userData) => {

#if ASSERTIONS
    assert(contextHandle, `Called emscripten_start_wasm_audio_worklet_thread_async() with a null Web Audio Context handle!`);
    assert(EmAudio[contextHandle], `Called emscripten_start_wasm_audio_worklet_thread_async() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_start_wasm_audio_worklet_thread_async() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif

    let audioContext = EmAudio[contextHandle],
      audioWorklet = audioContext.audioWorklet;

#if ASSERTIONS
    assert(stackLowestAddress != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
    assert(stackLowestAddress % 16 == 0, `AudioWorklet stack should be aligned to 16 bytes! (was ${stackLowestAddress} == ${stackLowestAddress%16} mod 16) Use e.g. memalign(16, stackSize) to align the stack!`);
    assert(stackSize != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
    assert(stackSize % 16 == 0, `AudioWorklet stack size should be a multiple of 16 bytes! (was ${stackSize} == ${stackSize%16} mod 16)`);
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
      {{{ makeDynCall('viip', 'callback') }}}(contextHandle, 0/*EM_FALSE*/, userData);
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

    // TODO: In MINIMAL_RUNTIME builds, read this file off of a preloaded Blob,
    // and/or embed from a string like with WASM_WORKERS==2 mode.
    audioWorklet.addModule('{{{ TARGET_BASENAME }}}.aw.js').then(() => {
#if WEBAUDIO_DEBUG
      console.log(`emscripten_start_wasm_audio_worklet_thread_async() addModule('audioworklet.js') completed`);
#endif
      audioWorklet.bootstrapMessage = new AudioWorkletNode(audioContext, 'message', {
        processorOptions: {
          // Assign the loaded AudioWorkletGlobalScope a Wasm Worker ID so that
          // it can utilized its own TLS slots, and it is recognized to not be
          // the main browser thread.
          '$ww': _wasmWorkersID++,
#if MINIMAL_RUNTIME
          'wasm': Module['wasm'],
          'mem': wasmMemory,
#else
          'wasm': wasmModule,
          'wasmMemory': wasmMemory,
#endif
          'sb': stackLowestAddress, // sb = stack base
          'sz': stackSize,          // sz = stack size
        }
      });
      audioWorklet.bootstrapMessage.port.onmessage = _EmAudioDispatchProcessorCallback;

      // AudioWorklets do not have a importScripts() function like Web Workers
      // do (and AudioWorkletGlobalScope does not allow dynamic import()
      // either), but instead, the main thread must load all JS code into the
      // worklet scope. Send the application main JS script to the audio
      // worklet.
      return audioWorklet.addModule(
#if MINIMAL_RUNTIME
        Module['js']
#else
        Module['mainScriptUrlOrBlob'] || _scriptName
#endif
      );
    }).then(() => {
#if WEBAUDIO_DEBUG
      console.log(`emscripten_start_wasm_audio_worklet_thread_async() addModule() of main application JS completed`);
#endif
      {{{ makeDynCall('viii', 'callback') }}}(contextHandle, 1/*EM_TRUE*/, userData);
    }).catch(audioWorkletCreationFailed);
  },

  $_EmAudioDispatchProcessorCallback: (e) => {
    let data = e.data;
    // '_wsc' is short for 'wasm call', trying to use an identifier name that
    // will never conflict with user code
    let wasmCall = data['_wsc'];
    wasmCall && getWasmTableEntry(wasmCall)(...data['x']);
  },

  emscripten_create_wasm_audio_worklet_processor_async: (contextHandle, options, callback, userData) => {
#if ASSERTIONS
    assert(contextHandle, `Called emscripten_create_wasm_audio_worklet_processor_async() with a null Web Audio Context handle!`);
    assert(EmAudio[contextHandle], `Called emscripten_create_wasm_audio_worklet_processor_async() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_create_wasm_audio_worklet_processor_async() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif

    options >>= 2;
    let audioParams = [],
      numAudioParams = HEAPU32[options+1],
      audioParamDescriptors = HEAPU32[options+2] >> 2,
      i = 0;

    while (numAudioParams--) {
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
      // Deliberately mangled and short names used here ('_wpn', the 'Worklet
      // Processor Name' used as a 'key' to verify the message type so as to
      // not get accidentally mixed with user submitted messages, the remainder
      // for space saving reasons, abbreviated from their variable names).
      '_wpn': UTF8ToString(HEAPU32[options]),
      'ap': audioParams,
      'ch': contextHandle,
      'cb': callback,
      'ud': userData
    });
  },

  emscripten_create_wasm_audio_worklet_node__deps: ['$emscriptenGetContextQuantumSize'],
  emscripten_create_wasm_audio_worklet_node: (contextHandle, name, options, callback, userData) => {
#if ASSERTIONS
    assert(contextHandle, `Called emscripten_create_wasm_audio_worklet_node() with a null Web Audio Context handle!`);
    assert(EmAudio[contextHandle], `Called emscripten_create_wasm_audio_worklet_node() with a nonexisting/already freed Web Audio Context handle ${contextHandle}!`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_create_wasm_audio_worklet_node() on a context handle ${contextHandle} that is not an AudioContext, but of type ${typeof EmAudio[contextHandle]}`);
#endif
    options >>= 2;

    function readChannelCountArray(heapIndex, numOutputs) {
      let channelCounts = [];
      while (numOutputs--) channelCounts.push(HEAPU32[heapIndex++]);
      return channelCounts;
    }

    let opts = options ? {
      numberOfInputs: HEAP32[options],
      numberOfOutputs: HEAP32[options+1],
      outputChannelCount: HEAPU32[options+2] ? readChannelCountArray(HEAPU32[options+2]>>2, HEAP32[options+1]) : void 0,
      processorOptions: {
        'cb': callback,
        'ud': userData,
        'sc': emscriptenGetContextQuantumSize(contextHandle)
      }
    } : void 0;

#if WEBAUDIO_DEBUG
    console.log(`Creating AudioWorkletNode "${UTF8ToString(name)}" on context=${contextHandle} with options:`);
    console.dir(opts);
#endif
    return emscriptenRegisterAudioObject(new AudioWorkletNode(EmAudio[contextHandle], UTF8ToString(name), opts));
  },
#endif // ~AUDIO_WORKLET

  emscripten_audio_context_quantum_size__deps: ['$emscriptenGetContextQuantumSize'],
  emscripten_audio_context_quantum_size: (contextHandle) => {
#if ASSERTIONS
    assert(EmAudio[contextHandle], `Called emscripten_audio_context_quantum_size() with an invalid Web Audio Context handle ${contextHandle}`);
    assert(EmAudio[contextHandle] instanceof (window.AudioContext || window.webkitAudioContext), `Called emscripten_audio_context_quantum_size() on handle ${contextHandle} that is not an AudioContext, but of type ${EmAudio[contextHandle]}`);
#endif
    return emscriptenGetContextQuantumSize(contextHandle);
  },

  emscripten_audio_node_connect: (source, destination, outputIndex, inputIndex) => {
    var srcNode = EmAudio[source];
    var dstNode = EmAudio[destination];
#if ASSERTIONS
    assert(srcNode, `Called emscripten_audio_node_connect() with an invalid AudioNode handle ${source}`);
    assert(srcNode instanceof window.AudioNode, `Called emscripten_audio_node_connect() on handle ${source} that is not an AudiotNode, but of type ${srcNode}`);
    assert(dstNode, `Called emscripten_audio_node_connect() with an invalid AudioNode handle ${destination}!`);
    assert(dstNode instanceof (window.AudioContext || window.webkitAudioContext) || dstNode instanceof window.AudioNode, `Called emscripten_audio_node_connect() on handle ${destination} that is not an AudioContext or AudioNode, but of type ${dstNode}`);
#endif
#if WEBAUDIO_DEBUG
    console.log(`Connecting audio node ID ${source} to audio node ID ${destination} (${srcNode} to ${dstNode})`);
#endif
    srcNode.connect(dstNode.destination || dstNode, outputIndex, inputIndex);
  },

  emscripten_current_thread_is_audio_worklet: () => typeof AudioWorkletGlobalScope !== 'undefined',

  emscripten_audio_worklet_post_function_v: (audioContext, funcPtr) => {
    (audioContext ? EmAudio[audioContext].audioWorklet.bootstrapMessage.port : globalThis['messagePort']).postMessage({'_wsc': funcPtr, 'x': [] }); // "WaSm Call"
  },

  $emscripten_audio_worklet_post_function_1: (audioContext, funcPtr, arg0) => {
    (audioContext ? EmAudio[audioContext].audioWorklet.bootstrapMessage.port : globalThis['messagePort']).postMessage({'_wsc': funcPtr, 'x': [arg0] }); // "WaSm Call"
  },

  emscripten_audio_worklet_post_function_vi__deps: ['$emscripten_audio_worklet_post_function_1'],
  emscripten_audio_worklet_post_function_vi(audioContext, funcPtr, arg0) {
    emscripten_audio_worklet_post_function_1(audioContext, funcPtr, arg0)
  },

  emscripten_audio_worklet_post_function_vd__deps: ['$emscripten_audio_worklet_post_function_1'],
  emscripten_audio_worklet_post_function_vd(audioContext, funcPtr, arg0) {
    emscripten_audio_worklet_post_function_1(audioContext, funcPtr, arg0)
  },

  $emscripten_audio_worklet_post_function_2: (audioContext, funcPtr, arg0, arg1) => {
    (audioContext ? EmAudio[audioContext].audioWorklet.bootstrapMessage.port : globalThis['messagePort']).postMessage({'_wsc': funcPtr, 'x': [arg0, arg1] }); // "WaSm Call"
  },

  emscripten_audio_worklet_post_function_vii__deps: ['$emscripten_audio_worklet_post_function_2'],
  emscripten_audio_worklet_post_function_vii: (audioContext, funcPtr, arg0, arg1) => {
    emscripten_audio_worklet_post_function_2(audioContext, funcPtr, arg0, arg1);
  },

  emscripten_audio_worklet_post_function_vdd__deps: ['$emscripten_audio_worklet_post_function_2'],
  emscripten_audio_worklet_post_function_vdd: (audioContext, funcPtr, arg0, arg1) => {
    emscripten_audio_worklet_post_function_2(audioContext, funcPtr, arg0, arg1);
  },

  $emscripten_audio_worklet_post_function_3: (audioContext, funcPtr, arg0, arg1, arg2) => {
    (audioContext ? EmAudio[audioContext].audioWorklet.bootstrapMessage.port : globalThis['messagePort']).postMessage({'_wsc': funcPtr, 'x': [arg0, arg1, arg2] }); // "WaSm Call"
  },
  emscripten_audio_worklet_post_function_viii__deps: ['$emscripten_audio_worklet_post_function_3'],
  emscripten_audio_worklet_post_function_viii: (audioContext, funcPtr, arg0, arg1, arg2) => {
    emscripten_audio_worklet_post_function_3(audioContext, funcPtr, arg0, arg1, arg2);
  },
  emscripten_audio_worklet_post_function_vddd__deps: ['$emscripten_audio_worklet_post_function_3'],
  emscripten_audio_worklet_post_function_vddd: (audioContext, funcPtr, arg0, arg1, arg2) => {
    emscripten_audio_worklet_post_function_3(audioContext, funcPtr, arg0, arg1, arg2);
  },

  emscripten_audio_worklet_post_function_sig__deps: ['$readAsmConstArgs'],
  emscripten_audio_worklet_post_function_sig: (audioContext, funcPtr, sigPtr, varargs) => {
#if ASSERTIONS
    assert(audioContext >= 0);
    assert(funcPtr);
    assert(sigPtr);
    assert(UTF8ToString(sigPtr)[0] != 'v', 'Do NOT specify the return argument in the signature string for a call to emscripten_audio_worklet_post_function_sig(), just pass the function arguments.');
    assert(varargs);
#endif
    (audioContext ? EmAudio[audioContext].audioWorklet.bootstrapMessage.port : globalThis['messagePort']).postMessage({'_wsc': funcPtr, 'x': readAsmConstArgs(sigPtr, varargs) });
  }
};

addToLibrary(LibraryWebAudio);
