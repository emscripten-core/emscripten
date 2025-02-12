#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

// Tests two stereo audio inputs being copied to two stereo outputs.

// This needs to be big enough for the 2x stereo outputs, 2x inputs and the worker stack
#define AUDIO_STACK_SIZE 6144

// Shared file playback and bootstrap
#include "audioworklet_test_shared.inc"

// Callback to process and copy the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
  audioProcessedCount++;

  // Twin stereo in and out
  assert(numInputs == 2 && numOutputs == 2);
  assert(inputs[0].numberOfChannels == 2 && inputs[1].numberOfChannels == 2);
  assert(outputs[0].numberOfChannels == 2 && outputs[1].numberOfChannels == 2);
  // All with the same number of samples
  assert(inputs[0].samplesPerChannel == inputs[1].samplesPerChannel);
  assert(inputs[0].samplesPerChannel == outputs[0].samplesPerChannel);
  assert(outputs[0].samplesPerChannel == outputs[1].samplesPerChannel);
  // Now with all known quantities we can memcpy the data
  int totalSamples = outputs[0].samplesPerChannel * outputs[0].numberOfChannels;
  memcpy(outputs[0].data, inputs[0].data, totalSamples * sizeof(float));
  memcpy(outputs[1].data, inputs[1].data, totalSamples * sizeof(float));
  return true;
}

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  if (!success) {
    printf("Audio worklet node creation failed\n");
    return;
  }
  printf("Audio worklet processor created\n");
  printf("Click to toggle audio playback\n");

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

  // Register the counter that exits the test after one second of mixing
  emscripten_set_timeout_loop(&playedAndMixed, 16, NULL);
}
