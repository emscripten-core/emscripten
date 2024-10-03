#include <emscripten/webaudio.h>
#include <emscripten/threading.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Tests that
// - _emscripten_thread_supports_atomics_wait() returns true in a Wasm Audio Worklet.
// - emscripten_futex_wake() does not crash in a Wasm Audio Worklet.
// - emscripten_futex_wait() does not crash in a Wasm Audio Worklet.
// - emscripten_get_now() does not crash in a Wasm Audio Worklet.

int futexLocation = 0;
int testSuccess = 0;

bool ProcessAudio(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData) {
  int supportsAtomicWait = _emscripten_thread_supports_atomics_wait();
  printf("supportsAtomicWait: %d\n", supportsAtomicWait);
  assert(!supportsAtomicWait);
  emscripten_futex_wake(&futexLocation, 1);
  printf("%f\n", emscripten_get_now());

  emscripten_futex_wait(&futexLocation, 1, /*maxWaitMs=*/2);
  testSuccess = 1;

  return false;
}

EM_JS(void, InitHtmlUi, (EMSCRIPTEN_WEBAUDIO_T audioContext), {
  let startButton = document.createElement('button');
  startButton.innerHTML = 'Start playback';
  document.body.appendChild(startButton);

  audioContext = emscriptenGetAudioObject(audioContext);
  startButton.onclick = () => {
    audioContext.resume();
  };
});

bool PollTestSuccess(double, void *) {
  if (testSuccess) {
    printf("Test success!\n");
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    return false;
  }
  return true;
}

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  int outputChannelCounts[1] = { 1 };
  EmscriptenAudioWorkletNodeCreateOptions options = { .numberOfInputs = 0, .numberOfOutputs = 1, .outputChannelCounts = outputChannelCounts };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "noise-generator", &options, &ProcessAudio, 0);
  emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);
  InitHtmlUi(audioContext);
}

void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  WebAudioWorkletProcessorCreateOptions opts = { .name = "noise-generator" };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, 0);
}

uint8_t wasmAudioWorkletStack[4096];

int main() {
  emscripten_set_timeout_loop(PollTestSuccess, 10, 0);
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0);
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);
}
