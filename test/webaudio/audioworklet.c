#include <emscripten/webaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Steps to use Wasm-based AudioWorklets:
  1. Create a Web Audio AudioContext either via manual JS code and calling
     emscriptenRegisterAudioObject() from JS, or by calling
     emscripten_create_audio_context() (shown in this sample)
  2. Initialize a Wasm AudioWorklet scope on the audio context by calling
     emscripten_start_wasm_audio_worklet_thread_async(). This shares the Wasm
     Module, Memory, etc. to the AudioWorklet scope, and establishes the stack
     space for the Audio Worklet.
     This needs to be called exactly once during page's lifetime. There is no
     mechanism in Web Audio to shut down/uninitialize the scope.
  3. Create one or more of Audio Worklet Processors with the desired name and
     AudioParam configuration.
  4. Instantiate Web Audio audio graph nodes from the above created worklet
     processors, specifying the desired input-output configurations and Wasm-side
     function callbacks to call for each node.
  5. Add the graph nodes to the Web Audio graph, and the audio callbacks should
     begin to fire.
*/

// REPORT_RESULT is defined when running in Emscripten test harness. You can
// strip these out in your own project.
#ifdef REPORT_RESULT
_Thread_local int testTlsVariable = 1;
int lastTlsVariableValueInAudioThread = 1;
#endif

// This function will be called for every fixed-size buffer of audio samples to be processed.
bool ProcessAudio(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData) {
#ifdef REPORT_RESULT
  assert(testTlsVariable == lastTlsVariableValueInAudioThread);
  ++testTlsVariable;
  lastTlsVariableValueInAudioThread = testTlsVariable;
  assert(emscripten_current_thread_is_audio_worklet());
#endif

  // Produce noise in all output channels.
  for(int i = 0; i < numOutputs; ++i)
    for(int j = 0; j < outputs[i].samplesPerChannel*outputs[i].numberOfChannels; ++j)
      outputs[i].data[j] = (rand() / (float)RAND_MAX * 2.0f - 1.0f) * 0.3f;

  // We generated audio and want to keep this processor going. Return false here to shut down.
  return true;
}

EM_JS(void, InitHtmlUi, (EMSCRIPTEN_WEBAUDIO_T audioContext), {
  // Add a button on the page to toggle playback as a response to user click.
  let startButton = document.createElement('button');
  startButton.innerHTML = 'Toggle playback';
  document.body.appendChild(startButton);

  audioContext = emscriptenGetAudioObject(audioContext);
  startButton.onclick = () => {
    if (audioContext.state != 'running') {
      audioContext.resume();
    } else {
      audioContext.suspend();
    }
  };
});

#ifdef REPORT_RESULT
bool main_thread_tls_access(double time, void *userData) {
  // Try to mess the TLS variable on the main thread, with the expectation that
  // it should not change the TLS value on the AudioWorklet thread.
  testTlsVariable = (int)time;
  if (lastTlsVariableValueInAudioThread >= 100) {
    REPORT_RESULT(0);
    return false;
  }
  return true;
}
#endif

// This callback will fire after the Audio Worklet Processor has finished being
// added to the Worklet global scope.
void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  if (!success) return;

  // Specify the input and output node configurations for the Wasm Audio
  // Worklet. A simple setup with single mono output channel here, and no
  // inputs.
  int outputChannelCounts[1] = { 1 };

  EmscriptenAudioWorkletNodeCreateOptions options = {
    .numberOfInputs = 0,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };

  // Instantiate the noise-generator Audio Worklet Processor.
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "noise-generator", &options, &ProcessAudio, 0);
  // Connect the audio worklet node to the graph.
  emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);

#ifdef REPORT_RESULT
  emscripten_set_timeout_loop(main_thread_tls_access, 10, 0);
#endif

  InitHtmlUi(audioContext);
}

// This callback will fire when the Wasm Module has been shared to the
// AudioWorklet global scope, and is now ready to begin adding Audio Worklet
// Processors.
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  if (!success) return;

  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "noise-generator",
  };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, 0);
}

// Define a global stack space for the AudioWorkletGlobalScope. Note that all
// AudioWorkletProcessors and/or AudioWorkletNodes on the given Audio Context
// all share the same AudioWorkerGlobalScope, i.e. they all run on the same one
// audio thread (multiple nodes/processors do not each get their own thread).
// Hence one stack is enough.
uint8_t wasmAudioWorkletStack[4096];

int main() {
  srand(time(NULL));

  assert(!emscripten_current_thread_is_audio_worklet());

  // Create an audio context
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0 /* use default constructor options */);

  // and kick off Audio Worklet scope initialization, which shares the Wasm
  // Module and Memory to the AudioWorklet scope and initializes its stack.
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);
}
