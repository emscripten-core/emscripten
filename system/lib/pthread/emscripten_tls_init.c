/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/emmalloc.h>
#include <pthread.h>

// Uncomment to trace TLS allocations.
// #define DEBUG_TLS
#ifdef DEBUG_TLS
#include <stdio.h>
#endif

// linker-generated symbol that loads static TLS data at the given location.
extern void __wasm_init_tls(void *memory);

extern int __cxa_thread_atexit(void (*)(void *), void *, void *);

extern int __dso_handle;

static void free_tls(void* tls_block) {
#ifdef DEBUG_TLS
  printf("tls free: thread[%p] dso[%p] <- %p\n", pthread_self(), &__dso_handle, tls_block);
#endif
  emscripten_builtin_free(tls_block);
}

void* emscripten_tls_init(void) {
  size_t tls_size = __builtin_wasm_tls_size();
  size_t tls_align = __builtin_wasm_tls_align();
  if (!tls_size) {
    return NULL;
  }
  void *tls_block = emscripten_builtin_memalign(tls_align, tls_size);
#ifdef DEBUG_TLS
  printf("tls init: thread[%p] dso[%p] -> %p\n", pthread_self(), &__dso_handle, tls_block);
#endif
  __wasm_init_tls(tls_block);
  __cxa_thread_atexit(free_tls, tls_block, &__dso_handle);
  return tls_block;
}
