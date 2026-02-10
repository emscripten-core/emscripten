// Test that emscripten_futex_wait() works in a Wasm Worker.

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
  printf("Waking worker thread from futex wait.\n");
  emscripten_set_timeout(wake_worker_after_delay, 500, 0);
}

void worker_main() {
  printf("Worker sleeping for futex wait.\n");
  emscripten_wasm_worker_post_function_v(0, wake_worker);
  int rc = emscripten_futex_wait(&futex_value, 0, INFINITY);
  printf("emscripten_futex_wait returned with code %d.\n", rc);
#ifdef REPORT_RESULT
  REPORT_RESULT(rc);
#endif
}

int main() {
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), worker_main);
}
