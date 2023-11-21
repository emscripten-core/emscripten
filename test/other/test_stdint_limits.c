#include <limits.h>
#include <stdio.h>
#include <stdint.h>

int main () {
  printf("INT_MIN: %d\n", INT_MIN);
  printf("INT_MAX: %d\n", INT_MAX);

  printf("LONG_MIN: %ld\n", LONG_MIN);
  printf("LONG_MAX: %ld\n", LONG_MAX);

  printf("LLONG_MIN: %lld\n", LLONG_MIN);
  printf("LLONG_MAX: %lld\n", LLONG_MAX);

  printf("INTPTR_MIN: %ld\n", INTPTR_MIN);
  printf("INTPTR_MAX: %ld\n", INTPTR_MAX);

  printf("PTRDIFF_MIN: %ti\n", PTRDIFF_MIN);
  printf("PTRDIFF_MAX: %ti\n", PTRDIFF_MAX);

  printf("INTPTR_MIN: %ld\n", INTPTR_MIN);
  printf("INTPTR_MAX: %ld\n", INTPTR_MAX);

  printf("UINTPTR_MAX: %lu\n", UINTPTR_MAX);

  printf("SIZE_MAX: %zu\n", SIZE_MAX);
  return 0;
}
