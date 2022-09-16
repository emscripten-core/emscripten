/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <malloc.h>
#include <syscall_arch.h>
#include <time.h>
#include <unistd.h>

#include <emscripten.h>
#include <emscripten/heap.h>
#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

#include "lock.h"

/*
 * WASI support code. These are compiled with the program, and call out
 * using wasi APIs, which can be provided either by a wasi VM or by our
 * emitted JS.
 */

// libc

void abort() {
  _Exit(1);
}

_Static_assert(CLOCK_REALTIME == __WASI_CLOCKID_REALTIME, "must match");
_Static_assert(CLOCK_MONOTONIC == __WASI_CLOCKID_MONOTONIC, "must match");
_Static_assert(CLOCK_PROCESS_CPUTIME_ID == __WASI_CLOCKID_PROCESS_CPUTIME_ID, "must match");
_Static_assert(CLOCK_THREAD_CPUTIME_ID == __WASI_CLOCKID_THREAD_CPUTIME_ID, "must match");

#define NSEC_PER_SEC (1000 * 1000 * 1000)

struct timespec __wasi_timestamp_to_timespec(__wasi_timestamp_t timestamp) {
  return (struct timespec){.tv_sec = timestamp / NSEC_PER_SEC,
                           .tv_nsec = timestamp % NSEC_PER_SEC};
}

int clock_getres(clockid_t clk_id, struct timespec *tp) {
  // See https://github.com/bytecodealliance/wasmtime/issues/3714
  if (clk_id > __WASI_CLOCKID_THREAD_CPUTIME_ID || clk_id < 0) {
    errno = EINVAL;
    return -1;
  }
  __wasi_timestamp_t res;
  __wasi_errno_t error = __wasi_clock_res_get(clk_id, &res);
  if (error != __WASI_ERRNO_SUCCESS) {
    return __wasi_syscall_ret(error);
  }
  *tp = __wasi_timestamp_to_timespec(res);
  return 0;
}

// mmap support is nonexistent. TODO: emulate simple mmaps using
// stdio + malloc, which is slow but may help some things?

// Mark these as weak so that wasmfs does not collide with it. That is, if
// wasmfs is in use, we want to use that and not this.
__attribute__((__weak__)) int _mmap_js(size_t length,
                                       int prot,
                                       int flags,
                                       int fd,
                                       size_t offset,
                                       int* allocated,
                                       void** addr) {
  return -ENOSYS;
}

__attribute__((__weak__)) int _munmap_js(
  intptr_t addr, size_t length, int prot, int flags, int fd, size_t offset) {
  return -ENOSYS;
}

// open(), etc. - we just support the standard streams, with no
// corner case error checking; everything else is not permitted.
// TODO: full file support for WASI, or an option for it
// open()
__attribute__((__weak__))
int __syscall_openat(int dirfd, intptr_t path, int flags, ...) {
  if (!strcmp((const char*)path, "/dev/stdin")) {
    return STDIN_FILENO;
  }
  if (!strcmp((const char*)path, "/dev/stdout")) {
    return STDOUT_FILENO;
  }
  if (!strcmp((const char*)path, "/dev/stderr")) {
    return STDERR_FILENO;
  }
  return -EPERM;
}

__attribute__((__weak__)) int __syscall_ioctl(int fd, int op, ...) {
  return -ENOSYS;
}

__attribute__((__weak__)) int __syscall_fcntl64(int fd, int cmd, ...) {
  return -ENOSYS;
}

__attribute__((__weak__)) int __syscall_fstat64(int fd, intptr_t buf) {
  return -ENOSYS;
}

// There is no good source of entropy without an import. Make this weak so that
// it can be replaced with a pRNG or a proper import.
__attribute__((__weak__))
int getentropy(void* buffer, size_t length) {
  abort();
}

// Emscripten additions

extern void emscripten_notify_memory_growth(size_t memory_index);

// Should never be called in standalone mode
void emscripten_memcpy_big(void *restrict dest, const void *restrict src, size_t n) {
  __builtin_unreachable();
}

size_t emscripten_get_heap_max() {
  // In standalone mode we don't have any wasm instructions to access the max
  // memory size so the best we can do (without calling an import) is return
  // the current heap size.
  return emscripten_get_heap_size();
}

int emscripten_resize_heap(size_t size) {
#ifdef EMSCRIPTEN_MEMORY_GROWTH
  size_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
  assert(old_size < size);
  ssize_t diff = (size - old_size + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE;
  size_t result = __builtin_wasm_memory_grow(0, diff);
  if (result != (size_t)-1) {
    // Success, update JS (see https://github.com/WebAssembly/WASI/issues/82)
    emscripten_notify_memory_growth(0);
    return 1;
  }
#endif
  return 0;
}

double emscripten_get_now(void) {
  return (1000 * clock()) / (double)CLOCKS_PER_SEC;
}

// C++ ABI

// Emscripten disables exception catching by default, but not throwing. That
// allows users to see a clear error if a throw happens, and 99% of the
// overhead is in the catching, so this is a reasonable tradeoff.
// For now, in a standalone build just terminate. TODO nice error message
//
// Define these symbols as weak so that when we build with exceptions
// enabled (using wasm-eh) we get the real versions of these functions
// as defined in libc++abi.

__attribute__((__weak__))
void __cxa_throw(void* ptr, void* type, void* destructor) {
  abort();
}

__attribute__((__weak__))
void* __cxa_allocate_exception(size_t thrown_size) {
  abort();
}

// WasmFS integration. We stub out file preloading and such, that are not
// expected to work anyhow.

size_t _wasmfs_get_num_preloaded_files() { return 0; }

size_t _wasmfs_get_num_preloaded_dirs() { return 0; }

int _wasmfs_get_preloaded_file_size(int index) { return 0; }

int _wasmfs_get_preloaded_file_mode(int index) { return 0; }

void _wasmfs_copy_preloaded_file_data(int index, void* buffer) {}

void _wasmfs_get_preloaded_parent_path(int index, void* buffer) {}

void _wasmfs_get_preloaded_child_path(int index, void* buffer) {}

void _wasmfs_get_preloaded_path_name(int index, void* buffer) {}

// Import the VM's fd_write under a different name. Then we can interpose in
// between it and WasmFS's fd_write. That is, libc calls fd_write, which WasmFS
// implements. And WasmFS will forward actual writing to stdout/stderr to the
// VM's fd_write. (This allows WasmFS to do work in the middle, for example, it
// could support embedded files and other functionality.)
__attribute__((import_module("wasi_snapshot_preview1"),
               import_name("fd_write"))) __wasi_errno_t
imported__wasi_fd_write(__wasi_fd_t fd,
                        const __wasi_ciovec_t* iovs,
                        size_t iovs_len,
                        __wasi_size_t* nwritten);

// Write a buffer + a newline.
static void wasi_writeln(__wasi_fd_t fd, char* buffer) {
  struct __wasi_ciovec_t iovs[2];
  iovs[0].buf = (uint8_t*)buffer;
  iovs[0].buf_len = strlen(buffer);
  iovs[1].buf = (uint8_t*)"\n";
  iovs[1].buf_len = 1;
  __wasi_size_t nwritten;
  imported__wasi_fd_write(fd, iovs, 2, &nwritten);
}

void _emscripten_out(char* text) { wasi_writeln(1, text); }

void _emscripten_err(char* text) { wasi_writeln(2, text); }
