#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Test emscripten_navigator_hardware_concurrency() and emscripten_atomics_is_lock_free() functions

void test() {
  // Assume that test suite does have navigator.hardwareConcurrency.
  assert(emscripten_navigator_hardware_concurrency() >= 2);
  assert(emscripten_atomics_is_lock_free(1));
  assert(emscripten_atomics_is_lock_free(2));
  assert(emscripten_atomics_is_lock_free(4));
  // Chrome is buggy, see
  // https://bugs.chromium.org/p/chromium/issues/detail?id=1167449
  //assert(emscripten_atomics_is_lock_free(8));
  assert(!emscripten_atomics_is_lock_free(31));
}

void worker_main() {
  test();
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

char stack[1024];

int main() {
  test();
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
