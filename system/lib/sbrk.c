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
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#if __EMSCRIPTEN_PTHREADS__ // for error handling, see below
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef __EMSCRIPTEN_TRACING__
void emscripten_memprof_sbrk_grow(intptr_t old, intptr_t new);
#endif

#include <emscripten/heap.h>

#ifndef EMSCRIPTEN_NO_ERRNO
#define SET_ERRNO() { errno = ENOMEM; }
#else
#define SET_ERRNO()
#endif

extern size_t __heap_base;

static uintptr_t sbrk_val = (uintptr_t)&__heap_base;

uintptr_t* emscripten_get_sbrk_ptr() {
#ifdef __PIC__
  // In relocatable code we may call emscripten_get_sbrk_ptr() during startup,
  // potentially *before* the setup of the dynamically-linked __heap_base, when
  // using SAFE_HEAP. (SAFE_HEAP instruments *all* memory accesses, so even the
  // code doing dynamic linking itself ends up instrumented, which is why we can
  // get such an instrumented call before sbrk_val has its proper value.)
  if (sbrk_val == 0) {
    sbrk_val = (uintptr_t)&__heap_base;
  }
#endif
  return &sbrk_val;
}

// Enforce preserving a minimal alignof(maxalign_t) alignment for sbrk.
#define SBRK_ALIGNMENT (__alignof__(max_align_t))

void *sbrk(intptr_t increment_) {
  uintptr_t old_size;
  uintptr_t increment = (uintptr_t)increment_;
  increment = (increment + (SBRK_ALIGNMENT-1)) & ~(SBRK_ALIGNMENT-1);
#if __EMSCRIPTEN_PTHREADS__
  // Our default dlmalloc uses locks around each malloc/free, so no additional
  // work is necessary to keep things threadsafe, but we also make sure sbrk
  // itself is threadsafe so alternative allocators work. We do that by looping
  // and retrying if we hit interference with another thread.
  uintptr_t expected;
  while (1) {
#endif // __EMSCRIPTEN_PTHREADS__
    uintptr_t* sbrk_ptr = emscripten_get_sbrk_ptr();
#if __EMSCRIPTEN_PTHREADS__
    uintptr_t old_brk = __c11_atomic_load((_Atomic(uintptr_t)*)sbrk_ptr, __ATOMIC_SEQ_CST);
#else
    uintptr_t old_brk = *sbrk_ptr;
#endif
    uintptr_t new_brk = old_brk + increment;
    // Check for a 32-bit overflow, which would indicate that we are trying to
    // allocate over 4GB, which is never possible in wasm32.
    if (increment > 0 && (uint32_t)new_brk <= (uint32_t)old_brk) {
      goto Error;
    }
    old_size = emscripten_get_heap_size();
    if (new_brk > old_size) {
      // Try to grow memory.
      if (!emscripten_resize_heap(new_brk)) {
        goto Error;
      }
    }
#if __EMSCRIPTEN_PTHREADS__
    // Attempt to update the dynamic top to new value. Another thread may have
    // beat this one to the update, in which case we will need to start over
    // by iterating the loop body again.
    expected = old_brk;
    __c11_atomic_compare_exchange_strong(
        (_Atomic(uintptr_t)*)sbrk_ptr,
        &expected, new_brk,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    if (expected != old_brk) {
      continue;
    }
#else // __EMSCRIPTEN_PTHREADS__
    *sbrk_ptr = new_brk;
#endif // __EMSCRIPTEN_PTHREADS__

#ifdef __EMSCRIPTEN_TRACING__
    emscripten_memprof_sbrk_grow(old_brk, new_brk);
#endif
    return (void*)old_brk;

#if __EMSCRIPTEN_PTHREADS__
  }
#endif // __EMSCRIPTEN_PTHREADS__

Error:
  SET_ERRNO();
  return (void*)-1;
}

int brk(void* ptr) {
#if __EMSCRIPTEN_PTHREADS__
  // FIXME
  printf("brk() is not theadsafe yet, https://github.com/emscripten-core/emscripten/issues/10006");
  abort();
#else
  uintptr_t last = (uintptr_t)sbrk(0);
  if (sbrk((uintptr_t)ptr - last) == (void*)-1) {
    return -1;
  }
  return 0;
#endif
}
