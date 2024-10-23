#include <emscripten/console.h>
#include <emscripten/emscripten.h>
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

void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void worker_main() {
  test();
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

char stack[1024];

int main() {
  test();
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
  emscripten_exit_with_live_runtime();
}
