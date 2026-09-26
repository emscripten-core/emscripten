/*
 * A simple memcpy optimized for wasm.
 */

#include <stdint.h>
#include <string.h>
#include <emscripten/emscripten.h>
#include "libc.h"
#include "emscripten_internal.h"

// Use the simple/naive version of memcpy when building with asan
#if __has_feature(address_sanitizer)

static void *__memcpy(void *dest, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;
  while(n--) *d++ = *s++;
  return dest;
}

#else

static void *__memcpy(void *restrict dest, const void *restrict src, size_t n) {
  // memory.copy traps on OOB zero-length copies, but memcpy must not.
  if (n) {
    __builtin_wasm_memory_copy(0, 0, dest, src, n);
  }
  return dest;
}

#endif

weak_alias(__memcpy, emscripten_builtin_memcpy);
weak_alias(__memcpy, memcpy);
