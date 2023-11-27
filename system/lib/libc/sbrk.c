/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
*/

// libc files are compiled as -std=c99 which doesn't normally declare
// max_align_t.
#if __STDC_VERSION__ < 201112L
#define __NEED_max_align_t
#endif

#ifndef EMSCRIPTEN_NO_ERRNO
#include <errno.h>
#endif
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __EMSCRIPTEN_SHARED_MEMORY__ // for error handling, see below
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

#define ALIGN_UP(ptr, alignment) ((uintptr_t)((((uintptr_t)(ptr)) + ((alignment)-1)) & ~((alignment)-1)))

// First bumps sbrk() up to the given alignment, and then by the given increment.
// Returns old brk pointer. Note that the old brk pointer might not be aligned
// to the requested alignment, but the caller is guaranteed that they can round
// up the returned pointer to the alignment they requested, and the given
// increment will fit. That is, ROUND_UP(old_brk, alignment) + increment == new_brk.
void *sbrk_aligned(uintptr_t alignment, intptr_t increment_) {
  uintptr_t old_size;
  uintptr_t increment = (uintptr_t)increment_;
  uintptr_t* sbrk_ptr = emscripten_get_sbrk_ptr();
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
  // Our default dlmalloc uses locks around each malloc/free, so no additional
  // work is necessary to keep things threadsafe, but we also make sure sbrk
  // itself is threadsafe so alternative allocators work. We do that by looping
  // and retrying if we hit interference with another thread.
  uintptr_t expected;
  while (1) {
#endif // __EMSCRIPTEN_SHARED_MEMORY__
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
    uintptr_t old_brk = __c11_atomic_load((_Atomic(uintptr_t)*)sbrk_ptr, __ATOMIC_SEQ_CST);
#else
    uintptr_t old_brk = *sbrk_ptr;
#endif
    uintptr_t new_brk = ALIGN_UP(old_brk, alignment) + increment;
    // Check for an overflow, which would indicate that we are trying to
    // allocate over maximum addressable memory.
    if (increment > 0 && new_brk <= old_brk) {
      goto Error;
    }
    old_size = emscripten_get_heap_size();
    if (new_brk > old_size) {
      // Try to grow memory.
      if (!emscripten_resize_heap(new_brk)) {
        goto Error;
      }
    }
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
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
#else // __EMSCRIPTEN_SHARED_MEMORY__
    *sbrk_ptr = new_brk;
#endif // __EMSCRIPTEN_SHARED_MEMORY__

#ifdef __EMSCRIPTEN_TRACING__
    emscripten_memprof_sbrk_grow(old_brk, new_brk);
#endif
    return (void*)old_brk;

#ifdef __EMSCRIPTEN_SHARED_MEMORY__
  }
#endif // __EMSCRIPTEN_SHARED_MEMORY__

Error:
  SET_ERRNO();
  return (void*)-1;
}

void *sbrk(intptr_t increment_) {
  return sbrk_aligned(SBRK_ALIGNMENT, ALIGN_UP(increment_, SBRK_ALIGNMENT));
}

int brk(void* ptr) {
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
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
