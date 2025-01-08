#include <stdio.h>
#include <unistd.h>
#include <emscripten/emmalloc.h>
#include <emscripten/heap.h>

size_t round_to_4k(size_t val){
  return (val + 4095) & ~4095;
}

void print_stats(int cnt) {
  printf("dynamic heap %d: %zu\n", cnt, round_to_4k(emmalloc_dynamic_heap_size()));
  printf("free dynamic memory %d: %zu\n", cnt, round_to_4k(emmalloc_free_dynamic_memory()));
  printf("unclaimed heap memory %d: %zu\n", cnt, round_to_4k(emmalloc_unclaimed_heap_memory()));
  printf("sbrk %d: %#zx\n", cnt, round_to_4k((size_t)sbrk(0)));
}

int main() {
  printf("heap size: %zu\n", emscripten_get_heap_size());
  print_stats(0);

  void *ptr = malloc(32*1024*1024);
  void *ptr2 = malloc(4*1024*1024);
  printf("%d\n", (int)(ptr && ptr2));
  print_stats(1);

  int success = emmalloc_trim(0);
  printf("1st trim: %d\n", success);
  print_stats(1);

  success = emmalloc_trim(0);
  printf("2nd trim: %d\n", success);
  print_stats(2);
  free(ptr2);

  success = emmalloc_trim(100000);
  printf("3rd trim: %d\n", success);
  print_stats(3);

  success = emmalloc_trim(100000);
  printf("4th trim: %d\n", success);
  print_stats(4);

  success = emmalloc_trim(0);
  printf("5th trim: %d\n", success);
  print_stats(5);
}
