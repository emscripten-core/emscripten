#include <cstdint>
#include <emscripten.h>
#include <emscripten/wasm_worker.h>

static void worker_loop() {
  for (;;) {
    delete new std::uint8_t{0};
  }
}

static void main_loop() {
  static unsigned ticks;
  new std::uint8_t{0};
  if (++ticks == 120) {
    emscripten_force_exit(0);
  }
}

int main() {
  emscripten_wasm_worker_post_function_v(emscripten_malloc_wasm_worker(1024 * 1024), worker_loop);
  emscripten_set_main_loop(main_loop, 0, false);
  return 0;
}
