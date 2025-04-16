#include <stdio.h>
#include <string.h>

#include <emscripten/heap.h>

int main() {
  char buf[1024];
  void* oob_address = (void*)(emscripten_get_heap_size() + 10);
  memcpy(buf, oob_address, 0);
  printf("done\n");
}
