#include <emscripten/wasm_worker.h>
#include <emscripten/webaudio.h>
#include <assert.h>

// Tests that these audio worklet compatible functions work, details in comments below:
//
// - _emscripten_thread_supports_atomics_wait()
// - emscripten_lock_init()
// - emscripten_lock_busyspin_wait_acquire()
// - emscripten_lock_release()
// - emscripten_get_now() in AW

// This needs to be big enough for a stereo output (1024 with a 128 frame) + working stack
#define AUDIO_STACK_SIZE 2048

// Define DISABLE_LOCKS to run the test without locking, which should statistically always fail
//#define DISABLE_LOCKS

// Internal, found in 'system/lib/pthread/threading_internal.h' (and requires building with -pthread)
int _emscripten_thread_supports_atomics_wait(void);

typedef enum {
  // The test hasn't yet started
  TEST_NOT_STARTED,
  // Worklet ready and running the test
  TEST_RUNNING,
  // Main thread is finished, wait on worklet
  TEST_DONE_MAIN,
  // Test finished
  TEST_DONE
} Test;

// Global audio context
EMSCRIPTEN_WEBAUDIO_T context;
// Lock used in all the tests
emscripten_lock_t testLock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;
// Which test is running (sometimes in the worklet, sometimes in the main thread)
_Atomic Test whichTest = TEST_NOT_STARTED;
// Time at which the test starts taken in main()
double startTime = 0;

// Counter for main, accessed only by main
int howManyMain = 0;
// Counter for the audio worklet, accessed only by the AW
int howManyProc = 0;

// Our dummy container
typedef struct {
  uint32_t val0;
  uint32_t val1;
  uint32_t val2;
} Dummy;

// Start values
void initDummy(Dummy* dummy) {
  dummy->val0 = 4;
  dummy->val1 = 1;
  dummy->val2 = 2;
}

void printDummy(Dummy* dummy) {
  emscripten_outf("Values: %u, %u, %u", dummy->val0, dummy->val1, dummy->val2);
}

// Run a simple calculation that will only be stable *if* all values are atomically updated
// (Currently approx. 200'000x from each thread)
void runCalcs(Dummy* dummy, int num) {
  for (int n = 0; n < num; n++) {
#ifndef DISABLE_LOCKS
    int have = emscripten_lock_busyspin_wait_acquire(&testLock, 10);
    assert(have);
#endif
    dummy->val0 += dummy->val1 * dummy->val2;
    dummy->val1 += dummy->val2 * dummy->val0;
    dummy->val2 += dummy->val0 * dummy->val1;
    dummy->val0 /= 4;
    dummy->val1 /= 3;
    dummy->val2 /= 2;
    emscripten_lock_release(&testLock);
  }
}

void stopping() {
  emscripten_out("Expect: 811100370, 759556424, 723197652");
  emscripten_out("Ending test");
  emscripten_destroy_audio_context(context);
  emscripten_force_exit(0);
}

// AW callback
bool process(int numInputs, const AudioSampleFrame* inputs, int numOutputs, AudioSampleFrame* outputs, int numParams, const AudioParamFrame* params, void* data) {
  assert(emscripten_current_thread_is_audio_worklet());
  switch (whichTest) {
  case TEST_NOT_STARTED:
    whichTest = TEST_RUNNING;
    break;
  case TEST_RUNNING:
  case TEST_DONE_MAIN:
    if (howManyProc-- > 0) {
      runCalcs((Dummy*) data, 267); // <-- process gets called 3.75x more than main
    } else {
      if (whichTest == TEST_DONE_MAIN) {
        emscripten_outf("Worklet done after %dms (expect: > 2s)", (int) (emscripten_get_now() - startTime));
        // Both loops are finished
        whichTest = TEST_DONE;
      }
    }
    break;
  case TEST_DONE:
    return false;
  }
  return true;
}

// Main thread callback
bool mainLoop(double time, void* data) {
  assert(!emscripten_current_thread_is_audio_worklet());
  switch (whichTest) {
  case TEST_NOT_STARTED:
    break;
  case TEST_RUNNING:
    if (howManyMain-- > 0) {
      runCalcs((Dummy*) data, 1000);
    } else {
      emscripten_outf("Main thread done after %dms (expect: > 2s)", (int) (emscripten_get_now() - startTime));
      // Done here, so signal to process()
      whichTest = TEST_DONE_MAIN;
    }
    break;
  case TEST_DONE_MAIN:
    // Wait for process() to finish
    break;
  case TEST_DONE:
    printDummy((Dummy*) data);
    // 32-bit maths with locks *should* result in these:
    assert(((Dummy*) data)->val0 == 811100370
        && ((Dummy*) data)->val1 == 759556424
        && ((Dummy*) data)->val2 == 723197652);
    stopping();
    return false;
  }
  return true;
}

EMSCRIPTEN_KEEPALIVE void startTest() {
  startTime = emscripten_get_now();
  if (emscripten_audio_context_state(context) != AUDIO_CONTEXT_STATE_RUNNING) {
    emscripten_resume_audio_context_sync(context);
  }
  howManyMain = 200;
  howManyProc = 750; // <-- process gets called 3.75x more than main
}

// HTML button to manually run the test
EM_JS(void, addButton, (), {
  var button = document.createElement("button");
  button.appendChild(document.createTextNode("Start Test"));
  document.body.appendChild(button);
  document.onclick = () => {
    if (globalThis._startTest) {
      _startTest();
    }
  };
});

// Audio processor created, now register the audio callback
void processorCreated(EMSCRIPTEN_WEBAUDIO_T ctx, bool success, void* data) {
  assert(success && "Audio worklet failed in processorCreated()");
  emscripten_out("Audio worklet processor created");
  // Single mono output
  int outputChannelCounts[1] = { 1 };
  EmscriptenAudioWorkletNodeCreateOptions opts = {
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T worklet = emscripten_create_wasm_audio_worklet_node(ctx, "locks-test", &opts, &process, data);
  emscripten_audio_node_connect(worklet, ctx, 0, 0);
}

// Worklet thread inited, now create the audio processor
void initialised(EMSCRIPTEN_WEBAUDIO_T ctx, bool success, void* data) {
  assert(success && "Audio worklet failed in initialised()");
  emscripten_out("Audio worklet initialised");
  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "locks-test"
  };
  emscripten_create_wasm_audio_worklet_processor_async(ctx, &opts, &processorCreated, data);
}

int main() {
  emscripten_lock_init(&testLock);
  Dummy* dummy = (Dummy*) malloc(sizeof(Dummy));
  initDummy(dummy);

  char* const workletStack = memalign(16, AUDIO_STACK_SIZE);
  assert(workletStack);
  // Audio processor callback setup
  context = emscripten_create_audio_context(NULL);
  assert(context);
  emscripten_start_wasm_audio_worklet_thread_async(context, workletStack, AUDIO_STACK_SIZE, initialised, dummy);

  emscripten_set_timeout_loop(mainLoop, 10, dummy);
  addButton();
  startTest(); // <-- May need a manual click to start

  emscripten_exit_with_live_runtime();
}
