// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// wasmfs.cpp will implement a new file system that replaces the existing JS
// filesystem. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#include "file.h"
#include "file_table.h"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <errno.h>
#include <mutex>
#include <stdlib.h>
#include <sys/stat.h>
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

  // If the file descriptor newfd was previously open, it will just be
  // overwritten silently.
  fileTable[newfd] = oldOpenFile;
  return newfd;
}

long __syscall_dup(long fd) {
  auto fileTable = FileTable::get();

  // Check that an open file exists corresponding to the given fd.
  auto openFile = fileTable[fd];
  if (!openFile) {
    return -(EBADF);
  }

  return fileTable.add(openFile);
}

__wasi_errno_t __wasi_fd_write(__wasi_fd_t fd,
                               const __wasi_ciovec_t* iovs,
                               size_t iovs_len,
                               __wasi_size_t* nwritten) {
  std::shared_ptr<OpenFileState> openFile = FileTable::get()[fd];

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  auto file = openFile->get().getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;

    file->get().write(buf, len);
    num += len;
  }
  *nwritten = num;

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd,
                              __wasi_filedelta_t offset,
                              __wasi_whence_t whence,
                              __wasi_filesize_t* newoffset) {
  emscripten_console_log(
    "__wasi_fd_seek has been temporarily stubbed and is inert");
  abort();
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  auto fileTable = FileTable::get();

  // Remove openFileState entry from fileTable.
  fileTable[fd] = nullptr;

  return __WASI_ERRNO_SUCCESS;
}

long __syscall_fstat64(long fd, long buf) {
  // Release FileTable lock after accessing desired open file.
  std::shared_ptr<File> file = FileTable::get()[fd]->get().getFile();

  struct stat* buffer = (struct stat*)buf;

  auto fileInfo = file->get();

  if (file->is<Directory>()) {
    buffer->st_size = 4096;
  } else if (file->is<DataFile>()) {
    buffer->st_size = fileInfo.size();
  } else { // TODO: add size of symlinks
    buffer->st_size = 0;
  }

  // ATTN: hard-coded constant values are copied from the existing JS file
  // system. Specific values were chosen to match existing library_fs.js values.
  buffer->st_dev =
    1; // ID of device containing file: Hardcode 1 for now, no meaning at the
  // moment for Emscripten.
  buffer->st_mode = fileInfo.mode();
  buffer->st_ino = fd;
  // The number of hard links is 1 since they are unsupported.
  buffer->st_nlink = 1;
  buffer->st_uid = 0;
  buffer->st_gid = 0;
  buffer->st_rdev =
    1; // Device ID (if special file) No meaning right now for Emscripten.

  // The syscall docs state this is hardcoded to # of 512 byte blocks.
  buffer->st_blocks = (buffer->st_size + 511) / 512;
  buffer->st_blksize =
    4096; // Specifies the preferred blocksize for efficient disk I/O.
  buffer->st_atim.tv_sec = fileInfo.atime();
  buffer->st_mtim.tv_sec = fileInfo.mtime();
  buffer->st_ctim.tv_sec = fileInfo.ctime();

  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t __wasi_fd_read(__wasi_fd_t fd,
                              const __wasi_iovec_t* iovs,
                              size_t iovs_len,
                              __wasi_size_t* nread) {
  std::shared_ptr<OpenFileState> openFile = FileTable::get()[fd];

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  auto file = openFile->get().getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  __wasi_size_t num = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    __wasi_size_t len = iovs[i].buf_len;

    file->get().read(buf, len);
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

  std::vector<std::string> pathParts;

  char newPathName[strlen((char*)pathname) + 1];
  strcpy(newPathName, (char*)pathname);

  // TODO: Support relative paths. i.e. specify cwd if path is relative.
  // TODO: Other path parsing edge cases.
  char* current;

  current = strtok(newPathName, "/\n");
  while (current != NULL) {
    pathParts.push_back(current);
    current = strtok(NULL, "/\n");
  }

  std::shared_ptr<File> curr = getRootDirectory();
  for (int i = 0; i < pathParts.size(); i++) {

    auto directory = curr->dynCast<Directory>();

    // If file is nullptr, then the file was not a Directory.
    // TODO: Change this to accommodate symlinks
    if (!directory) {
      return -(ENOTDIR);
    }

    // Find the next entry in the current directory entry
#ifdef WASMFS_DEBUG
    directory->get().printKeys();
#endif
    curr = directory->get().getEntry(pathParts[i]);

    // Requested entry (file or directory
    if (!curr) {
      return -(ENOENT);
    }

#ifdef WASMFS_DEBUG
    std::vector<char> temp(pathParts[i].begin(), pathParts[i].end());
    emscripten_console_log(&temp[0]);
#endif
  }

  auto openFile = std::make_shared<OpenFileState>(0, curr);

  return FileTable::get().add(openFile);
}
}
