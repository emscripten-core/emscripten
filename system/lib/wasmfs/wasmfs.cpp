// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// wasmfs.cpp will implement a new file system that replaces the existing JS filesystem.
// Current Status: Work in Progress.
// See https://github.com/emscripten-core/emscripten/issues/15041.

#include "file.h"
#include "file_table.h"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <errno.h>
#include <mutex>
#include <stdlib.h>
#include <utility>
#include <vector>
#include <wasi/api.h>

extern "C" {

using namespace wasmfs;

__wasi_fd_t __syscall_dup2(__wasi_fd_t oldfd, __wasi_fd_t newfd) {
  FileTable::Handle fileTable = FileTable::get();

  auto oldOpenFile = fileTable[oldfd];
  // If oldfd is not a valid file descriptor, then the call fails,
  // and newfd is not closed.
  if (!oldOpenFile) {
    return __WASI_ERRNO_BADF;
  }

  if (oldfd == newfd) {
    return oldfd;
  }

  if (newfd < 0) {
    return __WASI_ERRNO_BADF;
  }

  // If the file descriptor newfd was previously open, it is closed
  // before being reused; the close is performed silently.
  if (fileTable[newfd]) {
    fileTable.remove(newfd);
  }

  fileTable[newfd] = oldOpenFile;
  return newfd;
}

__wasi_fd_t __syscall_dup(__wasi_fd_t fd) {

  FileTable::Handle fileTable = FileTable::get();
  if (!fileTable[fd]) {
    return __WASI_ERRNO_BADF;
  }
  // Find the first free open file entry
  return fileTable.add(fileTable[fd]);
}

__wasi_errno_t __wasi_fd_write(
  __wasi_fd_t fd, const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  // Get the corresponding OpenFile from the open file table
  FileTable::Handle fileTable = FileTable::get();
  if (!fileTable[fd]) {
    return __WASI_ERRNO_BADF;
  }

  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;

    fileTable[fd]->get().getFile()->get().write(buf, len);
    num += len;
  }
  *nwritten = num;

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t __wasi_fd_seek(
  __wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t* newoffset) {
  emscripten_console_log("__wasi_fd_seek has been temporarily stubbed and is inert");
  return __WASI_ERRNO_INVAL;
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  emscripten_console_log("__wasi_fd_close has been temporarily stubbed and is inert");
  return __WASI_ERRNO_INVAL;
}

__wasi_errno_t __wasi_fd_read(
  __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) {
  emscripten_console_log("__wasi_fd_read has been temporarily stubbed and is inert");
  return __WASI_ERRNO_INVAL;
}
}
