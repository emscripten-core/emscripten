#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>
#include <threads.h>

__thread int tls = 1;

void main_thread_func() {
  assert(!emscripten_current_thread_is_wasm_worker());
  EM_ASM(out($0), tls);
#ifdef REPORT_RESULT
  REPORT_RESULT(tls);
#endif
}

void worker_main() {
  assert(emscripten_current_thread_is_wasm_worker());
  assert(tls != 42);
  assert(tls != 0);
  assert(tls == 1);
  tls = 123456; // Try to write garbage data to the memory location.
  emscripten_wasm_worker_post_function_v(0, main_thread_func);
}

char stack[1024];

int main() {
  EM_ASM(out($0), tls);
  assert(!emscripten_current_thread_is_wasm_worker());
  tls = 42;
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
