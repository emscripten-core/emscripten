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
    return -EBADF;
  }

  if (newfd < 0) {
    return -EBADF;
  }

  if (oldfd == newfd) {
    return oldfd;
  }

  // If the file descriptor newfd was previously open, it will just be
  // overwritten silently.
  fileTable[newfd] = oldOpenFile.unlocked();
  return newfd;
}

long __syscall_dup(long fd) {
  auto fileTable = FileTable::get();

  // Check that an open file exists corresponding to the given fd.
  auto openFile = fileTable[fd];
  if (!openFile) {
    return -EBADF;
  }

  return fileTable.add(openFile.unlocked());
}

// This enum specifies whether file offset will be provided by the open file
// state or provided by argument in the case of pread or pwrite.
enum class OffsetHandling { OpenFileState, Argument };

// Internal write function called by __wasi_fd_write and __wasi_fd_pwrite
// Receives an open file state offset.
// Optionally sets open file state offset.
static __wasi_errno_t writeAtOffset(OffsetHandling setOffset,
                                    __wasi_fd_t fd,
                                    const __wasi_ciovec_t* iovs,
                                    size_t iovs_len,
                                    __wasi_size_t* nwritten,
                                    __wasi_filesize_t offset = 0) {
  if (iovs_len < 0 || offset < 0) {
    return __WASI_ERRNO_INVAL;
  }

  auto openFile = FileTable::get()[fd];

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  auto lockedOpenFile = openFile.locked();
  auto file = lockedOpenFile.getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  // TODO: change to add support for symlinks.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  off_t currOffset = setOffset == OffsetHandling::OpenFileState
                       ? lockedOpenFile.position()
                       : offset;
  off_t oldOffset = currOffset;
  auto finish = [&] {
    *nwritten = currOffset - oldOffset;
    if (setOffset == OffsetHandling::OpenFileState) {
      lockedOpenFile.position() = currOffset;
    }
  };
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    off_t len = iovs[i].buf_len;

    // Check if the sum of the buf_len values overflows an off_t (63 bits).
    if (addWillOverFlow(currOffset, len)) {
      return __WASI_ERRNO_FBIG;
    }

    // Check if buf_len specifies a positive length buffer but buf is a
    // null pointer
    if (!buf && len > 0) {
      return __WASI_ERRNO_INVAL;
    }

    auto result = lockedFile.write(buf, len, currOffset);

    if (result != __WASI_ERRNO_SUCCESS) {
      finish();
      return result;
    }
    currOffset += len;
  }
  finish();
  return __WASI_ERRNO_SUCCESS;
}

// Internal read function called by __wasi_fd_read and __wasi_fd_pread
// Receives an open file state offset.
// Optionally sets open file state offset.
static __wasi_errno_t readAtOffset(OffsetHandling setOffset,
                                   __wasi_fd_t fd,
                                   const __wasi_iovec_t* iovs,
                                   size_t iovs_len,
                                   __wasi_size_t* nread,
                                   __wasi_filesize_t offset = 0) {
  if (iovs_len < 0 || offset < 0) {
    return __WASI_ERRNO_INVAL;
  }

  auto openFile = FileTable::get()[fd];

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  auto lockedOpenFile = openFile.locked();
  auto file = lockedOpenFile.getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  // TODO: change to add support for symlinks.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  off_t currOffset = setOffset == OffsetHandling::OpenFileState
                       ? lockedOpenFile.position()
                       : offset;
  off_t oldOffset = currOffset;
  auto finish = [&] {
    *nread = currOffset - oldOffset;
    if (setOffset == OffsetHandling::OpenFileState) {
      lockedOpenFile.position() = currOffset;
    }
  };
  size_t size = lockedFile.size();
  for (size_t i = 0; i < iovs_len; i++) {
    // Check if currOffset has exceeded size of file data.
    ssize_t dataLeft = size - currOffset;
    if (dataLeft <= 0) {
      break;
    }

    uint8_t* buf = iovs[i].buf;

    // Check if buf_len specifies a positive length buffer
    // but buf is a null pointer.
    if (!buf && iovs[i].buf_len > 0) {
      return __WASI_ERRNO_INVAL;
    }

    size_t bytesToRead = std::min(size_t(dataLeft), iovs[i].buf_len);

    auto result = lockedFile.read(buf, bytesToRead, currOffset);

    if (result != __WASI_ERRNO_SUCCESS) {
      finish();
      return result;
    }
    currOffset += bytesToRead;
  }
  finish();
  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t __wasi_fd_write(__wasi_fd_t fd,
                               const __wasi_ciovec_t* iovs,
                               size_t iovs_len,
                               __wasi_size_t* nwritten) {
  return writeAtOffset(
    OffsetHandling::OpenFileState, fd, iovs, iovs_len, nwritten);
}

__wasi_errno_t __wasi_fd_read(__wasi_fd_t fd,
                              const __wasi_iovec_t* iovs,
                              size_t iovs_len,
                              __wasi_size_t* nread) {
  return readAtOffset(OffsetHandling::OpenFileState, fd, iovs, iovs_len, nread);
}

__wasi_errno_t __wasi_fd_pwrite(__wasi_fd_t fd,
                                const __wasi_ciovec_t* iovs,
                                size_t iovs_len,
                                __wasi_filesize_t offset,
                                __wasi_size_t* nwritten) {
  return writeAtOffset(
    OffsetHandling::Argument, fd, iovs, iovs_len, nwritten, offset);
}

__wasi_errno_t __wasi_fd_pread(__wasi_fd_t fd,
                               const __wasi_iovec_t* iovs,
                               size_t iovs_len,
                               __wasi_filesize_t offset,
                               __wasi_size_t* nread) {
  return readAtOffset(
    OffsetHandling::Argument, fd, iovs, iovs_len, nread, offset);
}

__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd) {
  auto fileTable = FileTable::get();

  // Remove openFileState entry from fileTable.
  fileTable[fd] = nullptr;

  return __WASI_ERRNO_SUCCESS;
}

long __syscall_fstat64(long fd, long buf) {
  auto openFile = FileTable::get()[fd];

  if (!openFile) {
    return -EBADF;
  }

  auto file = openFile.locked().getFile();

  struct stat* buffer = (struct stat*)buf;

  auto lockedFile = file->locked();

  if (file->is<Directory>()) {
    buffer->st_size = 4096;
  } else if (file->is<DataFile>()) {
    buffer->st_size = lockedFile.size();
  } else { // TODO: add size of symlinks
    buffer->st_size = 0;
  }

  // ATTN: hard-coded constant values are copied from the existing JS file
  // system. Specific values were chosen to match existing library_fs.js values.
  buffer->st_dev =
    1; // ID of device containing file: Hardcode 1 for now, no meaning at the
  // moment for Emscripten.
  buffer->st_mode = lockedFile.mode();
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
  buffer->st_atim.tv_sec = lockedFile.atime();
  buffer->st_mtim.tv_sec = lockedFile.mtime();
  buffer->st_ctim.tv_sec = lockedFile.ctime();

  return __WASI_ERRNO_SUCCESS;
}

__wasi_fd_t __syscall_open(long pathname, long flags, long mode) {
  int accessMode = (flags & O_ACCMODE);
  bool canWrite = false;

  if (accessMode == O_WRONLY || accessMode == O_RDWR) {
    canWrite = true;
  }

  // TODO: remove assert when all functionality is complete.
  assert(((flags) & ~(O_CREAT | O_EXCL | O_DIRECTORY | O_TRUNC | O_APPEND |
                      O_RDWR | O_WRONLY | O_RDONLY | O_LARGEFILE)) == 0);

  auto pathParts = splitPath((char*)pathname);

  if (pathParts.empty()) {
    return -EINVAL;
  }

  auto base = pathParts[pathParts.size() - 1];

  // Root directory
  if (pathParts.size() == 1 && pathParts[0] == "/") {
    auto openFile =
      std::make_shared<OpenFileState>(0, flags, getRootDirectory());
    return FileTable::get().add(openFile);
  }

  long err;
  auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

  // Parent node doesn't exist.
  if (!parentDir) {
    return err;
  }

  auto lockedParentDir = parentDir->locked();

  auto curr = lockedParentDir.getEntry(base);

  // The requested node was not found.
  if (!curr) {
    // If curr is the last element and the create flag is specified
    // If O_DIRECTORY is also specified, still create a regular file:
    // https://man7.org/linux/man-pages/man2/open.2.html#BUGS
    if (flags & O_CREAT) {
      // Create an empty in-memory file.
      auto created = std::make_shared<MemoryFile>(mode);

      lockedParentDir.setEntry(base, created);
      auto openFile = std::make_shared<OpenFileState>(0, flags, created);

      return FileTable::get().add(openFile);
    } else {
      return -ENOENT;
    }
  }

  // Fail if O_DIRECTORY is specified and pathname is not a directory
  if (flags & O_DIRECTORY && !curr->is<Directory>()) {
    return -ENOTDIR;
  }

  // Return an error if the file exists and O_CREAT and O_EXCL are specified.
  if (flags & O_EXCL && flags & O_CREAT) {
    return -EEXIST;
  }

  auto openFile = std::make_shared<OpenFileState>(0, flags, curr);

  return FileTable::get().add(openFile);
}

long __syscall_mkdir(long path, long mode) {
  auto pathParts = splitPath((char*)path);

  if (pathParts.empty()) {
    return -EINVAL;
  }
  // Root (/) directory.
  if (pathParts.empty() || pathParts.size() == 1 && pathParts[0] == "/") {
    return -EEXIST;
  }

  auto base = pathParts[pathParts.size() - 1];

  long err;
  auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

  if (!parentDir) {
    // parent node doesn't exist
    return err;
  }

  auto lockedParentDir = parentDir->locked();

  auto curr = lockedParentDir.getEntry(base);

  // Check if the requested directory already exists.
  if (curr) {
    return -EEXIST;
  } else {
    // Create an empty in-memory directory.
    auto created = std::make_shared<Directory>(mode);

    lockedParentDir.setEntry(base, created);
    return 0;
  }
}

__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd,
                              __wasi_filedelta_t offset,
                              __wasi_whence_t whence,
                              __wasi_filesize_t* newoffset) {
  auto openFile = FileTable::get()[fd];
  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }
  auto lockedOpenFile = openFile.locked();

  off_t position;
  if (whence == SEEK_SET) {
    position = offset;
  } else if (whence == SEEK_CUR) {
    position = lockedOpenFile.position() + offset;
  } else if (whence == SEEK_END) {
    // Only the open file stat is altered in seek. Locking the underlying data
    // file here once is sufficient.
    position = lockedOpenFile.getFile()->locked().size() + offset;
  } else {
    return __WASI_ERRNO_INVAL;
  }

  if (position < 0) {
    return __WASI_ERRNO_INVAL;
  }

  lockedOpenFile.position() = position;

  if (newoffset) {
    *newoffset = position;
  }

  return __WASI_ERRNO_SUCCESS;
}
}
