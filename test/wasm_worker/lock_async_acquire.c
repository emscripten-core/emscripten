#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <assert.h>

// Tests emscripten_lock_async_acquire() function.

emscripten_lock_t lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;

// Two shared variables, always a delta distance of one from each other.
volatile int sharedState0 = 0;
volatile int sharedState1 = 1;

bool testFinished = false;

int numTimesMainThreadAcquiredLock = 0;
int numTimesWasmWorkerAcquiredLock = 0;

void work() {
  // emscripten_console_log("work");
  volatile int x = sharedState0;
  volatile int y = sharedState1;
  assert(x == y+1 || y == x+1);

  if (emscripten_current_thread_is_wasm_worker()) {
    ++numTimesWasmWorkerAcquiredLock;
  } else {
    ++numTimesMainThreadAcquiredLock;
  }

  if (x < y) {
    x = y + 1;
    if (emscripten_current_thread_is_wasm_worker()) {
      emscripten_wasm_worker_sleep(/*nsecs=*/(rand()%100000));
    }
    sharedState0 = x;
  } else {
    y = x + 1;
    if (emscripten_current_thread_is_wasm_worker())
      emscripten_wasm_worker_sleep(/*nsecs=*/(rand()%100000));
    sharedState1 = y;

    if (y > 100 && numTimesMainThreadAcquiredLock && numTimesWasmWorkerAcquiredLock) {
      if (!testFinished) {
        emscripten_console_log("test finished");
#ifdef REPORT_RESULT
        REPORT_RESULT(0);
#endif
      }
      testFinished = true;
    }
  }
}

void schedule_work(void *userData);

void lock_async_acquired(volatile void *addr, uint32_t val, ATOMICS_WAIT_RESULT_T waitResult, void *userData) {
  // emscripten_console_log("async lock acquired");
  assert(addr == &lock);
  assert(val == 0 || val == 1);
  assert(waitResult == ATOMICS_WAIT_OK);
  assert(userData == (void*)42);
  work();
  emscripten_lock_release(&lock);

  if (!testFinished) {
    emscripten_set_timeout(schedule_work, 10, 0);
  }
}

void schedule_work(void *userData) {
  if (emscripten_current_thread_is_wasm_worker() && emscripten_random() > 0.5) {
    emscripten_lock_waitinf_acquire(&lock);
    // emscripten_console_log("sync lock acquired");
    work();
    emscripten_lock_release(&lock);
    if (!testFinished)
      emscripten_set_timeout(schedule_work, 0, 0);
  } else {
    emscripten_lock_async_acquire(&lock, lock_async_acquired, (void*)42, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
  }
}

void start_worker(int arg) {
  schedule_work(0);
}

int main() {
#define NUM_THREADS 10
  for (int i = 0; i < NUM_THREADS; ++i) {
    emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(1024);
    emscripten_wasm_worker_post_function_vi(worker, start_worker, 0);
  }

  schedule_work(0);
}
