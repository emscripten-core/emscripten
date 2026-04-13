#include <stdlib.h>

#include <emscripten.h>
#include <emscripten/wasm_worker.h>

static void worker_loop(void) {
  for (;;) {
    free(malloc(1));
  }
}

static void main_loop(void) {
  static unsigned ticks;
  malloc(1);
  if (++ticks == 120) {
    emscripten_force_exit(0);
  }
}

int main(void) {
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024 * 1024), worker_loop);
  emscripten_set_main_loop(main_loop, 0, false);
  return 0;
}
