#include "emscripten_internal.h" // for emscripten_memset_big

#if defined(__has_feature) && __has_feature(address_sanitizer)
// build an uninstrumented version of memset
__attribute__((no_sanitize("address"))) void *__musl_memset(void *str, int c, size_t n);
__attribute__((no_sanitize("address"))) void *__memset(void *str, int c, size_t n);
#endif

__attribute__((__weak__)) void *__musl_memset(void *str, int c, size_t n);
__attribute__((__weak__)) void *__memset(void *str, int c, size_t n);

#if defined(EMSCRIPTEN_OPTIMIZE_FOR_OZ)

void *__memset(void *str, int c, size_t n) {
  return _emscripten_memset_bulkmem(str, c, n);
}

#else

#define memset __memset
#include "musl/src/string/memset.c"
#undef memset

#endif

weak_alias(__memset, emscripten_builtin_memset);
weak_alias(__memset, memset);
