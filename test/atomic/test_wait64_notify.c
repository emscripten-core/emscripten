#include <emscripten/atomic.h>
#include <emscripten/html5.h> // for emscripten_performance_now()
#include <emscripten.h>
#include <assert.h>

#ifdef __EMSCRIPTEN_WASM_WORKERS__
#include <emscripten/wasm_worker.h>
#else
#include <pthread.h>
#endif

// Test emscripten_atomic_wait_u64() and emscripten_atomic_notify() functions.

volatile int64_t addr = 0;

void run_test() {
  emscripten_out("worker_main");
  emscripten_atomic_store_u64((void*)&addr, 0x100000000ull);

  emscripten_out("Waiting on address with unexpected value should return 'not-equal'");
  ATOMICS_WAIT_RESULT_T ret = emscripten_atomic_wait_u64((int64_t*)&addr, 0x200000000ull, /*timeout=*/-1);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting on address with unexpected value should return 'not-equal' also if timeout==0");
  ret = emscripten_atomic_wait_u64((int64_t*)&addr, 0x200000000ull, /*timeout=*/0);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting for 0 nanoseconds should return 'timed-out'");
  ret = emscripten_atomic_wait_u64((int64_t*)&addr, 0x100000000ull, /*timeout=*/0);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_out("Waiting for >0 nanoseconds should return 'timed-out'");
  ret = emscripten_atomic_wait_u64((int64_t*)&addr, 0x100000000ull, /*timeout=*/1000);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_out("Waiting for infinitely long should return 'ok'");
  emscripten_atomic_store_u64((void*)&addr, 0x300000000ull);
  ret = emscripten_atomic_wait_u64((int64_t*)&addr, 0x300000000ull, /*timeout=*/-1);
  assert(ret == ATOMICS_WAIT_OK);

  emscripten_out("Test finished");
}


// This test run in both wasm workers and pthreads mode
#ifdef __EMSCRIPTEN_WASM_WORKERS__

char stack[1024];

void worker_main() {
  run_test();
#ifdef REPORT_RESULT
  REPORT_RESULT(addr >> 32);
#endif
}

#else

pthread_t t;

void* thread_main(void* arg) {
  run_test();
  return 0;
}

#endif

bool main_loop(double time, void *userData) {
  if (addr == 0x300000000ull) {
    // Burn one second to make sure worker finishes its test.
    emscripten_out("main: seen worker running");
    double t0 = emscripten_performance_now();
    while(emscripten_performance_now() < t0 + 1000);

    // Wake the waiter
    emscripten_out("main: waking worker");
    emscripten_atomic_notify((int32_t*)&addr, 1);

#ifndef __EMSCRIPTEN_WASM_WORKERS__
    pthread_join(t, NULL);
#endif

    return false;
  }
  return true;
}

int main() {
  emscripten_out("main: creating worker");

#ifdef __EMSCRIPTEN_WASM_WORKERS__
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
#else
  pthread_create(&t, NULL, thread_main, NULL);
#endif

  emscripten_out("main: entering timeout loop to wait for wasm worker to run");
  emscripten_set_timeout_loop(main_loop, 50, 0);
}
