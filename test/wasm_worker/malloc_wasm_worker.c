#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Test emscripten_malloc_wasm_worker() and emscripten_current_thread_is_wasm_worker() functions

void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void worker_main() {
  emscripten_out("Hello from wasm worker!");
  assert(emscripten_current_thread_is_wasm_worker());
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

int main() {
  assert(!emscripten_current_thread_is_wasm_worker());
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */1024);
  assert(worker);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
  emscripten_exit_with_live_runtime();
}
