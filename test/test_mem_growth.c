#include <assert.h>
#include <stdio.h>

#include <emscripten/emmalloc.h>
#include <emscripten/html5.h>
#include <emscripten/heap.h>

#define EXPECTED_FINAL_HEAP_SIZE (4*1024*1024*1024ll - 65536)

int main() {
  size_t prevheapsize = 0;
  int count = 0;
  while (1) {
    size_t heap_size = emscripten_get_heap_size();
    if (prevheapsize != heap_size) {
      printf("Heap size: %zu inc: %zu count: %d\n", heap_size, heap_size - prevheapsize, count);
    }

    prevheapsize = heap_size;
    void *ptr = malloc(16*1024*1024);
    if (!ptr) {
      printf("Cannot malloc anymore. Final heap size: %zu\n", emscripten_get_heap_size());
      assert(emscripten_get_heap_size() == EXPECTED_FINAL_HEAP_SIZE);
      return 0;
    }
    count++;
  }
}
