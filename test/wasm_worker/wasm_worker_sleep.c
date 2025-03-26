#include <emscripten/html5.h>
#include <emscripten/wasm_worker.h>

void worker_main() {
  double t0 = emscripten_performance_now();
  emscripten_wasm_worker_sleep(/*nsecs=*/1500*1000000);
  double t1 = emscripten_performance_now();
#ifdef REPORT_RESULT
  REPORT_RESULT(t1-t0 >= 1500);
#endif
}

char stack[1024];

int main() {
  emscripten_wasm_worker_t worker = emscripten_create_wasm_worker(stack, sizeof(stack));
  emscripten_wasm_worker_post_function_v(worker, worker_main);
}
