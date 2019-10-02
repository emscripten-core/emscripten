/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <wasi/wasi.h>

/*
 * WASI support code. These are compiled with the program, and call out
 * using wasi APIs, which can be provided either by a wasi VM or by our
 * emitted JS.
 */

// libc

void exit(int status) {
  __wasi_proc_exit(status);
  __builtin_unreachable();
}

void abort() {
  exit(1);
}

// mmap support is nonexistent. TODO: emulate simple mmaps using
// stdio + malloc, which is slow but may help some things?

int __map_file(int x, int y) {
  return ENOSYS;
}

int __syscall91(int x, int y) { // munmap
  return ENOSYS;
}

// Musl lock internals. As we assume wasi is single-threaded for now, these
// are no-ops.

void __lock(void* ptr) {}
void __unlock(void* ptr) {}

// Emscripten additions

void *emscripten_memcpy_big(void *restrict dest, const void *restrict src, size_t n) {
  // This normally calls out into JS which can do a single fast operation,
  // but with wasi we can't do that. As this is called when n >= 8192, we
  // can just split into smaller calls.
  // TODO optimize, maybe build our memcpy with a wasi variant, maybe have
  //      a SIMD variant, etc.
  const int CHUNK = 8192;
  unsigned char* d = (unsigned char*)dest;
  unsigned char* s = (unsigned char*)src;
  while (n > 0) {
    size_t curr_n = n;
    if (curr_n > CHUNK) curr_n = CHUNK;
    memcpy(d, s, curr_n);
    d += CHUNK;
    s += CHUNK;
    n -= curr_n;
  }
  return dest;
}

static const int WASM_PAGE_SIZE = 65536;

// Note that this does not support memory growth in JS because we don't update the JS
// heaps. Wasm and wasi lack a good API for that.
int emscripten_resize_heap(size_t size) {
  size_t result = __builtin_wasm_memory_grow(0, (size + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE);
  return result != (size_t)-1;
}
