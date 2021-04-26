// XXX EMSCRIPTEN ASAN: build an uninstrumented version of memmove
#if defined(__EMSCRIPTEN__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define memmove __attribute__((no_sanitize("address"))) emscripten_builtin_memmove
#endif
#endif

#ifdef EMSCRIPTEN_OPTIMIZE_FOR_OZ

#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);

void *memmove(void *dest, const void *src, size_t n) {
  if (dest < src) return memcpy(dest, src, n);
  unsigned char *d = (unsigned char *)dest + n;
  const unsigned char *s = (const unsigned char *)src + n;
#pragma clang loop unroll(disable)
  while(n--) *--d = *--s;
  return dest;
}

#else

#include "musl/src/string/memmove.c"

#endif
