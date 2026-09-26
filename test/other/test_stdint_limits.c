#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

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

  printf("WCHAR_MIN: %d\n", WCHAR_MIN);
  printf("WCHAR_MAX: %d\n", WCHAR_MAX);

  printf("WINT_MIN: %d\n", WINT_MIN);
  printf("WINT_MAX: %d\n", WINT_MAX);

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

  printf("INT_LEAST8_MIN: %" PRId8 "\n", INT_LEAST8_MIN);
  printf("INT_LEAST8_MAX: %" PRId8 "\n", INT_LEAST8_MAX);
  printf("INT_LEAST16_MIN: %" PRId16 "\n", INT_LEAST16_MIN);
  printf("INT_LEAST16_MAX: %" PRId16 "\n", INT_LEAST16_MAX);
  printf("INT_LEAST32_MIN: %" PRId32 "\n", INT_LEAST32_MIN);
  printf("INT_LEAST32_MAX: %" PRId32 "\n", INT_LEAST32_MAX);
  printf("INT_LEAST64_MIN: %" PRId64 "\n", INT_LEAST64_MIN);
  printf("INT_LEAST64_MAX: %" PRId64 "\n", INT_LEAST64_MAX);

  printf("INT_FAST8_MIN: %" PRId8 "\n", INT_FAST8_MIN);
  printf("INT_FAST8_MAX: %" PRId8 "\n", INT_FAST8_MAX);
  printf("INT_FAST16_MIN: %" PRId16 "\n", INT_FAST16_MIN);
  printf("INT_FAST16_MAX: %" PRId16 "\n", INT_FAST16_MAX);
  printf("INT_FAST32_MIN: %" PRId32 "\n", INT_FAST32_MIN);
  printf("INT_FAST32_MAX: %" PRId32 "\n", INT_FAST32_MAX);
  printf("INT_FAST64_MIN: %" PRId64 "\n", INT_FAST64_MIN);
  printf("INT_FAST64_MAX: %" PRId64 "\n", INT_FAST64_MAX);

  printf("UINT8_MAX: %" PRIu8 "\n", UINT8_MAX);
  printf("UINT16_MAX: %" PRIu16 "\n", UINT16_MAX);
  printf("UINT32_MAX: %" PRIu32 "\n", UINT32_MAX);
  printf("UINT64_MAX: %" PRIu64 "\n", UINT64_MAX);

  printf("UINT_LEAST8_MAX: %" PRIu8 "\n", UINT_LEAST8_MAX);
  printf("UINT_LEAST16_MAX: %" PRIu16 "\n", UINT_LEAST16_MAX);
  printf("UINT_LEAST32_MAX: %" PRIu32 "\n", UINT_LEAST32_MAX);
  printf("UINT_LEAST64_MAX: %" PRIu64 "\n", UINT_LEAST64_MAX);

  printf("UINT_FAST8_MAX: %" PRIu8 "\n", UINT_FAST8_MAX);
  printf("UINT_FAST16_MAX: %" PRIu16 "\n", UINT_FAST16_MAX);
  printf("UINT_FAST32_MAX: %" PRIu32 "\n", UINT_FAST32_MAX);
  printf("UINT_FAST64_MAX: %" PRIu64 "\n", UINT_FAST64_MAX);

  // Test macros for creating integer constants
  printf("INT8_C: (" PRId8 ") %" PRId8 "\n", INT8_C(42));
  printf("INT16_C: (" PRId16 ") %" PRId16 "\n", INT16_C(42));
  printf("INT32_C: (" PRId32 ") %" PRId32 "\n", INT32_C(42));
  printf("INT64_C: (" PRId64 ") %" PRId64 "\n", INT64_C(42));

  printf("INTU8_C: (" PRIu8 ") %" PRIu8 "\n", UINT8_C(42));
  printf("INTU16_C: (" PRIu16 ") %" PRIu16 "\n", UINT16_C(42));
  printf("INTU32_C: (" PRIu32 ") %" PRIu32 "\n", UINT32_C(42));
  printf("INTU64_C: (" PRIu64 ") %" PRIu64 "\n", UINT64_C(42));
  return 0;
}
