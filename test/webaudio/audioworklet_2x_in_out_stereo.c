#include <assert.h>
#include <string.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

// Tests two stereo audio inputs being copied to two stereo outputs.

// This needs to be big enough for the 2x stereo outputs, 2x inputs and the worker stack
#define AUDIO_STACK_SIZE 6144

// Shared file playback and bootstrap
#include "audioworklet_test_shared.inc"

// Callback to process and copy the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
#ifdef TEST_AND_EXIT
  audioProcessedCount++;
#endif

  // Twin stereo in and out
  assert(numInputs == 2 && numOutputs == 2);
  assert(inputs[0].numberOfChannels == 0 || inputs[0].numberOfChannels == 2);
  assert(inputs[1].numberOfChannels == 0 || inputs[1].numberOfChannels == 2);
  assert(outputs[0].numberOfChannels == 2);
  assert(outputs[1].numberOfChannels == 2);
  // All with the same number of samples
  assert(inputs[0].samplesPerChannel == inputs[1].samplesPerChannel);
  assert(inputs[0].samplesPerChannel == outputs[0].samplesPerChannel);
  assert(outputs[0].samplesPerChannel == outputs[1].samplesPerChannel);
  // Now with all known quantities we can memcpy all the data (or zero it if the
  // channels are disabled)
  int totalBytes = outputs[0].samplesPerChannel * outputs[0].numberOfChannels * sizeof(float);
  if (inputs[0].numberOfChannels > 0) {
    memcpy(outputs[0].data, inputs[0].data, totalBytes);
  } else {
    memset(outputs[0].data, 0, totalBytes);
  }
  if (inputs[1].numberOfChannels > 0) {
    memcpy(outputs[1].data, inputs[1].data, totalBytes);
  } else {
    memset(outputs[1].data, 0, totalBytes);
  }
  return true;
}

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  assert(success && "Audio worklet failed in processorCreated()");
  emscripten_out("Audio worklet processor created");
  emscripten_out("Click to toggle audio playback");

  // Two stereo outputs, two inputs
  int outputChannelCounts[2] = { 2, 2 };
  EmscriptenAudioWorkletNodeCreateOptions opts = {
    .numberOfInputs  = 2,
    .numberOfOutputs = 2,
    .outputChannelCounts = outputChannelCounts
  };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(context, "mixer", &opts, &process, NULL);
  // Both outputs connected to the context
  emscripten_audio_node_connect(worklet, context, 0, 0);
  emscripten_audio_node_connect(worklet, context, 1, 0);

  // Create the two stereo source nodes and connect them to the two inputs
  // Note: we can connect the sources to the same input and it'll get mixed for us, but that's not the point
  beatID = createTrack(context, "audio_files/emscripten-beat.mp3", true);
  if (beatID) {
    emscripten_audio_node_connect(beatID, worklet, 0, 0);
  }
  bassID = createTrack(context, "audio_files/emscripten-bass.mp3", true);
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
