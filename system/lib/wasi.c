/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * C++ exception handling support stubs. This is included when exception
 * throwing is disabled - so no exceptions should exist at all. If the code still
 * uses them, these stubs will throw at runtime.
 */

#include <emscripten.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wasi.h"

// WASI APIs

// libc

void exit(int status) {
  __wasi_proc_exit(status);
  __builtin_unreachable();
}

void abort() {
  exit(1);
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
  // TODO optimize, maybe build our memcpy with a wasi variant?
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

// I/O syscalls - we support printf etc., but no filesystem access for now.

static intptr_t _vararg;

static void set_vararg(intptr_t vararg) {
  _vararg = vararg;
}

static int get_vararg_i32() {
  int ret = *(int*)_vararg;
  _vararg += 4;
  return ret;
}

int __syscall6(int id, intptr_t vararg) { // close
  return 0;
}

int __syscall54(int id, intptr_t vararg) { // ioctl
  return 0;
}

int __syscall140(int id, intptr_t vararg) { // llseek
  return 0;
}

int __syscall146(int id, intptr_t vararg) { // writev
  set_vararg(vararg);
  // hack to support printf, similar to library_syscalls.js handling of SYSCALLS_REQUIRE_FILESYSTEM=0
  int stream = get_vararg_i32();
  // Luckily iovs are identical in musl and wasi
  struct __wasi_ciovec_t* iov = (struct __wasi_ciovec_t*)get_vararg_i32();
  size_t iovcnt = get_vararg_i32();
  size_t ret = 0;
  for (int i = 0; i < iovcnt; i++) {
    size_t num;
    int result = __wasi_fd_write(stream, iov, 1, &num);
    // TODO: error handling
    ret += num;
    iov++;
  }
  return ret;
}

// Memory management

extern intptr_t emscripten_get_sbrk_ptr();

static const size_t WASM_PAGE_SIZE = 65536;

void *sbrk(intptr_t increment) {
  intptr_t old_brk = emscripten_get_sbrk_ptr();
  // TODO: overflow checks
  intptr_t updated_brk = old_brk + increment;
  uintptr_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
  if (updated_brk >= old_size) {
    // Try to grow memory.
    intptr_t diff = updated_brk - old_size;
    uintptr_t result = __builtin_wasm_memory_grow(0, (diff + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE);
    if (result == SIZE_MAX) {
      errno = ENOMEM;
      return (void*)-1;
    }
  }
  return (void*)old_brk;
}
