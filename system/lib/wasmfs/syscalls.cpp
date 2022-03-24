// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// syscalls.cpp will implement the syscalls of the new file system replacing the
// old JS version. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#include <dirent.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <errno.h>
#include <mutex>
#include <poll.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syscall_arch.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <wasi/api.h>
#include <syscall_arch.h>

#include "backend.h"
#include "file.h"
#include "file_table.h"
#include "paths.h"
#include "pipe_backend.h"
#include "wasmfs.h"

// File permission macros for wasmfs.
// Used to improve readability compared to those in stat.h
#define WASMFS_PERM_READ 0444

#define WASMFS_PERM_WRITE 0222

#define WASMFS_PERM_EXECUTE 0111

// In Linux, the maximum length for a filename is 255 bytes.
#define WASMFS_NAME_MAX 255

extern "C" {

using namespace wasmfs;

int __syscall_dup3(long oldfd, long newfd, long flags) {
  if (flags & !O_CLOEXEC) {
    // TODO: Test this case.
    return -EINVAL;
  }

  auto fileTable = wasmFS.getFileTable().locked();
  auto oldOpenFile = fileTable.getEntry(oldfd);
  if (!oldOpenFile) {
    return -EBADF;
  }
  if (newfd < 0) {
    return -EBADF;
  }
  if (oldfd == newfd) {
    return -EINVAL;
  }

  // If the file descriptor newfd was previously open, it will just be
  // overwritten silently.
  fileTable.setEntry(newfd, oldOpenFile);
  return newfd;
}

int __syscall_dup(long fd) {
  auto fileTable = wasmFS.getFileTable().locked();

  // Check that an open file exists corresponding to the given fd.
  auto openFile = fileTable.getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  return fileTable.addEntry(openFile);
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

  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  // TODO: Check open file access mode for write permissions.

  auto lockedOpenFile = openFile->locked();
  auto file = lockedOpenFile.getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  // TODO: change to add support for symlinks.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  off_t currOffset = setOffset == OffsetHandling::OpenFileState
                       ? lockedOpenFile.getPosition()
                       : offset;
  off_t oldOffset = currOffset;
  auto finish = [&] {
    *nwritten = currOffset - oldOffset;
    if (setOffset == OffsetHandling::OpenFileState &&
        lockedOpenFile.getFile()->isSeekable()) {
      lockedOpenFile.setPosition(currOffset);
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

  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);

  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  // TODO: Check open file access mode for read permissions.

  auto lockedOpenFile = openFile->locked();
  auto file = lockedOpenFile.getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  // TODO: change to add support for symlinks.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  off_t currOffset = setOffset == OffsetHandling::OpenFileState
                       ? lockedOpenFile.getPosition()
                       : offset;
  off_t oldOffset = currOffset;
  auto finish = [&] {
    *nread = currOffset - oldOffset;
    if (setOffset == OffsetHandling::OpenFileState &&
        lockedOpenFile.getFile()->isSeekable()) {
      lockedOpenFile.setPosition(currOffset);
    }
  };
  size_t size = lockedFile.getSize();
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
  auto fileTable = wasmFS.getFileTable().locked();
  if (!fileTable.getEntry(fd)) {
    return __WASI_ERRNO_BADF;
  }
  fileTable.setEntry(fd, nullptr);
  return __WASI_ERRNO_SUCCESS;
}

__wasi_errno_t __wasi_fd_sync(__wasi_fd_t fd) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  // Nothing to flush for anything but a data file, but also not an error either
  // way. TODO: in the future we may want syncing of directories.
  auto dataFile = openFile->locked().getFile()->dynCast<DataFile>();
  if (dataFile) {
    dataFile->locked().flush();
  }

  return __WASI_ERRNO_SUCCESS;
}

backend_t wasmfs_get_backend_by_fd(int fd) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return NullBackend;
  }
  return openFile->locked().getFile()->getBackend();
}

// This function is exposed to users to allow them to obtain a backend_t for a
// specified path.
backend_t wasmfs_get_backend_by_path(const char* path) {
  auto parsed = path::parseFile(path);
  if (parsed.getError()) {
    // Could not find the file.
    return NullBackend;
  }
  return parsed.getFile()->getBackend();
}

int __syscall_fstatat64(long dirfd, long path, long buf, long flags) {
  // Only accept valid flags.
  if (flags & ~(AT_EMPTY_PATH | AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW)) {
    // TODO: Test this case.
    return -EINVAL;
  }
  std::shared_ptr<File> file;
  if ((flags & AT_EMPTY_PATH) && strcmp((char*)path, "") == 0) {
    // Don't parse a path, just use `dirfd` directly.
    if (dirfd == AT_FDCWD) {
      // TODO: Test this case.
      file = wasmFS.getCWD();
    } else {
      auto openFile = wasmFS.getFileTable().locked().getEntry(dirfd);
      if (!openFile) {
        return -EBADF;
      }
      file = openFile->locked().getFile();
    }
  } else {
    // Parse the relative path.
    // TODO: Handle AT_SYMLINK_NOFOLLOW once we traverse symlinks correctly.
    auto parsed = path::parseFile((char*)path, dirfd);
    if (auto err = parsed.getError()) {
      return err;
    }
    file = parsed.getFile();
  }

  // Extract the information from the file.
  auto lockedFile = file->locked();
  auto buffer = (struct stat*)buf;

  buffer->st_size = lockedFile.getSize();

  // ATTN: hard-coded constant values are copied from the existing JS file
  // system. Specific values were chosen to match existing library_fs.js
  // values.
  // ID of device containing file: Hardcode 1 for now, no meaning at the
  buffer->st_dev = 1;
  // moment for Emscripten.
  buffer->st_mode = lockedFile.getMode();
  buffer->st_ino = file->getIno();
  // The number of hard links is 1 since they are unsupported.
  buffer->st_nlink = 1;
  buffer->st_uid = 0;
  buffer->st_gid = 0;
  // Device ID (if special file) No meaning right now for Emscripten.
  buffer->st_rdev = 1;
  // The syscall docs state this is hardcoded to # of 512 byte blocks.
  buffer->st_blocks = (buffer->st_size + 511) / 512;
  // Specifies the preferred blocksize for efficient disk I/O.
  buffer->st_blksize = 4096;
  buffer->st_atim.tv_sec = lockedFile.getATime();
  buffer->st_mtim.tv_sec = lockedFile.getMTime();
  buffer->st_ctim.tv_sec = lockedFile.getCTime();
  return __WASI_ERRNO_SUCCESS;
}

int __syscall_stat64(long path, long buf) {
  return __syscall_fstatat64(AT_FDCWD, path, buf, 0);
}

int __syscall_lstat64(long path, long buf) {
  return __syscall_fstatat64(AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
}

int __syscall_fstat64(long fd, long buf) {
  return __syscall_fstatat64(fd, (long)"", buf, AT_EMPTY_PATH);
}

static __wasi_fd_t doOpen(path::ParsedParent parsed,
                          long flags,
                          mode_t mode,
                          backend_t backend = NullBackend) {
  int accessMode = (flags & O_ACCMODE);
  if (accessMode != O_WRONLY && accessMode != O_RDONLY &&
      accessMode != O_RDWR) {
    return -EINVAL;
  }

  // TODO: remove assert when all functionality is complete.
  assert((flags & ~(O_CREAT | O_EXCL | O_DIRECTORY | O_TRUNC | O_APPEND |
                    O_RDWR | O_WRONLY | O_RDONLY | O_LARGEFILE | O_CLOEXEC)) ==
         0);

  if (auto err = parsed.getError()) {
    return err;
  }
  auto& [parent, childName] = parsed.getParentChild();
  if (childName.size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  std::shared_ptr<File> child;
  {
    auto lockedParent = parent->locked();
    child = lockedParent.getChild(std::string(childName));
    // The requested node was not found.
    if (!child) {
      // If curr is the last element and the create flag is specified
      // If O_DIRECTORY is also specified, still create a regular file:
      // https://man7.org/linux/man-pages/man2/open.2.html#BUGS
      if (!(flags & O_CREAT)) {
        return -ENOENT;
      }

      // Inserting into an unlinked directory is not allowed.
      if (!lockedParent.getParent()) {
        return -ENOENT;
      }

      // Mask out everything except the permissions bits.
      mode &= S_IALLUGO;

      // If there is no explicitly provided backend, use the parent's backend.
      if (!backend) {
        backend = parent->getBackend();
      }

      // TODO: Check write permissions on the parent directory.
      // TODO: Forbid mounting new backends except under the root backend.
      // TODO: Check that the insert actually succeeds.
      auto created = backend->createFile(mode);
      lockedParent.insertChild(std::string(childName), created);
      auto openFile = std::make_shared<OpenFileState>(0, flags, created);
      return wasmFS.getFileTable().locked().addEntry(openFile);
    }
  }

  // Check user permissions.
  auto fileMode = child->locked().getMode();
  if ((accessMode == O_RDONLY || accessMode == O_RDWR) &&
      !(fileMode & WASMFS_PERM_READ)) {
    return -EACCES;
  }
  if ((accessMode == O_WRONLY || accessMode == O_RDWR) &&
      !(fileMode & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // Fail if O_DIRECTORY is specified and pathname is not a directory
  if (flags & O_DIRECTORY && !child->is<Directory>()) {
    return -ENOTDIR;
  }

  // Return an error if the file exists and O_CREAT and O_EXCL are specified.
  if (flags & O_EXCL && flags & O_CREAT) {
    return -EEXIST;
  }

  auto openFile = std::make_shared<OpenFileState>(0, flags, child);
  return wasmFS.getFileTable().locked().addEntry(openFile);
}

// This function is exposed to users and allows users to create a file in a
// specific backend. An fd to an open file is returned.
int wasmfs_create_file(char* pathname, mode_t mode, backend_t backend) {
  static_assert(std::is_same_v<decltype(doOpen(0, 0, 0, 0)), unsigned int>,
                "unexpected conversion from result of doOpen to int");
  return doOpen(path::parseParent((char*)pathname), O_CREAT, mode, backend);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_openat(long dirfd, long path, long flags, ...) {
  mode_t mode = 0;
  va_list v1;
  va_start(v1, flags);
  mode = va_arg(v1, int);
  va_end(v1);

  return doOpen(path::parseParent((char*)path, dirfd), flags, mode);
}

static long
doMkdir(path::ParsedParent parsed, long mode, backend_t backend = NullBackend) {
  if (auto err = parsed.getError()) {
    return err;
  }
  auto& [parent, childNameView] = parsed.getParentChild();
  std::string childName(childNameView);
  auto lockedParent = parent->locked();

  if (childName.size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  // Check if the requested directory already exists.
  if (lockedParent.getChild(childName)) {
    return -EEXIST;
  }

  // Mask rwx permissions for user, group and others, and the sticky bit.
  // This prevents users from entering S_IFREG for example.
  // https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
  mode &= S_IRWXUGO | S_ISVTX;

  // By default, the backend that the directory is created in is the same as
  // the parent directory. However, if a backend is passed as a parameter,
  // then that backend is used.
  if (!backend) {
    backend = parent->getBackend();
  }

  // TODO: Check write permissions in the parent.
  // TODO: Forbid mounting new backends except under the root backend.
  // TODO: Check that the insertion is successful.
  auto created = backend->createDirectory(mode);
  lockedParent.insertChild(childName, created);

  // Update the times.
  auto lockedFile = created->locked();
  time_t now = time(NULL);
  lockedFile.setATime(now);
  lockedFile.setMTime(now);
  lockedFile.setCTime(now);

  return 0;
}

// This function is exposed to users and allows users to specify a particular
// backend that a directory should be created within.
int wasmfs_create_directory(char* path, long mode, backend_t backend) {
  static_assert(std::is_same_v<decltype(doMkdir(0, 0, 0)), long>,
                "unexpected conversion from result of doMkdir to int");
  return doMkdir(path::parseParent(path), mode, backend);
}

// TODO: Test this.
int __syscall_mkdirat(long dirfd, long path, long mode) {
  return doMkdir(path::parseParent((char*)path, dirfd), mode);
}

int __syscall_mkdir(long path, long mode) {
  return doMkdir(path::parseParent((char*)path), mode);
}

__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd,
                              __wasi_filedelta_t offset,
                              __wasi_whence_t whence,
                              __wasi_filesize_t* newoffset) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }
  auto lockedOpenFile = openFile->locked();

  if (!lockedOpenFile.getFile()->isSeekable()) {
    return __WASI_ERRNO_SPIPE;
  }

  off_t position;
  if (whence == SEEK_SET) {
    position = offset;
  } else if (whence == SEEK_CUR) {
    position = lockedOpenFile.getPosition() + offset;
  } else if (whence == SEEK_END) {
    // Only the open file state is altered in seek. Locking the underlying
    // data file here once is sufficient.
    position = lockedOpenFile.getFile()->locked().getSize() + offset;
  } else {
    return __WASI_ERRNO_INVAL;
  }

  if (position < 0) {
    return __WASI_ERRNO_INVAL;
  }

  lockedOpenFile.setPosition(position);

  if (newoffset) {
    *newoffset = position;
  }

  return __WASI_ERRNO_SUCCESS;
}

long doChdir(std::shared_ptr<File>& file) {
  auto dir = file->dynCast<Directory>();
  if (!dir) {
    return -ENOTDIR;
  }
  wasmFS.setCWD(dir);
  return 0;
}

int __syscall_chdir(long path) {
  auto parsed = path::parseFile((char*)path);
  if (auto err = parsed.getError()) {
    return err;
  }
  return doChdir(parsed.getFile());
}

int __syscall_fchdir(long fd) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  return doChdir(openFile->locked().getFile());
}

int __syscall_getcwd(long buf, long size) {
  // Check if buf points to a bad address.
  if (!buf && size > 0) {
    return -EFAULT;
  }

  // Check if the size argument is zero and buf is not a null pointer.
  if (buf && size == 0) {
    return -EINVAL;
  }

  auto curr = wasmFS.getCWD();

  std::string result = "";

  while (curr != wasmFS.getRootDirectory()) {
    auto parent = curr->locked().getParent();
    // Check if the parent exists. The parent may not exist if the CWD or one
    // of its ancestors has been unlinked.
    if (!parent) {
      return -ENOENT;
    }

    auto parentDir = parent->dynCast<Directory>();

    auto name = parentDir->locked().getName(curr);
    result = '/' + name + result;
    curr = parentDir;
  }

  // Check if the cwd is the root directory.
  if (result.empty()) {
    result = "/";
  }

  auto res = result.c_str();
  int len = strlen(res) + 1;

  // Check if the size argument is less than the length of the absolute
  // pathname of the working directory, including null terminator.
  if (len >= size) {
    return -ENAMETOOLONG;
  }

  // Return value is a null-terminated c string.
  strcpy((char*)buf, res);

  return len;
}

__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* stat) {
  // TODO: This is only partial implementation of __wasi_fd_fdstat_get. Enough
  // to get __wasi_fd_is_valid working.
  // There are other fields in the stat structure that we should really
  // be filling in here.
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  if (openFile->locked().getFile()->is<Directory>()) {
    stat->fs_filetype = __WASI_FILETYPE_DIRECTORY;
  } else {
    stat->fs_filetype = __WASI_FILETYPE_REGULAR_FILE;
  }
  return __WASI_ERRNO_SUCCESS;
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_unlinkat(long dirfd, long path, long flags) {
  if (flags & ~AT_REMOVEDIR) {
    // TODO: Test this case.
    return -EINVAL;
  }
  // It is invalid for rmdir paths to end in ".", but we need to distinguish
  // this case from the case of `parseParent` returning (root, '.') when parsing
  // "/", so we need to find the invalid "/." manually.
  if (flags == AT_REMOVEDIR) {
    std::string_view p((char*)path);
    // Ignore trailing '/'.
    while (!p.empty() && p.back() == '/') {
      p.remove_suffix(1);
    }
    if (p.size() >= 2 && p.substr(p.size() - 2) == std::string_view("/.")) {
      return -EINVAL;
    }
  }
  auto parsed = path::parseParent((char*)path, dirfd);
  if (auto err = parsed.getError()) {
    return err;
  }
  auto& [parent, childNameView] = parsed.getParentChild();
  std::string childName(childNameView);
  auto lockedParent = parent->locked();
  auto file = lockedParent.getChild(childName);
  if (!file) {
    return -ENOENT;
  }
  // Disallow removing the root directory, even if it is empty.
  if (file == wasmFS.getRootDirectory()) {
    return -EBUSY;
  }

  auto lockedFile = file->locked();
  if (auto dir = file->dynCast<Directory>()) {
    if (flags != AT_REMOVEDIR) {
      return -EISDIR;
    }
    // A directory can only be removed if it has no entries.
    if (dir->locked().getNumEntries() > 0) {
      return -ENOTEMPTY;
    }
  } else {
    // A normal file or symlink.
    if (flags == AT_REMOVEDIR) {
      return -ENOTDIR;
    }
  }

  // Cannot unlink/rmdir if the parent dir doesn't have write permissions.
  if (!(lockedParent.getMode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // Input is valid, perform the unlink.
  if (!lockedParent.removeChild(childName)) {
    return -EPERM;
  }
  return 0;
}

int __syscall_rmdir(long path) {
  return __syscall_unlinkat(AT_FDCWD, path, AT_REMOVEDIR);
}

int __syscall_getdents64(long fd, long dirp, long count) {
  dirent* result = (dirent*)dirp;

  // Check if the result buffer is too small.
  if (count / sizeof(dirent) == 0) {
    return -EINVAL;
  }

  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  auto lockedOpenFile = openFile->locked();

  auto dir = lockedOpenFile.getFile()->dynCast<Directory>();
  if (!dir) {
    return -ENOTDIR;
  }
  auto lockedDir = dir->locked();

  // A directory's position corresponds to the index in its entries vector.
  int index = lockedOpenFile.getPosition();

  // If this directory has been unlinked and has no parent, then it is
  // completely empty.
  auto parent = lockedDir.getParent();
  if (!parent) {
    return 0;
  }

  std::vector<Directory::Entry> entries = {
    {".", File::DirectoryKind, dir->getIno()},
    {"..", File::DirectoryKind, parent->getIno()}};
  auto dirEntries = lockedDir.getEntries();
  entries.insert(entries.end(), dirEntries.begin(), dirEntries.end());

  off_t bytesRead = 0;
  for (; index < entries.size() && bytesRead + sizeof(dirent) <= count;
       index++) {
    auto& entry = entries[index];
    result->d_ino = entry.ino;
    result->d_off = index + 1;
    result->d_reclen = sizeof(dirent);
    switch (entry.kind) {
      case File::UnknownKind:
        result->d_type = DT_UNKNOWN;
        break;
      case File::DataFileKind:
        result->d_type = DT_REG;
        break;
      case File::DirectoryKind:
        result->d_type = DT_DIR;
        break;
      case File::SymlinkKind:
        result->d_type = DT_LNK;
        break;
      default:
        result->d_type = DT_UNKNOWN;
        break;
    }
    assert(entry.name.size() + 1 <= sizeof(result->d_name));
    strcpy(result->d_name, entry.name.c_str());
    ++result;
    bytesRead += sizeof(dirent);
  }

  // Update position
  lockedOpenFile.setPosition(index);

  return bytesRead;
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_renameat(long olddirfd,
                       long oldpath,
                       long newdirfd,
                       long newpath) {
  // Rename is the only syscall that needs to (or is allowed to) acquire locks
  // on two directories at once. It requires locks on both the old and new
  // parent directories to ensure that the moved file can be atomically removed
  // from the old directory and added to the new directory without something
  // changing that would prevent the move.
  //
  // To prevent deadlock in the case of simultaneous renames, serialize renames
  // with an additional global lock.
  static std::mutex renameMutex;
  std::lock_guard<std::mutex> renameLock(renameMutex);

  // Get the old directory.
  auto parsedOld = path::parseParent((char*)oldpath, olddirfd);
  if (auto err = parsedOld.getError()) {
    return err;
  }
  auto& [oldParent, oldFileNameView] = parsedOld.getParentChild();
  std::string oldFileName(oldFileNameView);

  // Get the new directory.
  auto parsedNew = path::parseParent((char*)newpath, newdirfd);
  if (auto err = parsedNew.getError()) {
    return err;
  }
  auto& [newParent, newFileNameView] = parsedNew.getParentChild();
  std::string newFileName(newFileNameView);

  if (newFileNameView.size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  // Lock both directories.
  auto lockedOldParent = oldParent->locked();
  auto lockedNewParent = newParent->locked();

  // Get the source and destination files.
  auto oldFile = lockedOldParent.getChild(oldFileName);
  auto newFile = lockedNewParent.getChild(newFileName);

  if (!oldFile) {
    return -ENOENT;
  }

  // If the source and destination are the same, do nothing.
  if (oldFile == newFile) {
    return 0;
  }

  // Never allow renaming or overwriting the root.
  auto root = wasmFS.getRootDirectory();
  if (oldFile == root || newFile == root) {
    return -EBUSY;
  }

  // Cannot modify either directory without write permissions.
  if (!(lockedOldParent.getMode() & WASMFS_PERM_WRITE) ||
      !(lockedNewParent.getMode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // TODO: Check that the source and parent directories have the same backends.

  // Check that oldDir is not an ancestor of newDir.
  for (auto curr = newParent; curr != root; curr = curr->locked().getParent()) {
    if (curr == oldFile) {
      return -EINVAL;
    }
  }

  // The new file must be removed if it already exists.
  if (newFile) {
    if (auto newDir = newFile->dynCast<Directory>()) {
      // Cannot overwrite a directory with a non-directory.
      auto oldDir = oldFile->dynCast<Directory>();
      if (!oldDir) {
        return -EISDIR;
      }
      // Cannot overwrite a non-empty directory.
      if (newDir->locked().getNumEntries() > 0) {
        return -ENOTEMPTY;
      }
    } else {
      // Cannot overwrite a non-directory with a directory.
      if (oldFile->is<Directory>()) {
        return -ENOTDIR;
      }
    }

    // Remove the overwritten file.
    if (!lockedNewParent.removeChild(newFileName)) {
      return -EPERM;
    }
  }

  // Unlink the oldpath and add the oldpath to the new parent dir.
  if (!lockedOldParent.removeChild(oldFileName)) {
    // TODO: Put the file that was going to be overwritten back!
    return -EPERM;
  }
  if (!lockedNewParent.insertChild(newFileName, oldFile)) {
    // TODO: Put all the removed files back!
    return -EPERM;
  }

  return 0;
}

int __syscall_rename(long oldpath, long newpath) {
  return __syscall_renameat(AT_FDCWD, oldpath, AT_FDCWD, newpath);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_symlinkat(long target, long newdirfd, long linkpath) {
  auto parsed = path::parseParent((char*)linkpath, newdirfd);
  if (auto err = parsed.getError()) {
    return err;
  }
  auto& [parent, childNameView] = parsed.getParentChild();
  if (childNameView.size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }
  auto lockedParent = parent->locked();
  std::string childName(childNameView);
  if (lockedParent.getChild(childName)) {
    return -EEXIST;
  }

  auto backend = parent->getBackend();
  auto created = backend->createSymlink((char*)target);
  if (!lockedParent.insertChild(childName, created)) {
    return -EPERM;
  }

  return 0;
}

int __syscall_symlink(long target, long linkpath) {
  return __syscall_symlinkat(target, AT_FDCWD, linkpath);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_readlinkat(long dirfd, long path, long buf, long bufsize) {
  auto parsed = path::parseFile((char*)path, dirfd);
  if (auto err = parsed.getError()) {
    return err;
  }
  auto link = parsed.getFile()->dynCast<Symlink>();
  if (!link) {
    return -EINVAL;
  }
  const auto& target = link->getTarget();
  auto bytes = std::min((size_t)bufsize, target.size());
  memcpy((char*)buf, target.c_str(), bytes);
  return bytes;
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_utimensat(long dirFD, long path_, long times_, long flags) {
  const char* path = (const char*)path_;
  const struct timespec* times = (const struct timespec*)times_;
  if (flags & ~AT_SYMLINK_NOFOLLOW) {
    // TODO: Test this case.
    return -EINVAL;
  }

  // TODO: Handle AT_SYMLINK_NOFOLLOW once we traverse symlinks correctly.
  auto parsed = path::parseFile(path, dirFD);
  if (auto err = parsed.getError()) {
    return err;
  }

  // TODO: Set tv_nsec (nanoseconds) as well.
  // TODO: Handle tv_nsec being UTIME_NOW or UTIME_OMIT.
  // TODO: Check for write access to the file (see man page for specifics).
  time_t aSeconds, mSeconds;
  if (times == NULL) {
    aSeconds = time(NULL);
    mSeconds = aSeconds;
  } else {
    aSeconds = times[0].tv_sec;
    mSeconds = times[1].tv_sec;
  }

  auto locked = parsed.getFile()->locked();
  locked.setATime(aSeconds);
  locked.setMTime(mSeconds);

  return 0;
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_fchmodat(long dirfd, long path, long mode, ...) {
  int flags = 0;
  va_list v1;
  va_start(v1, mode);
  flags = va_arg(v1, int);
  va_end(v1);

  if (flags & ~AT_SYMLINK_NOFOLLOW) {
    // TODO: Test this case.
    return -EINVAL;
  }
  // TODO: Handle AT_SYMLINK_NOFOLLOW once we traverse symlinks correctly.
  auto parsed = path::parseFile((char*)path, dirfd);
  if (auto err = parsed.getError()) {
    return err;
  }
  parsed.getFile()->locked().setMode(mode);
  return 0;
}

int __syscall_chmod(long path, long mode) {
  return __syscall_fchmodat(AT_FDCWD, path, mode, 0);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_faccessat(long dirfd, long path, long amode, long flags) {
  // The input must be F_OK (check for existence) or a combination of [RWX]_OK
  // flags.
  if (amode != F_OK && (amode & ~(R_OK | W_OK | X_OK))) {
    return -EINVAL;
  }
  if (flags & ~(AT_EACCESS | AT_SYMLINK_NOFOLLOW)) {
    // TODO: Test this case.
    return -EINVAL;
  }

  // TODO: Handle AT_SYMLINK_NOFOLLOW once we traverse symlinks correctly.
  auto parsed = path::parseFile((char*)path, dirfd);
  if (auto err = parsed.getError()) {
    return err;
  }

  if (amode != F_OK) {
    auto mode = parsed.getFile()->locked().getMode();
    if ((amode & R_OK) && !(mode & WASMFS_PERM_READ)) {
      return -EACCES;
    }
    if ((amode & W_OK) && !(mode & WASMFS_PERM_WRITE)) {
      return -EACCES;
    }
    if ((amode & X_OK) && !(mode & WASMFS_PERM_EXECUTE)) {
      return -EACCES;
    }
  }

  return 0;
}

static long doTruncate(std::shared_ptr<File>& file, off_t size) {
  auto dataFile = file->dynCast<DataFile>();
  // TODO: support for symlinks.
  if (!dataFile) {
    return __WASI_ERRNO_ISDIR;
  }

  auto locked = dataFile->locked();
  if (!(locked.getMode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  if (size < 0) {
    return -EINVAL;
  }

  // TODO: error handling for allocation errors. atm with exceptions disabled,
  //       however, C++ backends using std::vector for storage have no way to
  //       report that, and will abort in malloc.
  locked.setSize(size);
  return 0;
}

int __syscall_truncate64(long path, uint64_t size) {
  auto parsed = path::parseFile((char*)path);
  if (auto err = parsed.getError()) {
    return err;
  }
  return doTruncate(parsed.getFile(), size);
}

int __syscall_ftruncate64(long fd, uint64_t size) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  auto ret = doTruncate(openFile->locked().getFile(), size);
  // XXX It is not clear from the docs why ftruncate would differ from
  //     truncate here. However, on Linux this definitely happens, and the old
  //     FS matches that as well, so do the same here.
  if (ret == -EACCES) {
    ret = -EINVAL;
  }
  return ret;
}

int __syscall_pipe(long fd) {
  auto* fds = (__wasi_fd_t*)fd;

  // Make a pipe: Two PipeFiles that share a single data source between them, so
  // that writing to one can be read in the other.
  //
  // No backend is needed here, so pass in nullptr for that.
  auto data = std::make_shared<PipeData>();
  auto reader = std::make_shared<PipeFile>(S_IRUGO, data);
  auto writer = std::make_shared<PipeFile>(S_IWUGO, data);

  auto fileTable = wasmFS.getFileTable().locked();
  fds[0] =
    fileTable.addEntry(std::make_shared<OpenFileState>(0, O_RDONLY, reader));
  fds[1] =
    fileTable.addEntry(std::make_shared<OpenFileState>(0, O_WRONLY, writer));

  return 0;
}

// int poll(struct pollfd* fds, nfds_t nfds, int timeout);
int __syscall_poll(long fds_, long nfds, long timeout) {
  struct pollfd* fds = (struct pollfd*)fds_;
  auto fileTable = wasmFS.getFileTable().locked();

  // Process the list of FDs and compute their revents masks. Count the number
  // of nonzero such masks, which is our return value.
  long nonzero = 0;
  for (nfds_t i = 0; i < nfds; i++) {
    auto* pollfd = &fds[i];
    auto fd = pollfd->fd;
    if (fd < 0) {
      // Negative FDs are ignored in poll().
      pollfd->revents = 0;
      continue;
    }
    // Assume invalid, unless there is an open file.
    auto mask = POLLNVAL;
    auto openFile = fileTable.getEntry(fd);
    if (openFile) {
      mask = 0;
      auto flags = openFile->getFlags();
      auto readBit = pollfd->events & POLLOUT;
      if (readBit && (flags == O_WRONLY || flags == O_RDWR)) {
        mask |= readBit;
      }
      auto writeBit = pollfd->events & POLLIN;
      if (writeBit && (flags == O_RDONLY || flags == O_RDWR)) {
        // If there is data in the file, then there is also the ability to read.
        // TODO: Does this need to consider the position as well? That is, if
        //       the position is at the end, we can't read from the current
        //       position at least.
        if (openFile->locked().getFile()->locked().getSize() > 0) {
          mask |= writeBit;
        }
      }
      // TODO: get mask from File dynamically using a poll() hook?
    }
    // TODO: set the state based on the state of the other end of the pipe, for
    //       pipes (POLLERR | POLLHUP)
    if (mask) {
      nonzero++;
    }
    pollfd->revents = mask;
  }
  // TODO: This should block based on the timeout. The old FS did not do so due
  //       to web limitations, which we should perhaps revisit (especially with
  //       pthreads and asyncify).
  return nonzero;
}

} // extern "C"
