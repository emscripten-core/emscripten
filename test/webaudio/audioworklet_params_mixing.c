#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

// Tests processing two stereo audio inputs being mixed to a single stereo audio
// output in process() (by adding the inputs together).

// This needs to be big enough for the stereo output, 2x inputs, 2x params and the worker stack
#define AUDIO_STACK_SIZE 5120

// Shared file playback and bootstrap
#include "audioworklet_test_shared.inc"

// Callback to process and mix the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* __unused params, void* __unused data) {
  audioProcessedCount++;

  // Single stereo output
  assert(numOutputs == 1 && outputs[0].numberOfChannels == 2);
  int outSamplesPerChannel = outputs[0].samplesPerChannel;
  for (int n = 0; n < numInputs; n++) {
    // And all inputs are also stereo
    assert(inputs[n].numberOfChannels == 2 || inputs[n].numberOfChannels == 0);
    // This should always be the case
    assert(inputs[n].samplesPerChannel == outSamplesPerChannel);
  }
  // Interestingly, params varies per browser. Chrome won't have a length > 1
  // unless the value changes, and FF has all the samples even for a k-rate
  // parameter. The only given is that two params are incoming.
  assert(numParams = 2);
  assert(params[0].length == 1 || params[0].length == outSamplesPerChannel);
  assert(params[1].length == 1 || params[1].length == outSamplesPerChannel);
  // We can now do a quick mix since we know the layouts
  if (numInputs > 0) {
    int totalSamples = outSamplesPerChannel * outputs[0].numberOfChannels;
    // Simple copy of single input's audio data, checking that we have channels
    // (since a muted input has zero channels)
    float* outputData = outputs[0].data;
    if (inputs[0].numberOfChannels != 0) {
      memcpy(outputData, inputs[0].data, totalSamples * sizeof(float));
    } else {
      // And for muted we need to full the buffer with zeroes otherwise it play the previous frame
      memset(outputData, 0, totalSamples * sizeof(float));
    }
    // Grab the mix level parameter (with either a length of 1 or the samples per channel)
    const AudioParamFrame* mixLevel = &params[0];
    for (int n = 1; n < numInputs; n++) {
      if (inputs[n].numberOfChannels != 0) {
        float* inputData = inputs[n].data;
        for (int i = totalSamples - 1; i >= 0; i--) {
          // Output and input buffers are stereo planar in this example so we
          // need to get a mixLevel->data[] per channel, hence the quick % (and
          // as noticed in the wild, implementations have either one or all
          // entries, regardless of the param spec we passed in)
          float mixLevelValue = mixLevel->data[(mixLevel->length > 1) ? (i % outSamplesPerChannel) : 0];
          outputData[i] += inputData[i] * mixLevelValue;
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
    // name. Chrome and FF work with 0 but Safari requires the correct "0".
    var param = worklet.parameters.get("0");
    if (param) {
      param.setTargetAtTime((param.value > 0.5) ? 0 : 1, 0 /* same as context.currentTime */, 0.5);
    }
  }
});

// Registered keypress event to call the JS doFade()
bool onPress(int __unused type, const EmscriptenKeyboardEvent* e, void* data) {
  if (!e->repeat && data) {
    printf("Toggling fade\n");
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
  printf("Keypress to fade the beat in or out\n");

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
    emscripten_audio_node_connect(beatID, worklet, 0, 1);
  }
  bassID = createTrack(context, "audio_files/emscripten-bass.mp3", true);
  if (bassID) {
    emscripten_audio_node_connect(bassID, worklet, 0, 0);
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
