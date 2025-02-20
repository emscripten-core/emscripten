// Shared code for the audio worklet mixer tests.

// Helper for MEMORY64 to cast an audio context or type to a void*
#define WA_2_VOIDP(ctx) ((void*) (intptr_t) ctx)
// Helper for MEMORY64 to cast a void* to an audio context or type
#define VOIDP_2_WA(ptr) ((EMSCRIPTEN_WEBAUDIO_T) (intptr_t) ptr)

// Count the audio callbacks and return after 375 frames (1 second with the default 128 size)
//
// *** Remove this in your own code ***
//
volatile int audioProcessedCount = 0;
bool playedAndMixed(double time, void* data) {
  if (audioProcessedCount >= 375) {
    emscripten_force_exit(0);
    return false;
  }
  return true;
}

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

// Forward declaration
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data);

// Worklet thread inited, now create the audio processor
void initialised(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
  if (!success) {
    printf("Audio worklet failed to initialise\n");
    return;
  }
  printf("Audio worklet initialised\n");

  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "mixer",
  };
  emscripten_create_wasm_audio_worklet_processor_async(context, &opts, &processorCreated, NULL);
}

// Common entry point for the mixer tests
int main() {
  static char workletStack[AUDIO_STACK_SIZE];
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(NULL);
  emscripten_start_wasm_audio_worklet_thread_async(context, workletStack, sizeof workletStack, &initialised, NULL);
#ifndef BROWSER_TEST
  // Special case: browser tests need to exit instantly, interactive tests need to wait
  emscripten_runtime_keepalive_push();
#endif
  return 0;
}
