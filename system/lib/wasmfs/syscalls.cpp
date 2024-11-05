// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// syscalls.cpp will implement the syscalls of the new file system replacing the
// old JS version. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#define _LARGEFILE64_SOURCE // For F_GETLK64 etc

#include <dirent.h>
#include <emscripten/emscripten.h>
#include <emscripten/heap.h>
#include <emscripten/html5.h>
#include <errno.h>
#include <mutex>
#include <poll.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <syscall_arch.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <wasi/api.h>

#include "backend.h"
#include "file.h"
#include "file_table.h"
#include "paths.h"
#include "pipe_backend.h"
#include "special_files.h"
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

int __syscall_dup3(int oldfd, int newfd, int flags) {
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
  (void)fileTable.setEntry(newfd, oldOpenFile);
  return newfd;
}

int __syscall_dup(int fd) {
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
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  if (iovs_len < 0 || offset < 0) {
    return __WASI_ERRNO_INVAL;
  }

  auto lockedOpenFile = openFile->locked();
  auto file = lockedOpenFile.getFile()->dynCast<DataFile>();
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  if (setOffset == OffsetHandling::OpenFileState) {
    if (lockedOpenFile.getFlags() & O_APPEND) {
      off_t size = lockedFile.getSize();
      if (size < 0) {
        // Translate to WASI standard of positive return codes.
        return -size;
      }
      offset = size;
      lockedOpenFile.setPosition(offset);
    } else {
      offset = lockedOpenFile.getPosition();
    }
  }

  // TODO: Check open file access mode for write permissions.

  size_t bytesWritten = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    const uint8_t* buf = iovs[i].buf;
    off_t len = iovs[i].buf_len;

    // Check if buf_len specifies a positive length buffer but buf is a
    // null pointer
    if (!buf && len > 0) {
      return __WASI_ERRNO_INVAL;
    }

    // Check if the sum of the buf_len values overflows an off_t (63 bits).
    if (addWillOverFlow(offset, (__wasi_filesize_t)bytesWritten)) {
      return __WASI_ERRNO_FBIG;
    }

    auto result = lockedFile.write(buf, len, offset + bytesWritten);
    if (result < 0) {
      // This individual write failed. Report the error unless we've already
      // written some bytes, in which case report a successful short write.
      if (bytesWritten > 0) {
        break;
      }
      return -result;
    }
    // The write was successful.
    bytesWritten += result;
    if (result < len) {
      // The write was short, so stop here.
      break;
    }
  }
  *nwritten = bytesWritten;
  if (setOffset == OffsetHandling::OpenFileState &&
      lockedOpenFile.getFile()->isSeekable()) {
    lockedOpenFile.setPosition(offset + bytesWritten);
  }
  if (bytesWritten) {
    lockedFile.updateMTime();
  }
  return __WASI_ERRNO_SUCCESS;
}

// Internal read function called by __wasi_fd_read and __wasi_fd_pread
// Receives an open file state offset.
// Optionally sets open file state offset.
// TODO: combine this with writeAtOffset because the code is nearly identical.
static __wasi_errno_t readAtOffset(OffsetHandling setOffset,
                                   __wasi_fd_t fd,
                                   const __wasi_iovec_t* iovs,
                                   size_t iovs_len,
                                   __wasi_size_t* nread,
                                   __wasi_filesize_t offset = 0) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  auto lockedOpenFile = openFile->locked();

  if (setOffset == OffsetHandling::OpenFileState) {
    offset = lockedOpenFile.getPosition();
  }

  if (iovs_len < 0 || offset < 0) {
    return __WASI_ERRNO_INVAL;
  }

  // TODO: Check open file access mode for read permissions.

  auto file = lockedOpenFile.getFile()->dynCast<DataFile>();

  // If file is nullptr, then the file was not a DataFile.
  if (!file) {
    return __WASI_ERRNO_ISDIR;
  }

  auto lockedFile = file->locked();

  size_t bytesRead = 0;
  for (size_t i = 0; i < iovs_len; i++) {
    uint8_t* buf = iovs[i].buf;
    size_t len = iovs[i].buf_len;

    if (!buf && len > 0) {
      return __WASI_ERRNO_INVAL;
    }

    // TODO: Check for overflow when adding offset + bytesRead.
    auto result = lockedFile.read(buf, len, offset + bytesRead);
    if (result < 0) {
      // This individual read failed. Report the error unless we've already read
      // some bytes, in which case report a successful short read.
      if (bytesRead > 0) {
        break;
      }
      return -result;
    }

    // The read was successful.

    // Backends must only return len or less.
    assert(result <= len);

    bytesRead += result;
    if (result < len) {
      // The read was short, so stop here.
      break;
    }
  }
  *nread = bytesRead;
  if (setOffset == OffsetHandling::OpenFileState &&
      lockedOpenFile.getFile()->isSeekable()) {
    lockedOpenFile.setPosition(offset + bytesRead);
  }
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
  std::shared_ptr<DataFile> closee;
  {
    // Do not hold the file table lock while performing the close.
    auto fileTable = wasmFS.getFileTable().locked();
    auto entry = fileTable.getEntry(fd);
    if (!entry) {
      return __WASI_ERRNO_BADF;
    }
    closee = fileTable.setEntry(fd, nullptr);
  }
  if (closee) {
    // Translate to WASI standard of positive return codes.
    int ret = -closee->locked().close();
    assert(ret >= 0);
    return ret;
  }
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
    auto ret = dataFile->locked().flush();
    assert(ret <= 0);
    // Translate to WASI standard of positive return codes.
    return -ret;
  }

  return __WASI_ERRNO_SUCCESS;
}

int __syscall_fdatasync(int fd) {
  // TODO: Optimize this to avoid unnecessarily flushing unnecessary metadata.
  return __wasi_fd_sync(fd);
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

static timespec ms_to_timespec(double ms) {
  long long seconds = ms / 1000;
  timespec ts;
  ts.tv_sec = seconds; // seconds
  ts.tv_nsec = (ms - (seconds * 1000)) * 1000 * 1000; // nanoseconds
  return ts;
}

int __syscall_newfstatat(int dirfd, intptr_t path, intptr_t buf, int flags) {
  // Only accept valid flags.
  if (flags & ~(AT_EMPTY_PATH | AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW)) {
    // TODO: Test this case.
    return -EINVAL;
  }
  auto parsed = path::getFileAt(dirfd, (char*)path, flags);
  if (auto err = parsed.getError()) {
    return err;
  }
  auto file = parsed.getFile();

  // Extract the information from the file.
  auto lockedFile = file->locked();
  auto buffer = (struct stat*)buf;

  off_t size = lockedFile.getSize();
  if (size < 0) {
    return size;
  }
  buffer->st_size = size;

  // ATTN: hard-coded constant values are copied from the existing JS file
  // system. Specific values were chosen to match existing library_fs.js
  // values.
  // ID of device containing file: Hardcode 1 for now, no meaning at the
  // moment for Emscripten.
  buffer->st_dev = 1;
  buffer->st_mode = lockedFile.getMode();
  buffer->st_ino = file->getIno();
  // The number of hard links is 1 since they are unsupported.
  buffer->st_nlink = 1;
  buffer->st_uid = 0;
  buffer->st_gid = 0;
  // Device ID (if special file) No meaning right now for Emscripten.
  buffer->st_rdev = 0;
  // The syscall docs state this is hardcoded to # of 512 byte blocks.
  buffer->st_blocks = (buffer->st_size + 511) / 512;
  // Specifies the preferred blocksize for efficient disk I/O.
  buffer->st_blksize = 4096;
  buffer->st_atim = ms_to_timespec(lockedFile.getATime());
  buffer->st_mtim = ms_to_timespec(lockedFile.getMTime());
  buffer->st_ctim = ms_to_timespec(lockedFile.getCTime());
  return __WASI_ERRNO_SUCCESS;
}

int __syscall_stat64(intptr_t path, intptr_t buf) {
  return __syscall_newfstatat(AT_FDCWD, path, buf, 0);
}

int __syscall_lstat64(intptr_t path, intptr_t buf) {
  return __syscall_newfstatat(AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
}

int __syscall_fstat64(int fd, intptr_t buf) {
  return __syscall_newfstatat(fd, (intptr_t) "", buf, AT_EMPTY_PATH);
}

// When calling doOpen(), we may request an FD be returned, or we may not need
// that return value (in which case no FD need be allocated, and we return 0 on
// success).
enum class OpenReturnMode { FD, Nothing };

static __wasi_fd_t doOpen(path::ParsedParent parsed,
                          int flags,
                          mode_t mode,
                          backend_t backend = NullBackend,
                          OpenReturnMode returnMode = OpenReturnMode::FD) {
  int accessMode = (flags & O_ACCMODE);
  if (accessMode != O_WRONLY && accessMode != O_RDONLY &&
      accessMode != O_RDWR) {
    return -EINVAL;
  }

  // TODO: remove assert when all functionality is complete.
  assert((flags & ~(O_CREAT | O_EXCL | O_DIRECTORY | O_TRUNC | O_APPEND |
                    O_RDWR | O_WRONLY | O_RDONLY | O_LARGEFILE | O_NOFOLLOW |
                    O_CLOEXEC | O_NONBLOCK)) == 0);

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
      std::shared_ptr<File> created;
      if (backend == parent->getBackend()) {
        created = lockedParent.insertDataFile(std::string(childName), mode);
        if (!created) {
          // TODO Receive a specific error code, and report it here. For now,
          //      report a generic error.
          return -EIO;
        }
      } else {
        created = backend->createFile(mode);
        if (!created) {
          // TODO Receive a specific error code, and report it here. For now,
          //      report a generic error.
          return -EIO;
        }
        [[maybe_unused]] bool mounted =
          lockedParent.mountChild(std::string(childName), created);
        assert(mounted);
      }
      // TODO: Check that the insert actually succeeds.
      if (returnMode == OpenReturnMode::Nothing) {
        return 0;
      }

      std::shared_ptr<OpenFileState> openFile;
      if (auto err = OpenFileState::create(created, flags, openFile)) {
        assert(err < 0);
        return err;
      }
      return wasmFS.getFileTable().locked().addEntry(openFile);
    }
  }

  if (auto link = child->dynCast<Symlink>()) {
    if (flags & O_NOFOLLOW) {
      return -ELOOP;
    }
    // TODO: The link dereference count starts back at 0 here. We could
    // propagate it from the previous path parsing instead.
    auto target = link->getTarget();
    auto parsedLink = path::getFileFrom(parent, target);
    if (auto err = parsedLink.getError()) {
      return err;
    }
    child = parsedLink.getFile();
  }
  assert(!child->is<Symlink>());

  // Return an error if the file exists and O_CREAT and O_EXCL are specified.
  if ((flags & O_EXCL) && (flags & O_CREAT)) {
    return -EEXIST;
  }

  if (child->is<Directory>() && accessMode != O_RDONLY) {
    return -EISDIR;
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

  // Note that we open the file before truncating it because some backends may
  // truncate opened files more efficiently (e.g. OPFS).
  std::shared_ptr<OpenFileState> openFile;
  if (auto err = OpenFileState::create(child, flags, openFile)) {
    assert(err < 0);
    return err;
  }

  // If O_TRUNC, truncate the file if possible.
  if (flags & O_TRUNC) {
    if (!child->is<DataFile>()) {
      return -EISDIR;
    }
    if ((fileMode & WASMFS_PERM_WRITE) == 0) {
      return -EACCES;
    }
    // Try to truncate the file, continuing silently if we cannot.
    (void)child->cast<DataFile>()->locked().setSize(0);
  }

  return wasmFS.getFileTable().locked().addEntry(openFile);
}

// This function is exposed to users and allows users to create a file in a
// specific backend. An fd to an open file is returned.
int wasmfs_create_file(char* pathname, mode_t mode, backend_t backend) {
  static_assert(std::is_same_v<decltype(doOpen(0, 0, 0, 0)), unsigned int>,
                "unexpected conversion from result of doOpen to int");
  return doOpen(
    path::parseParent((char*)pathname), O_CREAT | O_EXCL, mode, backend);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_openat(int dirfd, intptr_t path, int flags, ...) {
  mode_t mode = 0;
  va_list v1;
  va_start(v1, flags);
  mode = va_arg(v1, int);
  va_end(v1);

  return doOpen(path::parseParent((char*)path, dirfd), flags, mode);
}

int __syscall_mknodat(int dirfd, intptr_t path, int mode, int dev) {
  assert(dev == 0); // TODO: support special devices
  if (mode & S_IFDIR) {
    return -EINVAL;
  }
  if (mode & S_IFIFO) {
    return -EPERM;
  }
  return doOpen(path::parseParent((char*)path, dirfd),
                O_CREAT | O_EXCL,
                mode,
                NullBackend,
                OpenReturnMode::Nothing);
}

static int
doMkdir(path::ParsedParent parsed, int mode, backend_t backend = NullBackend) {
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

  if (!(lockedParent.getMode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // By default, the backend that the directory is created in is the same as
  // the parent directory. However, if a backend is passed as a parameter,
  // then that backend is used.
  if (!backend) {
    backend = parent->getBackend();
  }

  if (backend == parent->getBackend()) {
    if (!lockedParent.insertDirectory(childName, mode)) {
      // TODO Receive a specific error code, and report it here. For now, report
      //      a generic error.
      return -EIO;
    }
  } else {
    auto created = backend->createDirectory(mode);
    if (!created) {
      // TODO Receive a specific error code, and report it here. For now, report
      //      a generic error.
      return -EIO;
    }
    [[maybe_unused]] bool mounted = lockedParent.mountChild(childName, created);
    assert(mounted);
  }

  // TODO: Check that the insertion is successful.

  return 0;
}

// This function is exposed to users and allows users to specify a particular
// backend that a directory should be created within.
int wasmfs_create_directory(char* path, int mode, backend_t backend) {
  static_assert(std::is_same_v<decltype(doMkdir(0, 0, 0)), int>,
                "unexpected conversion from result of doMkdir to int");
  return doMkdir(path::parseParent(path), mode, backend);
}

// TODO: Test this.
int __syscall_mkdirat(int dirfd, intptr_t path, int mode) {
  return doMkdir(path::parseParent((char*)path, dirfd), mode);
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
    off_t size = lockedOpenFile.getFile()->locked().getSize();
    if (size < 0) {
      // Translate to WASI standard of positive return codes.
      return -size;
    }
    position = size + offset;
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

static int doChdir(std::shared_ptr<File>& file) {
  auto dir = file->dynCast<Directory>();
  if (!dir) {
    return -ENOTDIR;
  }
  wasmFS.setCWD(dir);
  return 0;
}

int __syscall_chdir(intptr_t path) {
  auto parsed = path::parseFile((char*)path);
  if (auto err = parsed.getError()) {
    return err;
  }
  return doChdir(parsed.getFile());
}

int __syscall_fchdir(int fd) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  return doChdir(openFile->locked().getFile());
}

int __syscall_getcwd(intptr_t buf, size_t size) {
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
    return -ERANGE;
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
int __syscall_unlinkat(int dirfd, intptr_t path, int flags) {
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
  return lockedParent.removeChild(childName);
}

int __syscall_rmdir(intptr_t path) {
  return __syscall_unlinkat(AT_FDCWD, path, AT_REMOVEDIR);
}

// wasmfs_unmount is similar to __syscall_unlinkat, but assumes AT_REMOVEDIR is
// true and will only unlink mountpoints (Empty and nonempty).
int wasmfs_unmount(intptr_t path) {
  auto parsed = path::parseParent((char*)path, AT_FDCWD);
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

  if (auto dir = file->dynCast<Directory>()) {
    if (parent->getBackend() == dir->getBackend()) {
      // The child is not a valid mountpoint.
      return -EINVAL;
    }
  } else {
    // A normal file or symlink.
    return -ENOTDIR;
  }

  // Input is valid, perform the unlink.
  return lockedParent.removeChild(childName);
}

int __syscall_getdents64(int fd, intptr_t dirp, size_t count) {
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

  off_t bytesRead = 0;
  const auto& dirents = openFile->dirents;
  for (; index < dirents.size() && bytesRead + sizeof(dirent) <= count;
       index++) {
    const auto& entry = dirents[index];
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
int __syscall_renameat(int olddirfd,
                       intptr_t oldpath,
                       int newdirfd,
                       intptr_t newpath) {
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

  // Both parents must have the same backend.
  if (oldParent->getBackend() != newParent->getBackend()) {
    return -EXDEV;
  }

  // Check that oldDir is not an ancestor of newDir.
  for (auto curr = newParent; curr != root; curr = curr->locked().getParent()) {
    if (curr == oldFile) {
      return -EINVAL;
    }
  }

  // The new file will be removed if it already exists.
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
  }

  // Perform the move.
  if (auto err = lockedNewParent.insertMove(newFileName, oldFile)) {
    assert(err < 0);
    return err;
  }
  return 0;
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_symlinkat(intptr_t target, int newdirfd, intptr_t linkpath) {
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
  if (!lockedParent.insertSymlink(childName, (char*)target)) {
    return -EPERM;
  }
  return 0;
}

int __syscall_symlink(intptr_t target, intptr_t linkpath) {
  return __syscall_symlinkat(target, AT_FDCWD, linkpath);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_readlinkat(int dirfd,
                         intptr_t path,
                         intptr_t buf,
                         size_t bufsize) {
  // TODO: Handle empty paths.
  auto parsed = path::parseFile((char*)path, dirfd, path::NoFollowLinks);
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

static double timespec_to_ms(timespec ts) {
  if (ts.tv_nsec == UTIME_OMIT) {
    return INFINITY;
  }
  if (ts.tv_nsec == UTIME_NOW) {
    return emscripten_date_now();
  }
  return double(ts.tv_sec) * 1000 + double(ts.tv_nsec) / (1000 * 1000);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_utimensat(int dirFD, intptr_t path_, intptr_t times_, int flags) {
  const char* path = (const char*)path_;
  const struct timespec* times = (const struct timespec*)times_;
  if (flags & ~AT_SYMLINK_NOFOLLOW) {
    // TODO: Test this case.
    return -EINVAL;
  }

  // Add AT_EMPTY_PATH as Linux (and so, musl, and us) has a nonstandard
  // behavior in which an empty path means to operate on whatever is in dirFD
  // (directory or not), which is exactly the behavior of AT_EMPTY_PATH (but
  // without passing that in). See "C library/kernel ABI differences" in
  // https://man7.org/linux/man-pages/man2/utimensat.2.html
  //
  // TODO: Handle AT_SYMLINK_NOFOLLOW once we traverse symlinks correctly.
  auto parsed = path::getFileAt(dirFD, path, flags | AT_EMPTY_PATH);
  if (auto err = parsed.getError()) {
    return err;
  }

  // TODO: Handle tv_nsec being UTIME_NOW or UTIME_OMIT.
  // TODO: Check for write access to the file (see man page for specifics).
  double aTime, mTime;

  if (times == nullptr) {
    aTime = mTime = emscripten_date_now();
  } else {
    aTime = timespec_to_ms(times[0]);
    mTime = timespec_to_ms(times[1]);
  }

  auto locked = parsed.getFile()->locked();
  if (aTime != INFINITY) {
    locked.setATime(aTime);
  }
  if (mTime != INFINITY) {
    locked.setMTime(mTime);
  }

  return 0;
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_fchmodat2(int dirfd, intptr_t path, int mode, int flags) {
  if (flags & ~AT_SYMLINK_NOFOLLOW) {
    // TODO: Test this case.
    return -EINVAL;
  }
  auto parsed = path::getFileAt(dirfd, (char*)path, flags);
  if (auto err = parsed.getError()) {
    return err;
  }
  auto lockedFile = parsed.getFile()->locked();
  lockedFile.setMode(mode);
  // On POSIX, ctime is updated on metadata changes, like chmod.
  lockedFile.updateCTime();
  return 0;
}

int __syscall_chmod(intptr_t path, int mode) {
  return __syscall_fchmodat2(AT_FDCWD, path, mode, 0);
}

int __syscall_fchmod(int fd, int mode) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  auto lockedFile = openFile->locked().getFile()->locked();
  lockedFile.setMode(mode);
  lockedFile.updateCTime();
  return 0;
}

int __syscall_fchownat(
  int dirfd, intptr_t path, int owner, int group, int flags) {
  // Only accept valid flags.
  if (flags & ~(AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW)) {
    // TODO: Test this case.
    return -EINVAL;
  }
  auto parsed = path::getFileAt(dirfd, (char*)path, flags);
  if (auto err = parsed.getError()) {
    return err;
  }

  // Ignore the actual owner and group because we don't track those.
  // TODO: Update metadata time stamp.
  return 0;
}

int __syscall_fchown32(int fd, int owner, int group) {
  return __syscall_fchownat(fd, (intptr_t) "", owner, group, AT_EMPTY_PATH);
}

// TODO: Test this with non-AT_FDCWD values.
int __syscall_faccessat(int dirfd, intptr_t path, int amode, int flags) {
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

static int doTruncate(std::shared_ptr<File>& file, off_t size) {
  auto dataFile = file->dynCast<DataFile>();

  if (!dataFile) {
    return -EISDIR;
  }

  auto locked = dataFile->locked();
  if (!(locked.getMode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  if (size < 0) {
    return -EINVAL;
  }

  int ret = locked.setSize(size);
  assert(ret <= 0);
  return ret;
}

int __syscall_truncate64(intptr_t path, off_t size) {
  auto parsed = path::parseFile((char*)path);
  if (auto err = parsed.getError()) {
    return err;
  }
  return doTruncate(parsed.getFile(), size);
}

int __syscall_ftruncate64(int fd, off_t size) {
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

static bool isTTY(std::shared_ptr<File>& file) {
  // TODO: Full TTY support. For now, just see stdin/out/err as terminals and
  //       nothing else.
  return file == SpecialFiles::getStdin() ||
         file == SpecialFiles::getStdout() || file == SpecialFiles::getStderr();
}

int __syscall_ioctl(int fd, int request, ...) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  if (!isTTY(openFile->locked().getFile())) {
    return -ENOTTY;
  }
  // TODO: Full TTY support. For now this is limited, and matches the old FS.
  switch (request) {
    case TCGETA:
    case TCGETS:
    case TCSETA:
    case TCSETAW:
    case TCSETAF:
    case TCSETS:
    case TCSETSW:
    case TCSETSF:
    case TIOCGWINSZ:
    case TIOCSWINSZ: {
      // TTY operations that we do nothing for anyhow can just be ignored.
      return 0;
    }
    default: {
      return -EINVAL; // not supported
    }
  }
}

int __syscall_pipe(intptr_t fd) {
  auto* fds = (__wasi_fd_t*)fd;

  // Make a pipe: Two PipeFiles that share a single data source between them, so
  // that writing to one can be read in the other.
  //
  // No backend is needed here, so pass in nullptr for that.
  auto data = std::make_shared<PipeData>();
  auto reader = std::make_shared<PipeFile>(S_IRUGO, data);
  auto writer = std::make_shared<PipeFile>(S_IWUGO, data);

  std::shared_ptr<OpenFileState> openReader, openWriter;
  (void)OpenFileState::create(reader, O_RDONLY, openReader);
  (void)OpenFileState::create(writer, O_WRONLY, openWriter);

  auto fileTable = wasmFS.getFileTable().locked();
  fds[0] = fileTable.addEntry(openReader);
  fds[1] = fileTable.addEntry(openWriter);

  return 0;
}

// int poll(struct pollfd* fds, nfds_t nfds, int timeout);
int __syscall_poll(intptr_t fds_, int nfds, int timeout) {
  struct pollfd* fds = (struct pollfd*)fds_;
  auto fileTable = wasmFS.getFileTable().locked();

  // Process the list of FDs and compute their revents masks. Count the number
  // of nonzero such masks, which is our return value.
  int nonzero = 0;
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
      auto flags = openFile->locked().getFlags();
      auto accessMode = flags & O_ACCMODE;
      auto readBit = pollfd->events & POLLOUT;
      if (readBit && (accessMode == O_WRONLY || accessMode == O_RDWR)) {
        mask |= readBit;
      }
      auto writeBit = pollfd->events & POLLIN;
      if (writeBit && (accessMode == O_RDONLY || accessMode == O_RDWR)) {
        // If there is data in the file, then there is also the ability to read.
        // TODO: Does this need to consider the position as well? That is, if
        // the position is at the end, we can't read from the current position
        // at least. If we update this, make sure the size isn't an error!
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

int __syscall_fallocate(int fd, int mode, off_t offset, off_t len) {
  assert(mode == 0); // TODO, but other modes were never supported in the old FS

  auto fileTable = wasmFS.getFileTable().locked();
  auto openFile = fileTable.getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }

  auto dataFile = openFile->locked().getFile()->dynCast<DataFile>();
  // TODO: support for symlinks.
  if (!dataFile) {
    return -ENODEV;
  }

  auto locked = dataFile->locked();
  if (!(locked.getMode() & WASMFS_PERM_WRITE)) {
    return -EBADF;
  }

  if (offset < 0 || len <= 0) {
    return -EINVAL;
  }

  // TODO: We could only fill zeros for regions that were completely unused
  //       before, which for a backend with sparse data storage could make a
  //       difference. For that we'd need a new backend API.
  auto newNeededSize = offset + len;
  off_t size = locked.getSize();
  if (size < 0) {
    return size;
  }
  if (newNeededSize > size) {
    if (auto err = locked.setSize(newNeededSize)) {
      assert(err < 0);
      return err;
    }
  }

  return 0;
}

int __syscall_fcntl64(int fd, int cmd, ...) {
  auto fileTable = wasmFS.getFileTable().locked();
  auto openFile = fileTable.getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }

  switch (cmd) {
    case F_DUPFD: {
      int newfd;
      va_list v1;
      va_start(v1, cmd);
      newfd = va_arg(v1, int);
      va_end(v1);
      if (newfd < 0) {
        return -EINVAL;
      }

      // Find the first available fd at arg or after.
      // TODO: Should we check for a limit on the max FD number, if we have one?
      while (1) {
        if (!fileTable.getEntry(newfd)) {
          (void)fileTable.setEntry(newfd, openFile);
          return newfd;
        }
        newfd++;
      }
    }
    case F_GETFD:
    case F_SETFD:
      // FD_CLOEXEC makes no sense for a single process.
      return 0;
    case F_GETFL:
      return openFile->locked().getFlags();
    case F_SETFL: {
      int flags;
      va_list v1;
      va_start(v1, cmd);
      flags = va_arg(v1, int);
      va_end(v1);
      // This syscall should ignore most flags.
      flags = flags & ~(O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_EXCL |
                        O_NOCTTY | O_TRUNC);
      // Also ignore this flag which musl always adds constantly, but does not
      // matter for us.
      flags = flags & ~O_LARGEFILE;
      // On linux only a few flags can be modified, and we support only a subset
      // of those. Error on anything else.
      auto supportedFlags = flags & O_APPEND;
      if (flags != supportedFlags) {
        return -EINVAL;
      }
      openFile->locked().setFlags(flags);
      return 0;
    }
    case F_GETLK: {
      // If these constants differ then we'd need a case for both.
      static_assert(F_GETLK == F_GETLK64);
      flock* data;
      va_list v1;
      va_start(v1, cmd);
      data = va_arg(v1, flock*);
      va_end(v1);
      // We're always unlocked for now, until we implement byte-range locks.
      data->l_type = F_UNLCK;
      return 0;
    }
    case F_SETLK:
    case F_SETLKW: {
      static_assert(F_SETLK == F_SETLK64);
      static_assert(F_SETLKW == F_SETLKW64);
      // Always error for now, until we implement byte-range locks.
      return -EACCES;
    }
    default: {
      // TODO: support any remaining cmds
      return -EINVAL;
    }
  }
}

static int
doStatFS(std::shared_ptr<File>& file, size_t size, struct statfs* buf) {
  if (size != sizeof(struct statfs)) {
    // We only know how to write to a standard statfs, not even a truncated one.
    return -EINVAL;
  }

  // NOTE: None of the constants here are true. We're just returning safe and
  //       sane values, that match the long-existing JS FS behavior (except for
  //       the inode number, where we can do better).
  buf->f_type = 0;
  buf->f_bsize = 4096;
  buf->f_frsize = 4096;
  buf->f_blocks = 1000000;
  buf->f_bfree = 500000;
  buf->f_bavail = 500000;
  buf->f_files = file->getIno();
  buf->f_ffree = 1000000;
  buf->f_fsid = {0, 0};
  buf->f_flags = ST_NOSUID;
  buf->f_namelen = 255;
  return 0;
}

int __syscall_statfs64(intptr_t path, size_t size, intptr_t buf) {
  auto parsed = path::parseFile((char*)path);
  if (auto err = parsed.getError()) {
    return err;
  }
  return doStatFS(parsed.getFile(), size, (struct statfs*)buf);
}

int __syscall_fstatfs64(int fd, size_t size, intptr_t buf) {
  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }
  return doStatFS(openFile->locked().getFile(), size, (struct statfs*)buf);
}

int _mmap_js(size_t length,
             int prot,
             int flags,
             int fd,
             off_t offset,
             int* allocated,
             void** addr) {
  // PROT_EXEC is not supported (although we pretend to support the absence of
  // PROT_READ or PROT_WRITE).
  if ((prot & PROT_EXEC)) {
    return -EPERM;
  }

  if (!length) {
    return -EINVAL;
  }

  // One of MAP_PRIVATE, MAP_SHARED, or MAP_SHARED_VALIDATE must be used.
  int mapType = flags & MAP_TYPE;
  if (mapType != MAP_PRIVATE && mapType != MAP_SHARED &&
      mapType != MAP_SHARED_VALIDATE) {
    return -EINVAL;
  }

  if (mapType == MAP_SHARED_VALIDATE) {
    WASMFS_UNREACHABLE("TODO: MAP_SHARED_VALIDATE");
  }

  auto openFile = wasmFS.getFileTable().locked().getEntry(fd);
  if (!openFile) {
    return -EBADF;
  }

  std::shared_ptr<DataFile> file;

  // Keep the open file info locked only for as long as we need that.
  {
    auto lockedOpenFile = openFile->locked();

    // Check permissions. We always need read permissions, since we need to read
    // the data in the file to map it.
    if ((lockedOpenFile.getFlags() & O_ACCMODE) == O_WRONLY) {
      return -EACCES;
    }

    // According to the POSIX spec it is possible to write to a file opened in
    // read-only mode with MAP_PRIVATE flag, as all modifications will be
    // visible only in the memory of the current process.
    if ((prot & PROT_WRITE) != 0 && mapType != MAP_PRIVATE &&
        (lockedOpenFile.getFlags() & O_ACCMODE) != O_RDWR) {
      return -EACCES;
    }

    file = lockedOpenFile.getFile()->dynCast<DataFile>();
  }

  if (!file) {
    return -ENODEV;
  }

  // TODO: On MAP_SHARED, install the mapping on the DataFile object itself so
  // that reads and writes can be redirected to the mapped region and so that
  // the mapping can correctly outlive the file being closed. This will require
  // changes to emscripten_mmap.c as well.

  // Align to a wasm page size, as we expect in the future to get wasm
  // primitives to do this work, and those would presumably be aligned to a page
  // size. Aligning now avoids confusion later.
  uint8_t* ptr = (uint8_t*)emscripten_builtin_memalign(WASM_PAGE_SIZE, length);
  if (!ptr) {
    return -ENOMEM;
  }

  auto nread = file->locked().read(ptr, length, offset);
  if (nread < 0) {
    // The read failed. Report the error, but first free the allocation.
    emscripten_builtin_free(ptr);
    return nread;
  }

  // From here on, we have succeeded, and can mark the allocation as having
  // occurred (which means that the caller has the responsibility to free it).
  *allocated = true;
  *addr = (void*)ptr;

  // The read must be of a valid amount, or we have had an internal logic error.
  assert(nread <= length);

  // mmap clears any extra bytes after the data itself.
  memset(ptr + nread, 0, length - nread);

  return 0;
}

int _msync_js(
  intptr_t addr, size_t length, int prot, int flags, int fd, off_t offset) {
  // TODO: This is not correct! Mappings should be associated with files, not
  // fds. Only need to sync if shared and writes are allowed.
  int mapType = flags & MAP_TYPE;
  if (mapType == MAP_SHARED && (prot & PROT_WRITE)) {
    __wasi_ciovec_t iovec;
    iovec.buf = (uint8_t*)addr;
    iovec.buf_len = length;
    __wasi_size_t nwritten;
    // Translate from WASI positive error codes to negative error codes.
    return -__wasi_fd_pwrite(fd, &iovec, 1, offset, &nwritten);
  }
  return 0;
}

int _munmap_js(
  intptr_t addr, size_t length, int prot, int flags, int fd, off_t offset) {
  // TODO: This is not correct! Mappings should be associated with files, not
  // fds.
  // TODO: Syncing should probably be handled in __syscall_munmap instead.
  return _msync_js(addr, length, prot, flags, fd, offset);
}

// Stubs (at least for now)

int __syscall_accept4(int sockfd,
                      intptr_t addr,
                      intptr_t addrlen,
                      int flags,
                      int dummy1,
                      int dummy2) {
  return -ENOSYS;
}

int __syscall_bind(
  int sockfd, intptr_t addr, size_t alen, int dummy, int dummy2, int dummy3) {
  return -ENOSYS;
}

int __syscall_connect(
  int sockfd, intptr_t addr, size_t len, int dummy, int dummy2, int dummy3) {
  return -ENOSYS;
}

int __syscall_socket(
  int domain, int type, int protocol, int dummy1, int dummy2, int dummy3) {
  return -ENOSYS;
}

int __syscall_listen(
  int sockfd, int backlock, int dummy1, int dummy2, int dummy3, int dummy4) {
  return -ENOSYS;
}

int __syscall_getsockopt(int sockfd,
                         int level,
                         int optname,
                         intptr_t optval,
                         intptr_t optlen,
                         int dummy) {
  return -ENOSYS;
}

int __syscall_getsockname(
  int sockfd, intptr_t addr, intptr_t len, int dummy, int dummy2, int dummy3) {
  return -ENOSYS;
}

int __syscall_getpeername(
  int sockfd, intptr_t addr, intptr_t len, int dummy, int dummy2, int dummy3) {
  return -ENOSYS;
}

int __syscall_sendto(
  int sockfd, intptr_t msg, size_t len, int flags, intptr_t addr, size_t alen) {
  return -ENOSYS;
}

int __syscall_sendmsg(
  int sockfd, intptr_t msg, int flags, intptr_t addr, size_t alen, int dummy) {
  return -ENOSYS;
}

int __syscall_recvfrom(int sockfd,
                       intptr_t msg,
                       size_t len,
                       int flags,
                       intptr_t addr,
                       intptr_t alen) {
  return -ENOSYS;
}

int __syscall_recvmsg(
  int sockfd, intptr_t msg, int flags, int dummy, int dummy2, int dummy3) {
  return -ENOSYS;
}

int __syscall_fadvise64(int fd, off_t offset, off_t length, int advice) {
  // Advice is currently ignored. TODO some backends might use it
  return 0;
}

int __syscall__newselect(int nfds,
                         intptr_t readfds_,
                         intptr_t writefds_,
                         intptr_t exceptfds_,
                         intptr_t timeout_) {
  // TODO: Implement this syscall. For now, we return an error code,
  //       specifically ENOMEM which is valid per the docs:
  //          ENOMEM Unable to allocate memory for internal tables
  //          https://man7.org/linux/man-pages/man2/select.2.html
  return -ENOMEM;
}

} // extern "C"
