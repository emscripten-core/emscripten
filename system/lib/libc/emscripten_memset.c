#if __has_feature(address_sanitizer)
#define memset emscripten_builtin_memset
#endif

#ifdef EMSCRIPTEN_OPTIMIZE_FOR_OZ

#include <stddef.h>

void *memset(void *str, int c, size_t n) {
  unsigned char *s = (unsigned char *)str;
#pragma clang loop unroll(disable)
  while(n--) *s++ = c;
  return str;
}

#else

#include "musl/src/string/memset.c"

#endif
