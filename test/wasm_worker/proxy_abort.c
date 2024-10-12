#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// Tests behavior of __proxy: 'abort' and __proxy: 'abort_debug' in Wasm Workers.

void proxied_js_function(void);

int might_throw(void(*func)()) {
  int threw = EM_ASM_INT({
    // Patch over assert() so that it does not abort execution on assert failure, but instead
    // throws a catchable exception.
    assert = function(condition, text) {
      if (!condition) {
        throw 'Assertion failed' + (text ? ": " + text : "");
      }
    };

    try {
      getWasmTableEntry($0)();
    } catch(e) {
      console.error('Threw an exception: ' + e);
      return e.toString().includes('this function has been declared to only be callable from the main browser thread');
    }
    console.error('Function did not throw');
    return 0;
  }, func);
  return threw;
}

void test() {
  proxied_js_function();
}

void worker_main() {
  REPORT_RESULT(might_throw(test));
}

char stack[1024];

int main() {
  proxied_js_function(); // Should be callable from main thread
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), worker_main);
}
