#include "emscripten_internal.h"

// XXX EMSCRIPTEN ASAN: build an uninstrumented version of memset
#if defined(__EMSCRIPTEN__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
__attribute__((no_sanitize("address"))) static void *__memset(void *str, int c, size_t n);
__attribute__((no_sanitize("address"))) void *__musl_memset(void *str, int c, size_t n);
#endif
#endif

#ifdef EMSCRIPTEN_OPTIMIZE_FOR_OZ

#include <stddef.h>

static void *__memset(void *str, int c, size_t n) {
  unsigned char *s = (unsigned char *)str;
#pragma clang loop unroll(disable)
  while(n--) *s++ = c;
  return str;
}

#else

__attribute__((__weak__)) void *__musl_memset(void *str, int c, size_t n);
#define memset __musl_memset
#include "musl/src/string/memset.c"
#undef memset

static void *__memset(void *str, int c, size_t n) {
#ifndef EMSCRIPTEN_STANDALONE_WASM
  if (n >= 512) {
    emscripten_memset_big(str, c, n);
    return str;
  }
#endif
  return __musl_memset(str, c, n);
}

#endif

weak_alias(__memset, emscripten_builtin_memset);
weak_alias(__memset, memset);
