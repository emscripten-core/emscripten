#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Test emscripten_wasm_worker_post_function_*() API and EMSCRIPTEN_WASM_WORKER_ID_PARENT
// to send a message back from Worker to its parent thread.

void test_success(int i, double d) {
  emscripten_console_log("test_success");
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(i == 10);
  assert(d == 0.5);
#ifdef REPORT_RESULT
  REPORT_RESULT(i);
#endif
}

void worker_main() {
  emscripten_console_log("worker_main");
  assert(emscripten_current_thread_is_wasm_worker());
  emscripten_wasm_worker_post_function_sig(EMSCRIPTEN_WASM_WORKER_ID_PARENT, test_success, "id", 10, 0.5);
}

char stack[1024];

int main() {
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
