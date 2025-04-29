#include <stdio.h>
#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/wasm_worker.h>

using namespace emscripten;

int foo() {
  return 42;
}

void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
}

void run_in_worker() {
  emscripten_out("Hello from Wasm Worker!");
  int result = val::module_property("foo")().as<int>();
  assert(result == 42);
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

int main() {
  emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(/*stackSize: */1024);
  emscripten_wasm_worker_post_function_v(worker, run_in_worker);
  emscripten_exit_with_live_runtime();
}

EMSCRIPTEN_BINDINGS(xxx) {
  function("foo", &foo);
}
