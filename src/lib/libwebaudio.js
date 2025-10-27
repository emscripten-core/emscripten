#if AUDIO_WORKLET && !WASM_WORKERS
#error "Building with -sAUDIO_WORKLET also requires enabling -sWASM_WORKERS"
#endif
#if AUDIO_WORKLET && TEXTDECODER == 2
#warning "-sAUDIO_WORKLET does not support -sTEXTDECODER=2 since TextDecoder is not available in AudioWorkletGlobalScope. Text decoding will be unavailable in Audio Worklets. If you need string marshalling in Audio Worklets, build with -sTEXTDECODER=1."
#endif
#if AUDIO_WORKLET && SINGLE_FILE
#error "-sAUDIO_WORKLET does not support -sSINGLE_FILE"
#endif

var LibraryWebAudio = {
  $emAudio__deps: ['$HandleAllocator'],
  $emAudio: 'new HandleAllocator();',

  // Call this function from JavaScript to register a Wasm-side handle to an AudioContext that
  // you have already created manually without calling emscripten_create_audio_context().
  // Note: To let that AudioContext be garbage collected later, call the function
  // emscriptenDestroyAudioContext() to unbind it from Wasm.
  $emscriptenRegisterAudioObject: (object) => {
#if ASSERTIONS
    assert(object, 'Called emscriptenRegisterAudioObject() with a null object handle!');
#endif
    var id = emAudio.allocate(object);
#if WEBAUDIO_DEBUG
    dbg(`Registered new WebAudio object ${object} with ID ${id}`);
#endif
    return id;
  },

#if ASSERTIONS || WEBAUDIO_DEBUG
  $emAudioCheckHandle__internal: true,
  $emAudioCheckHandle(handle, methodName, isNode = false) {
#if WEBAUDIO_DEBUG
    dbg(`called ${methodName}() with ID ${handle}`);
#endif
#if ASSERTIONS
    assert(emAudio.has(handle), `Called ${methodName}() on a nonexisting handle ${handle}`);
    var obj = emAudio.get(handle);
    if (isNode == 2)  {
      // Some method accept either a node or an audio context
      assert(obj instanceof window.AudioNode || obj instanceof (window.AudioContext || window.webkitAudioContext), `Called ${methodName}() on a context handle ${handle} that is not an AudioNode, but of type ${typeof obj}`);
    } else if (isNode) {
      assert(obj instanceof window.AudioNode, `Called ${methodName}() on a context handle ${handle} that is not an AudioNode, but of type ${typeof obj}`);
    } else {
      assert(obj instanceof (window.AudioContext || window.webkitAudioContext), `Called ${methodName}() on a context handle ${handle} that is not an AudioContext, but of type ${typeof obj}`);
    }
  #endif
  },
#endif

  // Call this function from JavaScript to destroy a Wasm-side handle to an AudioContext.
  // After calling this function, it is no longer possible to reference this AudioContext
  // from Wasm code - and the GC can reclaim it after all references to it are cleared.
  $emscriptenDestroyAudioContext: 'emscripten_destroy_audio_context',

  // Call this function from JavaScript to get the Web Audio object corresponding to the given
  // Wasm handle ID.
  $emscriptenGetAudioObject: (objectHandle) => emAudio.get(objectHandle),

  // Performs the work of getting the AudioContext's render quantum size.
  $emscriptenGetContextQuantumSize: (contextHandle) => {
    // TODO: in a future release this will be something like:
    //   return emAudio.get(contextHandle).renderQuantumSize || 128;
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
    // Safari added unprefixed AudioContext support in Safari 14.5 on iOS: https://caniuse.com/audio-api
#if MIN_SAFARI_VERSION < 140500 || ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
    var ctx = window.AudioContext || window.webkitAudioContext;
#if ASSERTIONS
    if (!ctx) console.error('emscripten_create_audio_context failed! Web Audio is not supported.');
#endif
#endif

    var opts = options ? {
      latencyHint: UTF8ToString({{{ makeGetValue('options', C_STRUCTS.EmscriptenWebAudioCreateAttributes.latencyHint, '*') }}}) || undefined,
      sampleRate: {{{ makeGetValue('options', C_STRUCTS.EmscriptenWebAudioCreateAttributes.sampleRate, 'u32') }}} || undefined
    } : undefined;

#if WEBAUDIO_DEBUG
    dbg(`Creating new WebAudio context with parameters:`);
    console.dir(opts);
#endif

#if MIN_SAFARI_VERSION < 140500 || ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
    return ctx && emscriptenRegisterAudioObject(new ctx(opts));
#else
    // We are targeting an environment where we assume that AudioContext() API unconditionally exists.
    return emscriptenRegisterAudioObject(new AudioContext(opts));
#endif
  },

  emscripten_resume_audio_context_async: (contextHandle, callback, userData) => {
    var audio = emAudio.get(contextHandle);
    function cb(state) {
#if WEBAUDIO_DEBUG
      dbg(`emscripten_resume_audio_context_async() callback: New audio state="${audio.state}", ID=${state}`);
#endif
      {{{ makeDynCall('viip', 'callback') }}}(contextHandle, state, userData);
    }
#if WEBAUDIO_DEBUG
    dbg('emscripten_resume_audio_context_async() resuming...');
#endif
    audio.resume().then(() => { cb(1/*running*/) }).catch(() => { cb(0/*suspended*/) });
  },

  emscripten_resume_audio_context_sync: (contextHandle) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_resume_audio_context_sync');
#endif
    emAudio.get(contextHandle).resume();
  },

  emscripten_audio_context_state: (contextHandle) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_audio_context_state');
#endif
    return ['suspended', 'running', 'closed', 'interrupted'].indexOf(emAudio.get(contextHandle).state);
  },

  emscripten_destroy_audio_context: (contextHandle) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_destroy_audio_context');
#endif
    emAudio.get(contextHandle).suspend();
    emAudio.free(contextHandle);
  },

  emscripten_destroy_web_audio_node: (objectHandle) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(objectHandle, 'emscripten_destroy_web_audio_node', true);
#endif
    // Explicitly disconnect the node from Web Audio graph before letting it GC,
    // to work around browser bugs such as https://webkit.org/b/222098#c23
    emAduio.get(objectHandle).disconnect();
    emAudio.free(objectHandle);
  },

#if AUDIO_WORKLET
  // emscripten_start_wasm_audio_worklet_thread_async() doesn't use stackAlloc,
  // etc., but the created worklet does.
  emscripten_start_wasm_audio_worklet_thread_async__deps: [
    '$_wasmWorkersID',
    '$_emAudioDispatchProcessorCallback',
    '$stackAlloc', '$stackRestore', '$stackSave'],
  emscripten_start_wasm_audio_worklet_thread_async: (contextHandle, stackLowestAddress, stackSize, callback, userData) => {

#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_start_wasm_audio_worklet_thread_async');
#endif

    var audioContext = emAudio.get(contextHandle);
    var audioWorklet = audioContext.audioWorklet;

#if ASSERTIONS
    assert(stackLowestAddress != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
    assert(stackLowestAddress % 16 == 0, `AudioWorklet stack should be aligned to 16 bytes! (was ${stackLowestAddress} == ${stackLowestAddress%16} mod 16) Use e.g. memalign(16, stackSize) to align the stack!`);
    assert(stackSize != 0, 'AudioWorklets require a dedicated stack space for audio data marshalling between Wasm and JS!');
    assert(stackSize % 16 == 0, `AudioWorklet stack size should be a multiple of 16 bytes! (was ${stackSize} == ${stackSize%16} mod 16)`);
    assert(!audioContext.audioWorkletInitialized, 'emscripten_create_wasm_audio_worklet() was already called for AudioContext ' + contextHandle + '! Only call this function once per AudioContext!');
    audioContext.audioWorkletInitialized = 1;
#endif

#if WEBAUDIO_DEBUG
    dbg(`emscripten_start_wasm_audio_worklet_thread_async() adding audioworklet.js...`);
#endif

    var audioWorkletCreationFailed = () => {
#if ASSERTIONS || WEBAUDIO_DEBUG
      dbg(`emscripten_start_wasm_audio_worklet_thread_async() addModule() failed!`);
#endif
      {{{ makeDynCall('viip', 'callback') }}}(contextHandle, 0/*EM_FALSE*/, userData);
    };

    // Does browser not support AudioWorklets?
    if (!audioWorklet) {
#if ASSERTIONS || WEBAUDIO_DEBUG
      if (location.protocol == 'http:') {
        console.error(`AudioWorklets are not supported. This is possibly due to running the page over unsecure http:// protocol. Try running over https://, or debug via a localhost-based server, which should also allow AudioWorklets to function.`);
      } else {
        console.error(`AudioWorklets are not supported by current browser.`);
      }
#endif
      return audioWorkletCreationFailed();
    }

    audioWorklet.addModule({{{ wasmWorkerJs }}}).then(() => {
#if WEBAUDIO_DEBUG
      dbg(`emscripten_start_wasm_audio_worklet_thread_async() addModule() completed`);
#endif

#if MIN_FIREFOX_VERSION < 138 || MIN_CHROME_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
      // If this browser does not support the up-to-date AudioWorklet standard
      // that has a MessagePort over to the AudioWorklet, then polyfill that by
      // instantiating a dummy AudioWorkletNode to get a MessagePort over.
      // Firefox added support in https://hg-edge.mozilla.org/integration/autoland/rev/ab38a1796126f2b3fc06475ffc5a625059af59c1
      // Chrome ticket: https://crbug.com/446920095
      // Safari ticket: https://webkit.org/b/299386
      if (!audioWorklet['port']) {
        audioWorklet['port'] = {
          postMessage: (msg) => {
            if (msg['_boot']) {
              audioWorklet.bootstrapMessage = new AudioWorkletNode(audioContext, 'em-bootstrap', {
                processorOptions: msg
              });
              audioWorklet.bootstrapMessage['port'].onmessage = (msg) => {
                audioWorklet['port'].onmessage(msg);
              }
            } else {
              audioWorklet.bootstrapMessage['port'].postMessage(msg);
            }
          }
        }
      }
#endif

      audioWorklet['port'].postMessage({
        // This is the bootstrap message to the Audio Worklet.
        '_boot': 1,
        // Assign the loaded AudioWorkletGlobalScope a Wasm Worker ID so that
        // it can utilized its own TLS slots, and it is recognized to not be
        // the main browser thread.
        wwID: _wasmWorkersID++,
#if MINIMAL_RUNTIME
        wasm: Module['wasm'],
#else
        wasm: wasmModule,
#endif
        wasmMemory,
        stackLowestAddress, // sb = stack base
        stackSize,          // sz = stack size
      });
      audioWorklet['port'].onmessage = _emAudioDispatchProcessorCallback;
      {{{ makeDynCall('viip', 'callback') }}}(contextHandle, 1/*EM_TRUE*/, userData);
    }).catch(audioWorkletCreationFailed);
  },

  $_emAudioDispatchProcessorCallback__deps: ['$getWasmTableEntry'],
  $_emAudioDispatchProcessorCallback: (e) => {
    var data = e.data;
    // '_wsc' is short for 'wasm call', trying to use an identifier name that
    // will never conflict with user code. This is used to call both the 3-param
    // call (handle, true, userData) and the variable argument post functions.
    var wasmCall = data['_wsc'];
    wasmCall && getWasmTableEntry(wasmCall)(...data.args);
  },

  emscripten_create_wasm_audio_worklet_processor_async: (contextHandle, options, callback, userData) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_create_wasm_audio_worklet_processor_async');
#endif

    var processorName = UTF8ToString({{{ makeGetValue('options', C_STRUCTS.WebAudioWorkletProcessorCreateOptions.name, '*') }}});

#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
    var numAudioParams = {{{ makeGetValue('options', C_STRUCTS.WebAudioWorkletProcessorCreateOptions.numAudioParams, 'i32') }}};
    var audioParamDescriptors = {{{ makeGetValue('options', C_STRUCTS.WebAudioWorkletProcessorCreateOptions.audioParamDescriptors, '*') }}};
    var audioParams = [];
    var paramIndex = 0;
    while (numAudioParams--) {
      audioParams.push({
        name: paramIndex++,
        defaultValue: {{{ makeGetValue('audioParamDescriptors', C_STRUCTS.WebAudioParamDescriptor.defaultValue, 'float') }}},
        minValue: {{{ makeGetValue('audioParamDescriptors', C_STRUCTS.WebAudioParamDescriptor.minValue, 'float') }}},
        maxValue: {{{ makeGetValue('audioParamDescriptors', C_STRUCTS.WebAudioParamDescriptor.maxValue, 'float') }}},
        automationRate: ({{{ makeGetValue('audioParamDescriptors', C_STRUCTS.WebAudioParamDescriptor.automationRate, 'i32') }}} ? 'k' : 'a') + '-rate',
      });
      audioParamDescriptors += {{{ C_STRUCTS.WebAudioParamDescriptor.__size__ }}};
    }
#elif ASSERTIONS
    var numAudioParams = {{{ makeGetValue('options', C_STRUCTS.WebAudioWorkletProcessorCreateOptions.numAudioParams, 'i32') }}};
    assert(numAudioParams == 0 && "Rebuild with -sAUDIO_WORKLET_SUPPORT_AUDIO_PARAMS to utilize AudioParams");
#endif

#if WEBAUDIO_DEBUG
    console.log(`emscripten_create_wasm_audio_worklet_processor_async() creating a new AudioWorklet processor with name ${processorName}`);
#endif

    emAudio.get(contextHandle).audioWorklet['port'].postMessage({
      // Deliberately mangled and short names used here ('_wpn', the 'Worklet
      // Processor Name' used as a 'key' to verify the message type so as to
      // not get accidentally mixed with user submitted messages, the remainder
      // for space saving reasons, abbreviated from their variable names).
      '_wpn': processorName,
#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
      audioParams,
#endif
      contextHandle,
      callback,
      userData,
    });
  },

  emscripten_create_wasm_audio_worklet_node__deps: ['$emscriptenGetContextQuantumSize'],
  emscripten_create_wasm_audio_worklet_node: (contextHandle, name, options, callback, userData) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_create_wasm_audio_worklet_node');
#endif

    function readChannelCountArray(heapIndex, numOutputs) {
      if (!heapIndex) return undefined;
      heapIndex = {{{ getHeapOffset('heapIndex', 'i32') }}};
      var channelCounts = [];
      while (numOutputs--) channelCounts.push(HEAPU32[heapIndex++]);
      return channelCounts;
    }

    var optionsOutputs = options ? {{{ makeGetValue('options', C_STRUCTS.EmscriptenAudioWorkletNodeCreateOptions.numberOfOutputs, 'i32') }}} : 0;
    var opts = options ? {
      numberOfInputs: {{{ makeGetValue('options', C_STRUCTS.EmscriptenAudioWorkletNodeCreateOptions.numberOfInputs, 'i32') }}},
      numberOfOutputs: optionsOutputs,
      outputChannelCount: readChannelCountArray({{{ makeGetValue('options', C_STRUCTS.EmscriptenAudioWorkletNodeCreateOptions.outputChannelCounts, 'i32*') }}}, optionsOutputs),
      channelCount: {{{ makeGetValue('options', C_STRUCTS.EmscriptenAudioWorkletNodeCreateOptions.channelCount, 'u32') }}} || undefined,
      channelCountMode: [/*'max'*/,'clamped-max','explicit'][{{{ makeGetValue('options', C_STRUCTS.EmscriptenAudioWorkletNodeCreateOptions.channelCountMode, 'i32') }}}],
      channelInterpretation: [/*'speakers'*/,'discrete'][{{{ makeGetValue('options', C_STRUCTS.EmscriptenAudioWorkletNodeCreateOptions.channelInterpretation, 'i32') }}}],
      processorOptions: {
        callback,
        userData,
        samplesPerChannel: emscriptenGetContextQuantumSize(contextHandle),
      }
    } : undefined;

#if WEBAUDIO_DEBUG
    dbg(`Creating AudioWorkletNode "${UTF8ToString(name)}" on context=${contextHandle} with options:`);
    console.dir(opts);
#endif
    return emscriptenRegisterAudioObject(new AudioWorkletNode(emAudio.get(contextHandle), UTF8ToString(name), opts));
  },
#endif // ~AUDIO_WORKLET

  emscripten_audio_context_quantum_size__deps: ['$emscriptenGetContextQuantumSize'],
  emscripten_audio_context_quantum_size: (contextHandle) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_audio_context_quantum_size')
#endif
    return emscriptenGetContextQuantumSize(contextHandle);
  },

  emscripten_audio_context_sample_rate: (contextHandle) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(contextHandle, 'emscripten_audio_context_sample_rate');
#endif
    return emAudio.get(contextHandle)['sampleRate'];
  },

  emscripten_audio_node_connect: (source, destination, outputIndex, inputIndex) => {
#if ASSERTIONS || WEBAUDIO_DEBUG
    emAudioCheckHandle(source, 'emscripten_audio_node_connect', 1);
    emAudioCheckHandle(destination, 'emscripten_audio_node_connect', 2);
#endif
    var srcNode = emAudio.get(source);
    var dstNode = emAudio.get(destination);
#if WEBAUDIO_DEBUG
    dbg(`Connecting audio node ID ${source} to audio node ID ${destination} (${srcNode} to ${dstNode})`);
#endif
    srcNode.connect(dstNode.destination || dstNode, outputIndex, inputIndex);
  },

  emscripten_current_thread_is_audio_worklet: () => ENVIRONMENT_IS_AUDIO_WORKLET,

  emscripten_audio_worklet_post_function_v: (audioContext, funcPtr) => {
    (audioContext ? emAudio.get(audioContext).audioWorklet['port'] : port).postMessage({'_wsc': funcPtr, args: [] }); // "WaSm Call"
  },

  $emscripten_audio_worklet_post_function_1: (audioContext, funcPtr, arg0) => {
    (audioContext ? emAudio.get(audioContext).audioWorklet['port'] : port).postMessage({'_wsc': funcPtr, args: [arg0] }); // "WaSm Call"
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
    (audioContext ? emAudio.get(audioContext).audioWorklet['port'] : port).postMessage({'_wsc': funcPtr, args: [arg0, arg1] }); // "WaSm Call"
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
    (audioContext ? emAudio.get(audioContext).audioWorklet['port'] : port).postMessage({'_wsc': funcPtr, args: [arg0, arg1, arg2] }); // "WaSm Call"
  },
  emscripten_audio_worklet_post_function_viii__deps: ['$emscripten_audio_worklet_post_function_3'],
  emscripten_audio_worklet_post_function_viii: (audioContext, funcPtr, arg0, arg1, arg2) => {
    emscripten_audio_worklet_post_function_3(audioContext, funcPtr, arg0, arg1, arg2);
  },
  emscripten_audio_worklet_post_function_vddd__deps: ['$emscripten_audio_worklet_post_function_3'],
  emscripten_audio_worklet_post_function_vddd: (audioContext, funcPtr, arg0, arg1, arg2) => {
    emscripten_audio_worklet_post_function_3(audioContext, funcPtr, arg0, arg1, arg2);
  },

  emscripten_audio_worklet_post_function_sig__deps: ['$readEmAsmArgs'],
  emscripten_audio_worklet_post_function_sig: (audioContext, funcPtr, sigPtr, varargs) => {
#if ASSERTIONS
    assert(audioContext >= 0);
    assert(funcPtr);
    assert(sigPtr);
    assert(UTF8ToString(sigPtr)[0] != 'v', 'Do NOT specify the return argument in the signature string for a call to emscripten_audio_worklet_post_function_sig(), just pass the function arguments.');
    assert(varargs);
#endif
    (audioContext ? emAudio.get(audioContext).audioWorklet['port'] : port).postMessage({'_wsc': funcPtr, args: readEmAsmArgs(sigPtr, varargs) });
  }
};

autoAddDeps(LibraryWebAudio, '$emAudio');
#if ASSERTIONS || WEBAUDIO_DEBUG
autoAddDeps(LibraryWebAudio, '$emAudioCheckHandle');
#endif

addToLibrary(LibraryWebAudio);
