#include <assert.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdio.h>

emscripten_semaphore_t sem = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(1);

void on_acquire(volatile void* address, uint32_t value,
                ATOMICS_WAIT_RESULT_T waitResult, void* userData) {
  assert(address == &sem);
  assert(value == 1);
  assert(waitResult == ATOMICS_WAIT_OK);
  printf("on_acquire: releasing semaphore.\n");
  emscripten_semaphore_release(&sem, 1);
  printf("on_acquire: released semaphore.\n");
  exit(0);
}

int main() {
  printf("main: async acquiring semaphore.\n");
  emscripten_semaphore_async_acquire(&sem, 1, on_acquire, 0, 100);
  printf("main: try-acquiring semaphore.\n");
  int idx = emscripten_semaphore_try_acquire(&sem, 1);
  assert(idx == 0);
  printf("main: semaphore acquired.\n");
  emscripten_semaphore_release(&sem, 1);
  printf("main: semaphore released.\n");
  emscripten_exit_with_live_runtime();
  return 1;
}
