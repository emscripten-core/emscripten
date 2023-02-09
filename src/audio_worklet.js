// This file is the main bootstrap script for Wasm Audio Worklets loaded in an Emscripten application.
// Build with -sAUDIO_WORKLET=1 linker flag to enable targeting Audio Worklets.

// AudioWorkletGlobalScope does not have a onmessage/postMessage() functionality at the global scope, which
// means that after creating an AudioWorkletGlobalScope and loading this script into it, we cannot
// postMessage() information into it like one would do with Web Workers.

// Instead, we must create an AudioWorkletProcessor class, then instantiate a Web Audio graph node from it
// on the main thread. Using its message port and the node constructor's
// "processorOptions" field, we can share the necessary bootstrap information from the main thread to
// the AudioWorkletGlobalScope.

function createWasmAudioWorkletProcessor(audioParams) {
  class WasmAudioWorkletProcessor extends AudioWorkletProcessor {
    constructor(args) {
      super();

      // Copy needed stack allocation functions from the Module object
      // to global scope, these will be accessed in hot paths, so maybe
      // they'll be a bit faster to access directly, rather than referencing
      // them as properties of the Module object.
      globalThis.stackAlloc = Module['stackAlloc'];
      globalThis.stackSave = Module['stackSave'];
      globalThis.stackRestore = Module['stackRestore'];
      globalThis.HEAPU32 = Module['HEAPU32'];
      globalThis.HEAPF32 = Module['HEAPF32'];

      // Capture the Wasm function callback to invoke.
      let opts = args.processorOptions;
      this.callbackFunction = Module['wasmTable'].get(opts['cb']);
      this.userData = opts['ud'];
    }

    static get parameterDescriptors() {
      return audioParams;
    }

    process(inputList, outputList, parameters) {
      // Marshal all inputs and parameters to the Wasm memory on the thread stack,
      // then perform the wasm audio worklet call,
      // and finally marshal audio output data back.

      let numInputs = inputList.length,
        numOutputs = outputList.length,
        numParams = 0, i, j, k, dataPtr,
        stackMemoryNeeded = (numInputs + numOutputs) * 8,
        oldStackPtr = stackSave(),
        inputsPtr, outputsPtr, outputDataPtr, paramsPtr,
        didProduceAudio, paramArray;

      // Calculate how much stack space is needed.
      for(i of inputList) stackMemoryNeeded += i.length * 512;
      for(i of outputList) stackMemoryNeeded += i.length * 512;
      for(i in parameters) stackMemoryNeeded += parameters[i].byteLength + 8, ++numParams;

      // Allocate the necessary stack space.
      inputsPtr = stackAlloc(stackMemoryNeeded);

      // Copy input audio descriptor structs and data to Wasm
      k = inputsPtr >> 2;
      dataPtr = inputsPtr + numInputs * 8;
      for(i of inputList) {
        // Write the AudioSampleFrame struct instance
        HEAPU32[k++] = i.length;
        HEAPU32[k++] = dataPtr;
        // Marshal the input audio sample data for each audio channel of this input
        for(j of i) {
          HEAPF32.set(j, dataPtr>>2);
          dataPtr += 512;
        }
      }

      // Copy output audio descriptor structs to Wasm
      outputsPtr = dataPtr;
      k = outputsPtr >> 2;
      outputDataPtr = (dataPtr += numOutputs * 8) >> 2;
      for(i of outputList) {
        // Write the AudioSampleFrame struct instance
        HEAPU32[k++] = i.length;
        HEAPU32[k++] = dataPtr;
        // Reserve space for the output data
        dataPtr += 512 * i.length;
      }

      // Copy parameters descriptor structs and data to Wasm
      paramsPtr = dataPtr;
      k = paramsPtr >> 2;
      dataPtr += numParams * 8;
      for(i = 0; paramArray = parameters[i++];) {
        // Write the AudioParamFrame struct instance
        HEAPU32[k++] = paramArray.length;
        HEAPU32[k++] = dataPtr;
        // Marshal the audio parameters array
        HEAPF32.set(paramArray, dataPtr>>2);
        dataPtr += paramArray.length*4;
      }

      // Call out to Wasm callback to perform audio processing
      if (didProduceAudio = this.callbackFunction(numInputs, inputsPtr, numOutputs, outputsPtr, numParams, paramsPtr, this.userData)) {
        // Read back the produced audio data to all outputs and their channels.
        // (A garbage-free function TypedArray.copy(dstTypedArray, dstOffset, srcTypedArray, srcOffset, count) would sure be handy..
        //  but web does not have one, so manually copy all bytes in)
        for(i of outputList) {
          for(j of i) {
            for(k = 0; k < 128; ++k) {
              j[k] = HEAPF32[outputDataPtr++];
            }
          }
        }
      }

      stackRestore(oldStackPtr);

      // Return 'true' to tell the browser to continue running this processor. (Returning 1 or any other truthy value won't work in Chrome)
      return !!didProduceAudio;
    }
  }
  return WasmAudioWorkletProcessor;
}

// Specify a worklet processor that will be used to receive messages to this AudioWorkletGlobalScope.
// We never connect this initial AudioWorkletProcessor to the audio graph to do any audio processing.
class BootstrapMessages extends AudioWorkletProcessor {
  constructor(arg) {
    super();
    // Initialize the global Emscripten Module object that contains e.g. the Wasm Module and Memory objects.
    // After this we are ready to load in the main application JS script, which the main thread will addModule()
    // to this scope.
    globalThis.Module = arg['processorOptions'];
#if !MINIMAL_RUNTIME
    // Default runtime relies on an injected instantiateWasm() function to initialize the Wasm Module.
    globalThis.Module['instantiateWasm'] = (info, receiveInstance) => {
      var instance = new WebAssembly.Instance(Module['wasm'], info);
      receiveInstance(instance, Module['wasm']);
      return instance.exports;
    };
#endif
#if WEBAUDIO_DEBUG
    console.log('AudioWorklet global scope looks like this:');
    console.dir(globalThis);
#endif
    // Listen to messages from the main thread. These messages will ask this scope to create the real
    // AudioWorkletProcessors that call out to Wasm to do audio processing.
    let p = globalThis['messagePort'] = this.port;
    p.onmessage = (msg) => {
      let d = msg.data;
      if (d['_wpn']) { // '_wpn' is short for 'Worklet Processor Node', using an identifier that will never conflict with user messages
#if MODULARIZE
        // Instantiate the MODULARIZEd Module function, which is stored for us under the special global
        // name AudioWorkletModule in MODULARIZE+AUDIO_WORKLET builds.
        if (globalThis.AudioWorkletModule) {
          AudioWorkletModule(Module); // This populates the Module object with all the Wasm properties
          delete globalThis.AudioWorkletModule; // We have now instantiated the Module function, can discard it from global scope
        }
#endif
        // Register a real AudioWorkletProcessor that will actually do audio processing.
        registerProcessor(d['_wpn'], createWasmAudioWorkletProcessor(d['audioParams']));
#if WEBAUDIO_DEBUG
        console.log(`Registered a new WasmAudioWorkletProcessor "${d['_wpn']}" with AudioParams: ${d['audioParams']}`);
#endif
        // Post a Wasm Call message back telling that we have now registered the AudioWorkletProcessor class,
        // and should trigger the user onSuccess callback of the emscripten_create_wasm_audio_worklet_processor_async() call.
        p.postMessage({'_wsc': d['callback'], 'x': [d['contextHandle'], 1/*EM_TRUE*/, d['userData']] }); // "WaSm Call"
      } else if (d['_wsc']) { // '_wsc' is short for 'wasm call', using an identifier that will never conflict with user messages
        Module['wasmTable'].get(d['_wsc'])(...d['x']);
      };
    }
  }

  // No-op, not doing audio processing in this processor. It is just for receiving bootstrap messages.
  // However browsers require it to still be present. It should never be called because we never add a
  // node to the graph with this processor, although it does look like Chrome does still call this function.
  process() {
    // keep this function a no-op. Chrome redundantly wants to call this even though this processor is never added to the graph.
  }
};

// Register the dummy processor that will just receive messages.
registerProcessor("message", BootstrapMessages);
