#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/eventloop.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Tests that calling emscripten_terminate_wasm_worker() properly terminates
// a Wasm Worker.

static volatile int worker_started = 0;

void this_function_should_not_be_called(void *userData) {
  worker_started = -1;
  emscripten_console_error("this_function_should_not_be_called");
#ifdef REPORT_RESULT
  REPORT_RESULT(1/*fail*/);
#endif
}

void test_passed(void *userData) {
  if (worker_started == 1) {
    emscripten_console_error("test_passed");
#ifdef REPORT_RESULT
    REPORT_RESULT(0/*ok*/);
#endif
  }
}

void worker_main() {
  worker_started = 1;
  emscripten_console_error("Hello from wasm worker!");
  // Schedule a function to be called, that should never happen, since the Worker
  // dies before that.
  emscripten_set_timeout(this_function_should_not_be_called, 2000, 0);
}

char stack[1024];

int should_throw(void(*func)()) {
  int threw = EM_ASM_INT({
    try {
      dynCall('v', Number($0));
    } catch(e) {
      console.error('Threw an exception like expected: ' + e);
      return 1;
    }
    console.error('Function was expected to throw, but did not!');
    return 0;
  }, func);
  return threw;
}

emscripten_wasm_worker_t worker = 0;

void post_bad_function() {
  // Try to post a function to the worker, this should throw
  emscripten_wasm_worker_post_function_vi(worker, (void(*)(int))this_function_should_not_be_called, 0);
}

void terminate_worker(void *userData) {
  emscripten_terminate_wasm_worker(worker);
  assert(should_throw(post_bad_function));
}

int main() {
  worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);

  // Terminate the worker after a small delay
  emscripten_set_timeout(terminate_worker, 1000, 0);

  // Wait a while, if the bad function does not trigger, then the test succeeds.
  emscripten_set_timeout(test_passed, 3000, 0);
}
