#include <emscripten/atomic.h>
#include <emscripten/html5.h>
#include <assert.h>

#ifdef __EMSCRIPTEN_WASM_WORKERS__
#include <emscripten/wasm_worker.h>
#else
#include <pthread.h>
#endif

// Test emscripten_atomic_wait_u32() and emscripten_atomic_notify() functions.

volatile int32_t addr = 0;

void run_test() {
  emscripten_out("worker_main");
  emscripten_atomic_store_u32((void*)&addr, 1);

  emscripten_out("Waiting on address with unexpected value should return 'not-equal'");
  ATOMICS_WAIT_RESULT_T ret = emscripten_atomic_wait_u32((int32_t*)&addr, 2, /*timeout=*/-1);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting on address with unexpected value should return 'not-equal' also if timeout==0");
  ret = emscripten_atomic_wait_u32((int32_t*)&addr, 2, /*timeout=*/0);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting for 0 nanoseconds should return 'timed-out'");
  ret = emscripten_atomic_wait_u32((int32_t*)&addr, 1, /*timeout=*/0);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_out("Waiting for >0 nanoseconds should return 'timed-out'");
  ret = emscripten_atomic_wait_u32((int32_t*)&addr, 1, /*timeout=*/1000);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_out("Waiting for infinitely long should return 'ok'");
  emscripten_atomic_store_u32((void*)&addr, 3);
  ret = emscripten_atomic_wait_u32((int32_t*)&addr, 3, /*timeout=*/-1);
  assert(ret == ATOMICS_WAIT_OK);

  emscripten_out("Test finished");
}


// This test run in both wasm workers and pthreads mode
#ifdef __EMSCRIPTEN_WASM_WORKERS__

void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void worker_main() {
  run_test();
  assert(addr == 3);
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

#else

pthread_t t;

void* thread_main(void* arg) {
  run_test();
  return 0;
}

#endif


bool main_loop(double time, void *userData) {
  if (addr == 3) {
    // Burn one second to make sure worker finishes its test.
    emscripten_out("main: seen worker running");
    double t0 = emscripten_performance_now();
    while (emscripten_performance_now() < t0 + 1000);

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
  static char stack[1024];
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
  emscripten_runtime_keepalive_push();
#else
  pthread_create(&t, NULL, thread_main, NULL);
#endif

  emscripten_out("main: entering timeout loop to wait for wasm worker to run");
  emscripten_set_timeout_loop(main_loop, 50, 0);
}
