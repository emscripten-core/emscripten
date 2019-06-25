// XXX EMSCRIPTEN ASAN: build an uninstrumented version of memset
#if defined(__EMSCRIPTEN__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define memset __attribute__((no_sanitize("address"))) emscripten_builtin_memset
#endif
#endif

#include "musl/src/string/memset.c"
