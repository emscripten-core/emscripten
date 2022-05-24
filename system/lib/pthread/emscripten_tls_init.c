/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Included for emscripten_builtin_free / emscripten_builtin_malloc
// TODO(sbc): Should these be in their own header to avoid emmalloc here?
#include <emscripten/emmalloc.h>
#include <emscripten/threading.h>

#include "pthread_impl.h"

// Uncomment to trace TLS allocations.
// #define DEBUG_TLS
#ifdef DEBUG_TLS
#include <stdio.h>
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
__asm__(".globaltype is_not_primary_tls, i32\n"
        "is_not_primary_tls:\n");

static void set_non_primary(void) {
  __asm__("i32.const 1\n"
          "global.set is_not_primary_tls\n");
}

static int is_non_primary(void) {
  int val;
  __asm__("global.get is_not_primary_tls\n"
          "local.set %0" : "=r" (val));
  return val;
}

void _emscripten_tls_free() {
  void* tls_block = __builtin_wasm_tls_base();
  if (tls_block && is_non_primary()) {
#ifdef DEBUG_TLS
    printf("tls free: thread=%p dso=%p <- %p\n", pthread_self(), &__dso_handle, tls_block);
#endif
    emscripten_builtin_free(tls_block);
  }
}

void* _emscripten_tls_init(void) {
  size_t tls_size = __builtin_wasm_tls_size();
  void* tls_block = __builtin_wasm_tls_base();
  if (is_non_primary() || (!tls_block && tls_size)) {
    set_non_primary();
    size_t tls_align = __builtin_wasm_tls_align();
    tls_block = emscripten_builtin_memalign(tls_align, tls_size);
  }
#ifdef DEBUG_TLS
  printf("tls init: size=%zu thread=%p dso=%p -> %p\n", tls_size, pthread_self(), &__dso_handle, tls_block);
#endif
  __wasm_init_tls(tls_block);
  return tls_block;
}
