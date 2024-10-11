#include <emscripten/emscripten.h>
#include <emscripten/console.h>
#include <emscripten/em_asm.h>
#include <emscripten/wasm_worker.h>
#include <assert.h>

// This is the code example in site/source/docs/api_reference/wasm_workers.rst
void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void run_in_worker() {
  emscripten_out("Hello from wasm worker!\n");
  EM_ASM(typeof checkStackCookie == 'function' && checkStackCookie());
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

int main() {
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stack size: */1024);
  assert(worker);
  emscripten_wasm_worker_post_function_v(worker, run_in_worker);
  emscripten_exit_with_live_runtime();
}
