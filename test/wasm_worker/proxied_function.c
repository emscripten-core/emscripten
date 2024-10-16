// This test checks whether attempting to call a proxied JS function (one with __proxy signature) will throw
// an exception.

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

void proxied_js_function(void);

void test_finished() {
  REPORT_RESULT(0);
}

void run_in_worker() {
  int threw = EM_ASM_INT({
    try {
      _proxied_js_function();
    } catch(e) {
      console.error(e);
      return 1;
    }
    return 0;
  });

  if (!threw) {
    emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, test_finished);
  }
}

int main() {
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), run_in_worker);
  // Pin a dependency from C code to the JS function to avoid needing to mess
  // with cmdline export directives
  proxied_js_function();
}
