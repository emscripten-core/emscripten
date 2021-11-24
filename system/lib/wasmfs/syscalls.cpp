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

extern "C" {

using namespace wasmfs;

// Copy the file specified by the pathname into JS.
// Return a pointer to the JS buffer in HEAPU8.
// The buffer will also contain the file length.
// Caller must free the returned pointer.
void* emscripten_wasmfs_read_file(char* path) {
  struct stat file;
  int err = 0;
  err = stat(path, &file);
  if (err < 0) {
    emscripten_console_error("Fatal error in FS.readFile");
    abort();
  }

  // The function will return a pointer to a buffer with the file length in the
  // first 8 bytes. The remaining bytes will contain the buffer contents. This
  // allows the caller to use HEAPU8.subarray(buf + 8, buf + 8 + length).
  off_t size = file.st_size;
  uint8_t* result = (uint8_t*)malloc((size + sizeof(size)));
  *(uint32_t*)result = size;

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    emscripten_console_error("Fatal error in FS.readFile");
    abort();
  }
  int numRead = pread(fd, result + sizeof(size), size, 0);
  // TODO: Generalize this so that it is thread-proof.
  // Must guarantee that the file size has not changed by the time it is read.
  assert(numRead == size);
  err = close(fd);
  if (err < 0) {
    emscripten_console_error("Fatal error in FS.readFile");
    abort();
  }

  return result;
}

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
  // TODO: Add mode for symlinks.
  if (file->is<Directory>()) {
    buffer->st_mode |= S_IFDIR;
  } else if (file->is<DataFile>()) {
    buffer->st_mode |= S_IFREG;
  }
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

  if (pathParts.empty()) {
    return -EINVAL;
  }

  auto base = pathParts.back();

  long err;
  auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

  // Parent node doesn't exist.
  if (!parentDir) {
    return err;
  }

  auto lockedParentDir = parentDir->locked();

  // TODO: In future PR, edit function to just return the requested file instead
  // of having to first obtain the parent dir.
  auto curr = lockedParentDir.getEntry(base);
  if (curr) {
    struct stat* buffer = (struct stat*)buf;
    return doStat(curr, buffer);
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

__wasi_fd_t __syscall_open(long pathname, long flags, ...) {
  int accessMode = (flags & O_ACCMODE);
  bool canWrite = false;

  if (accessMode == O_WRONLY || accessMode == O_RDWR) {
    canWrite = true;
  }

  // TODO: remove assert when all functionality is complete.
  assert(
    ((flags) & ~(O_CREAT | O_EXCL | O_DIRECTORY | O_TRUNC | O_APPEND | O_RDWR |
                 O_WRONLY | O_RDONLY | O_LARGEFILE | O_CLOEXEC)) == 0);

  auto pathParts = splitPath((char*)pathname);

  if (pathParts.empty()) {
    return -EINVAL;
  }

  auto base = pathParts.back();

  // Root directory
  if (pathParts.size() == 1 && pathParts[0] == "/") {
    auto openFile =
      std::make_shared<OpenFileState>(0, flags, wasmFS.getRootDirectory());
    return wasmFS.getLockedFileTable().add(openFile);
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
      // Since mode is optional, mode is specified using varargs.
      mode_t mode = 0;
      va_list vl;
      va_start(vl, flags);
      mode = va_arg(vl, int);
      va_end(vl);
      // Mask all permissions sent via mode.
      mode &= S_IALLUGO;
      // Create an empty in-memory file.
      auto backend = lockedParentDir.getBackend();
      auto created = backend->createFile(mode);

      // TODO: When rename is implemented make sure that one can atomically
      // remove the file from the source directory and then set its parent to
      // the dest directory.
      lockedParentDir.setEntry(base, created);
      auto openFile = std::make_shared<OpenFileState>(0, flags, created);

      return wasmFS.getLockedFileTable().add(openFile);
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

  return wasmFS.getLockedFileTable().add(openFile);
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

  auto base = pathParts.back();

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
    // Mask rwx permissions for user, group and others, and the sticky bit.
    // This prevents users from entering S_IFREG for example.
    // https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
    mode &= S_IRWXUGO | S_ISVTX;
    // Create an empty in-memory directory.
    auto backend = lockedParentDir.getBackend();
    auto created = backend->createDirectory(mode);

    lockedParentDir.setEntry(base, created);
    return 0;
  }
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

  if (pathParts.empty()) {
    return -ENOENT;
  }

  long err;
  auto dir = getDir(pathParts.begin(), pathParts.end(), err);

  if (!dir) {
    return err;
  }

  wasmFS.setCWD(dir);
  return 0;
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

  return 0;
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
  // TODO: Change to check root directory pointer instead of path.
  // This can be done when path parsing is refactored.
  // Current state just matches JS file system behaviour.
  if (pathParts.size() == 1 && pathParts[0] == "/") {
    return -EBUSY;
  }

  if (pathParts.empty()) {
    return -ENOENT;
  }

  auto base = pathParts.back();

  long err;
  auto parentDir = getDir(pathParts.begin(), pathParts.end() - 1, err);

  // Parent node doesn't exist.
  if (!parentDir) {
    return err;
  }

  // Hold the locked directory to prevent the state from being changed during
  // the operation.
  auto lockedParentDir = parentDir->locked();

  auto curr = lockedParentDir.getEntry(base);

  if (!curr) {
    return -ENOENT;
  }

  // rmdir checks if the target is a directory and if the directory is empty.
  auto targetDir = curr->dynCast<Directory>();
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
  if (!(lockedParentDir.mode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // Input is valid, perform the unlink.
  lockedParentDir.unlinkEntry(base);
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
    // TODO: Enforce that the name can fit in the d_name field.
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

  if (oldPathParts.empty()) {
    return -EINVAL;
  }

  // In Linux, renaming the root directory returns EBUSY.
  // TODO: Fix this when path parsing is refactored.
  std::vector<std::string> root = {"/"};
  if (oldPathParts == root) {
    return -EBUSY;
  }

  auto oldBase = oldPathParts.back();

  long err;
  auto oldParentDir = getDir(oldPathParts.begin(), oldPathParts.end() - 1, err);

  if (!oldParentDir) {
    return err;
  }

  // For this operation to be atomic we must lock the source parent directory.
  auto lockedOldParentDir = oldParentDir->locked();

  auto oldPath = lockedOldParentDir.getEntry(oldBase);

  if (!oldPath) {
    return -ENOENT;
  }

  // Obtain the destination parent directory to see if it exists.
  auto newPathParts = splitPath((char*)new_path);

  if (newPathParts.empty()) {
    return -EINVAL;
  }

  // In Linux, renaming a directory to the root directory returns ENOTEMPTY.
  // TODO: Fix this when path parsing is refactored.
  if (newPathParts == root) {
    return -ENOTEMPTY;
  }

  auto newBase = newPathParts.back();

  // oldPath is the forbidden ancestor.
  auto newParentDir =
    getDir(newPathParts.begin(), newPathParts.end() - 1, err, oldPath);

  // If the destination parent directory doesn't exist, the source file cannot
  // be moved.
  if (!newParentDir) {
    return err;
  }

  // Edge case: a/b -> a/c share the same parent. Trylocking the same parent
  // with a recursive mutex should work, so we don't need to treat that case
  // specially here.
  auto maybeLockedNewParentDir = newParentDir->maybeLocked();
  if (!maybeLockedNewParentDir) {
    return -EBUSY;
  }

  auto lockedNewParentDir = std::move(*maybeLockedNewParentDir);
  auto newPath = lockedNewParentDir.getEntry(newBase);

  // If old_path and new_path are the same, do nothing.
  if (newPath == oldPath) {
    return 0;
  }

  // Cannot move from source directory without write permissions.
  if (!(lockedOldParentDir.mode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // Cannot move to a destination parent directory without write permissions.
  if (!(lockedNewParentDir.mode() & WASMFS_PERM_WRITE)) {
    return -EACCES;
  }

  // new path must be removed if it exists.
  if (newPath) {
    if (oldPath->is<DataFile>()) {
      // Cannot overwrite a file with a directory.
      if (newPath->is<Directory>()) {
        return -EISDIR;
      }
    } else if (oldPath->is<Directory>()) {
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
  lockedOldParentDir.unlinkEntry(oldBase);
  lockedNewParentDir.setEntry(newBase, oldPath);

  return 0;
}
}
