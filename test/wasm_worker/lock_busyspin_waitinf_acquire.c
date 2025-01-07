#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <assert.h>

// Tests emscripten_lock_busyspin_waitinf_acquire().

emscripten_lock_t lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;

volatile int sharedState = 0;

void worker_main() {
  emscripten_lock_busyspin_waitinf_acquire(&lock);
  emscripten_atomic_add_u32((void*)&sharedState, 1);
#ifdef REPORT_RESULT
  REPORT_RESULT(sharedState);
#endif
}

char stack[1024];

void releaseLock(void *userData) {
  emscripten_atomic_sub_u32((void*)&sharedState, 1);
  emscripten_lock_release(&lock);
}

int main() {
  // Acquire the lock at startup.
  emscripten_lock_busyspin_waitinf_acquire(&lock);
  emscripten_atomic_add_u32((void*)&sharedState, 1);

  // Spawn a Worker to try to take the lock. It will succeed only after releaseLock()
  // gets called.
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);

  emscripten_set_timeout(releaseLock, 1000, 0);
}
