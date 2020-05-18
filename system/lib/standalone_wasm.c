/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

/*
 * WASI support code. These are compiled with the program, and call out
 * using wasi APIs, which can be provided either by a wasi VM or by our
 * emitted JS.
 */

// libc

void _Exit(int status) {
  __wasi_proc_exit(status);
  __builtin_unreachable();
}

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

long __map_file(int x, int y) {
  return -ENOSYS;
}

long __syscall91(int x, int y) { // munmap
  return -ENOSYS;
}

// mmap2()
long __syscall192(long addr, long len, long prot, long flags, long fd, long off) {
  return -ENOSYS;
}

// open(), etc. - we just support the standard streams, with no
// corner case error checking; everything else is not permitted.
// TODO: full file support for WASI, or an option for it
// open()
long __syscall5(const char* path, long flags, ...) {
  if (!strcmp(path, "/dev/stdin")) return STDIN_FILENO;
  if (!strcmp(path, "/dev/stdout")) return STDOUT_FILENO;
  if (!strcmp(path, "/dev/stderr")) return STDERR_FILENO;
  return -EPERM;
}

// ioctl()
int __syscall54(int fd, int op, ...) {
  return -ENOSYS;
}

// fcntl64()
long __syscall221(long fd, long cmd, ...) {
  return -ENOSYS;
}

// Emscripten additions

void *emscripten_memcpy_big(void *restrict dest, const void *restrict src, size_t n) {
  // This normally calls out into JS which can do a single fast operation,
  // but with wasi we can't do that. As this is called when n >= 512, we
  // can just split into smaller calls.
  // TODO optimize, maybe build our memcpy with a wasi variant, maybe have
  //      a SIMD variant, etc.
  const int CHUNK = 508;
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

extern void emscripten_notify_memory_growth(size_t memory_index);

int emscripten_resize_heap(size_t size) {
#ifdef __EMSCRIPTEN_MEMORY_GROWTH__
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
void
__cxa_throw(void* ptr, void* type, void* destructor) {
  abort();
}

void* __cxa_allocate_exception(size_t thrown_size) {
  abort();
}
