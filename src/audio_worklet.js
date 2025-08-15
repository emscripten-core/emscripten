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
      // context that created this processor. Note for when moving to Web Audio
      // 1.1: the typed array passed to process() should be the same size as this
      // 'render quantum size', and this exercise of passing in the value
      // shouldn't be required (to be verified)
      this.samplesPerChannel = opts.samplesPerChannel;
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

      // Calculate how much stack space is needed.
      var bytesPerChannel = this.samplesPerChannel * {{{ getNativeTypeSize('float') }}};
      var stackMemoryNeeded = (numInputs + numOutputs) * {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
      var numParams = 0;
      for (entry of inputList) stackMemoryNeeded += entry.length * bytesPerChannel;
      for (entry of outputList) stackMemoryNeeded += entry.length * bytesPerChannel;
      for (entry in parameters) {
        stackMemoryNeeded += parameters[entry].byteLength + {{{ C_STRUCTS.AudioParamFrame.__size__ }}};
        ++numParams;
      }

      // Allocate the necessary stack space.
      var oldStackPtr = stackSave();
      var inputsPtr = stackAlloc(stackMemoryNeeded);

      // Copy input audio descriptor structs and data to Wasm ('structPtr' is
      // reused as the working start to each struct record, 'dataPtr' start of
      // the data section, usually after all structs).
      var structPtr = inputsPtr;
      var dataPtr = inputsPtr + numInputs * {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
      for (entry of inputList) {
        // Write the AudioSampleFrame struct instance
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.numberOfChannels, 'entry.length', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.samplesPerChannel, 'this.samplesPerChannel', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.data, 'dataPtr', '*') }}};
        structPtr += {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
        // Marshal the input audio sample data for each audio channel of this input
        for (subentry of entry) {
          HEAPF32.set(subentry, {{{ getHeapOffset('dataPtr', 'float') }}});
          dataPtr += bytesPerChannel;
        }
      }

      // Copy output audio descriptor structs to Wasm
      var outputsPtr = dataPtr;
      structPtr = outputsPtr;
      var outputDataPtr = (dataPtr += numOutputs * {{{ C_STRUCTS.AudioSampleFrame.__size__ }}});
      for (entry of outputList) {
        // Write the AudioSampleFrame struct instance
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.numberOfChannels, 'entry.length', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.samplesPerChannel, 'this.samplesPerChannel', 'u32') }}};
        {{{ makeSetValue('structPtr', C_STRUCTS.AudioSampleFrame.data, 'dataPtr', '*') }}};
        structPtr += {{{ C_STRUCTS.AudioSampleFrame.__size__ }}};
        // Reserve space for the output data
        dataPtr += bytesPerChannel * entry.length;
      }

      // Copy parameters descriptor structs and data to Wasm
      var paramsPtr = dataPtr;
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

      // Call out to Wasm callback to perform audio processing
      var didProduceAudio = this.callback(numInputs, inputsPtr, numOutputs, outputsPtr, numParams, paramsPtr, this.userData);
      if (didProduceAudio) {
        // Read back the produced audio data to all outputs and their channels.
        // (A garbage-free function TypedArray.copy(dstTypedArray, dstOffset,
        // srcTypedArray, srcOffset, count) would sure be handy..  but web does
        // not have one, so manually copy all bytes in)
        outputDataPtr = {{{ getHeapOffset('outputDataPtr', 'float') }}};
        for (entry of outputList) {
          for (subentry of entry) {
            // repurposing structPtr for now
            for (structPtr = 0; structPtr < this.samplesPerChannel; ++structPtr) {
              subentry[structPtr] = HEAPF32[outputDataPtr++];
            }
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
