#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <emscripten/emmalloc.h>
#include <emscripten/heap.h>

size_t round_to_4k(size_t val){
  return (val + 4095) & ~4095;
}

void print_stats(const char* title) {
  printf("%s: dynamic heap:          %zu\n", title, round_to_4k(emmalloc_dynamic_heap_size()));
  printf("%s: free dynamic memory:   %zu\n", title, round_to_4k(emmalloc_free_dynamic_memory()));
  printf("%s: unclaimed heap memory: %zu\n", title, round_to_4k(emmalloc_unclaimed_heap_memory()));
  printf("%s: sbrk:                  %#zx\n", title, round_to_4k((size_t)sbrk(0)));
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

  did_free = emmalloc_trim(0);
  assert(did_free);
  printf("1st trim: did_free=%d\n", did_free);
  print_stats("1");

  did_free = emmalloc_trim(0);
  assert(!did_free);
  printf("2nd trim: did_free=%d\n", did_free);
  print_stats("2");
  free(ptr2);

  did_free = emmalloc_trim(100000);
  assert(did_free);
  printf("3rd trim: did_free=%d\n", did_free);
  print_stats("3");

  did_free = emmalloc_trim(100000);
  assert(!did_free);
  printf("4th trim: did_free=%d\n", did_free);
  print_stats("4");

  did_free = emmalloc_trim(0);
  assert(did_free);
  printf("5th trim: did_free=%d\n", did_free);
  print_stats("5");
}
