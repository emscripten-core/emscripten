#include <emscripten.h>
#include <wasi/api.h>

#include "wasmfs_internal.h"

#define WEAK __attribute__((weak))

WEAK
int __syscall_ioctl(int fd, int request, ...) {
  return 0;
}

WEAK
__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd,
                              __wasi_filedelta_t offset,
                              __wasi_whence_t whence,
                              __wasi_filesize_t* newoffset) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
__wasi_errno_t __wasi_fd_write(__wasi_fd_t fd,
                               const __wasi_ciovec_t* iovs,
                               size_t iovs_len,
                               __wasi_size_t* nwritten) {
  return _wasmfs_no_fs_fd_write(fd, iovs, iovs_len, nwritten);
}

WEAK
__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* stat) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
__wasi_errno_t __wasi_fd_sync(__wasi_fd_t fd) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
int __syscall_newfstatat(int dirfd, intptr_t path, intptr_t buf, int flags) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
int __syscall_stat64(intptr_t path, intptr_t buf) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
int __syscall_lstat64(intptr_t path, intptr_t buf) {
  return __WASI_ERRNO_SUCCESS;
}

WEAK
int __syscall_fstat64(int fd, intptr_t buf) {
  return __WASI_ERRNO_SUCCESS;
}

// The following are really just needed due to the current architecture of
// WasmFS and how it calls back from JS into wasm. When we remove those calls
// (when we move that code to wasm) they should not be needed.

WEAK
int __syscall_getcwd(intptr_t buf, size_t size) {
  return -1;
}

WEAK
void* _wasmfs_read_file(char* path) {
  return NULL;
}

WEAK
char* _wasmfs_get_cwd(void) {
  return NULL;
}
