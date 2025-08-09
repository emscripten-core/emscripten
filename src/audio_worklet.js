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

if (ENVIRONMENT_IS_AUDIO_WORKLET) {

function createWasmAudioWorkletProcessor(audioParams) {
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

      // Create up-front as many typed views for marshalling the output data as
      // may be required (with an arbitrary maximum of 16, for the case where a
      // multi-MB stack is passed), allocated at the *top* of the worklet's
      // stack (and whose addresses are fixed). The 'minimum alloc' firstly
      // stops STACK_OVERFLOW_CHECK failing (since the stack will be full, and
      // 16 being the minimum allocation size due to alignments) and leaves room
      // for a single AudioSampleFrame as a minumum.
      this.maxBuffers = Math.min(((wwParams.stackSize - /*minimum alloc*/ 16) / this.bytesPerChannel) | 0, /*sensible limit*/ 16);
#if ASSERTIONS
      console.assert(this.maxBuffers > 0, `AudioWorklet needs more stack allocating (at least ${this.bytesPerChannel})`);
#endif
      // These are still alloc'd to take advantage of the overflow checks, etc.
      var oldStackPtr = stackSave();
      var viewDataIdx = {{{ getHeapOffset('stackAlloc(this.maxBuffers * this.bytesPerChannel)', 'float') }}};
#if WEBAUDIO_DEBUG
      console.log(`AudioWorklet creating ${this.maxBuffers} buffer one-time views (for a stack size of ${wwParams.stackSize} at address 0x${(viewDataIdx * 4).toString(16)})`);
#endif
      this.outputViews = [];
      for (var n = this.maxBuffers; n > 0; n--) {
        // Added in reverse so the lowest indices are closest to the stack top
        this.outputViews.unshift(
          HEAPF32.subarray(viewDataIdx, viewDataIdx += this.samplesPerChannel)
        );
      }
      stackRestore(oldStackPtr);

#if ASSERTIONS
      // Explicitly verify this later in process()
      this.ctorOldStackPtr = oldStackPtr;
#endif
    }

    static get parameterDescriptors() {
      return audioParams;
    }

    /**
     * @param {Object} parameters
     */
    process(inputList, outputList, parameters) {
      // Marshal all inputs and parameters to the Wasm memory on the thread stack,
      // then perform the wasm audio worklet call,
      // and finally marshal audio output data back.

      var numInputs = inputList.length;
      var numOutputs = outputList.length;

      var entry; // reused list entry or index
      var subentry; // reused channel or other array in each list entry or index

      // Calculate the required stack and output buffer views
      var stackMemoryNeeded = (numInputs + numOutputs) * {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
      for (entry of inputList) {
        stackMemoryNeeded += entry.length * this.bytesPerChannel;
      }
      var outputViewsNeeded = 0;
      for (entry of outputList) {
        outputViewsNeeded += entry.length;
      }
      stackMemoryNeeded += outputViewsNeeded * this.bytesPerChannel;
      var numParams = 0;
      for (entry in parameters) {
        stackMemoryNeeded += parameters[entry].byteLength + {{{ C_STRUCTS.AudioParamFrame.__size__ }}};
        ++numParams;
      }
#if ASSERTIONS
      console.assert(outputViewsNeeded <= this.outputViews.length, `Too many AudioWorklet outputs (need ${outputViewsNeeded} but have stack space for ${this.outputViews.length})`);
#endif

      var oldStackPtr = stackSave();
      // Allocate the necessary stack space. All pointer variables are always in
      // bytes; 'dataPtr' is the start of the data section, advancing as space
      // for structs and data is taken; 'structPtr' is reused as the working
      // start to each struct record.
      // Ordinarily 'dataPtr' would be 16-byte aligned, from the internal
      // _emscripten_stack_alloc(), as were the output views, and so to ensure
      // the views fall on the correct addresses (and we finish at the stacktop)
      // bytes are added and the start advanced.
      var alignedNeededStack = (stackMemoryNeeded + 15) & ~15;
      var dataPtr = stackAlloc(alignedNeededStack);

      // Copy input audio descriptor structs and data to Wasm (recall, structs
      // first, audio data after). 'inputsPtr' is the start of the C callback's
      // input AudioSampleFrame.
      var /*const*/ inputsPtr = dataPtr;
      var structPtr = inputsPtr;
      dataPtr += numInputs * {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
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

      // Copy parameters descriptor structs and data to Wasm. 'paramsPtr' is the
      // start of the C callback's input AudioParamFrame.
      var /*const*/ paramsPtr = dataPtr;
      structPtr = paramsPtr;
      dataPtr += numParams * {{{ C_STRUCTS.AudioParamFrame.__size__ }}};
      for (entry = 0; subentry = parameters[entry++];) {
        // Write the AudioParamFrame struct instance
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioParamFrame.length, 'subentry.length', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioParamFrame.data, 'dataPtr', '*') }}};
        structPtr += {{{ C_STRUCTS.AudioParamFrame.__size__ }}};
        // Marshal the audio parameters array
        HEAPF32.set(subentry, {{{ getHeapOffset('dataPtr', 'float') }}});
        dataPtr += subentry.length * {{{ getNativeTypeSize('float') }}};
      }

      // TODO: why does Chrome wasm64 (Chrome has weird rules for params) need
      // the manual alignment here? An off-by-one somewhere? outputsPtr is
      // getting clobbered otherwise, is the struct correctly aligned? Do more
      // stack allocs instead? Probably needs alignments between struct writes?
      dataPtr += alignedNeededStack - stackMemoryNeeded;

      // Copy output audio descriptor structs to Wasm. 'outputsPtr' is the start
      // of the C callback's output AudioSampleFrame.
      // Note: dataPtr after the struct offsets should now be 16-byte aligned.
      var /*const*/ outputsPtr = dataPtr;
      structPtr = outputsPtr;
      dataPtr += numOutputs * {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
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
            console.assert(subentry.byteLength == this.bytesPerChannel, `AudioWorklet unexpected output buffer size (expected ${this.bytesPerChannel} got ${subentry.byteLength})`);
          }
        }
      }
#endif

      // Call out to Wasm callback to perform audio processing
      var didProduceAudio = this.callback(numInputs, inputsPtr, numOutputs, outputsPtr, numParams, paramsPtr, this.userData);
      if (didProduceAudio) {
        // Read back the produced audio data to all outputs and their channels.
        // The preallocated 'outputViews' already have the correct offsets and
        // sizes into the stack (recall from the ctor that they run backwards).
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

var messagePort;

// Specify a worklet processor that will be used to receive messages to this
// AudioWorkletGlobalScope.  We never connect this initial AudioWorkletProcessor
// to the audio graph to do any audio processing.
class BootstrapMessages extends AudioWorkletProcessor {
  constructor(arg) {
    super();
    startWasmWorker(arg.processorOptions)
#if WEBAUDIO_DEBUG
    console.log('AudioWorklet global scope looks like this:');
    console.dir(globalThis);
#endif
    // Listen to messages from the main thread. These messages will ask this
    // scope to create the real AudioWorkletProcessors that call out to Wasm to
    // do audio processing.
    messagePort = this.port;
    /** @suppress {checkTypes} */
    messagePort.onmessage = async (msg) => {
      let d = msg.data;
      if (d['_wpn']) {
        // '_wpn' is short for 'Worklet Processor Node', using an identifier
        // that will never conflict with user messages
        // Register a real AudioWorkletProcessor that will actually do audio processing.
        registerProcessor(d['_wpn'], createWasmAudioWorkletProcessor(d.audioParams));
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
        messagePort.postMessage({'_wsc': d.callback, args: [d.contextHandle, 1/*EM_TRUE*/, {{{ to64('d.userData') }}}] });
      } else if (d['_wsc']) {
        getWasmTableEntry(d['_wsc'])(...d.args);
      };
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

} // ENVIRONMENT_IS_AUDIO_WORKLET
