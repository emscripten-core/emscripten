/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <stdlib.h>
#include <wasi/wasi.h>
#include <wasi/wasi-helpers.h>

int __wasi_syscall_ret(__wasi_errno_t code) {
  if (code == __WASI_ESUCCESS) return 0;
  // We depend on the fact that wasi codes are identical to our errno codes.
  errno = code;
  return -1;
}
