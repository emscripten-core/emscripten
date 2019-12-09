/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <stdlib.h>
#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

int __wasi_syscall_ret(__wasi_errno_t code) {
  if (code == __WASI_ERRNO_SUCCESS) return 0;
  // We depend on the fact that wasi codes are identical to our errno codes.
  errno = code;
  return -1;
}

int  __wasi_fd_is_valid(__wasi_fd_t fd) {
  __wasi_fdstat_t statbuf;
  int err = __wasi_fd_fdstat_get(fd, &statbuf);
  if (err != __WASI_ERRNO_SUCCESS) {
    errno = err;
    return 0;
  }
  return 1;
}
