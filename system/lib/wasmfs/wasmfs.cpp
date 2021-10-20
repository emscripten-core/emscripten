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
  fileTable[newfd] = oldOpenFile.unlocked();
  return newfd;
}

long __syscall_dup(long fd) {
  auto fileTable = FileTable::get();

  // Check that an open file exists corresponding to the given fd.
  auto openFile = fileTable[fd];
  if (!openFile) {
    return -(EBADF);
  }

  return fileTable.add(openFile.unlocked());
}

__wasi_errno_t __wasi_fd_write(__wasi_fd_t fd,
                               const __wasi_ciovec_t* iovs,
                               size_t iovs_len,
                               __wasi_size_t* nwritten) {
  return offsetWrite(fd, iovs, iovs_len, NULL, nwritten, true);
}

__wasi_errno_t __wasi_fd_read(__wasi_fd_t fd,
                              const __wasi_iovec_t* iovs,
                              size_t iovs_len,
                              __wasi_size_t* nread) {
  return offsetRead(fd, iovs, iovs_len, NULL, nread, true);
}

__wasi_errno_t __wasi_fd_pwrite(__wasi_fd_t fd,
                                const __wasi_ciovec_t* iovs,
                                size_t iovs_len,
                                __wasi_filesize_t offset,
                                __wasi_size_t* nwritten) {
  return offsetWrite(fd, iovs, iovs_len, offset, nwritten, false);
}

__wasi_errno_t __wasi_fd_pread(__wasi_fd_t fd,
                               const __wasi_iovec_t* iovs,
                               size_t iovs_len,
                               __wasi_filesize_t offset,
                               __wasi_size_t* nread) {
  return offsetRead(fd, iovs, iovs_len, offset, nread, false);
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
    return -(EBADF);
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
    directory->locked().printKeys();
#endif
    curr = directory->locked().getEntry(pathParts[i]);

    // Requested entry (file or directory)
    if (!curr) {
      if (i == pathParts.size() - 1 && flags & O_CREAT) {
        // If curr is the last element and the create flag is specified
        auto lockedDir = directory->locked();

        // Create an empty in-memory file.
        auto created = std::make_shared<MemoryFile>(mode);

        lockedDir.setEntry(pathParts[i], created);
        auto openFile = std::make_shared<OpenFileState>(0, flags, created);

        return FileTable::get().add(openFile);
      } else {
        return -(ENOENT);
      }
    }

#ifdef WASMFS_DEBUG
    std::vector<char> temp(pathParts[i].begin(), pathParts[i].end());
    emscripten_console_log(&temp[0]);
#endif
  }

  // Fail if O_DIRECTORY is specified and pathname is not a directory
  if (flags & O_DIRECTORY && !curr->is<Directory>()) {
    return -(ENOTDIR);
  }

  // If the file exists and O_EXCL and O_CREAT are true
  if (flags & O_EXCL && flags & O_CREAT) {
    return -(EEXIST);
  }

  auto openFile = std::make_shared<OpenFileState>(0, flags, curr);

  return FileTable::get().add(openFile);
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
