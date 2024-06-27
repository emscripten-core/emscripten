#include <emscripten/webaudio.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Tests that
// - audioworklets and workers can be used at the same time.
// - an audioworklet can emscripten_futex_wake() a waiting worker.
// - global values can be shared between audioworklets and workers.

int workletToWorkerFutexLocation = 0;
int workletToWorkerFlag = 0;

void run_in_worker() {
  while (0 == emscripten_futex_wait(&workletToWorkerFutexLocation, 0, 30000)) {
    if (workletToWorkerFlag == 1) {
      printf("Test success\n");
      break;
    }
  }

#ifdef REPORT_RESULT
  REPORT_RESULT(workletToWorkerFlag == 1);
#endif
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

  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(0);
  emscripten_start_wasm_audio_worklet_thread_async(context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack), WebAudioWorkletThreadInitialized, 0);
}
