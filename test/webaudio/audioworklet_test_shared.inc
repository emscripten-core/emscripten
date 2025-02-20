// Shared code for the audio worklet mixer tests.

// Helper for MEMORY64 to cast an audio context or type to a void*
#define WA_2_VOIDP(ctx) ((void*) (intptr_t) ctx)
// Helper for MEMORY64 to cast a void* to an audio context or type
#define VOIDP_2_WA(ptr) ((EMSCRIPTEN_WEBAUDIO_T) (intptr_t) ptr)

// Attribute that marks a function, parameter, variable, etc., as unused
// (silencing any -Wunused warnings).
#ifndef __unused
#define __unused __attribute__((unused))
#endif

// Count the audio callbacks and return after 375 frames (1 second with the
// default 128 size).
//
// *** Remove this in your own code ***
//
#ifdef TEST_AND_EXIT
volatile int audioProcessedCount = 0;
bool playedAndMixed(double __unused time, void* __unused data) {
#if TEST_AND_EXIT > 0
  // Special interactive case to play for 10 seconds
  if (audioProcessedCount >= 3750) {
#else
  if (audioProcessedCount >= 375) {
#endif
    emscripten_force_exit(0);
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
#if __wasm64__
    // Number() wrapper is a workaround for UTF8ToString() needing a JS number
    // and from64() not being available in EM_JS macros. Fix in UTF8ToString?
    url = Number(url);
#endif
    audio.src = UTF8ToString(url);
    audio.loop = looping;
    var track = context.createMediaElementSource(audio);
    return emscriptenRegisterAudioObject(track);
  }
  return 0;
})

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
})

// Registered click event to (1) enable audio playback and (2) toggle playing the tracks
bool onClick(int __unused type, const EmscriptenMouseEvent* __unused e, void* data) {
  EMSCRIPTEN_WEBAUDIO_T ctx = VOIDP_2_WA(data);
  if (emscripten_audio_context_state(ctx) != AUDIO_CONTEXT_STATE_RUNNING) {
    emscripten_out("Resuming playback");
    emscripten_resume_audio_context_sync(ctx);
  }
  emscripten_out("Toggling audio playback");
  toggleTrack(beatID);
  toggleTrack(bassID);
  return false;
}

// Forward declaration
void processorCreated(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data);

// Worklet thread inited, now create the audio processor
void initialised(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* __unused data) {
  assert(success && "Audio worklet failed in initialised()");
  emscripten_out("Audio worklet initialised");

  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "mixer"
  };
  emscripten_create_wasm_audio_worklet_processor_async(context, &opts, &processorCreated, NULL);
}

// To be implemented by the test code, allowing initialised() to be changed.
EmscriptenStartWebAudioWorkletCallback getStartCallback(void);

// Common entry point for the mixer tests
int main(void) {
  char* const workletStack = memalign(16, AUDIO_STACK_SIZE);
  emscripten_outf("Audio worklet stack at 0x%p", workletStack);
  assert(workletStack);

  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(NULL);
  emscripten_start_wasm_audio_worklet_thread_async(context, workletStack, AUDIO_STACK_SIZE, getStartCallback(), NULL);

#ifdef TEST_AND_EXIT
   // We're in the test harness and exiting is via playedAndMixed()
#if TEST_AND_EXIT > 0
  emscripten_out("In test mode, will exit after 10 seconds of playback");
#else
  emscripten_out("In test mode, will exit after 1 second of playback");
#endif
  emscripten_exit_with_live_runtime();
#endif

  return EXIT_SUCCESS;
}
