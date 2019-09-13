/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
*/

#ifndef EMSCRIPTEN_NO_ERRNO
#include <errno.h>
#endif
#include <stddef.h>
#include <stdint.h>

#define WASM_PAGE_SIZE 65536

#ifdef __cplusplus
extern "C" {
#endif

extern intptr_t* emscripten_get_sbrk_ptr(void);
extern int emscripten_resize_heap(size_t requested_size);
extern size_t emscripten_get_heap_size(void);

#ifdef __cplusplus
}
#endif

void *sbrk(intptr_t increment) {
  intptr_t* sbrk_ptr = emscripten_get_sbrk_ptr();
  intptr_t old_brk = *sbrk_ptr;
  // TODO: overflow checks
  intptr_t new_brk = old_brk + increment;
#if __wasm__
  uintptr_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
#else
  uintptr_t old_size = emscripten_get_heap_size();
#endif
  // TODO In a multithreaded build dlmalloc uses locks around each malloc/free,
  //      which means we don't need to use atomics here. In theory however
  //      someone could use sbrk outside of dlmalloc in a racy manner.
  if (new_brk > old_size) {
    // Try to grow memory.
    intptr_t diff = new_brk - old_size;
    if (!emscripten_resize_heap(new_brk)) {
#ifndef EMSCRIPTEN_NO_ERRNO
      errno = ENOMEM;
#endif
      return (void*)-1;
    }
  }
  *sbrk_ptr = new_brk;
  return (void*)old_brk;
}

int brk(intptr_t ptr) {
  intptr_t last = (intptr_t)sbrk(0);
  if (sbrk(ptr - last) == (void*)-1) {
    return -1;
  }
  return 0;
}
