#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

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

  printf("SSIZE_MAX: %zu\n", SSIZE_MAX);

  // Fixed width types
  printf("INT8_MIN: %" PRId8 "\n", INT8_MIN);
  printf("INT8_MAX: %" PRId8 "\n", INT8_MAX);
  printf("INT16_MIN: %" PRId16 "\n", INT16_MIN);
  printf("INT16_MAX: %" PRId16 "\n", INT16_MAX);
  printf("INT32_MIN: %" PRId32 "\n", INT32_MIN);
  printf("INT32_MAX: %" PRId32 "\n", INT32_MAX);
  printf("INT64_MIN: %" PRId64 "\n", INT64_MIN);
  printf("INT64_MAX: %" PRId64 "\n", INT64_MAX);

  printf("UINT8_MAX: %" PRIu8 "\n", UINT8_MAX);
  printf("UINT16_MAX: %" PRIu16 "\n", UINT16_MAX);
  printf("UINT32_MAX: %" PRIu32 "\n", UINT32_MAX);
  printf("UINT64_MAX: %" PRIu64 "\n", UINT64_MAX);
  return 0;
}
