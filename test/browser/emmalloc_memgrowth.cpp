#include <stdio.h>
#include <stdlib.h>
#include <emscripten/heap.h>

uint64_t nextAllocationSize = 16*1024*1024;
bool allocHasFailed = false;

void grow_memory() {
  uint8_t *ptr = (uint8_t*)malloc((size_t)nextAllocationSize);
  EM_ASM({}, ptr); // Pass ptr out to confuse LLVM that it is used, so it won't optimize it away in -O1 and higher.
  size_t heapSize = emscripten_get_heap_size();
  printf("Allocated %zu: %d. Heap size: %zu\n", (size_t)nextAllocationSize, ptr ? 1 : 0, heapSize);
  if (ptr) {
    if (!allocHasFailed) {
      nextAllocationSize *= 2;
      // Make sure we don't overflow, and also exercise malloc(-1) to gracefully return 0 in ABORTING_MALLOC=0 mode.
      if (nextAllocationSize > 0xFFFFFFFFULL)
        nextAllocationSize = 0xFFFFFFFFULL;
    }
  } else {
    nextAllocationSize /= 2;
    allocHasFailed = true;
  }
}

int main() {
  // Exhaust all available memory.
  for(int i = 0; i < 50; ++i) {
    grow_memory();
  }
  // If we get this far without crashing on OOM, we are ok!
  printf("Test finished!\n");
  return 0;
}
