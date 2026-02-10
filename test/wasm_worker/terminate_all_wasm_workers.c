#include <emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Tests that calling emscripten_terminate_all_wasm_workers() properly terminates
// each child Wasm Worker of the calling thread.

static volatile int worker_started = 0;

void this_function_should_not_be_called(void *userData) {
  worker_started = -1;
  emscripten_err("this_function_should_not_be_called");
#ifdef REPORT_RESULT
  REPORT_RESULT(1/*fail*/);
#endif
}

void test_passed(void *userData) {
  if (worker_started == 2) {
    emscripten_err("test_passed");
#ifdef REPORT_RESULT
    REPORT_RESULT(0/*ok*/);
#endif
  }
}

void worker_main() {
  ++worker_started;
  emscripten_err("Hello from wasm worker!");
  // Schedule a function to be called, that should never happen, since the Worker
  // dies before that.
  emscripten_set_timeout(this_function_should_not_be_called, 2000, 0);
}

char stack1[1024];
char stack2[1024];

int should_throw(void(*func)(emscripten_wasm_worker_t worker), emscripten_wasm_worker_t worker) {
  int threw = EM_ASM_INT({
    try {
      dynCall('vi', $0, $1);
    } catch(e) {
      console.error('Threw an exception like expected: ' + e);
      return 1;
    }
    console.error('Function was expected to throw, but did not!');
    return 0;
  }, func, worker);
  return threw;
}

emscripten_wasm_worker_t worker[2];

void post_bad_function(emscripten_wasm_worker_t worker) {
  // Try to post a function to the worker, this should throw
  emscripten_wasm_worker_post_function_vi(worker, (void(*)(int))this_function_should_not_be_called, 0);
}

void terminate_worker(void *userData) {
  emscripten_terminate_all_wasm_workers();
  assert(should_throw(post_bad_function, worker[0]));
  assert(should_throw(post_bad_function, worker[1]));
}

int main() {
  worker[0] = emscripten_create_wasm_worker(stack1, sizeof(stack1));
  worker[1] = emscripten_create_wasm_worker(stack2, sizeof(stack2));
  emscripten_wasm_worker_post_function_v(worker[0], worker_main);
  emscripten_wasm_worker_post_function_v(worker[1], worker_main);

  // Terminate both workers after a small delay
  emscripten_set_timeout(terminate_worker, 1000, 0);

  // Wait a while, if the bad function does not trigger, then the test succeeds.
  emscripten_set_timeout(test_passed, 3000, 0);
}
