#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <emscripten/em_js.h>
#include <emscripten/webaudio.h>

// Tests two stereo audio inputs being copied to two stereo outputs.

// This needs to be big enough for the 2x stereo outputs, 2x inputs and the worker stack
#define AUDIO_STACK_SIZE 6144

// Helper for MEMORY64 to cast an audio context or type to a void*
#define WA_2_VOIDP(ctx) ((void*) (intptr_t) ctx)
// Helper for MEMORY64 to cast a void* to an audio context or type
#define VOIDP_2_WA(ptr) ((EMSCRIPTEN_WEBAUDIO_T) (intptr_t) ptr)

// REPORT_RESULT is defined when running in Emscripten test harness.
#ifdef REPORT_RESULT
// Count the mixed frames and return after 375 frames (1 second with the default 128 size)
volatile int audioProcessedCount = 0;
bool playedAndMixed(double time, void* data) {
  if (audioProcessedCount >= 375) {
    REPORT_RESULT(0);
    return false;
  }
  return true;
}
#endif

// ID to the beat and bass loops
EMSCRIPTEN_WEBAUDIO_T beatID = 0;
EMSCRIPTEN_WEBAUDIO_T bassID = 0;

// Creates a MediaElementAudioSourceNode with the supplied URL (which is
// registered as an internal audio object and the ID returned).
EM_JS(EMSCRIPTEN_WEBAUDIO_T, createTrack, (EMSCRIPTEN_WEBAUDIO_T ctxID, const char* url, bool looping), {
  var context = emscriptenGetAudioObject(ctxID);
  if (context) {
    var audio = document.createElement('audio');
    audio.src = UTF8ToString(url);
    audio.loop = looping;
    var track = context.createMediaElementSource(audio);
    return emscriptenRegisterAudioObject(track);
  }
  return 0;
});

// Toggles the play/pause of a MediaElementAudioSourceNode given its ID
EM_JS(void, toggleTrack, (EMSCRIPTEN_WEBAUDIO_T srcID), {
  var source = emscriptenGetAudioObject(srcID);
  if (source) {
    var audio = source.mediaElement;
    if (audio) {
      if (audio.paused) {
        audio.currentTime = 0;
        audio.play();
      } else {
        audio.pause();
      }
    }
  }
});

// Callback to process and copy the audio tracks
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
#ifdef REPORT_RESULT
  audioProcessedCount++;
#endif
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

// Registered click even to (1) enable audio playback and (2) toggle playing the tracks
bool onClick(int type, const EmscriptenMouseEvent* e, void* data) {
  EMSCRIPTEN_WEBAUDIO_T ctx = VOIDP_2_WA(data);
  if (emscripten_audio_context_state(ctx) != AUDIO_CONTEXT_STATE_RUNNING) {
    printf("Resuming playback\n");
    emscripten_resume_audio_context_sync(ctx);
  }
  printf("Toggling audio playback\n");
  toggleTrack(beatID);
    toggleTrack(bassID);
  return false;
}

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  if (success) {
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
    
    emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, WA_2_VOIDP(context), false, &onClick);
    
#ifdef REPORT_RESULT
    emscripten_set_timeout_loop(&playedAndMixed, 16, NULL);
#endif
  } else {
    printf("Audio worklet node creation failed\n");
  }
}

// Worklet thread inited, now create the audio processor
void initialised(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  if (success) {
    printf("Audio worklet initialised\n");

    WebAudioWorkletProcessorCreateOptions opts = {
      .name = "mixer",
    };
    emscripten_create_wasm_audio_worklet_processor_async(context, &opts, &processorCreated, NULL);
  } else {
    printf("Audio worklet failed to initialise\n");
  }
}

int main() {
  static char workletStack[AUDIO_STACK_SIZE];
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(NULL);
  emscripten_start_wasm_audio_worklet_thread_async(context, workletStack, sizeof workletStack, &initialised, NULL);
  return 0;
}
