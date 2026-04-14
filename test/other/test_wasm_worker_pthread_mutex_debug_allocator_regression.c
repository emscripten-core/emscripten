// Regression test for https://github.com/emscripten-core/emscripten/issues/26619

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>

static _Atomic int running_threads = 0;

static void malloc_loop(void) {
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
  (void)arg;
  emscripten_outf("main_callback");
  malloc_loop();
  emscripten_outf("done");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

int main(void) {
  emscripten_outf("main");
  emscripten_wasm_worker_post_function_v(
    emscripten_malloc_wasm_worker(1024 * 1024),
    worker_callback
  );
  emscripten_async_call(main_callback, NULL, 0);
  return 0;
}
