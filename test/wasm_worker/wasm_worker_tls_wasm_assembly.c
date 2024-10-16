#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Verify that the global heap is not overrun (reinitialized) on worker creation.
int globalData = 1;

int get_tls_variable(void);
void set_tls_variable(int var);

void main_thread_func() {
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(globalData == 3);
#ifdef REPORT_RESULT
  REPORT_RESULT(get_tls_variable());
#endif
}

void worker_main() {
  assert(emscripten_current_thread_is_wasm_worker());
  assert(get_tls_variable() == 0);
  assert(globalData == 2);
  globalData = 3;
  set_tls_variable(123456); // Try to write garbage data to the memory location.
  emscripten_wasm_worker_post_function_v(0, main_thread_func);
}

char stack[1024];

int main() {
  assert(!emscripten_current_thread_is_wasm_worker());
  assert(globalData == 1);
  globalData = 2;
  set_tls_variable(42);
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
