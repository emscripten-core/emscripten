#include <emscripten/html5.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <assert.h>

// Test emscripten_wasm_wait_i32() and emscripten_wasm_notify() functions.

volatile int32_t addr = 0;

void worker_main()
{
  emscripten_console_log("worker_main");
  emscripten_atomic_store_u32((void*)&addr, 1);

  emscripten_console_log("Waiting on address with unexpected value should return 'not-equal'");
  ATOMICS_WAIT_RESULT_T ret = emscripten_wasm_wait_i32((int32_t*)&addr, 2, /*timeout=*/-1);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_console_log("Waiting on address with unexpected value should return 'not-equal' also if timeout==0");
  ret = emscripten_wasm_wait_i32((int32_t*)&addr, 2, /*timeout=*/0);
  assert(ret == ATOMICS_WAIT_NOT_EQUAL);

  emscripten_console_log("Waiting for 0 nanoseconds should return 'timed-out'");
  ret = emscripten_wasm_wait_i32((int32_t*)&addr, 1, /*timeout=*/0);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_console_log("Waiting for >0 nanoseconds should return 'timed-out'");
  ret = emscripten_wasm_wait_i32((int32_t*)&addr, 1, /*timeout=*/1000);
  assert(ret == ATOMICS_WAIT_TIMED_OUT);

  emscripten_console_log("Waiting for infinitely long should return 'ok'");
  ret = emscripten_wasm_wait_i32((int32_t*)&addr, 1, /*timeout=*/-1);
  assert(ret == ATOMICS_WAIT_OK);

  emscripten_console_log("Test finished");

#ifdef REPORT_RESULT
  REPORT_RESULT(addr);
#endif
}

char stack[1024];

EM_BOOL main_loop(double time, void *userData)
{
  if (addr == 1)
  {
    // Burn one second to make sure worker finishes its test.
    emscripten_console_log("main: seen worker running");
    double t0 = emscripten_performance_now();
    while(emscripten_performance_now() < t0 + 1000);

    // Wake the waiter
    emscripten_console_log("main: waking worker");
    addr = 2;
    emscripten_wasm_notify((int32_t*)&addr, 1);

    return EM_FALSE;
  }
  return EM_TRUE;
}

int main()
{
  emscripten_console_log("main: creating worker");
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_console_log("main: posting function");
  emscripten_wasm_worker_post_function_v(worker, worker_main);

  emscripten_console_log("main: entering timeout loop to wait for wasm worker to run");
  emscripten_set_timeout_loop(main_loop, 1000, 0);
}
