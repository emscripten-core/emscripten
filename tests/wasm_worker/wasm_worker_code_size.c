#include <emscripten.h>
#include <emscripten/wasm_worker.h>

// This file contains an absolute minimal sized Wasm Worker example, to keep a check on generated code size.

void run_in_worker()
{
  EM_ASM(console.log("Hello from wasm worker!"));
}

int main()
{
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), run_in_worker);
}
