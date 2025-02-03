#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

// Tests processing two stereo audio inputs being mixed to a single stereo audio
// output in process() (by adding the inputs together).

// This needs to be big enough for the stereo output, 2x inputs and the worker stack
#define AUDIO_STACK_SIZE 4096

// Shared file playback and bootstrap
#include "audioworklet_test_shared.inc"

// Callback to process and mix the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* __unused params, void* __unused data) {
  audioProcessedCount++;

  // Single stereo output
  assert(numOutputs == 1 && outputs[0].numberOfChannels == 2);
  for (int n = 0; n < numInputs; n++) {
    // And all inputs are also stereo
    assert(inputs[n].numberOfChannels == 2 || inputs[n].numberOfChannels == 0);
    // This should always be the case
    assert(inputs[n].samplesPerChannel == outputs[0].samplesPerChannel);
  }
  // Interestingly, params won't have a length > 1 unless the value changes, but
  // we do know two params are incoming
  assert(numParams = 2);
  // We can now do a quick mix since we know the layouts
  if (numInputs > 0) {
    int totalSamples = outputs[0].samplesPerChannel * outputs[0].numberOfChannels;
    float* outputData = outputs[0].data;
    memcpy(outputData, inputs[0].data, totalSamples * sizeof(float));
    for (int n = 1; n < numInputs; n++) {
      // It's possible to have an input with no channels
      if (inputs[n].numberOfChannels == 2) {
        float* inputData = inputs[n].data;
        for (int i = totalSamples - 1; i >= 0; i--) {
          outputData[i] += inputData[i] * params[0].data[(params[0].length > 1) ? i : 0]; // world's worst mixer...
        }
      }
    }
  }
  return true;
}

// Grabs the known worklet parameter and fades in or out (depending on whether
// it's already fading up or down, we reverse the fade direction).
EM_JS(void, doFade, (EMSCRIPTEN_AUDIO_WORKLET_NODE_T workletID), {
  var worklet = emscriptenGetAudioObject(workletID);
  if (worklet) {
    // Emscripten's API creates these from a C array, indexing them instead of a
    // name, so technically 0 is "0" but we might as well use numerical indices.
    var param = worklet.parameters.get(0);
    if (param) {
      param.setTargetAtTime((param.value > 0.5) ? 0 : 1, 0 /* same as context.currentTime */, 0.5);
    }
  }
});

// Registered keypress event to call the JS doFade()
bool onPress(int __unused type, const EmscriptenKeyboardEvent* e, void* data) {
  if (!e->repeat && data) {
    doFade(VOIDP_2_WA(data));
  }
  return false;
}

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* __unused data) {
  if (!success) {
    printf("Audio worklet node creation failed\n");
    return;
  }
  printf("Audio worklet processor created\n");
  printf("Click to toggle audio playback\n");
  printf("Keypress to fade the baseline in or out\n");

  // Stereo output, two inputs
  int outputChannelCounts[1] = { 2 };
  EmscriptenAudioWorkletNodeCreateOptions opts = {
    .numberOfInputs  = 2,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(context, "mixer", &opts, &process, NULL);
  emscripten_audio_node_connect(worklet, context, 0, 0);

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
  // And a keypress to do affect the fader
  emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, WA_2_VOIDP(worklet), false, &onPress);

  // Register the counter that exits the test after one second of mixing
  emscripten_set_timeout_loop(&playedAndMixed, 16, NULL);
}

// Worklet thread inited, now create the audio processor
void initialisedWithParams(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* __unused data) {
  if (!success) {
    printf("Audio worklet failed to initialise\n");
    return;
  }
  printf("Audio worklet initialised\n");

  // Custom audio params we'll use as a fader
  WebAudioParamDescriptor faderParam[] = {
    {
      // This a-rate (one entry per sample) is used to set the mix level
      .defaultValue = 1.0f,
      .minValue = 0.0f,
      .maxValue = 1.0f,
      .automationRate = WEBAUDIO_PARAM_A_RATE
    }, {
      // This k-rate (one entry per frame) is used just to test
      .defaultValue = 0.0f,
      .minValue = -100.0f,
      .maxValue = 100.0f,
      .automationRate = WEBAUDIO_PARAM_K_RATE
    }
  };
  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "mixer",
    .numAudioParams = 2,
    .audioParamDescriptors = faderParam
  };
  emscripten_create_wasm_audio_worklet_processor_async(context, &opts, &processorCreated, NULL);
}

// This implementation has no custom start-up requirements
EmscriptenStartWebAudioWorkletCallback getStartCallback() {
  return &initialisedWithParams;
}
