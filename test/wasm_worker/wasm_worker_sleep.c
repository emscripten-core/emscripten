#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/html5.h>
#include <emscripten/wasm_worker.h>

int exitStatus = 1;

void do_exit() {
  emscripten_out("do_exit");
  emscripten_terminate_all_wasm_workers();
  emscripten_force_exit(0);
}

void worker_main() {
  double t0 = emscripten_performance_now();
  emscripten_wasm_worker_sleep(/*nsecs=*/1500*1000000);
  double t1 = emscripten_performance_now();
  assert(t1-t0 >= 1500);
  exitStatus = t1-t0 >= 1500;
  emscripten_wasm_worker_post_function_v(EMSCRIPTEN_WASM_WORKER_ID_PARENT, do_exit);
}

char stack[1024];

int main() {
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
  emscripten_exit_with_live_runtime();
}
