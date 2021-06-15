// Register our audio processors if the code loads in an AudioWorkletGlobalScope
if (typeof AudioWorkletGlobalScope === "function") {
  class TestProcessor extends AudioWorkletProcessor {
    constructor() {
      super();
      Module["_signalTestSuccess"]();
    }

    // We need a dummy process method otherwise `registerProcessor` fails
    process(inputs, outputs, parameters) {
      return true;
    }
  }

  // Register the processor as per the audio worklet spec
  registerProcessor('test-processor', TestProcessor);
}