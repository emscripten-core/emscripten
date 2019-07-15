// XXX EMSCRIPTEN ASAN: build an uninstrumented version of fcntl
#if defined(__EMSCRIPTEN__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define fcntl __attribute__((no_sanitize("address"))) fcntl
#endif
#endif

#include "musl/src/fcntl/fcntl.c"
