// XXX EMSCRIPTEN ASAN: build an uninstrumented version of memset
#if defined(__EMSCRIPTEN__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define memset __attribute__((no_sanitize("address"))) emscripten_builtin_memset
#endif
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
