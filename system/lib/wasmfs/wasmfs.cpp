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

long __syscall_dup2(long oldfd, long newfd) {
  auto fileTable = FileTable::get();

  auto oldOpenFile = fileTable[oldfd];
  // If oldfd is not a valid file descriptor, then the call fails,
  // and newfd is not closed.
  if (!oldOpenFile) {
    return -(EBADF);
  }

  if (newfd < 0) {
    return -(EBADF);
  }

  if (oldfd == newfd) {
    return oldfd;
  }

  // If the file descriptor newfd was previously open, it will just be overwritten silently.
  fileTable[newfd] = oldOpenFile;
  return newfd;
}

long __syscall_dup(long fd) {
  auto fileTable = FileTable::get();

  // Check that an open file exists corresponding to the given fd.
  auto currentOpenFile = fileTable[fd];
  if (!currentOpenFile) {
    return -(EBADF);
  }

  return fileTable.add(currentOpenFile);
}

__wasi_errno_t __wasi_fd_write(
  __wasi_fd_t fd, const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  auto currentOpenFile = [&fd] {
    auto fileTable = FileTable::get();
    return fileTable[fd];
  }();

  if (!currentOpenFile) {
    return __WASI_ERRNO_BADF;
  }

  if (!currentOpenFile->get().getFile()->is<DataFile>()) {
    // TODO: Reading a directory file
    return __WASI_ERRNO_ISDIR;
  }
  auto file = currentOpenFile->get().getFile()->cast<DataFile>()->get();

  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;

    file.write(buf, len);
    num += len;
  }
  *nwritten = num;

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t __wasi_fd_seek(
  __wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t* newoffset) {
  emscripten_console_log("__wasi_fd_seek has been temporarily stubbed and is inert");
  abort();
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  auto fileTable = FileTable::get();

  // Remove openFileState entry from fileTable.
  fileTable[fd] = nullptr;

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t __wasi_fd_read(
  __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) {
  auto currentOpenFile = [&fd] {
    auto fileTable = FileTable::get();
    return fileTable[fd];
  }();

  if (!currentOpenFile) {
    return __WASI_ERRNO_BADF;
  }

  if (!currentOpenFile->get().getFile()->is<DataFile>()) {
    // TODO: Reading a directory file
    return __WASI_ERRNO_ISDIR;
  }
  auto file = currentOpenFile->get().getFile()->cast<DataFile>()->get();
  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;

    file.read(buf, len);
    num += len;
  }
  *nread = num;
  return __WASI_ERRNO_INVAL;
}

__wasi_fd_t __syscall_open(long pathname, long flags, long mode) {
  int accessMode = (flags & O_ACCMODE);
  bool canWrite = false;

  if (accessMode == O_WRONLY || accessMode == O_RDWR) {
    canWrite = true;
  }

  std::string interim = "";
  std::vector<std::string> pathParts;

  auto newPathName = (const char*)pathname;

  for (int i = 0; newPathName[i] != '\0'; i++) {
    if (newPathName[i] != '/') {
      interim += newPathName[i];
    } else {
      // Avoid adding empty string
      if (i > 0) {
        pathParts.push_back(interim);
      }
      interim = "";
    }
  }
  // Case where there is no / at the end of the string.
  pathParts.push_back(interim);

  std::shared_ptr<File> curr = getRootDirectory();
  for (int i = 0; i < pathParts.size(); i++) {

    // If it is a file and we have not reached the end of the path, exit
    if (curr->is<DataFile>() && i != pathParts.size() - 1) {
      return -(EBADF);
    }

    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    curr->get().printKeys();
#endif
    curr = curr->cast<Directory>()->get().getEntry(pathParts[i]);

#ifdef WASMFS_DEBUG
    std::vector<char> temp(pathParts[i].begin(), pathParts[i].end());
    emscripten_console_log(&temp[0]);
#endif
  }
  auto currentOpenFile = std::make_shared<OpenFileState>(0, curr);

  auto fileTable = FileTable::get();

  return fileTable.add(currentOpenFile);
}
}
