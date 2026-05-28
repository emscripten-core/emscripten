#include <assert.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <stdlib.h>

// This test can be run under pthreads *or* Wasm Workers
#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
_Atomic bool done = false;
#else
#include <emscripten/wasm_worker.h>
#endif

// Tests emscripten_lock_busyspin_waitinf_acquire().

emscripten_lock_t lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;

volatile int sharedState = 0;

void worker_main() {
  emscripten_out("worker_main");
  emscripten_lock_busyspin_waitinf_acquire(&lock);
  emscripten_atomic_add_u32((void*)&sharedState, 1);
  assert(sharedState == 1);
#ifdef __EMSCRIPTEN_PTHREADS__
  emscripten_out("done");
  done = true;
  exit(0);
#else
  REPORT_RESULT(0);
#endif
}

#ifdef __EMSCRIPTEN_PTHREADS__
pthread_t t;

void* pthread_main(void* arg) {
  worker_main();
  return NULL;
}

void nothing(void* userData) {
  if (!done) {
    emscripten_set_timeout(nothing, 100, 0);
  }
}
#else
char stack[4096];
#endif

void releaseLock(void *userData) {
  emscripten_out("releaseLock");
  emscripten_atomic_sub_u32((void*)&sharedState, 1);
  emscripten_lock_release(&lock);
}

int main() {
  emscripten_out("in main");
  // Acquire the lock at startup.
  emscripten_lock_busyspin_waitinf_acquire(&lock);
  emscripten_atomic_add_u32((void*)&sharedState, 1);

#ifdef __EMSCRIPTEN_PTHREADS__
  // Spawn a Pthread to try to take the lock. It will succeed only after
  // releaseLock() gets called.
  pthread_create(&t, NULL, pthread_main, NULL);
  // Add an infinite timeout to make sure the node runtime stays alive
  // after main returns.
  // See https://github.com/emscripten-core/emscripten/issues/23092
  emscripten_set_timeout(nothing, 100, 0);
#else
  // Spawn a Worker to try to take the lock. It will succeed only after releaseLock()
  // gets called.
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
#endif

  emscripten_set_timeout(releaseLock, 1000, 0);
  return 0;
}
