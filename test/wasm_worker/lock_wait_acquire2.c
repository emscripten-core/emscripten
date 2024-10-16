#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <assert.h>

// Tests emscripten_lock_wait_acquire() between two Wasm Workers.

emscripten_lock_t lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;

void worker1_main() {
  emscripten_console_log("worker1 main try_acquiring lock");
  bool success = emscripten_lock_try_acquire(&lock); // Expect no contention on free lock.
  emscripten_console_log("worker1 try_acquire lock finished");
  assert(success);
  emscripten_console_log("worker1 try_acquire lock success, sleeping 1000 msecs");
  emscripten_wasm_worker_sleep(1000 * 1000000ull);

  emscripten_console_log("worker1 slept 1000 msecs, releasing lock");
  emscripten_lock_release(&lock);
  emscripten_console_log("worker1 released lock");
}

void worker2_main() {
  emscripten_console_log("worker2 main sleeping 500 msecs");
  emscripten_wasm_worker_sleep(500 * 1000000ull);
  emscripten_console_log("worker2 slept 500 msecs, try_acquiring lock");
  bool success = emscripten_lock_try_acquire(&lock); // At this time, the other thread should have the lock.
  emscripten_console_log("worker2 try_acquire lock finished");
  assert(!success);

  // Wait enough time to cover over the time that the other thread held the lock.
  success = emscripten_lock_wait_acquire(&lock, 2000 * 1000000ull);
  emscripten_console_log("worker2 wait_acquired lock");
  assert(success);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

char stack1[1024];
char stack2[1024];

int main() {
  emscripten_wasm_worker_t worker1 = emscripten_create_wasm_worker(stack1, sizeof(stack1));
  emscripten_wasm_worker_t worker2 = emscripten_create_wasm_worker(stack2, sizeof(stack2));
  emscripten_wasm_worker_post_function_v(worker1, worker1_main);
  emscripten_wasm_worker_post_function_v(worker2, worker2_main);
}
