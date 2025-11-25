#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <emscripten/emmalloc.h>
#include <emscripten/heap.h>

size_t round_to_4k(size_t val){
  return (val + 4095) & ~4095;
}

void print_stats(const char* title) {
  /*
  // Uncomment for debugging:
  printf("%s: dynamic heap:          %zu\n", title, round_to_4k(emmalloc_dynamic_heap_size()));
  printf("%s: free dynamic memory:   %zu\n", title, round_to_4k(emmalloc_free_dynamic_memory()));
  printf("%s: unclaimed heap memory: %zu\n", title, round_to_4k(emmalloc_unclaimed_heap_memory()));
  printf("%s: sbrk:                  %#zx\n", title, round_to_4k((size_t)sbrk(0)));
  */
}

int main() {
  int did_free;
  printf("heap size: %zu\n", emscripten_get_heap_size());
  print_stats("init");

  void *ptr = malloc(32*1024*1024);
  void *ptr2 = malloc(4*1024*1024);
  assert(ptr);
  assert(ptr2);
  print_stats("after alloc");

  // After having allocated memory, trim the heap. This may or may not actually
  // trim (it is unspecified whether the heap is trimmed after initial mallocs)
  did_free = emmalloc_trim(0);
  // But print for posterity whether we did actually trim.
  printf("1st trim: did_free=%d\n", did_free);
  print_stats("1");

  // Trimming again back-to-back after a trim should not trim more.
  did_free = emmalloc_trim(0);
  printf("2nd trim: did_free=%d\n", did_free);
  print_stats("2");
  assert(!did_free);

  // Free one block. That should allow memory to be trimmed.
  free(ptr2);
  did_free = emmalloc_trim(0);
  printf("3rd trim: did_free=%d\n", did_free);
  print_stats("3");
  assert(did_free);

  // Free second block. That should also allow more memory to be trimmed.
  // Try trimming by leaving a padding.
  free(ptr);
  did_free = emmalloc_trim(100000);
  printf("4th trim: did_free=%d\n", did_free);
  print_stats("4");
  assert(did_free);
  assert(emmalloc_free_dynamic_memory() >= 100000);

  // Now try re-trimming by also discarding the padding. This should trim
  // more.
  did_free = emmalloc_trim(0);
  printf("5th trim: did_free=%d\n", did_free);
  print_stats("5");
  assert(did_free);
}
