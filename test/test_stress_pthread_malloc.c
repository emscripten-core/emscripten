#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/console.h>
#include <stdlib.h>
#include <stdio.h>

_Atomic int running_threads = 0;

static void malloc_loop() {
  // Busy loop here until both threads are up running
  running_threads += 1;
  while (running_threads != 2) {}

  for (int i = 0; i < 1000000; ++i) {
    free(malloc(1));
  }
}

static void worker_callback(void) {
  emscripten_outf("worker_callback");
  malloc_loop();
}

static void main_callback(void* arg) {
  emscripten_outf("main_callback");
  malloc_loop();
  emscripten_outf("done");
  emscripten_terminate_all_wasm_workers();
}

int main() {
  emscripten_outf("main");
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024 * 1024), worker_callback);
  emscripten_async_call(main_callback, NULL, 0);
  return 0;
}
