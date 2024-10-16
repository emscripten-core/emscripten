#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <assert.h>

// Tests emscripten_semaphore_try_acquire() on the main thread

emscripten_semaphore_t unavailable = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(0);
emscripten_semaphore_t available = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(1);

int main() {
  emscripten_console_log("try_acquiring unavailable semaphore should fail");
  int idx = emscripten_semaphore_try_acquire(&unavailable, 1);
  assert(idx == -1);

  emscripten_console_log("try_acquiring too many resources from an available semaphore should fail");
  idx = emscripten_semaphore_try_acquire(&available, 2);
  assert(idx == -1);

  emscripten_console_log("try_acquiring a resource from an available semaphore should succeed");
  idx = emscripten_semaphore_try_acquire(&available, 1);
  assert(idx == 0);

  emscripten_console_log("releasing semaphore resources on main thread should succeed");
  idx = emscripten_semaphore_release(&available, 10);
  assert(idx == 0);

  emscripten_console_log("try_acquiring multiple resources from an available semaphore should succeed");
  idx = emscripten_semaphore_try_acquire(&available, 9);
  assert(idx == 1);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
