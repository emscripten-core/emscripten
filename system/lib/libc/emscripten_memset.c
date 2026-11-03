/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */


#include <stdint.h>
#include <string.h>
#include "libc.h"

// Use the simple/naive version of memset when building with asan.
// Note: ASan's shadow memory poisoner calls REAL(memset) directly on shadow
// memory, so this must remain uninstrumented to prevent ASan from checking
// the shadow memory of the shadow memory itself.
#if __has_feature(address_sanitizer)

static __attribute__((no_sanitize("address"))) void *__memset(void *dest, int c, size_t n) {
  unsigned char *d = (unsigned char *)dest;
  while (n--) *d++ = (unsigned char)c;
  return dest;
}

#else

static void *__memset(void *dest, int c, size_t n) {
  // memory.fill traps on OOB zero-length sets, but memset must not.
  if (n) {
    __builtin_wasm_memory_fill(0, dest, c, n);
  }
  return dest;
}

#endif

weak_alias(__memset, emscripten_builtin_memset);
weak_alias(__memset, memset);
