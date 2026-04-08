#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>
#include <stdatomic.h>

// Test emscripten_navigator_hardware_concurrency() and emscripten_atomics_is_lock_free() functions

void test() {
  // Assume that test suite does have navigator.hardwareConcurrency.
  assert(emscripten_navigator_hardware_concurrency() >= 2);
  assert(emscripten_atomics_is_lock_free(1));
  assert(emscripten_atomics_is_lock_free(2));
  assert(emscripten_atomics_is_lock_free(4));
  assert(emscripten_atomics_is_lock_free(8));
  assert(!emscripten_atomics_is_lock_free(16));
  assert(!emscripten_atomics_is_lock_free(31));

  // Test compiler buildin __atomic_always_lock_free version
  assert(__atomic_always_lock_free(1, 0));
  assert(__atomic_always_lock_free(2, 0));
  assert(__atomic_always_lock_free(4, 0));
  assert(__atomic_always_lock_free(8, 0));
  assert(!__atomic_always_lock_free(16, 0));
  assert(!__atomic_always_lock_free(31, 0));

  // Test C11 atomic_is_lock_free
  struct { char a[1]; } one;
  struct { char a[2]; } two;
  struct { char a[4]; } four;
  struct { char a[8]; } eight;
  struct { char a[16]; } sixteen;
  struct { char a[31]; } thirty_one;
  assert(atomic_is_lock_free(&one));
  assert(atomic_is_lock_free(&two));
  assert(atomic_is_lock_free(&four));
  assert(atomic_is_lock_free(&eight));
  assert(!atomic_is_lock_free(&sixteen));
  assert(!atomic_is_lock_free(&thirty_one));
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
