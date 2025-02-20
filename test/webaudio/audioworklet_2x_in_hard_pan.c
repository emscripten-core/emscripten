#include <assert.h>
#include <string.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

// Tests two mono audio inputs being copied to the left and right channels of a
// single stereo output (with a hard pan).

// This needs to be big enough for the stereo output, 2x mono inputs and the worker stack
#define AUDIO_STACK_SIZE 3072

// Shared file playback and bootstrap
#include "audioworklet_test_shared.inc"

// Callback to process and copy the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
#ifdef TEST_AND_EXIT
  audioProcessedCount++;
#endif

  // Twin mono in (or disabled), single stereo out
  assert(numInputs == 2 && numOutputs == 1);
  assert(inputs[0].numberOfChannels == 0 || inputs[0].numberOfChannels == 1);
  assert(inputs[1].numberOfChannels == 0 || inputs[1].numberOfChannels == 1);
  assert(outputs[0].numberOfChannels == 2);
  // All with the same number of samples
  assert(inputs[0].samplesPerChannel == inputs[1].samplesPerChannel);
  assert(inputs[0].samplesPerChannel == outputs[0].samplesPerChannel);
  // Now with all known quantities we can memcpy the L&R data (or zero it if the
  // channels are disabled)
  int bytesPerChannel = outputs[0].samplesPerChannel * sizeof(float);
  float* outputData = outputs[0].data;
  if (inputs[0].numberOfChannels > 0) {
    memcpy(outputData, inputs[0].data, bytesPerChannel);
  } else {
    memset(outputData, 0, bytesPerChannel);
  }
  outputData += outputs[0].samplesPerChannel;
  if (inputs[1].numberOfChannels > 0) {
    memcpy(outputData, inputs[1].data, bytesPerChannel);
  } else {
    memset(outputData, 0, bytesPerChannel);
  }
  return true;
}

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  assert(success && "Audio worklet failed in processorCreated()");
  emscripten_out("Audio worklet processor created");
  emscripten_out("Click to toggle audio playback");

  // Stereo output, two inputs
  int outputChannelCounts[2] = { 2 };
  EmscriptenAudioWorkletNodeCreateOptions opts = {
    .numberOfInputs  = 2,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(context, "mixer", &opts, &process, NULL);
  emscripten_audio_node_connect(worklet, context, 0, 0);

  // Create the two mono source nodes and connect them to the two inputs
  // Note: we can connect the sources to the same input and it'll get mixed for us, but that's not the point
  beatID = createTrack(context, "audio_files/emscripten-beat-mono.mp3", true);
  if (beatID) {
    emscripten_audio_node_connect(beatID, worklet, 0, 0);
  }
  bassID = createTrack(context, "audio_files/emscripten-bass-mono.mp3", true);
  if (bassID) {
    emscripten_audio_node_connect(bassID, worklet, 0, 1);
  }

  // Register a click to start playback
  emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, WA_2_VOIDP(context), false, &onClick);

#ifdef TEST_AND_EXIT
  // Register the counter that exits the test after one second of playback
  emscripten_set_timeout_loop(&playedAndMixed, 16, NULL);
#endif
}

// This implementation has no custom start-up requirements
EmscriptenStartWebAudioWorkletCallback getStartCallback(void) {
  return &initialised;
}
