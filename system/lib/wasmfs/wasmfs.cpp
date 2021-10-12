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
  FileTable::Handle fileTable = FileTable::get();

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

  FileTable::Handle fileTable = FileTable::get();

  // Check that an open file exists corresponding to the given fd.
  auto currentOpenFile = fileTable[fd];
  if (!currentOpenFile) {
    return -(EBADF);
  }

  // Adds given OpenFileState to FileTable entries. Returns fd (insertion index in entries).
  // If no free space is found, currentOpenFile will be appended to the back of the entries.
  for (__wasi_fd_t i = 0;; i++) {
    if (!fileTable[i]) {
      // Free open file entry.
      fileTable[i] = currentOpenFile;
      return i;
    }
  }

  return -(EBADF);
}

__wasi_errno_t __wasi_fd_write(
  __wasi_fd_t fd, const __wasi_ciovec_t* iovs, size_t iovs_len, __wasi_size_t* nwritten) {
  FileTable::Handle fileTable = FileTable::get();
  if (!fileTable[fd]) {
    return __WASI_ERRNO_BADF;
  }

  auto file = fileTable[fd]->get().getFile()->get();

  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;

    static_cast<DataFile::Handle&>(file).write(buf, len);
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
  emscripten_console_log("__wasi_fd_close has been temporarily stubbed and is inert");
  abort();
}

__wasi_errno_t __wasi_fd_read(
  __wasi_fd_t fd, const __wasi_iovec_t* iovs, size_t iovs_len, __wasi_size_t* nread) {
  FileTable::Handle fileTable = FileTable::get();
  if (!fileTable[fd]) {
    return __WASI_ERRNO_BADF;
  }

  auto file = fileTable[fd]->get().getFile()->get();
  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;

    static_cast<DataFile::Handle&>(file).read(buf, len);
    num += len;
  }
  *nread = num;
  return __WASI_ERRNO_INVAL;
}

__wasi_fd_t __syscall_open(long pathname, long flags, long mode) {
  // Get the corresponding OpenFile from the open file table
  FileTable::Handle fileTable = FileTable::get();

  // Initialize or Retrieve root directory
  RootDirectory::Handle rootDirectory = RootDirectory::get();

  rootDirectory.printKeys();

  int accessMode = (flags & O_ACCMODE);
  bool canWrite = false;

  switch (accessMode) {
    case O_RDONLY:
      canWrite = false;
    case O_WRONLY:
      canWrite = true;
    case O_RDWR:
      canWrite = true;
    default:
      canWrite = false;
  }

  std::string interim = "";
  std::vector<std::string> pathParts;

  const char* newPathName = (const char*)pathname;
  std::string newString = std::string(newPathName);
  for (int i = 0; i < newString.size(); i++) {
    if (newString[i] != '/') {
      interim += newString[i];
    } else {
      pathParts.push_back(interim);
      interim = "";
    }
  }

  auto fileFromPath = RootDirectory::getSharedPtr();
  for (int i = 0; i < pathParts.size(); i++) {

    // If it is a file and we have not reached the end of the path, exit
    // if (fileFromPath->is<DataFile>() && i != pathParts.size() - 1) {
    //   return -(EBADF);
    // }

    // Find the next entry in the current directory entry
    fileFromPath->get().printKeys();
    fileFromPath = std::static_pointer_cast<Directory>(fileFromPath->get().getEntry(pathParts[i]));

    std::vector<char> temp(pathParts[i].begin(), pathParts[i].end());
    emscripten_console_log(&temp[0]);
  }
  auto currentOpenFile =
    std::make_shared<OpenFileState>(0, std::static_pointer_cast<File>(fileFromPath));

  for (__wasi_fd_t i = 0;; i++) {
    if (!fileTable[i]) {
      // Free open file entry.
      fileTable[i] = currentOpenFile;
      return i;
    }
  }

  return -(EBADF);
}
}
