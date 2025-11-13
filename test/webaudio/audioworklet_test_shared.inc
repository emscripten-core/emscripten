// Shared code for the audio worklet mixer tests.

// Helper for MEMORY64 to cast an audio context or type to a void*
#define WA_2_VOIDP(ctx) ((void*) (intptr_t) ctx)
// Helper for MEMORY64 to cast a void* to an audio context or type
#define VOIDP_2_WA(ptr) ((EMSCRIPTEN_WEBAUDIO_T) (intptr_t) ptr)


// Count the audio callbacks and return after 375 frames (1 second with the
// default 128 size).
//
// *** Remove this in your own code ***
//
#ifdef TEST_AND_EXIT
volatile int audioProcessedCount = 0;
bool playedAndMixed(double time, void* data) {
  if (audioProcessedCount >= 375) {
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
    // Number() wrapper is a workaround for UTF8ToString() needing a JS number
    // and from64() not being available in EM_JS macros. Fix in UTF8ToString?
    audio.src = UTF8ToString(Number(url));
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
bool onClick(int type, const EmscriptenMouseEvent* e, void* data) {
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
void initialised(EMSCRIPTEN_WEBAUDIO_T context, bool success, void* data) {
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
  // Set at least the latency hint to test the attribute setting (the render
  // size we take from the calling code if set).
  EmscriptenWebAudioCreateAttributes attrs = {
    .latencyHint = "balanced",
#ifdef RENDER_SIZE_HINT
    .renderSizeHint = RENDER_SIZE_HINT
#endif
  };
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&attrs);
  // With the 1.1 API this may be values other than 128
  emscripten_outf("Audio context created with quantum size of %d samples", emscripten_audio_context_quantum_size(context));

  // Here we create a fixed sized worklet stack, but could calculate taking the
  // quantum size above (ins and outs * bytes per quantum).
  char* const workletStack = memalign(16, AUDIO_STACK_SIZE);
  emscripten_outf("Audio worklet stack at 0x%p", workletStack);
  assert(workletStack);

  emscripten_start_wasm_audio_worklet_thread_async(context, workletStack, AUDIO_STACK_SIZE, getStartCallback(), NULL);

#ifdef TEST_AND_EXIT
   // We're in the test harness and exiting is via playedAndMixed()
  emscripten_out("In test mode, will exit after 1 second of playback");
#endif
  emscripten_exit_with_live_runtime();

  return EXIT_SUCCESS;
}
