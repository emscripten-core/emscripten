#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <assert.h>

// Test emscripten_wasm_wait_i64() and emscripten_wasm_notify() functions.

volatile int32_t addr = 0;

void worker_main()
{
  emscripten_console_log("worker_main");
  emscripten_wasm_worker_sleep(1000 * 1000000ull); // Sleep one second.
  emscripten_console_log("worker: addr = 1234");
  emscripten_atomic_store_u32((void*)&addr, 1234);
  emscripten_console_log("worker: notify async waiting main thread");
  emscripten_wasm_notify((int32_t*)&addr, 1);
}

char stack[1024];

int numCalled = 0;

void asyncWaitShouldTimeout(int32_t *ptr, uint32_t val, ATOMICS_WAIT_RESULT_T waitResult, void *userData)
{
  emscripten_console_log("main: asyncWaitShouldTimeout");
  ++numCalled;
  assert(numCalled == 1);
  assert(ptr == &addr);
  assert(val == 1);
  assert(userData == (void*)42);
  assert(waitResult == ATOMICS_WAIT_TIMED_OUT);
}

void asyncWaitFinishedShouldNotBeCalled(int32_t *ptr, uint32_t val, ATOMICS_WAIT_RESULT_T waitResult, void *userData)
{
  assert(0); // We should not reach here
}

void asyncWaitFinishedShouldBeOk(int32_t *ptr, uint32_t val, ATOMICS_WAIT_RESULT_T waitResult, void *userData)
{
  emscripten_console_log("main: asyncWaitFinished");
  assert(ptr == &addr);
  assert(val == 1);
  assert(userData == (void*)42);
  ++numCalled;
  assert(numCalled == 2);
  assert(waitResult == ATOMICS_WAIT_OK);
  emscripten_console_log("test finished");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}

int main()
{
  emscripten_console_log("main: creating worker");
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_console_log("main: posting function");
  emscripten_wasm_worker_post_function_v(worker, worker_main);

  addr = 1;

  emscripten_console_log("Async waiting on address with unexpected value should return 'not-equal'");
  ATOMICS_WAIT_TOKEN_T ret = emscripten_atomic_wait_async((int32_t*)&addr, 2, asyncWaitFinishedShouldNotBeCalled, (void*)42, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_console_log("Waiting on address with unexpected value should return 'not-equal' also if timeout==0");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 2, asyncWaitFinishedShouldNotBeCalled, (void*)42, 0);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_console_log("Waiting for 0 milliseconds should return 'timed-out'");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 1, asyncWaitFinishedShouldNotBeCalled, (void*)42, 0);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_console_log("Waiting for >0 milliseconds should return 'ok' (but successfully time out in first call to asyncWaitFinished)");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 1, asyncWaitShouldTimeout, (void*)42, 10);
  assert(EMSCRIPTEN_IS_VALID_WAIT_TOKEN(ret));

  emscripten_console_log("Waiting for infinitely long should return 'ok' (and return 'ok' in second call to asyncWaitFinished)");
  ret = emscripten_atomic_wait_async((int32_t*)&addr, 1, asyncWaitFinishedShouldBeOk, (void*)42, EMSCRIPTEN_WAIT_ASYNC_INFINITY);
  assert(EMSCRIPTEN_IS_VALID_WAIT_TOKEN(ret));
}
