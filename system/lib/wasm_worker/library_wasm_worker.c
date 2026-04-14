/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// libc files are compiled as -std=c99 which doesn't normally declare
// max_align_t.
#if __STDC_VERSION__ < 201112L
#define __NEED_max_align_t
#endif

#include "libc.h"
#include "stdio_impl.h"
#include "emscripten_internal.h"
#include "threading_internal.h"

#include <assert.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/heap.h>
#include <emscripten/stack.h>
#include <emscripten/console.h>
#include <malloc.h>
#include <sys/param.h> // For MAX()

#ifndef __EMSCRIPTEN_WASM_WORKERS__
#error __EMSCRIPTEN_WASM_WORKERS__ should be defined when building this file!
#endif

#define ROUND_UP(x, ALIGNMENT) (((x)+ALIGNMENT-1)&-ALIGNMENT)
#define SBRK_ALIGN (__alignof__(max_align_t))
#define STACK_ALIGN __BIGGEST_ALIGNMENT__

// Options:
// #define STACK_OVERFLOW_CHECK 0/1/2 : set to the current stack overflow check mode

void __wasm_init_tls(void *memory);

__attribute__((constructor(48)))
static void emscripten_wasm_worker_main_thread_initialize() {
  uintptr_t* sbrk_ptr = emscripten_get_sbrk_ptr();
  assert((*sbrk_ptr % STACK_ALIGN) == 0);
  if (__builtin_wasm_tls_align() > STACK_ALIGN) {
    *sbrk_ptr = ROUND_UP(*sbrk_ptr, __builtin_wasm_tls_align());
  }
  __wasm_init_tls((void*)*sbrk_ptr);
  *sbrk_ptr += ROUND_UP(__builtin_wasm_tls_size(), SBRK_ALIGN);
}

static FILE *volatile dummy_file = 0;
weak_alias(dummy_file, __stdin_used);
weak_alias(dummy_file, __stdout_used);
weak_alias(dummy_file, __stderr_used);

static void init_file_lock(FILE *f) {
  if (f && f->lock<0) f->lock = 0;
}

emscripten_wasm_worker_t emscripten_create_wasm_worker(void *stackPlusTLSAddress, size_t stackPlusTLSSize) {
  assert(stackPlusTLSAddress != 0);
  assert((uintptr_t)stackPlusTLSAddress % STACK_ALIGN == 0);
  assert(stackPlusTLSSize > 0);
  assert(stackPlusTLSSize % STACK_ALIGN == 0);

  // Guard against a programming oopsie: The target Worker's stack cannot be part of the calling
  // thread's stack.
  assert(emscripten_stack_get_base() <= (uintptr_t)stackPlusTLSAddress || emscripten_stack_get_end() >= (uintptr_t)stackPlusTLSAddress + stackPlusTLSSize
    && "When creating a Wasm Worker, its stack should be located either in global data or on the heap, not on the calling thread's own stack!");

#ifndef NDEBUG
  // The Worker's TLS area will be spliced off from the stack region, so the
  // stack needs to be at least as large as the TLS region.
  uint32_t tlsSize = ROUND_UP(__builtin_wasm_tls_size(), __builtin_wasm_tls_align());
  assert(stackPlusTLSSize > tlsSize);
#endif
  // The TLS region lives at the start of the stack region (the lowest address
  // of the stack).  Since the TLS data alignment may be larger than stack
  // alignment, we may need to round up the lowest stack address to meet this
  // requirement.
  if (__builtin_wasm_tls_align() > STACK_ALIGN) {
    uintptr_t tlsBase = (uintptr_t)stackPlusTLSAddress;
    tlsBase = ROUND_UP(tlsBase, __builtin_wasm_tls_align());
    size_t padding = tlsBase - (uintptr_t)stackPlusTLSAddress;
    stackPlusTLSAddress = (void*)tlsBase;
    stackPlusTLSSize -= padding;
  }

  if (!libc.threaded) {
    for (FILE *f=*__ofl_lock(); f; f=f->next) {
      init_file_lock(f);
    }
    __ofl_unlock();
    init_file_lock(__stdin_used);
    init_file_lock(__stdout_used);
    init_file_lock(__stderr_used);
    libc.threaded = 1;
  }

  // Unlike with ptheads, wasm workers never really exit and so this counter
  // only going one way here.
  if (!libc.threads_minus_1++) libc.need_locks = 1;

  emscripten_wasm_worker_t wwID = _emscripten_get_next_tid();
  if (!_emscripten_create_wasm_worker(wwID, stackPlusTLSAddress, stackPlusTLSSize))
    return 0;
  return wwID;
}

emscripten_wasm_worker_t emscripten_malloc_wasm_worker(size_t stackSize) {
  // Add the TLS size to the provided stackSize so that the allocation
  // will always be large enough to hold the worker TLS data.
  stackSize += ROUND_UP(__builtin_wasm_tls_size(), STACK_ALIGN);
  void* stackPlusTLSAddress = emscripten_builtin_memalign(MAX(__builtin_wasm_tls_align(), STACK_ALIGN), stackSize);
  return emscripten_create_wasm_worker(stackPlusTLSAddress, stackSize);
}

void emscripten_wasm_worker_sleep(int64_t nsecs) {
  int32_t addr = 0;
  emscripten_atomic_wait_u32(&addr, 0, nsecs);
}

bool emscripten_current_thread_is_wasm_worker() {
  return emscripten_wasm_worker_self_id() != 0;
}
