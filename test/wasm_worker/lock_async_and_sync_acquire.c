#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdio.h>

emscripten_lock_t lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;

void on_acquire(volatile void* address, uint32_t value,
              ATOMICS_WAIT_RESULT_T waitResult, void* userData) {
  printf("on_acquire: releasing lock.\n");
  emscripten_lock_release(&lock);
  printf("on_acquire: released lock.\n");
  exit(0);
}

int main() {
  printf("main: async acquiring lock.\n");
  emscripten_lock_async_acquire(&lock, on_acquire, 0, 100);
  printf("main: busy-spin acquiring lock.\n");
  emscripten_lock_busyspin_waitinf_acquire(&lock);
  printf("main: lock acquired.\n");
  emscripten_lock_release(&lock);
  printf("main: lock released.\n");
  emscripten_exit_with_live_runtime();
  return 1;
}
