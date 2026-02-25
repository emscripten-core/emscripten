// Test that emscripten_futex_wait() works in a Wasm Worker.

#include <assert.h>
#include <errno.h>
#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/threading.h>
#include <emscripten/wasm_worker.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

_Atomic uint32_t futex_value = 0;

void wake_worker_after_delay(void *user_data) {
  futex_value = 1;
  emscripten_futex_wake(&futex_value, INT_MAX);
}

void wake_worker() {
  printf("Waking worker thread from futex wait...\n");
  emscripten_set_timeout(wake_worker_after_delay, 500, 0);
}

void worker_main() {
  printf("Worker sleeping on futex...\n");
  double start = emscripten_performance_now();
  int rc = emscripten_futex_wait(&futex_value, 0, 100);
  double end = emscripten_performance_now();
  printf("emscripten_futex_wait returned: %d after %.2fms\n", rc, end - start);
  assert(rc == -ETIMEDOUT);
  assert((end - start) >= 100);

  printf("Worker sleeping on futex with wakeup...\n");
  emscripten_wasm_worker_post_function_v(0, wake_worker);
  rc = emscripten_futex_wait(&futex_value, 0, INFINITY);
  printf("emscripten_futex_wait returned: %d\n", rc);
  assert(rc == 0);
  assert(futex_value == 1);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main() {
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), worker_main);
}
