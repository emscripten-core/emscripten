#include <emscripten/threading.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/webaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Tests that these audio worklet compatible functions work, details in comments below:
//
// - _emscripten_thread_supports_atomics_wait()
// - emscripten_lock_init()
// - emscripten_lock_try_acquire()
// - emscripten_lock_busyspin_wait_acquire()
// - emscripten_lock_busyspin_waitinf_acquire()
// - emscripten_lock_release()
// - emscripten_get_now()

// Internal, found in 'system/lib/pthread/threading_internal.h'
int _emscripten_thread_supports_atomics_wait(void);

typedef enum {
  // No wait support in audio worklets
  TEST_HAS_WAIT,
  // Acquired in main, fail in process
  TEST_TRY_ACQUIRE,
  // Keep acquired so time-out
  TEST_WAIT_ACQUIRE_FAIL,
  // Release in main, succeed in process
  TEST_WAIT_ACQUIRE,
  // Release in process after above
  TEST_RELEASE,
  // Released in process above, spin in main
  TEST_WAIT_INFINTE_1,
  // Release in process to stop spinning in main
  TEST_WAIT_INFINTE_2,
  // Call emscripten_get_now() in process
  TEST_GET_NOW,
  // Test finished
  TEST_DONE
} Test;

// Lock used in all the tests
emscripten_lock_t testLock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;
// Which test is running (sometimes in the worklet, sometimes in the main thread)
_Atomic Test whichTest = TEST_HAS_WAIT;
// Time at which the test starts taken in main()
double startTime = 0;

bool ProcessAudio(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData) {
  int result = 0;
  switch (whichTest) {
  case TEST_HAS_WAIT:
    // Should not have wait support here
    result = _emscripten_thread_supports_atomics_wait();
    printf("TEST_HAS_WAIT: %d (expect: 0)\n", result);
    assert(!result);
    whichTest = TEST_TRY_ACQUIRE;
    break;
  case TEST_TRY_ACQUIRE:
    // Was locked after init, should fail to acquire
    result = emscripten_lock_try_acquire(&testLock);
    printf("TEST_TRY_ACQUIRE: %d (expect: 0)\n", result);
    assert(!result);
    whichTest = TEST_WAIT_ACQUIRE_FAIL;
    break;
  case TEST_WAIT_ACQUIRE_FAIL:
    // Still locked so we fail to acquire
    result = emscripten_lock_busyspin_wait_acquire(&testLock, 100);
    printf("TEST_WAIT_ACQUIRE_FAIL: %d (expect: 0)\n", result);
    assert(!result);
    whichTest = TEST_WAIT_ACQUIRE;
  case TEST_WAIT_ACQUIRE:
    // Will get unlocked in main thread, so should quickly acquire
    result = emscripten_lock_busyspin_wait_acquire(&testLock, 100);
    printf("TEST_WAIT_ACQUIRE: %d  (expect: 1)\n", result);
    assert(result);
    whichTest = TEST_RELEASE;
    break;
  case TEST_RELEASE:
    // Unlock, check the result
    emscripten_lock_release(&testLock);
    result = emscripten_lock_try_acquire(&testLock);
    printf("TEST_RELEASE: %d (expect: 1)\n", result);
    assert(result);
    whichTest = TEST_WAIT_INFINTE_1;
    break;
  case TEST_WAIT_INFINTE_1:
    // Still locked when we enter here but move on in the main thread
    break;
  case TEST_WAIT_INFINTE_2:
    emscripten_lock_release(&testLock);
    whichTest = TEST_GET_NOW;
    break;
  case TEST_GET_NOW:
    result = (int) (emscripten_get_now() - startTime);
    printf("TEST_GET_NOW: %d  (expect: > 0)\n", result);
    assert(result > 0);
    whichTest = TEST_DONE;
  case TEST_DONE:
    return false;
  default:
    break;
  }
  return true;
}

EM_JS(void, InitHtmlUi, (EMSCRIPTEN_WEBAUDIO_T audioContext), {
  let startButton = document.createElement('button');
  startButton.innerHTML = 'Start playback';
  document.body.appendChild(startButton);

  audioContext = emscriptenGetAudioObject(audioContext);
  startButton.onclick = () => {
    audioContext.resume();
  };
});

bool MainLoop(double time, void* data) {
  switch (whichTest) {
  case TEST_WAIT_ACQUIRE:
    // Release here to acquire in process
    emscripten_lock_release(&testLock);
    break;
  case TEST_WAIT_INFINTE_1:
    // Spin here until released in process (but don't change test until we know this case ran)
    whichTest = TEST_WAIT_INFINTE_2;
    emscripten_lock_busyspin_waitinf_acquire(&testLock);
    printf("TEST_WAIT_INFINTE (from main)\n");
    break;
  case TEST_DONE:
    return false;
  default:
    break;
  }
  return true;
}

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  int outputChannelCounts[1] = { 1 };
  EmscriptenAudioWorkletNodeCreateOptions options = { .numberOfInputs = 0, .numberOfOutputs = 1, .outputChannelCounts = outputChannelCounts };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "noise-generator", &options, &ProcessAudio, NULL);
  emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);
  InitHtmlUi(audioContext);
}

void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  WebAudioWorkletProcessorCreateOptions opts = { .name = "noise-generator" };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, NULL);
}

uint8_t wasmAudioWorkletStack[2048];

int main() {
  // Main thread init and acquire (work passes to the processor)
  emscripten_lock_init(&testLock);
  int hasLock = emscripten_lock_busyspin_wait_acquire(&testLock, 0);
  assert(hasLock);

  startTime = emscripten_get_now();

  emscripten_set_timeout_loop(MainLoop, 10, NULL);
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(NULL);
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, NULL);
}
