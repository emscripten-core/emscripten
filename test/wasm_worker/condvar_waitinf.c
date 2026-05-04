#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/threading.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#else
#include <emscripten/wasm_worker.h>
#endif

emscripten_condvar_t condvar = EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER;
emscripten_lock_t mutex = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;

int globalVar = 0;
_Atomic bool waiterStarted = false;

#ifndef __EMSCRIPTEN_PTHREADS__
void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}
#endif

void waiter_main() {
  emscripten_out("waiter_main");
  emscripten_lock_waitinf_acquire(&mutex);
  emscripten_out("waiter: got mutex");
  assert(!globalVar);
  waiterStarted = true;

  while (!globalVar) {
    emscripten_out("waiter: condvar wait");
    emscripten_condvar_waitinf(&condvar, &mutex);
  }

  emscripten_out("waiter: done");
  emscripten_lock_release(&mutex);

#ifndef __EMSCRIPTEN_PTHREADS__
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT,
                                         do_exit);
#endif
}

void signaler_main() {
  emscripten_out("signaler_main");
  while (!waiterStarted) {
    // busy-wait for waiter
  }
  // At this point we know the waiter took the lock already.
  // That means that once we acquire the lock here the waiter
  // must be `emscripten_condvar_waitinf`.

  emscripten_out("signaler: aquiring lock");
  emscripten_lock_waitinf_acquire(&mutex);

  emscripten_out("signaler: incrementing globalVar");
  globalVar += 1;

  emscripten_out("signaler: signaling condition");
  emscripten_condvar_signal(&condvar, 1);

  emscripten_out("signaler: done");
  emscripten_lock_release(&mutex);
}

#ifdef __EMSCRIPTEN_PTHREADS__
void* waiter_pthread(void* arg) {
  waiter_main();
  return NULL;
}
void* signaler_pthread(void* arg) {
  signaler_main();
  return NULL;
}
#endif

int main() {
  emscripten_out("in main");

#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_t waiter;
  pthread_t signaler;
  pthread_create(&waiter, NULL, waiter_pthread, NULL);
  pthread_create(&signaler, NULL, signaler_pthread, NULL);
  pthread_join(waiter, NULL);
  pthread_join(signaler, NULL);
  emscripten_out("done");
#else
  emscripten_wasm_worker_t waiter = emscripten_malloc_wasm_worker(4096);
  emscripten_wasm_worker_post_function_v(waiter, waiter_main);

  emscripten_wasm_worker_t signaler = emscripten_malloc_wasm_worker(4096);
  emscripten_wasm_worker_post_function_v(signaler, signaler_main);
#endif
}
