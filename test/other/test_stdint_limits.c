#include <stdio.h>
#include <stdint.h>

int main () {
  printf("PTRDIFF_MIN: %ti\n", PTRDIFF_MIN);
  printf("PTRDIFF_MAX: %ti\n", PTRDIFF_MAX);

  printf("INTPTR_MIN: %ld\n", INTPTR_MIN);
  printf("INTPTR_MAX: %ld\n", INTPTR_MAX);

  printf("UINTPTR_MAX: %lu\n", UINTPTR_MAX);

  printf("SIZE_MAX: %zu\n", SIZE_MAX);
  return 0;
}
