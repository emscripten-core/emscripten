#include <emscripten/threading.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/webaudio.h>
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
//
// Define TEST_ON_WORKER to run on a worker, otherwise it runs on the main thread

// Global audio context
EMSCRIPTEN_WEBAUDIO_T context;

// Internal, found in 'system/lib/pthread/threading_internal.h' (and requires building with -pthread)
int _emscripten_thread_supports_atomics_wait(void);

typedef enum {
  // The test hasn't yet started
  TEST_NOT_STARTED,
  // No atomics wait support in audio worklets
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
// Which test is running (sometimes in the worklet, sometimes in the worker/main)
_Atomic Test whichTest = TEST_NOT_STARTED;
// Time at which the test starts taken in main()
double startTime = 0;

void do_exit() {
  emscripten_out("Test success");
  emscripten_destroy_audio_context(context);
#ifdef TEST_ON_WORKER
  emscripten_terminate_all_wasm_workers();
#endif
  emscripten_force_exit(0);
}

bool ProcessAudio(int numInputs, const AudioSampleFrame *inputs, int numOutputs, AudioSampleFrame *outputs, int numParams, const AudioParamFrame *params, void *userData) {
  assert(emscripten_current_thread_is_audio_worklet());

  int result = 0;
  switch (whichTest) {
  case TEST_NOT_STARTED:
    break;
  case TEST_HAS_WAIT:
    // Should not have wait support here
    result = _emscripten_thread_supports_atomics_wait();
    emscripten_outf("TEST_HAS_WAIT: %d (expect: 0)", result);
    assert(!result);
    whichTest = TEST_TRY_ACQUIRE;
    break;
  case TEST_TRY_ACQUIRE:
    // Was locked after init, should fail to acquire
    result = emscripten_lock_try_acquire(&testLock);
    emscripten_outf("TEST_TRY_ACQUIRE: %d (expect: 0)", result);
    assert(!result);
    whichTest = TEST_WAIT_ACQUIRE_FAIL;
    break;
  case TEST_WAIT_ACQUIRE_FAIL:
    // Still locked so we fail to acquire
    result = emscripten_lock_busyspin_wait_acquire(&testLock, 100);
    emscripten_outf("TEST_WAIT_ACQUIRE_FAIL: %d (expect: 0)", result);
    assert(!result);
    whichTest = TEST_WAIT_ACQUIRE;
    // Fall through here so the worker/main has a chance to unlock whilst spinning
  case TEST_WAIT_ACQUIRE:
    // Will get unlocked in worker/main, so should quickly acquire
    result = emscripten_lock_busyspin_wait_acquire(&testLock, 10000);
    emscripten_outf("TEST_WAIT_ACQUIRE: %d  (expect: 1)", result);
    assert(result);
    whichTest = TEST_RELEASE;
    break;
  case TEST_RELEASE:
    // Unlock, check the result
    emscripten_lock_release(&testLock);
    result = emscripten_lock_try_acquire(&testLock);
    emscripten_outf("TEST_RELEASE: %d (expect: 1)", result);
    assert(result);
    whichTest = TEST_WAIT_INFINTE_1;
    break;
  case TEST_WAIT_INFINTE_1:
    // Still locked when we enter here but move on in the worker/main
    break;
  case TEST_WAIT_INFINTE_2:
    emscripten_lock_release(&testLock);
    whichTest = TEST_GET_NOW;
    break;
  case TEST_GET_NOW:
    result = (int) (emscripten_get_now() - startTime);
    emscripten_outf("TEST_GET_NOW: %d  (expect: > 0)", result);
    assert(result > 0);
    whichTest = TEST_DONE;
    // Fall through here and stop playback (shutting down in the worker/main)
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

#ifdef TEST_ON_WORKER
void WorkerLoop() {
#else
bool MainLoop(double time, void* data) {
#endif
  assert(!emscripten_current_thread_is_audio_worklet());
  int didUnlock = false;
#ifdef TEST_ON_WORKER
  while (true) {
#endif
	  switch (whichTest) {
	  case TEST_NOT_STARTED:
		emscripten_out("Staring test (may need a button click)");
		whichTest = TEST_HAS_WAIT;
		break;
	  case TEST_WAIT_ACQUIRE:
		if (!didUnlock) {
		  emscripten_out("Worker/main releasing lock");
		  // Release here to acquire in process
		  emscripten_lock_release(&testLock);
		  didUnlock = true;
		}
		break;
	  case TEST_WAIT_INFINTE_1:
		// Spin here until released in process (but don't change test until we know this case ran)
		whichTest = TEST_WAIT_INFINTE_2;
		emscripten_lock_busyspin_waitinf_acquire(&testLock);
		emscripten_out("TEST_WAIT_INFINTE (from worker/main)");
		break;
	  case TEST_DONE:
		// Finished, exit from the main thread (and return out of this loop)
#ifdef TEST_ON_WORKER
		emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, &do_exit);
        // WorkerLoop contract (end the worker)
		return;
#else
        do_exit();
        // MainLoop contract (stop the repeats)
        return false;
#endif
	  default:
		break;
	  }
#ifdef TEST_ON_WORKER
    // Repeat every 10ms (except when TEST_DONE)
    emscripten_wasm_worker_sleep(10 * 1000000ULL);
  }
#else
    // Run again
    return true;
#endif
}

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  int outputChannelCounts[1] = { 1 };
  EmscriptenAudioWorkletNodeCreateOptions options = { .numberOfInputs = 0, .numberOfOutputs = 1, .outputChannelCounts = outputChannelCounts };
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext, "locks-test", &options, &ProcessAudio, NULL);
  emscripten_audio_node_connect(wasmAudioWorklet, audioContext, 0, 0);
  InitHtmlUi(audioContext);
}

void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  WebAudioWorkletProcessorCreateOptions opts = { .name = "locks-test" };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, AudioWorkletProcessorCreated, NULL);
}

uint8_t wasmAudioWorkletStack[2048];

int main() {
  // Main thread init and acquire (work passes to the audio processor)
  emscripten_lock_init(&testLock);
  int hasLock = emscripten_lock_busyspin_wait_acquire(&testLock, 0);
  assert(hasLock);

  startTime = emscripten_get_now();

  // Audio processor callback setup
  context = emscripten_create_audio_context(NULL);
  assert(context);
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, NULL);

  // Either call every 10ms or create a worker that sleeps every 10ms
#ifdef TEST_ON_WORKER
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(1024);
  emscripten_wasm_worker_post_function_v(worker, WorkerLoop);
#else
  emscripten_set_timeout_loop(MainLoop, 10, NULL);
#endif

  emscripten_exit_with_live_runtime();
}
