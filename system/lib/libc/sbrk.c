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

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __EMSCRIPTEN_SHARED_MEMORY__ // for error handling, see below
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef __EMSCRIPTEN_TRACING__
void emscripten_memprof_sbrk_grow(intptr_t old, intptr_t new);
#else
#define emscripten_memprof_sbrk_grow(...) ((void)0)
#endif

#include <emscripten/heap.h>

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

#ifdef __EMSCRIPTEN_SHARED_MEMORY__
#define READ_SBRK_PTR(sbrk_ptr) (__c11_atomic_load((_Atomic(uintptr_t)*)(sbrk_ptr), __ATOMIC_SEQ_CST))
#else
#define READ_SBRK_PTR(sbrk_ptr) (*(sbrk_ptr))
#endif

void *sbrk(intptr_t increment_) {
  uintptr_t increment = (uintptr_t)increment_;
  increment = (increment + (SBRK_ALIGNMENT-1)) & ~(SBRK_ALIGNMENT-1);
  uintptr_t *sbrk_ptr = (uintptr_t*)emscripten_get_sbrk_ptr();

  // To make sbrk thread-safe, implement a CAS loop to update the
  // value of sbrk_ptr.
  while (1) {
    uintptr_t old_brk = READ_SBRK_PTR(sbrk_ptr);
    uintptr_t new_brk = old_brk + increment;
    // Check for a) an overflow, which would indicate that we are trying to
    // allocate over maximum addressable memory. and b) if necessary,
    // increase the WebAssembly Memory size, and abort if that fails.
    if ((increment > 0 && new_brk <= old_brk)
     || (new_brk > emscripten_get_heap_size() && !emscripten_resize_heap(new_brk))) {
      errno = ENOMEM;
      return (void*)-1;
    }
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
    // Attempt to update the dynamic top to new value. Another thread may have
    // beat this one to the update, in which case we will need to start over
    // by iterating the loop body again.
    uintptr_t expected = old_brk;

    __c11_atomic_compare_exchange_strong((_Atomic(uintptr_t)*)sbrk_ptr,
      &expected, new_brk, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

    if (expected != old_brk) continue; // CAS failed, another thread raced in between.
#else
    *sbrk_ptr = new_brk;
#endif

    emscripten_memprof_sbrk_grow(old_brk, new_brk);
    return (void*)old_brk;
  }
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
