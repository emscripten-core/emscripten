/**
 * This is the JS side of the AudioWorklet processing that creates our
 * AudioWorkletProcessor that fetches the audio data from native code and 
 * copies it into the output buffers.
 * 
 * This is intentionally not made part of Emscripten AudioWorklet integration
 * because apps will usually want to a lot of control here (formats, channels, 
 * additional processors etc.)
 */

// Register our audio processors if the code loads in an AudioWorkletGlobalScope
if (typeof AudioWorkletGlobalScope === "function") {
  // This processor node is a simple proxy to the audio generator in native code.
  // It calls the native function then copies the samples into the output buffer
  class NativePassthroughProcessor extends AudioWorkletProcessor {
    process(inputs, outputs, parameters) {
      const output = outputs[0];
      const numSamples = output[0].length;
      
      // Run the native audio generator function
      const mem = Module["_generateAudio"](numSamples);

      // Copy the results into the output buffer, float-by-float deinterleaving the data
      let curSrc = mem/4;
      const chL = output[0];
      const chR = output[1];
      for (let s = 0; s < numSamples; ++s) {
        chL[s] = Module.HEAPF32[curSrc++];
        chR[s] = Module.HEAPF32[curSrc++];
      }
    
      return true;
    }
  }

  // Register the processor as per the audio worklet spec
  registerProcessor('native-passthrough-processor', NativePassthroughProcessor);
}