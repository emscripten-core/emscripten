#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/wasm_worker.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#endif

// Tests emscripten_semaphore_wait_acquire() and
// emscripten_semaphore_try_acquire() in Worker.

emscripten_semaphore_t sem = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(0);

void worker_main() {
  // 1. Immediate timeout (maxWaitNanoseconds = 0) when unavailable.
  double t0 = emscripten_performance_now();
  int ret = emscripten_semaphore_wait_acquire(&sem, 1, 0);
  double t1 = emscripten_performance_now();
  assert(ret == -1);
  assert(t1 - t0 < 50); // Should be near-instant
  assert(sem == 0);     // Should not have been decremented

  // 2. Timed wait that times out.
  t0 = emscripten_performance_now();
  ret = emscripten_semaphore_wait_acquire(&sem, 1, 100 * 1000000ull); // 100ms
  t1 = emscripten_performance_now();
  assert(ret == -1);
  assert(t1 - t0 >= 100);
  assert(sem == 0); // Should not have been decremented

  // 3. Immediate acquisition when available.
  emscripten_semaphore_release(&sem, 2);
  assert(sem == 2);
  ret = emscripten_semaphore_wait_acquire(&sem, 1, 100 * 1000000ull);
  assert(ret == 1); // returns old value minus 1
  // If sem was 2, and we acquire 1, it returns 2 - 1 = 1.
  assert(sem == 1);

  // 4. Try acquire
  ret = emscripten_semaphore_try_acquire(&sem, 1);
  assert(ret == 0);
  assert(sem == 0);

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

#ifdef __EMSCRIPTEN_PTHREADS__
void* thread_main(void* arg) {
  worker_main();
  return NULL;
}
#else
char stack[4096];
#endif

int main() {
#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_t t;
  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);
  printf("done\n");
#else
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
#endif
  return 0;
}
