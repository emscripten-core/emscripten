#include <emscripten/emmalloc.h>
#include <stdio.h>
#include <emscripten/html5.h>
#include <emscripten/memory.h>

EM_BOOL tick(double time, void *userData) {
  static size_t previous_size = 0;
  static int count = 0;
  size_t memory_size = emscripten_memory_get_size();
  if (previous_size != memory_size) {
    printf("Memory size: %zu inc: %zu count: %d\n", memory_size, memory_size - previous_size, count);
  }

  previous_size = memory_size;
  void *ptr = malloc(16*1024*1024);
  if (!ptr) {
    printf("Cannot malloc anymore. Final heap size: %llu\n", (unsigned long long)emscripten_memory_get_size());
#ifdef REPORT_RESULT
    REPORT_RESULT(emscripten_memory_get_size());
#endif
  }
  count++;
  return !!ptr;
}

int main() {
  emscripten_request_animation_frame_loop(tick, 0);
}
