#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Test that proxied JS functions cannot be called in Wasm Workers in ASSERTIONS builds.

void proxied_js_function(void);

int should_throw(void(*func)()) {
  int threw = EM_ASM_INT({
    // Patch over assert() so that it does not abort execution on assert failure, but instead
    // throws a catchable exception.
    assert = function(condition, text) {
      if (!condition) {
        throw 'Assertion failed' + (text ? ": " + text : "");
      }
    };

    try {
      dynCall('v', $0);
    } catch(e) {
      console.error('Threw an exception like expected: ' + e);
      return 1;
    }
    console.error('Function was expected to throw, but did not!');
    return 0;
  }, func);
  return threw;
}

void test() {
  proxied_js_function();
}

void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void worker_main() {
  assert(should_throw(test));
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

char stack[1024];

int main() {
  proxied_js_function();
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
  emscripten_exit_with_live_runtime();
}
