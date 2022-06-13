#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sanitizer/asan_interface.h>

int main() {
  char *x = malloc(10);
  memset(x, 0, 10);
  printf("poisoned: %d\n", __asan_address_is_poisoned(x));
  free(x);
  printf("poisoned: %d\n", __asan_address_is_poisoned(x));
  __asan_unpoison_memory_region(x, 10);
  printf("poisoned: %d\n", __asan_address_is_poisoned(x));
  return 0;
}
