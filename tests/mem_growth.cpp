#include <emscripten/emmalloc.h>
#include <stdio.h>
#include <emscripten/html5.h>
#include <emscripten/heap.h>

EM_BOOL tick(double time, void *userData) {
  static size_t prevheapsize = 0;
  static int count = 0;
  size_t heap_size = emscripten_get_heap_size();
  if (prevheapsize != heap_size) {
    printf("Heap size: %zu inc: %zu count: %d\n", heap_size, heap_size - prevheapsize, count);
  }

  prevheapsize = heap_size;
  void *ptr = malloc(16*1024*1024);
  if (!ptr) {
    printf("Cannot malloc anymore. Final heap size: %llu\n", (unsigned long long)emscripten_get_heap_size());
#ifdef REPORT_RESULT
    REPORT_RESULT(emscripten_get_heap_size());
#endif
  }
  count++;
  return !!ptr;
}

int main() {
  emscripten_request_animation_frame_loop(tick, 0);
}
