#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Test emscripten_wasm_worker_post_function_*() API and EMSCRIPTEN_WASM_WORKER_ID_PARENT
// to send a message back from Worker to its parent thread.

void test_success(int i, double d) {
  emscripten_out("test_success");
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(i == 10);
  assert(d == 0.5);
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void worker_main() {
  emscripten_out("worker_main");
  assert(emscripten_current_thread_is_wasm_worker());
  emscripten_wasm_worker_post_function_sig(EMSCRIPTEN_WASM_WORKER_ID_PARENT, test_success, "id", 10, 0.5);
}

char stack[1024];

int main() {
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
  emscripten_exit_with_live_runtime();
}
