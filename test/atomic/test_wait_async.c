#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <assert.h>

// Test emscripten_atomic_wait_u64() and emscripten_atomic_notify() functions.

volatile int32_t addr = 0;

void run_test() {
  emscripten_out("worker_main");
#if __EMSCRIPTEN_WASM_WORKERS__
  emscripten_wasm_worker_sleep(1000 * 1000000ull); // Sleep one second.
#else
  emscripten_thread_sleep(1000); // Sleep one second.
#endif
  emscripten_out("worker: addr = 1234");
  emscripten_atomic_store_u32((void*)&addr, 1234);
  emscripten_out("worker: notify async waiting main thread");
  emscripten_atomic_notify((int32_t*)&addr, 1);
}


int numCalled = 0;

void asyncWaitShouldTimeout(int32_t* ptr,
                            uint32_t val,
                            ATOMICS_WAIT_RESULT_T waitResult,
                            void* userData) {
  emscripten_out("main: asyncWaitShouldTimeout");
  ++numCalled;
  assert(numCalled == 1);
  assert(ptr == &addr);
  assert(val == 1);
  assert(userData == (void*)42);
  assert(waitResult == ATOMICS_WAIT_TIMED_OUT);
}

void asyncWaitFinishedShouldNotBeCalled(int32_t* ptr,
                                        uint32_t val,
                                        ATOMICS_WAIT_RESULT_T waitResult,
                                        void* userData) {
  assert(0); // We should not reach here
}

// This test run in both wasm workers and pthreads mode
#ifdef __EMSCRIPTEN_WASM_WORKERS__

void worker_main() {
  run_test();
}

#else

pthread_t t;

void* thread_main(void* arg) {
  run_test();
  return 0;
}

#endif

void asyncWaitFinishedShouldBeOk(int32_t* ptr,
                                 uint32_t val,
                                 ATOMICS_WAIT_RESULT_T waitResult,
                                 void* userData) {
  emscripten_out("main: asyncWaitFinished");
  assert(ptr == &addr);
  assert(val == 1);
  assert(userData == (void*)42);
  ++numCalled;
  assert(numCalled == 2);
  assert(waitResult == ATOMICS_WAIT_OK);
  emscripten_out("test finished");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
#if !defined(__EMSCRIPTEN_WASM_WORKERS__)
  pthread_join(t, NULL);
#endif
}

int main() {
  emscripten_out("main: creating worker");

#ifdef __EMSCRIPTEN_WASM_WORKERS__
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(1024);
  emscripten_wasm_worker_post_function_v(worker, worker_main);
#else
  pthread_create(&t, NULL, thread_main, NULL);
#endif

  addr = 1;

  emscripten_out("Async waiting on address with unexpected value should return 'not-equal'");
  ATOMICS_WAIT_TOKEN_T ret = emscripten_atomic_wait_async((int32_t*)&addr, 2, asyncWaitFinishedShouldNotBeCalled, (void*)42, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting on address with unexpected value should return 'not-equal' also if timeout==0");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 2, asyncWaitFinishedShouldNotBeCalled, (void*)42, 0);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_out("Waiting for 0 milliseconds should return 'timed-out'");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 1, asyncWaitFinishedShouldNotBeCalled, (void*)42, 0);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_out("Waiting for >0 milliseconds should return 'ok' (but successfully time out in first call to asyncWaitFinished)");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 1, asyncWaitShouldTimeout, (void*)42, 10);
  assert(EMSCRIPTEN_IS_VALID_WAIT_TOKEN(ret));

  emscripten_out("Waiting for infinitely long should return 'ok' (and return 'ok' in second call to asyncWaitFinished)");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 1, asyncWaitFinishedShouldBeOk, (void*)42, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
  assert(EMSCRIPTEN_IS_VALID_WAIT_TOKEN(ret));
}
