#include <stdio.h>
#include <emscripten/emmalloc.h>

size_t round_to_4k(size_t val) {
  return (val + 4095) & ~4095;
}

int main() {
  void *ptr = malloc(32*1024*1024);
  void *ptr2 = malloc(4*1024*1024);
  void *ptr3 = malloc(64*1024*1024);
  void *ptr4 = malloc(16*1024);
  void *ptr5 = malloc(2*1024*1024);
  printf("valid allocs: %d\n", (int)(ptr && ptr2 && ptr3 && ptr4 && ptr5));
  free(ptr2);
  free(ptr4);
  printf("emmalloc_validate_memory_regions: %d\n", emmalloc_validate_memory_regions());
  printf("emmalloc_dynamic_heap_size      : %zu\n", emmalloc_dynamic_heap_size());
  printf("emmalloc_free_dynamic_memory    : %zu\n", emmalloc_free_dynamic_memory());
  emmalloc_dump_free_dynamic_memory_fragmentation_map();
  printf("emmalloc_unclaimed_heap_memory  : %zu\n", round_to_4k(emmalloc_unclaimed_heap_memory()));
}
