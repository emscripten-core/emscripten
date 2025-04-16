/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/heap.h>
#include <emscripten/threading.h>
#include <emscripten/console.h>
#include <stdlib.h>
#include <malloc.h>

#include "pthread_impl.h"

// Uncomment to trace TLS allocations.
// #define DEBUG_TLS
#ifdef DEBUG_TLS
#define dbg(fmt, ...) emscripten_dbgf(fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

// linker-generated symbol that loads static TLS data at the given location.
extern void __wasm_init_tls(void *memory);

extern int __dso_handle;

// Create a thread-local wasm global which signal that the current thread is non
// to use the primary/static TLS region.  Once this gets set it forces that all
// future calls to emscripten_tls_init to dynamically allocate TLS.
// This is needed because emscripten re-uses module instances owned by the
// worker for new pthreads.  This in turn means that stale values of __tls_base
// from a previous pthreads need to be ignored.
// If this global is true then TLS needs to be dyanically allocated, if its
// flase we are free to use the existing/global __tls_base.
__asm__(".globaltype g_needs_dynamic_alloc, i32\n"
        "g_needs_dynamic_alloc:\n");

static void set_needs_dynamic_alloc(void) {
  __asm__("i32.const 1\n"
          "global.set g_needs_dynamic_alloc\n");
}

static int needs_dynamic_alloc(void) {
  int val;
  __asm__("global.get g_needs_dynamic_alloc\n"
          "local.set %0" : "=r" (val));
  return val;
}

void _emscripten_tls_free() {
  void* tls_block = __builtin_wasm_tls_base();
  if (tls_block && needs_dynamic_alloc()) {
    dbg("tls free: dso=%p <- %p", &__dso_handle, tls_block);
    emscripten_builtin_free(tls_block);
  }
}

void* _emscripten_tls_init(void) {
  size_t tls_size = __builtin_wasm_tls_size();
  void* tls_block = __builtin_wasm_tls_base();
  if (pthread_self()->tls_base) {
    // The TLS block for the main module is allocated alongside the pthread
    // itself and its stack.
    tls_block = pthread_self()->tls_base;
    pthread_self()->tls_base = NULL;
  } else if (needs_dynamic_alloc() || (!tls_block && tls_size)) {
    // For non-main modules we do a dynamic allocation.
    set_needs_dynamic_alloc();
    tls_block = emscripten_builtin_memalign(__builtin_wasm_tls_align(), tls_size);
  }
  dbg("tls init: size=%zu dso=%p -> %p:%p", tls_size, &__dso_handle, tls_block, ((char*)tls_block)+tls_size);
  __wasm_init_tls(tls_block);
  return tls_block;
}
