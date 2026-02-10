#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <assert.h>

// Tests emscripten_lock_busyspin_wait_acquire().

emscripten_lock_t lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;

void test() {
  // Expect no contention on free lock.
  bool success = emscripten_lock_busyspin_wait_acquire(&lock, 0);
  assert(success == true);

  double t0 = emscripten_performance_now();
  // We already have the lock, and emscripten_lock is not recursive, so this
  // should fail.
  success = emscripten_lock_busyspin_wait_acquire(&lock, 0);
  double t1 = emscripten_performance_now();
  assert(!success);
  // Shouldn't have taken too much time to try the lock.
  assert(t1 - t0 < 25);

  success = emscripten_lock_try_acquire(&lock);
  assert(!success); // We already have the lock.

  t0 = emscripten_performance_now();
  // We already have the lock, and emscripten_lock is not recursive, so this
  // should fail.
  success = emscripten_lock_busyspin_wait_acquire(&lock, 1000.0);
  t1 = emscripten_performance_now();
  assert(!success);
  // We should have waited for the requested duration for the lock.. apply some
  // slack since timing can have some noise.
  assert(t1 - t0 >= 900);

  emscripten_lock_release(&lock);
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
