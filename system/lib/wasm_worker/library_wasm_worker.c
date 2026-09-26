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

#ifdef __EMSCRIPTEN_PTHREADS__
#include "pthread_impl.h"
#endif

#ifndef __EMSCRIPTEN_WASM_WORKERS__
#error __EMSCRIPTEN_WASM_WORKERS__ should be defined when building this file!
#endif

// Comment this line to enable tracing of thread creation and destruction:
// #define PTHREAD_DEBUG
#ifdef PTHREAD_DEBUG
#define dbg(fmt, ...) emscripten_dbgf(fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
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

static size_t max_alignment() {
  return MAX(__builtin_wasm_tls_align(), STACK_ALIGN);
}

#ifdef __EMSCRIPTEN_PTHREADS__
_Static_assert(STACK_ALIGN >= _Alignof(struct pthread));

/*
 * pthread_key_create.c overrides this, meaning we only allocate space for TSD
 * if needed by the program.
 */
static volatile size_t dummy = 0;
weak_alias(dummy, __pthread_tsd_size);

/**
 * The layout of the wasm worker stack space in hybrid mode is as follow.
 * [ struct pthread ] [ pthread TSD slots ] [ ... stack ] [ TLS data ]
 *
 * As opposed to the layout for regular Wasm Workers which is just:
 * [ ... stack ] [ TLS data ]
 *
 * In either case these sections are all aligned to `max_alignment()`
 * which is the max alignment of any of the given chunks.
 */
void* _emscripten_init_pthread(void *stackPlusTLSAddress, size_t* stackPlusTLSSize, pid_t tid) {
  // TODO: Remove duplication with pthread_create
  pthread_t self = pthread_self();

  uintptr_t base = (uintptr_t)stackPlusTLSAddress;
  uintptr_t offset = base;

  size_t alignment = max_alignment();
  assert(base % alignment == 0);

  // 1. struct pthread comes first
  pthread_t new = (pthread_t)offset;
  memset(new, 0, sizeof(struct pthread));
  offset += ROUND_UP(sizeof(struct pthread), alignment);

  // 2. tsd slots
  if (__pthread_tsd_size) {
    new->tsd = (void*)offset;
    memset(new->tsd, 0, __pthread_tsd_size);
    offset += ROUND_UP(__pthread_tsd_size, alignment);
  }

  // 3. Remaining space is for TLS data + stack, which is handled by
  //    the wasm worker startup code.

  // Calculate updated stack size
  size_t new_stack_size = *stackPlusTLSSize - (offset - base);
  assert(new_stack_size % STACK_ALIGN == 0);
  assert(new_stack_size > 0);

  new->self = new;
  new->tid = tid;
  new->map_base = stackPlusTLSAddress;
  new->map_size = *stackPlusTLSSize;
  new->stack = (void*)(base + *stackPlusTLSSize);
  new->stack_size = new_stack_size;
  new->guard_size = __default_guardsize;
  new->detach_state = DT_DETACHED;
  new->robust_list.head = &new->robust_list.head;

  __tl_lock();

  new->next = self->next;
  new->prev = self;
  new->next->prev = new;
  new->prev->next = new;

  __tl_unlock();

  dbg("_emscripten_init_pthread: base=%#lx, end=%#lx, used=%zu "
      "stackold=%zu stacknew=%zu",
      base,
      base + *stackPlusTLSSize,
      offset - base,
      *stackPlusTLSSize,
      new_stack_size);
  *stackPlusTLSSize = new_stack_size;
  return (void*)offset;
}
#endif

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
  void* pthreadPtr = stackPlusTLSAddress;
#ifdef __EMSCRIPTEN_PTHREADS__
  stackPlusTLSAddress = _emscripten_init_pthread(stackPlusTLSAddress, &stackPlusTLSSize, wwID);
#endif
  if (!_emscripten_create_wasm_worker(wwID, stackPlusTLSAddress, stackPlusTLSSize, pthreadPtr))
    return 0;
  return wwID;
}

emscripten_wasm_worker_t emscripten_malloc_wasm_worker(size_t stackSize) {
  // Add the TLS size (and pthread metadata size) to the provided stackSize so
  // that the allocation will always be large enough.
  size_t alignment = max_alignment();
  size_t totalSize = stackSize + ROUND_UP(__builtin_wasm_tls_size(), alignment);
  #ifdef __EMSCRIPTEN_PTHREADS__
  totalSize += ROUND_UP(sizeof(struct pthread), alignment);
  totalSize += ROUND_UP(__pthread_tsd_size, alignment);
  #endif

  void* address = emscripten_builtin_memalign(alignment, totalSize);
  return emscripten_create_wasm_worker(address, totalSize);
}

void emscripten_wasm_worker_sleep(int64_t nsecs) {
  int32_t addr = 0;
  emscripten_atomic_wait_u32(&addr, 0, nsecs);
}

bool emscripten_current_thread_is_wasm_worker() {
  return emscripten_wasm_worker_self_id() != 0;
}
