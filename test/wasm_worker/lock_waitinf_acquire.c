#include <assert.h>
#include <emscripten.h>
#include <emscripten/em_math.h>
#include <emscripten/threading.h>
#include <stdlib.h>

#define _GNU_SOURCE
#include <unistd.h> // for gettid

// This test can be run under pthreads *or* Wasm Workers
#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#endif

#ifdef __EMSCRIPTEN_WASM_WORKERS__
#include <emscripten/wasm_worker.h>
#else
// When WASM_WORKERS is not defined we create dummy/fake version of
// emscripten_wasm_worker_sleep.
void emscripten_wasm_worker_sleep(int64_t nsecs) {
  emscripten_thread_sleep(nsecs / 1000000);
}
#endif

// Tests emscripten_lock_init(), emscripten_lock_waitinf_acquire() and emscripten_lock_release()

emscripten_lock_t lock = (emscripten_lock_t)12345315; // initialize with garbage

// Two shared variables, always a delta distance of one from each other.
volatile int sharedState0 = 0;
volatile int sharedState1 = 1;

#define NUM_THREADS 4

volatile int numWorkersAlive = NUM_THREADS;

void test_ended() {
  emscripten_outf("Worker %d last thread to finish. Reporting test end with sharedState0=%d, sharedState1=%d", gettid(), sharedState0, sharedState1);
  assert(sharedState0 == sharedState1 + 1 || sharedState1 == sharedState0 + 1);
  assert(sharedState0 == 4000);
  emscripten_out("done");
#if __EMSCRIPTEN_PTHREADS__
  exit(0);
#else
  REPORT_RESULT(0);
#endif
}

void worker_main() {
  emscripten_outf("Worker %d running...", gettid());
  // Create contention on the lock from each thread, and stress the shared state
  // in a racy way that would show a breakage if the lock is not watertight.
  for (int i = 0; i < 1000; ++i) {
    emscripten_lock_waitinf_acquire(&lock);
    volatile int x = sharedState0;
    volatile int y = sharedState1;
    assert(x == y+1 || y == x+1);
    if (x < y) {
      x = y + 1;
      emscripten_wasm_worker_sleep(/*nsecs=*/((uint64_t)(emscripten_math_random()*1000)));
      sharedState0 = x;
    } else {
      y = x + 1;
      emscripten_wasm_worker_sleep(/*nsecs=*/((uint64_t)(emscripten_math_random()*1000)));
      sharedState1 = y;
    }
    emscripten_lock_release(&lock);
  }

  emscripten_outf("Worker %d finished.", gettid());

  // Are we the last thread to finish? If so, test has ended.
  uint32_t v = emscripten_atomic_sub_u32((void*)&numWorkersAlive, 1);
  if (v == 1) {
    test_ended();
  }
}

#ifdef __EMSCRIPTEN_PTHREADS__
pthread_t threads[NUM_THREADS];

void* pthread_main(void* arg) {
  worker_main();
  return NULL;
}
#endif

int main() {
  emscripten_lock_init(&lock);

  for (int i = 0; i < NUM_THREADS; ++i) {
#ifdef __EMSCRIPTEN_PTHREADS__
    pthread_create(&threads[i], NULL, pthread_main, NULL);
#else
    emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(4096);
    emscripten_wasm_worker_post_function_v(worker, worker_main);
#endif
  }

#ifdef __EMSCRIPTEN_PTHREADS__
  for (int i = 0; i < NUM_THREADS; ++i) {
    pthread_join(threads[i], NULL);
  }
#endif

  return 0;
}
