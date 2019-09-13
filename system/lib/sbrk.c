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
#if __EMSCRIPTEN_PTHREADS__
  // Our default dlmalloc uses locks around each malloc/free, so no additional
  // work is necessary to keep things threadsafe, but we also make sure sbrk
  // itself is threadsafe so alternative allocators work. We do that by looping
  // and retrying if we hit interference with another thread.
  while (1) {
#endif // __EMSCRIPTEN_PTHREADS__

    intptr_t* sbrk_ptr = emscripten_get_sbrk_ptr();
#if __EMSCRIPTEN_PTHREADS__
    intptr_t old_brk = __c11_atomic_load((_Atomic(intptr_t)*)sbrk_ptr, __ATOMIC_SEQ_CST);
#else
    intptr_t old_brk = *sbrk_ptr;
#endif
    // TODO: overflow checks
    intptr_t new_brk = old_brk + increment;
#ifdef __wasm__
    uintptr_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
#else
    uintptr_t old_size = emscripten_get_heap_size();
#endif
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
#if __EMSCRIPTEN_PTHREADS__
    // Attempt to update the dynamic top to new value. Another thread may have
    // beat this one to the update, in which case we will need to start over
    // by iterating the loop body again.
    intptr_t expected = old_brk;
    __c11_atomic_compare_exchange_strong(
        (_Atomic(intptr_t)*)sbrk_ptr,
        &expected, new_brk,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    if (expected != old_brk) {
      continue;
    }
#else // __EMSCRIPTEN_PTHREADS__
    *sbrk_ptr = new_brk;
#endif // __EMSCRIPTEN_PTHREADS__
    return (void*)old_brk;

#if __EMSCRIPTEN_PTHREADS__
  }
#endif // __EMSCRIPTEN_PTHREADS__
}

int brk(intptr_t ptr) {
  intptr_t last = (intptr_t)sbrk(0);
  if (sbrk(ptr - last) == (void*)-1) {
    return -1;
  }
  return 0;
}
