// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
// syscalls.cpp will implement the syscalls of the new file system replacing the
// old JS version. Current Status: Work in Progress. See
// https://github.com/emscripten-core/emscripten/issues/15041.

#include "backend.h"
#include "file.h"
#include "file_table.h"
#include "wasmfs.h"
#include <dirent.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <errno.h>
#include <mutex>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <wasi/api.h>

// File permission macros for wasmfs.
// Used to improve readability compared to those in stat.h
#define WASMFS_PERM_WRITE 0222

// In Linux, the maximum length for a filename is 255 bytes.
#define WASMFS_NAME_MAX 255

extern "C" {

using namespace wasmfs;

long __syscall_dup3(long oldfd, long newfd, long flags) {
  auto fileTable = wasmFS.getLockedFileTable();

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
    return -EINVAL;
  }

  // If the file descriptor newfd was previously open, it will just be
  // overwritten silently.
  fileTable[newfd] = oldOpenFile.unlocked();
  return newfd;
}

long __syscall_dup(long fd) {
  auto fileTable = wasmFS.getLockedFileTable();

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

  auto openFile = wasmFS.getLockedFileTable()[fd];

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

  auto openFile = wasmFS.getLockedFileTable()[fd];

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
  auto fileTable = wasmFS.getLockedFileTable();

  // Remove openFileState entry from fileTable.
  fileTable[fd] = nullptr;

  return __WASI_ERRNO_SUCCESS;
}

backend_t wasmfs_get_backend_by_fd(int fd) {
  auto fileTable = wasmFS.getLockedFileTable();

  // Check that an open file exists corresponding to the given fd.
  auto openFile = fileTable[fd];
  if (!openFile) {
    return NullBackend;
  }

  auto lockedOpenFile = openFile.locked();
  return lockedOpenFile.getFile()->getBackend();
}

// This function is exposed to users to allow them to obtain a backend_t for a
// specified path.
backend_t wasmfs_get_backend_by_path(char* path) {
  auto pathParts = splitPath(path);

  long err;
  auto parsedPath = getParsedPath(pathParts, err);

  // Parent node doesn't exist.
  if (!parsedPath.parent) {
    return NullBackend;
  }

  return parsedPath.child ? parsedPath.child->getBackend() : NullBackend;
}

static long doStat(std::shared_ptr<File> file, struct stat* buffer) {
  auto lockedFile = file->locked();

  buffer->st_size = lockedFile.getSize();

  // ATTN: hard-coded constant values are copied from the existing JS file
  // system. Specific values were chosen to match existing library_fs.js
  // values.
  buffer->st_dev =
    1; // ID of device containing file: Hardcode 1 for now, no meaning at the
  // moment for Emscripten.
  buffer->st_mode = lockedFile.mode();
  buffer->st_ino = file->getIno();
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

long __syscall_stat64(long path, long buf) {
  auto pathParts = splitPath((char*)path);

  long err;
  auto parsedPath = getParsedPath(pathParts, err);

  // Parent node doesn't exist.
  if (!parsedPath.parent) {
    return err;
  }

  if (parsedPath.child) {
    struct stat* buffer = (struct stat*)buf;
    return doStat(parsedPath.child, buffer);
  } else {
    return -ENOENT;
  }
}

long __syscall_lstat64(long path, long buf) {
  // TODO: When symlinks are introduced, lstat will return information about the
  // link itself rather than the file it refers to.
  return __syscall_stat64(path, buf);
}

long __syscall_fstat64(long fd, long buf) {
  auto openFile = wasmFS.getLockedFileTable()[fd];

  if (!openFile) {
    return -EBADF;
  }
  struct stat* buffer = (struct stat*)buf;
  return doStat(openFile.locked().getFile(), buffer);
}

static __wasi_fd_t doOpen(char* pathname,
                          long flags,
                          mode_t mode,
                          backend_t backend = NullBackend) {
  int accessMode = (flags & O_ACCMODE);
  bool canWrite = false;

  if (accessMode == O_WRONLY || accessMode == O_RDWR) {
    canWrite = true;
  }

  // TODO: remove assert when all functionality is complete.
  assert(
    ((flags) & ~(O_CREAT | O_EXCL | O_DIRECTORY | O_TRUNC | O_APPEND | O_RDWR |
                 O_WRONLY | O_RDONLY | O_LARGEFILE | O_CLOEXEC)) == 0);

  auto pathParts = splitPath(pathname);

  long err;
  auto parsedPath = getParsedPath(pathParts, err);

  // Parent node doesn't exist.
  if (!parsedPath.parent) {
    return err;
  }

  if (pathParts.back().size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  // The requested node was not found.
  if (!parsedPath.child) {
    // If curr is the last element and the create flag is specified
    // If O_DIRECTORY is also specified, still create a regular file:
    // https://man7.org/linux/man-pages/man2/open.2.html#BUGS
    if (flags & O_CREAT) {
      // Mask all permissions sent via mode.
      mode &= S_IALLUGO;
      // Create an empty in-memory file.

      // By default, the backend that the file is created in is the same as the
      // parent directory. However, if a backend is passed as a parameter, then
      // that backend is used.
      if (!backend) {
        backend = parsedPath.parent->unlocked()->getBackend();
      }
      auto created = backend->createFile(mode);

      // TODO: When rename is implemented make sure that one can atomically
      // remove the file from the source directory and then set its parent to
      // the dest directory.
      parsedPath.parent->setEntry(pathParts.back(), created);
      auto openFile = std::make_shared<OpenFileState>(0, flags, created);

      return wasmFS.getLockedFileTable().add(openFile);
    } else {
      return -ENOENT;
    }
  }

  // Fail if O_DIRECTORY is specified and pathname is not a directory
  if (flags & O_DIRECTORY && !parsedPath.child->is<Directory>()) {
    return -ENOTDIR;
  }

  // Return an error if the file exists and O_CREAT and O_EXCL are specified.
  if (flags & O_EXCL && flags & O_CREAT) {
    return -EEXIST;
  }

  auto openFile = std::make_shared<OpenFileState>(0, flags, parsedPath.child);

  return wasmFS.getLockedFileTable().add(openFile);
}

// This function is exposed to users and allows users to create a file in a
// specific backend. An fd to an open file is returned.
__wasi_fd_t wasmfs_create_file(char* pathname, mode_t mode, backend_t backend) {
  return doOpen(pathname, O_CREAT, mode, backend);
}

__wasi_fd_t __syscall_open(long pathname, long flags, ...) {
  // Since mode is optional, mode is specified using varargs.
  mode_t mode = 0;
  va_list vl;
  va_start(vl, flags);
  mode = va_arg(vl, int);
  va_end(vl);

  return doOpen((char*)pathname, flags, mode);
}

static long doMkdir(char* path, long mode, backend_t backend = NullBackend) {
  auto pathParts = splitPath(path);

  long err;
  auto parsedPath = getParsedPath(pathParts, err);

  // Parent node doesn't exist.
  if (!parsedPath.parent) {
    return err;
  }

  if (pathParts.back().size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  // Check if the requested directory already exists.
  if (parsedPath.child) {
    return -EEXIST;
  } else {
    // Mask rwx permissions for user, group and others, and the sticky bit.
    // This prevents users from entering S_IFREG for example.
    // https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
    mode &= S_IRWXUGO | S_ISVTX;

    // By default, the backend that the directory is created in is the same as
    // the parent directory. However, if a backend is passed as a parameter,
    // then that backend is used.
    if (!backend) {
      backend = parsedPath.parent->unlocked()->getBackend();
    }
    // Create an empty in-memory directory.
    auto created = backend->createDirectory(mode);

    parsedPath.parent->setEntry(pathParts.back(), created);
    return 0;
  }
}

// This function is exposed to users and allows users to specify a particular
// backend that a directory should be created within.
long wasmfs_create_directory(char* path, long mode, backend_t backend) {
  return doMkdir(path, mode, backend);
}

long __syscall_mkdir(long path, long mode) {
  return doMkdir((char*)path, mode);
}

__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd,
                              __wasi_filedelta_t offset,
                              __wasi_whence_t whence,
                              __wasi_filesize_t* newoffset) {
  auto openFile = wasmFS.getLockedFileTable()[fd];
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
    // Only the open file state is altered in seek. Locking the underlying
    // data file here once is sufficient.
    position = lockedOpenFile.getFile()->locked().getSize() + offset;
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

long __syscall_chdir(long path) {
  auto pathParts = splitPath((char*)path);

  long err;
  auto parsedPath = getParsedPath(pathParts, err);

  if (!parsedPath.parent) {
    return err;
  }

  if (!parsedPath.child) {
    return -ENOENT;
  }

  auto childDir = parsedPath.child->dynCast<Directory>();

  if (childDir) {
    wasmFS.setCWD(childDir);
    return 0;
  } else {
    return -ENOTDIR;
  }
}

long __syscall_getcwd(long buf, long size) {
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

  // Check if the size argument is less than the length of the absolute
  // pathname of the working directory, including null terminator.
  if (strlen(res) >= size - 1) {
    return -ENAMETOOLONG;
  }

  // Return value is a null-terminated c string.
  strcpy((char*)buf, res);

  return buf;
}

__wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* stat) {
  // TODO: This is only partial implementation of __wasi_fd_fdstat_get. Enough
  // to get __wasi_fd_is_valid working.
  // There are other fields in the stat structure that we should really
  // be filling in here.
  auto openFile = wasmFS.getLockedFileTable()[fd];
  if (!openFile) {
    return __WASI_ERRNO_BADF;
  }

  if (openFile.locked().getFile()->is<Directory>()) {
    stat->fs_filetype = __WASI_FILETYPE_DIRECTORY;
  } else {
    stat->fs_filetype = __WASI_FILETYPE_REGULAR_FILE;
  }
  return __WASI_ERRNO_SUCCESS;
}

// This enum specifies whether rmdir or unlink is being performed.
enum class UnlinkMode { Rmdir, Unlink };

static long doUnlink(char* path, UnlinkMode unlinkMode) {
  auto pathParts = splitPath(path);

  // TODO: Ensure that . and .. are invalid when path parsing is updated.
  // This can be done when path parsing is refactored.

  long err;
  auto parsedPath = getParsedPath(pathParts, err);

  // Parent node doesn't exist.
  if (!parsedPath.parent) {
    return err;
  }

  if (!parsedPath.child) {
    return -ENOENT;
  }

  // Current state just matches JS file system behaviour.
  if (parsedPath.child == wasmFS.getRootDirectory()) {
    return -EBUSY;
  }

  // rmdir checks if the target is a directory and if the directory is empty.
  auto targetDir = parsedPath.child->dynCast<Directory>();
  if (unlinkMode == UnlinkMode::Rmdir) {

    if (!targetDir) {
      return -ENOTDIR;
    }

    // A directory can only be removed if it has zero entries.
    if (targetDir->locked().getNumEntries() > 0) {
      return -ENOTEMPTY;
    }
  } else {
    // unlink cannot remove a directory.
    if (targetDir) {
      return -EISDIR;
    }
  }

  // Cannot unlink/rmdir if the parent dir doesn't have write permissions.
  if (!(parsedPath.parent->mode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // Input is valid, perform the unlink.
  parsedPath.parent->unlinkEntry(pathParts.back());
  return 0;
}

long __syscall_rmdir(long path) {
  return doUnlink((char*)path, UnlinkMode::Rmdir);
}

long __syscall_unlink(long path) {
  return doUnlink((char*)path, UnlinkMode::Unlink);
}
long __syscall_getdents64(long fd, long dirp, long count) {
  auto openFile = wasmFS.getLockedFileTable()[fd];

  if (!openFile) {
    return -EBADF;
  }
  dirent* result = (dirent*)dirp;

  // Check if the result buffer is too small.
  if (count / sizeof(dirent) == 0) {
    return -EINVAL;
  }

  auto file = openFile.locked().getFile();

  auto directory = file->dynCast<Directory>();

  if (!directory) {
    return -ENOTDIR;
  }

  // Hold the locked directory to prevent the state from being changed during
  // the operation.
  auto lockedDir = directory->locked();

  off_t bytesRead = 0;
  // A directory's position corresponds to the index in its entries vector.
  int index = openFile.locked().position();

  // In the root directory, ".." refers to itself.
  auto dotdot =
    file == wasmFS.getRootDirectory() ? file : lockedDir.getParent();

  // If the directory is unlinked then the parent pointer should be null.
  if (!dotdot) {
    return -ENOENT;
  }

  // There are always two hardcoded directories "." and ".."
  std::vector<Directory::Entry> entries = {{".", file}, {"..", dotdot}};
  auto dirEntries = lockedDir.getEntries();
  entries.insert(entries.end(), dirEntries.begin(), dirEntries.end());

#ifdef WASMFS_DEBUG
  for (auto pair : entries) {
    emscripten_console_log(pair.name.c_str());
  }
#endif

  for (; index < entries.size() && bytesRead + sizeof(dirent) <= count;
       index++) {
    auto curr = entries[index];
    result->d_ino = curr.file->getIno();
    result->d_off = bytesRead + sizeof(dirent);
    result->d_reclen = sizeof(dirent);
    result->d_type =
      curr.file->is<Directory>() ? DT_DIR : DT_REG; // TODO: add symlinks.
    assert(curr.name.size() + 1 <= sizeof(result->d_name));
    strcpy(result->d_name, curr.name.c_str());
    ++result;
    bytesRead += sizeof(dirent);
  }

  // Set the directory's offset position:
  openFile.locked().position() = index;

  return bytesRead;
}

// TODO: Revisit this syscall after refactoring file system locking strategy.
long __syscall_rename(long old_path, long new_path) {
  // The rename syscall must be atomic to prevent other file system operations
  // from concurrently changing the directories. If it were not atomic, then the
  // file system could be left in an inconsistent state. For example, a
  // rename("src/a","dest/b") and an rmdir("/dest") could conflict. Assume that
  // the src/a file is unlinked first before moving it to the empty /dest
  // directory. In the meantime, another thread could remove the empty /dest
  // directory. src/a can still attach itself to the /dest directory since we
  // have its shared_ptr, and the syscall would succeed. However, this may be
  // unexpected for the user, who might have expected the subsequent
  // rmdir("/dest") to fail since /dest is no longer empty. Both syscalls
  // succeeding would result in a data loss, which would have been prevented if
  // either of them was blocked by the other. Thus rename should guarantee that
  // the order of operations is consistent.

  // Trylocking on the new_path parent is needed in the case where two renames
  // are operating on opposing sources and destinations. This would cause them
  // to lock the directories in reverse order and could cause deadlock.

  // Edge case: rename("dir", "dir/somename") - in this scenario it should not
  // be possible to rename the destination if the source is an ancestor.

  // Edge case: rename("dir/somename", "dir") - in this scenario it should not
  // be possible to rename the destination since by definition it is
  // non-empty.

  auto oldPathParts = splitPath((char*)old_path);

  // For this operation to be atomic we must lock the source parent directory.
  // getParsedPath() will return a locked parent directory.
  long err;
  auto oldParsedPath = getParsedPath(oldPathParts, err);

  if (!oldParsedPath.parent) {
    return err;
  }

  if (oldPathParts.back().size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  if (!oldParsedPath.child) {
    return -ENOENT;
  }

  // In Linux and WasmFS, renaming the root directory should return EBUSY.
  // In the JS file system it reports EINVAL.
  if (oldParsedPath.child == wasmFS.getRootDirectory()) {
    return -EBUSY;
  }

  // Obtain the destination parent directory to see if it exists.
  auto newPathParts = splitPath((char*)new_path);

  if (newPathParts.empty()) {
    return -ENOENT;
  }

  // In Linux, renaming a directory to the root directory returns ENOTEMPTY.
  // TODO: Fix this when path parsing is refactored.
  std::vector<std::string> root = {"/"};
  if (newPathParts == root) {
    return -ENOTEMPTY;
  }

  auto newBase = newPathParts.back();
  if (newBase.size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  // oldParsedPath.child is the forbidden ancestor.
  auto newParentDir = getDir(
    newPathParts.begin(), newPathParts.end() - 1, err, oldParsedPath.child);

  // If the destination parent directory doesn't exist, the source file cannot
  // be moved.
  if (!newParentDir) {
    return err;
  }

  // Edge case: a/b -> a/c share the same parent. Trylocking the same parent
  // with a recursive mutex should work, so we don't need to treat that case
  // specially here.
  // TODO: Should getParsedPath return a locked handle already or a shared
  // pointer and rely on the caller to lock the directory?
  auto maybeLockedNewParentDir = newParentDir->maybeLocked();
  if (!maybeLockedNewParentDir) {
    return -EBUSY;
  }

  auto lockedNewParentDir = std::move(*maybeLockedNewParentDir);
  auto newPath = lockedNewParentDir.getEntry(newBase);

  // If old_path and new_path are the same, do nothing.
  if (newPath == oldParsedPath.child) {
    return 0;
  }

  // Cannot move from source directory without write permissions.
  if (!(oldParsedPath.parent->mode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // Cannot move to a destination parent directory without write permissions.
  if (!(lockedNewParentDir.mode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // new path must be removed if it exists.
  if (newPath) {
    if (oldParsedPath.child->is<DataFile>()) {
      // Cannot overwrite a file with a directory.
      if (newPath->is<Directory>()) {
        return -EISDIR;
      }
    } else if (oldParsedPath.child->is<Directory>()) {
      auto newPathDirectory = newPath->dynCast<Directory>();

      // Cannot overwrite a directory with a file.
      if (!newPathDirectory) {
        return -ENOTDIR;
      }

      // This should also cover the case in where
      // the destination is an ancestor of the source:
      // rename("dir/subdir", "dir");
      if (newPathDirectory->locked().getNumEntries() > 0) {
        return -ENOTEMPTY;
      }
    } else {
      assert(false && "Unhandled file kind in rename");
    }
    lockedNewParentDir.unlinkEntry(newBase);
  }

  // Unlink the oldpath and add the oldpath to the new parent dir.
  oldParsedPath.parent->unlinkEntry(oldPathParts.back());
  lockedNewParentDir.setEntry(newBase, oldParsedPath.child);

  return 0;
}

long __syscall_symlink(char* old_path, char* new_path) {
  auto pathParts = splitPath(new_path);

  long err;
  auto parsedPath = getParsedPath(pathParts, err);

  if (!parsedPath.parent) {
    return err;
  }

  if (pathParts.back().size() > WASMFS_NAME_MAX) {
    return -ENAMETOOLONG;
  }

  if (parsedPath.child) {
    return -EEXIST;
  }

  auto backend = parsedPath.parent->unlocked()->getBackend();
  auto created = backend->createSymlink(old_path);
  parsedPath.parent->setEntry(pathParts.back(), created);

  return 0;
}

long __syscall_readlink(char* path, char* buf, size_t bufSize) {
  auto pathParts = splitPath(path);

  long err;
  auto parsedPath = getParsedPath(pathParts, err);
  if (!parsedPath.parent) {
    return err;
  }
  if (!parsedPath.child) {
    return -ENOENT;
  }
  if (!parsedPath.child->is<Symlink>()) {
    return -EINVAL;
  }

  const auto& target = parsedPath.child->dynCast<Symlink>()->getTarget();

  auto bytes = std::min(bufSize, target.size());
  memcpy(buf, target.c_str(), bytes);

  return bytes;
}
}
