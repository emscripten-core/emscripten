#include <assert.h>
#include <string.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

// Tests processing two stereo audio inputs being mixed to a single stereo audio
// output in process(), then applying a fade from the parameters. Since this can
// create variable parameter data sizes, depending on the browser, it's also the
// ideal to test audio worklets don't corrupt TLS variables.

// This needs to be big enough for the stereo output, 2x inputs, 2x params and
// the worker stack. To note that different browsers have different stack size
// requirement (see notes in process() plus the expansion of the params).
#define AUDIO_STACK_SIZE 6144

// Shared file playback and bootstrap
#include "audioworklet_test_shared.inc"

// TLS test value to verify the JS-side stays within its stack frame (this is
// zeroed in the *main* thread on startup).
__thread int tlsTest = 0x1337D00D;

// Callback to process and mix the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
#ifdef TEST_AND_EXIT
  audioProcessedCount++;
#endif

  // JS-setup code shouldn't stomp on this, plus the main thread didn't clear it
  assert(tlsTest == 0x1337D00D);
  // Single stereo output
  assert(numOutputs == 1);
  assert(outputs[0].numberOfChannels == 2);
  int outSamplesPerChannel = outputs[0].samplesPerChannel;
  for (int n = 0; n < numInputs; n++) {
    // And all inputs are also stereo (or disabled)
    assert(inputs[n].numberOfChannels == 2 || inputs[n].numberOfChannels == 0);
    // This should always be the case
    assert(inputs[n].samplesPerChannel == outSamplesPerChannel);
  }
  // Interestingly, params varies per browser. Chrome won't have a length > 1
  // unless the value changes, and FF has all 128 entries even for a k-rate
  // parameter. The only given for this test is that two params are incoming:
  assert(numParams == 2);
  assert(params[0].length == 1 || params[0].length == outSamplesPerChannel);
  assert(params[1].length == 1 || params[1].length == outSamplesPerChannel);
  // We can now do a quick mix since we know the layouts
  if (numInputs > 0) {
    int totalSamples = outSamplesPerChannel * outputs[0].numberOfChannels;
    // Simple copy of the first input's audio data, checking that we have
    // channels (since a muted input has zero channels).
    float* outputData = outputs[0].data;
    if (inputs[0].numberOfChannels > 0) {
      memcpy(outputData, inputs[0].data, totalSamples * sizeof(float));
    } else {
      // And for muted we need to fill the buffer with zeroes otherwise it repeats the last frame
      memset(outputData, 0, totalSamples * sizeof(float));
    }
    // Grab the mix level parameter and expand it to have one entry per output
    // sample. This simplifies the mixer and smooths out browser differences.
    // Output and input buffers are stereo planar, so the mix data just repeats.
    float* const mixLevel = alloca(totalSamples * sizeof(float));
    if (params[0].length > 1) {
      // This is the regular path, one entry per sample by number of channels
      for (int ch = outputs[0].numberOfChannels - 1; ch >= 0; ch--) {
        memcpy(mixLevel + ch * outSamplesPerChannel, params[0].data, outSamplesPerChannel * sizeof(float));
      }
    } else {
      // Chrome will take this path when the k-rate parameter doesn't change
      float singleLevel = params[0].data[0];
      for (int n = totalSamples - 1; n >= 0; n--) {
        mixLevel[n] = singleLevel;
      }
    }
    // Now add another inputs with the mix level
    for (int n = 1; n < numInputs; n++) {
      if (inputs[n].numberOfChannels > 0) {
        float* inputData = inputs[n].data;
        for (int i = totalSamples - 1; i >= 0; i--) {
          outputData[i] += inputData[i] * mixLevel[i];
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
})

// Registered keypress event to call the JS doFade()
bool onPress(int type, const EmscriptenKeyboardEvent* e, void* data) {
  if (!e->repeat && data) {
    emscripten_out("Toggling fade");
    doFade(VOIDP_2_WA(data));
  }
  return false;
}

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  assert(success && "Audio worklet failed in processorCreated()");
  emscripten_out("Audio worklet processor created");
  emscripten_out("Click to toggle audio playback");
  emscripten_out("Keypress to fade the beat in or out");

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

#ifdef TEST_AND_EXIT
  // Register the counter that exits the test after one second of mixing
  emscripten_set_timeout_loop(&playedAndMixed, 16, NULL);
#endif
}

// Worklet thread inited, now create the audio processor
void initialisedWithParams(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  assert(success && "Audio worklet failed initialised()");
  emscripten_out("Audio worklet initialised");

  // Clear the TLS variable (from the main thread)
  tlsTest = 0;
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
EmscriptenStartWebAudioWorkletCallback getStartCallback(void) {
  return &initialisedWithParams;
}
