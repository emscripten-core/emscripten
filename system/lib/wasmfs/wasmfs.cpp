// // Copyright 2021 The Emscripten Authors.  All rights reserved.
// // Emscripten is available under two separate licenses, the MIT license and the
// // University of Illinois/NCSA Open Source License.  Both these licenses can be
// // found in the LICENSE file.
// // wasmfs.cpp will implement a new file system that replaces the existing JS filesystem.
// // Current Status: Work in Progress.
// // See https://github.com/emscripten-core/emscripten/issues/15041.

#include <stdlib.h>
#define __NEED_struct_iovec
#include <emscripten/emscripten.h>
#include <wasi/api.h>

extern "C" {

int emscripten_wasmfs_printbuffer(__wasi_fd_t fd, const uint8_t* ptr, __wasi_size_t len);

__wasi_errno_t __wasi_fd_write(
  __wasi_fd_t fd, const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  // FD 1 = STDOUT and FD 2 = STDERR.
  // Temporary hardcoding of filedescriptor values.
  // TODO: May not want to proxy stderr (fd == 2) to the main thread.
  // This will not not show in HTML, a console.warn in a worker is suffficient.
  // This would be a change from the current FS.
  if (fd == 1 || fd == 2) {
    __wasi_size_t num = 0;
    for (size_t i = 0; i < iovs_len; i++) {
      const uint8_t* ptr = iovs[i].buf;
      __wasi_size_t len = iovs[i].buf_len;
      emscripten_wasmfs_printbuffer(fd, ptr, len);
      num += len;
    }
    *nwritten = num;
  }
  return 0;
}

__wasi_errno_t __wasi_fd_seek(
  __wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t* newoffset) {
  EM_ASM({out($0 + ' or __wasi_fd_seek has been temporarily stubbed and is inert')},
    __PRETTY_FUNCTION__);
  abort();
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  EM_ASM({out($0 + ' or __wasi_fd_close has been temporarily stubbed and is inert')},
    __PRETTY_FUNCTION__);
  abort();
}

__wasi_errno_t __wasi_fd_read(
  __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) {
  EM_ASM({out($0 + ' or __wasi_fd_read has been temporarily stubbed and is inert')},
    __PRETTY_FUNCTION__);
  abort();
}
}
