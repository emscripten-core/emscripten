#include <emscripten/webaudio.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <assert.h>

// Tests that
// - audioworklets and workers can be used at the same time.
// - an audioworklet can emscripten_futex_wake() a waiting worker.
// - global values can be shared between audioworklets and workers.

int workletToWorkerFutexLocation = 0;
int workletToWorkerFlag = 0;
EMSCRIPTEN_WEBAUDIO_T context;

void do_exit() {
  emscripten_terminate_all_wasm_workers();
  emscripten_destroy_audio_context(context);
  emscripten_force_exit(0);
}

void run_in_worker() {
  while (0 == emscripten_futex_wait(&workletToWorkerFutexLocation, 0, 30000)) {
    if (workletToWorkerFlag == 1) {
      emscripten_out("Test success");
      emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, &do_exit);
      break;
    }
  }
}

// This event will fire on the audio worklet thread.
void MessageReceivedInAudioWorkletThread() {
  assert(emscripten_current_thread_is_audio_worklet());
  workletToWorkerFlag = 1;
  emscripten_futex_wake(&workletToWorkerFutexLocation, 1);
}

void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, bool success, void *userData) {
  emscripten_audio_worklet_post_function_v(audioContext, MessageReceivedInAudioWorkletThread);
}

uint8_t wasmAudioWorkletStack[4096];

int main() {
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stackSize: */1024);
  emscripten_wasm_worker_post_function_v(worker, run_in_worker);

  context = emscripten_create_audio_context(0);
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);

  emscripten_exit_with_live_runtime();
}
