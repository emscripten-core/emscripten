#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/webaudio.h>
#include <emscripten/threading.h>
#include <stdatomic.h>

#ifdef REPORT_RESULT
volatile int audioProcessedCount = 0;
int valueAfterDestroy;
#endif

EMSCRIPTEN_AUDIO_WORKLET_NODE_T node_id;

bool ProcessAudio(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData) {
#ifdef REPORT_RESULT
  ++audioProcessedCount;
#endif

  // Produce noise in all output channels.
  for(int i = 0; i < numOutputs; ++i)
    for(int j = 0; j < outputs[i].samplesPerChannel*outputs[i].numberOfChannels; ++j)
      outputs[i].data[j] = (rand() / (float)RAND_MAX * 2.0f - 1.0f) * 0.3f;

  return true;
}

void observe_after_destroy(void * userData) {
  printf("Expected processed count to be %d, was %d\n", valueAfterDestroy, audioProcessedCount);

#ifdef REPORT_RESULT
  if (audioProcessedCount == valueAfterDestroy) {
      printf("Test PASSED!\n");
      REPORT_RESULT(0);
  } else {
    printf("Test FAILED!\n");
    REPORT_RESULT(1);
  }
#endif
}

void AudioWorkletDestroyed(void* userData) {
  emscripten_out("AudioWorkletDestroyed");
#ifdef REPORT_RESULT
  valueAfterDestroy = audioProcessedCount;
#endif
  emscripten_set_timeout(observe_after_destroy, 1000, 0);
}

void observe_after_start(void *userData) {
#ifdef REPORT_RESULT
  if (audioProcessedCount == 0) {
    printf("Test FAILED!\n");
    REPORT_RESULT(1);
  }
#endif

  emscripten_destroy_web_audio_node_async(node_id, &AudioWorkletDestroyed, 0);
}

// This callback will fire after the Audio Worklet Processor has finished being
// added to the Worklet global scope.
void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  if (!success) return;

  emscripten_out("AudioWorkletProcessorCreated");

  // Specify the input and output node configurations for the Wasm Audio
  // Worklet. A simple setup with single mono output channel here, and no
  // inputs.
  int outputChannelCounts[1] = { 1 };

  EmscriptenAudioWorkletNodeCreateOptions options = {
    .numberOfInputs = 0,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };

  // Instantiate the counter-incrementer Audio Worklet Processor.
  node_id = emscripten_create_wasm_audio_worklet_node(audioContext, "counter-incrementer", &options, &ProcessAudio, 0);
  emscripten_audio_node_connect(node_id, audioContext, 0, 0);

  // Wait 1s to check that the counter has started incrementing
  emscripten_set_timeout(observe_after_start, 1000, 0);
}

// This callback will fire when the audio worklet thread has been initialized.
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  if (!success) return;

  emscripten_out("WebAudioWorkletThreadInitialized");

  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "counter-incrementer",
  };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, 0);
}

uint8_t wasmAudioWorkletStack[4096];

int main() {
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(NULL);

  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);

  return 0;
}
