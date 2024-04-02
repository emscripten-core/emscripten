#include "emscripten_internal.h" // for emscripten_memset_big

#if defined(__has_feature) && __has_feature(address_sanitizer)
// build an uninstrumented version of memset
__attribute__((no_sanitize("address"))) void *__musl_memset(void *str, int c, size_t n);
__attribute__((no_sanitize("address"))) void *__memset(void *str, int c, size_t n);
#endif

__attribute__((__weak__)) void *__musl_memset(void *str, int c, size_t n);
__attribute__((__weak__)) void *__memset(void *str, int c, size_t n);

#if defined(EMSCRIPTEN_OPTIMIZE_FOR_OZ) || __has_feature(address_sanitizer)

void *__memset(void *str, int c, size_t n) {
#if !defined(EMSCRIPTEN_STANDALONE_WASM)
  if (n >= 512) {
    _emscripten_memset_js(str, c, n);
    return str;
  }
#endif

  unsigned char *s = (unsigned char *)str;
#pragma clang loop unroll(disable)
  while(n--) *s++ = c;
  return str;
}

#elif defined(__wasm_bulk_memory__)

void *__memset(void *str, int c, size_t n) {
  return _emscripten_memset_bulkmem(str, c, n);
}

#else

#define memset __musl_memset
#include "musl/src/string/memset.c"
#undef memset

void *__memset(void *str, int c, size_t n) {
#if !defined(EMSCRIPTEN_STANDALONE_WASM)
  if (n >= 512) {
    _emscripten_memset_js(str, c, n);
    return str;
  }
#endif

  return __musl_memset(str, c, n);
}

#endif

weak_alias(__memset, emscripten_builtin_memset);
weak_alias(__memset, memset);
