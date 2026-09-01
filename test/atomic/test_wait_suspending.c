#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <assert.h>

// Test emscripten_atomic_wait_suspending() function.

_Atomic int addr = 1;

void run_test() {
  emscripten_out("worker running");
#if __EMSCRIPTEN_WASM_WORKERS__
  emscripten_wasm_worker_sleep(1000 * 1000000ull); // Sleep one second.
#else
  emscripten_thread_sleep(1000); // Sleep one second.
#endif
  emscripten_out("worker: addr = 1234");
  addr = 1234;
  emscripten_out("worker: notify async waiting main thread");
  emscripten_atomic_notify((int32_t*)&addr, 1);
}


// This test run in both wasm workers and pthreads mode
#ifdef __EMSCRIPTEN_WASM_WORKERS__

void worker_main() {
  run_test();
}

#else

pthread_t t;

bool done = false;

void keepalive(void* user_data) {
  if (!done) {
    emscripten_set_timeout(keepalive, 100, NULL);
  }
}

void* thread_main(void* arg) {
  run_test();
  return 0;
}

#endif

int main() {
  emscripten_out("main: creating worker");

#ifdef __EMSCRIPTEN_WASM_WORKERS__
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(4096);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
#else
  pthread_create(&t, NULL, thread_main, NULL);
  // This is bit of hack to keep the node process alive.  Without this,
  // the whole node process will exit when we suspend them main thread below.
  emscripten_set_timeout(keepalive, 100, NULL);
#endif

  ATOMICS_WAIT_TOKEN_T ret;

  emscripten_out("Async waiting on address with unexpected value should return 'not-equal'");
  ret = emscripten_atomic_wait_suspending((int32_t*)&addr, 2, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting on address with unexpected value should return 'not-equal' also if timeout==0");
  ret = emscripten_atomic_wait_suspending((int32_t*)&addr, 2, 0);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting for 0 milliseconds should return 'timed-out'");
  ret = emscripten_atomic_wait_suspending((int32_t*)&addr, 1, 0);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_out("Waiting for >0 milliseconds should also timeout");
  ret = emscripten_atomic_wait_suspending((int32_t*)&addr, 1, 10);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_out("Waiting for infinitely long should return once we have been notified");
  ret = emscripten_atomic_wait_suspending((int32_t*)&addr, 1, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
  assert(ret == ATOMICS_WAIT_OK);
  assert(addr == 1234);

#ifdef __EMSCRIPTEN_WASM_WORKERS__
  emscripten_terminate_all_wasm_workers();
#else
  done = true;
  pthread_join(t, NULL);
#endif

  emscripten_out("test finished");
  return 0;
}
