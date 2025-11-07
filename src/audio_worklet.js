// This file is the main bootstrap script for Wasm Audio Worklets loaded in an
// Emscripten application.  Build with -sAUDIO_WORKLET linker flag to enable
// targeting Audio Worklets.

// AudioWorkletGlobalScope does not have a onmessage/postMessage() functionality
// at the global scope, which means that after creating an
// AudioWorkletGlobalScope and loading this script into it, we cannot
// postMessage() information into it like one would do with Web Workers.

// Instead, we must create an AudioWorkletProcessor class, then instantiate a
// Web Audio graph node from it on the main thread. Using its message port and
// the node constructor's "processorOptions" field, we can share the necessary
// bootstrap information from the main thread to the AudioWorkletGlobalScope.

#if MINIMAL_RUNTIME
var instantiatePromise;
#endif

if (ENVIRONMENT_IS_AUDIO_WORKLET) {

#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
function createWasmAudioWorkletProcessor(audioParams) {
#else
function createWasmAudioWorkletProcessor() {
#endif
  class WasmAudioWorkletProcessor extends AudioWorkletProcessor {
    constructor(args) {
      super();

      // Capture the Wasm function callback to invoke.
      let opts = args.processorOptions;
#if ASSERTIONS
      assert(opts.callback)
      assert(opts.samplesPerChannel)
#endif
      this.callback = {{{ makeDynCall('iipipipp', 'opts.callback') }}};
      this.userData = opts.userData;
      // Then the samples per channel to process, fixed for the lifetime of the
      // context that created this processor. Even though this 'render quantum
      // size' is fixed at 128 samples in the 1.0 spec, it will be variable in
      // the 1.1 spec. It's passed in now, just to prove it's settable, but will
      // eventually be a property of the  AudioWorkletGlobalScope (globalThis).
      this.samplesPerChannel = opts.samplesPerChannel;
      this.bytesPerChannel = this.samplesPerChannel * {{{ getNativeTypeSize('float') }}};

      // Prepare the output views; see createOutputViews(). The 'STACK_ALIGN'
      // deduction stops the STACK_OVERFLOW_CHECK failing (since the stack will
      // be full if we allocate all the available space) leaving room for a
      // single AudioSampleFrame as a minumum. There's an arbitrary maximum of
      // 64 frames, for the case where a multi-MB stack is passed.
      this.outputViews = new Array(Math.min(((wwParams.stackSize - {{{ STACK_ALIGN }}}) / this.bytesPerChannel) | 0, /*sensible limit*/ 64));
#if ASSERTIONS
      assert(this.outputViews.length > 0, `AudioWorklet needs more stack allocating (at least ${this.bytesPerChannel})`);
#endif
      this.createOutputViews();

#if ASSERTIONS
      // Explicitly verify this later in process(). Note to self, stackSave is a
      // bit of a misnomer as it simply gets the stack address.
      this.ctorOldStackPtr = stackSave();
#endif
    }

    /**
     * Create up-front as many typed views for marshalling the output data as
     * may be required, allocated at the *top* of the worklet's stack (and whose
     * addresses are fixed). 
     */
    createOutputViews() {
      // These are still alloc'd to take advantage of the overflow checks, etc.
      var oldStackPtr = stackSave();
      var viewDataIdx = {{{ getHeapOffset('stackAlloc(this.outputViews.length * this.bytesPerChannel)', 'float') }}};
#if WEBAUDIO_DEBUG
      console.log(`AudioWorklet creating ${this.outputViews.length} buffer one-time views (for a stack size of ${wwParams.stackSize} at address ${ptrToString(viewDataIdx * 4)})`);
#endif
      // Inserted in reverse so the lowest indices are closest to the stack top
      for (var n = this.outputViews.length - 1; n >= 0; n--) {
        this.outputViews[n] = HEAPF32.subarray(viewDataIdx, viewDataIdx += this.samplesPerChannel);
      }
      stackRestore(oldStackPtr);
    }

#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
    static get parameterDescriptors() {
      return audioParams;
    }
#endif

    /**
     * Marshals all inputs and parameters to the Wasm memory on the thread's
     * stack, then performs the wasm audio worklet call, and finally marshals
     * audio output data back.
     *
     * @param {Object} parameters
     */
#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
    process(inputList, outputList, parameters) {
#else
    /** @suppress {checkTypes} */
    process(inputList, outputList) {
#endif

#if ALLOW_MEMORY_GROWTH
      // Recreate the output views if the heap has changed
      // TODO: add support for GROWABLE_ARRAYBUFFERS
      if (HEAPF32.buffer != this.outputViews[0].buffer) {
        this.createOutputViews();
      }
#endif

      var numInputs = inputList.length;
      var numOutputs = outputList.length;

      var entry; // reused list entry or index
      var subentry; // reused channel or other array in each list entry or index

      // Calculate the required stack and output buffer views (stack is further
      // split into aligned structs and the raw float data).
      var stackMemoryStruct = (numInputs + numOutputs) * {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
      var stackMemoryData = 0;
      for (entry of inputList) {
        stackMemoryData += entry.length;
      }
      stackMemoryData *= this.bytesPerChannel;
      // Collect the total number of output channels (mapped to array views)
      var outputViewsNeeded = 0;
      for (entry of outputList) {
        outputViewsNeeded += entry.length;
      }
      stackMemoryData += outputViewsNeeded * this.bytesPerChannel;
      var numParams = 0;
#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
      for (entry in parameters) {
        ++numParams;
        stackMemoryStruct += {{{ C_STRUCTS.AudioParamFrame.__size__ }}};
        stackMemoryData += parameters[entry].byteLength;
      }
#endif
      var oldStackPtr = stackSave();
#if ASSERTIONS
      assert(oldStackPtr == this.ctorOldStackPtr, 'AudioWorklet stack address has unexpectedly moved');
      assert(outputViewsNeeded <= this.outputViews.length, `Too many AudioWorklet outputs (need ${outputViewsNeeded} but have stack space for ${this.outputViews.length})`);
#endif

      // Allocate the necessary stack space. All pointer variables are in bytes;
      // 'structPtr' starts at the first struct entry (all run sequentially)
      // and is the working start to each record; 'dataPtr' is the same for the
      // audio/params data, starting after *all* the structs.
      // 'structPtr' begins 16-byte aligned, allocated from the internal
      // _emscripten_stack_alloc(), as are the output views, and so to ensure
      // the views fall on the correct addresses (and we finish at stacktop) we
      // request additional bytes, taking this alignment into account, then
      // offset `dataPtr` by the difference.
      var stackMemoryAligned = (stackMemoryStruct + stackMemoryData + 15) & ~15;
      var structPtr = stackAlloc(stackMemoryAligned);
      var dataPtr = structPtr + (stackMemoryAligned - stackMemoryData);
#if ASSERTIONS
      // TODO: look at why stackAlloc isn't tripping the assertions
      assert(stackMemoryAligned <= wwParams.stackSize, `Not enough stack allocated to the AudioWorklet (need ${stackMemoryAligned}, got ${wwParams.stackSize})`);
#endif

      // Copy input audio descriptor structs and data to Wasm (recall, structs
      // first, audio data after). 'inputsPtr' is the start of the C callback's
      // input AudioSampleFrame.
      var /*const*/ inputsPtr = structPtr;
      for (entry of inputList) {
        // Write the AudioSampleFrame struct instance
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.numberOfChannels, 'entry.length', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.samplesPerChannel, 'this.samplesPerChannel', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.data, 'dataPtr', '*') }}};
        structPtr += {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
        // Marshal the input audio sample data for each audio channel of this input
        for (subentry of entry) {
          HEAPF32.set(subentry, {{{ getHeapOffset('dataPtr', 'float') }}});
          dataPtr += this.bytesPerChannel;
        }
      }

#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
      // Copy parameters descriptor structs and data to Wasm. 'paramsPtr' is the
      // start of the C callback's input AudioParamFrame.
      var /*const*/ paramsPtr = structPtr;
      for (entry = 0; subentry = parameters[entry++];) {
        // Write the AudioParamFrame struct instance
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioParamFrame.length, 'subentry.length', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioParamFrame.data, 'dataPtr', '*') }}};
        structPtr += {{{ C_STRUCTS.AudioParamFrame.__size__ }}};
        // Marshal the audio parameters array
        HEAPF32.set(subentry, {{{ getHeapOffset('dataPtr', 'float') }}});
        dataPtr += subentry.length * {{{ getNativeTypeSize('float') }}};
      }
#else
      var paramsPtr = 0;
#endif

      // Copy output audio descriptor structs to Wasm. 'outputsPtr' is the start
      // of the C callback's output AudioSampleFrame. 'dataPtr' will now be
      // aligned with the output views, ending at stacktop (which is why this
      // needs to be last).
      var /*const*/ outputsPtr = structPtr;
      for (entry of outputList) {
        // Write the AudioSampleFrame struct instance
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.numberOfChannels, 'entry.length', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.samplesPerChannel, 'this.samplesPerChannel', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.data, 'dataPtr', '*') }}};
        structPtr += {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
        // Advance the output pointer to the next output (matching the pre-allocated views)
        dataPtr += this.bytesPerChannel * entry.length;
      }

#if ASSERTIONS
      // If all the maths worked out, we arrived at the original stack address
      console.assert(dataPtr == oldStackPtr, `AudioWorklet stack missmatch (audio data finishes at ${dataPtr} instead of ${oldStackPtr})`);

      // Sanity checks. If these trip the most likely cause, beyond unforeseen
      // stack shenanigans, is that the 'render quantum size' changed after
      // construction (which shouldn't be possible).
      if (numOutputs) {
        // First that the output view addresses match the stack positions
        dataPtr -= this.bytesPerChannel;
        for (entry = 0; entry < outputViewsNeeded; entry++) {
          console.assert(dataPtr == this.outputViews[entry].byteOffset, 'AudioWorklet internal error in addresses of the output array views');
          dataPtr -= this.bytesPerChannel;
        }
        // And that the views' size match the passed in output buffers
        for (entry of outputList) {
          for (subentry of entry) {
            assert(subentry.byteLength == this.bytesPerChannel, `AudioWorklet unexpected output buffer size (expected ${this.bytesPerChannel} got ${subentry.byteLength})`);
          }
        }
      }
#endif

      // Call out to Wasm callback to perform audio processing
      var didProduceAudio = this.callback(numInputs, inputsPtr, numOutputs, outputsPtr, numParams, paramsPtr, this.userData);
      if (didProduceAudio) {
        // Read back the produced audio data to all outputs and their channels.
        // The preallocated 'outputViews' already have the correct offsets and
        // sizes into the stack (recall from createOutputViews() that they run
        // backwards).
        for (entry of outputList) {
          for (subentry of entry) {
            subentry.set(this.outputViews[--outputViewsNeeded]);
          }
        }
      }

      stackRestore(oldStackPtr);

      // Return 'true' to tell the browser to continue running this processor.
      // (Returning 1 or any other truthy value won't work in Chrome)
      return !!didProduceAudio;
    }
  }
  return WasmAudioWorkletProcessor;
}

#if MIN_FIREFOX_VERSION < 138 || MIN_CHROME_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
// If this browser does not support the up-to-date AudioWorklet standard
// that has a MessagePort over to the AudioWorklet, then polyfill that by
// a hacky AudioWorkletProcessor that provides the MessagePort.
// Firefox added support in https://hg-edge.mozilla.org/integration/autoland/rev/ab38a1796126f2b3fc06475ffc5a625059af59c1
// Chrome ticket: https://crbug.com/446920095
// Safari ticket: https://webkit.org/b/299386
/**
 * @suppress {duplicate, checkTypes}
 */
var port = globalThis.port || {};

// Specify a worklet processor that will be used to receive messages to this
// AudioWorkletGlobalScope.  We never connect this initial AudioWorkletProcessor
// to the audio graph to do any audio processing.
class BootstrapMessages extends AudioWorkletProcessor {
  constructor(arg) {
    super();
    startWasmWorker(arg.processorOptions)
    // Listen to messages from the main thread. These messages will ask this
    // scope to create the real AudioWorkletProcessors that call out to Wasm to
    // do audio processing.
    if (!(port instanceof MessagePort)) {
      this.port.onmessage = port.onmessage;
      /** @suppress {checkTypes} */
      port = this.port;
    }
  }

  // No-op, not doing audio processing in this processor. It is just for
  // receiving bootstrap messages.  However browsers require it to still be
  // present. It should never be called because we never add a node to the graph
  // with this processor, although it does look like Chrome does still call this
  // function.
  process() {
    // keep this function a no-op. Chrome redundantly wants to call this even
    // though this processor is never added to the graph.
  }
};

// Register the dummy processor that will just receive messages.
registerProcessor('em-bootstrap', BootstrapMessages);
#endif

port.onmessage = async (msg) => {
#if MINIMAL_RUNTIME
  // Wait for the module instantiation before processing messages.
  await instantiatePromise;
#endif
  let d = msg.data;
  if (d['_boot']) {
    startWasmWorker(d);
#if WEBAUDIO_DEBUG
    console.log('AudioWorklet global scope looks like this:');
    console.dir(globalThis);
#endif
  } else if (d['_wpn']) {
    // '_wpn' is short for 'Worklet Processor Node', using an identifier
    // that will never conflict with user messages
    // Register a real AudioWorkletProcessor that will actually do audio processing.
#if AUDIO_WORKLET_SUPPORT_AUDIO_PARAMS
    registerProcessor(d['_wpn'], createWasmAudioWorkletProcessor(d.audioParams));
#else
    registerProcessor(d['_wpn'], createWasmAudioWorkletProcessor());
#endif
#if WEBAUDIO_DEBUG
    console.log(`Registered a new WasmAudioWorkletProcessor "${d['_wpn']}" with AudioParams: ${d.audioParams}`);
#endif
    // Post a Wasm Call message back telling that we have now registered the
    // AudioWorkletProcessor, and should trigger the user onSuccess callback
    // of the emscripten_create_wasm_audio_worklet_processor_async() call.
    //
    // '_wsc' is short for 'wasm call', using an identifier that will never
    // conflict with user messages.
    //
    // Note: we convert the pointer arg manually here since the call site
    // ($_EmAudioDispatchProcessorCallback) is used with various signatures
    // and we do not know the types in advance.
    port.postMessage({'_wsc': d.callback, args: [d.contextHandle, 1/*EM_TRUE*/, {{{ to64('d.userData') }}}] });
  } else if (d['_wsc']) {
    getWasmTableEntry(d['_wsc'])(...d.args);
  };
}

} // ENVIRONMENT_IS_AUDIO_WORKLET
