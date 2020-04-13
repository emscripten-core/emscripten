// XXX EMSCRIPTEN ASAN: build an uninstrumented version of memmove
#if defined(__EMSCRIPTEN__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define memmove __attribute__((no_sanitize("address"))) emscripten_builtin_memmove
#endif
#endif

#include "musl/src/string/memmove.c"
