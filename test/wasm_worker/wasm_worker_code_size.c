#include <emscripten.h>
#include <emscripten/wasm_worker.h>

// This file contains an absolute minimal sized Wasm Worker example, to keep a check on generated code size.

EM_JS(void, hello, (void), {
  console.log("Hello from wasm worker!");
});

void run_in_worker() {
  hello();
}

int main() {
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), run_in_worker);
}
