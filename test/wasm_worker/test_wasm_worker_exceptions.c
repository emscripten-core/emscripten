#include <assert.h>
#include <stdlib.h>

#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <emscripten/em_asm.h>
#include <emscripten/wasm_worker.h>

void do_abort() {
  emscripten_out("Hello from wasm worker!");
  abort();
}

int main() {
  emscripten_out("in main");
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024), do_abort);
  emscripten_exit_with_live_runtime();
  assert(false && "should never get here");
}
