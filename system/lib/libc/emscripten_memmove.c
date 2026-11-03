/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <string.h>
#include "libc.h"

// Use the simple/naive version of memmove when building with asan
#if __has_feature(address_sanitizer)

static void *__memmove(void *dest, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;
  if (d < s) {
    while (n--) *d++ = *s++;
  } else {
    d += n;
    s += n;
    while (n--) *--d = *--s;
  }
  return dest;
}

#else

static void *__memmove(void *dest, const void *src, size_t n) {
  // memory.copy traps on OOB zero-length copies, but memmove must not.
  if (n) {
    __builtin_wasm_memory_copy(0, 0, dest, src, n);
  }
  return dest;
}

#endif

weak_alias(__memmove, emscripten_builtin_memmove);
weak_alias(__memmove, memmove);
