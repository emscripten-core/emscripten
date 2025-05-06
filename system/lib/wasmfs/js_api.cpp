// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <dirent.h>
#include <syscall_arch.h>
#include <unistd.h>
#include <emscripten/wasmfs.h>

#include "backend.h"
#include "file.h"
#include "paths.h"

// Some APIs return data using a thread-local allocation that is never freed.
// This is simpler and more efficient as it avoids the JS caller needing to free
// the allocation (which would have both the overhead of free, and also of a
// call back into wasm), but on the other hand it does mean more memory may be
// used. This seems a reasonable tradeoff as heavy workloads should ideally
// avoid the JS API anyhow.

using namespace wasmfs;

extern "C" {

// Copy the file specified by the pathname into JS.
// Return zero on success, errno on failure.
// Output point and length are written to `out_buf` and `out_size` params.
int _wasmfs_read_file(const char* path, uint8_t** out_buf, off_t* out_size) {
  static_assert(sizeof(off_t) == 8, "File offset type must be 64-bit");

  struct stat file;
  int err = 0;
  err = stat(path, &file);
  if (err < 0) {
    return errno;
  }

  off_t size = file.st_size;

  static thread_local uint8_t* buffer = nullptr;
  buffer = (uint8_t*)realloc(buffer, size);

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    return errno;
  }
  ssize_t numRead = read(fd, buffer, size);
  if (numRead < 0) {
    return errno;
  }
  // TODO: Generalize this so that it is thread-proof.
  // Must guarantee that the file size has not changed by the time it is read.
  assert(numRead == size);
  err = close(fd);
  if (err < 0) {
    return errno;
  }

  *out_size = size;
  *out_buf = buffer;
  return 0;
}

// Writes to a file, possibly creating it, and returns the number of bytes
// written successfully. If the file already exists, appends to it.
int _wasmfs_write_file(const char* pathname, char* data, size_t data_size) {
  auto parsedParent = path::parseParent(pathname);
  if (parsedParent.getError()) {
    return 0;
  }
  auto& [parent, childNameView] = parsedParent.getParentChild();
  std::string childName(childNameView);

  std::shared_ptr<File> child;
  {
    auto lockedParent = parent->locked();
    child = lockedParent.getChild(childName);
    if (!child) {
      // Lookup failed; try creating the file.
      child = lockedParent.insertDataFile(childName, 0777);
      if (!child) {
        // File creation failed; nothing else to do.
        return 0;
      }
    }
  }

  auto dataFile = child->dynCast<DataFile>();
  if (!dataFile) {
    // There is something here but it isn't a data file.
    return 0;
  }

  auto lockedFile = dataFile->locked();
  int err = lockedFile.open(O_WRONLY);
  if (err < 0) {
    emscripten_err("Fatal error in FS.writeFile");
    abort();
  }

  auto offset = lockedFile.getSize();
  auto result = lockedFile.write((uint8_t*)data, data_size, offset);
  if (result != __WASI_ERRNO_SUCCESS) {
    return 0;
  }

  err = lockedFile.close();
  if (err < 0) {
    emscripten_err("Fatal error in FS.writeFile");
    abort();
  }

  return data_size;
}

int _wasmfs_mkdir(const char* path, int mode) {
  return __syscall_mkdirat(AT_FDCWD, (intptr_t)path, mode);
}

int _wasmfs_rmdir(const char* path) {
  return __syscall_unlinkat(AT_FDCWD, (intptr_t)path, AT_REMOVEDIR);
}

int _wasmfs_open(const char* path, int flags, mode_t mode) {
  return __syscall_openat(AT_FDCWD, (intptr_t)path, flags, mode);
}

int _wasmfs_mknod(const char* path, mode_t mode, dev_t dev) {
  return __syscall_mknodat(AT_FDCWD, (intptr_t)path, mode, dev);
}

int _wasmfs_unlink(const char* path) {
  return __syscall_unlinkat(AT_FDCWD, (intptr_t)path, 0);
}

int _wasmfs_chdir(const char* path) { return __syscall_chdir((intptr_t)path); }

int _wasmfs_symlink(const char* old_path, const char* new_path) {
  return __syscall_symlinkat((intptr_t)old_path, AT_FDCWD, (intptr_t)new_path);
}

int _wasmfs_readlink(const char* path, char** out_ptr) {
  static thread_local char* readBuf = (char*)malloc(PATH_MAX);
  int bytes =
    __syscall_readlinkat(AT_FDCWD, (intptr_t)path, (intptr_t)readBuf, PATH_MAX);
  if (bytes < 0) {
    return bytes;
  }
  readBuf[bytes] = '\0';
  *out_ptr = readBuf;
  return 0;
}

int _wasmfs_write(int fd, void* buf, size_t count) {
  __wasi_ciovec_t iovs[1];
  iovs[0].buf = (uint8_t*)buf;
  iovs[0].buf_len = count;

  __wasi_size_t numBytes;
  __wasi_errno_t err = __wasi_fd_write(fd, iovs, 1, &numBytes);
  if (err) {
    return -err;
  }
  return numBytes;
}

int _wasmfs_pwrite(int fd, void* buf, size_t count, off_t offset) {
  __wasi_ciovec_t iovs[1];
  iovs[0].buf = (uint8_t*)buf;
  iovs[0].buf_len = count;

  __wasi_size_t numBytes;
  __wasi_errno_t err = __wasi_fd_pwrite(fd, iovs, 1, offset, &numBytes);
  if (err) {
    return -err;
  }
  return numBytes;
}

int _wasmfs_chmod(const char* path, mode_t mode) {
  return __syscall_chmod((intptr_t)path, mode);
}

int _wasmfs_fchmod(int fd, mode_t mode) { return __syscall_fchmod(fd, mode); }

int _wasmfs_lchmod(const char* path, mode_t mode) {
  return __syscall_fchmodat2(
    AT_FDCWD, (intptr_t)path, mode, AT_SYMLINK_NOFOLLOW);
}

int _wasmfs_llseek(int fd, off_t offset, int whence) {
  __wasi_filesize_t newOffset;
  int err = __wasi_fd_seek(fd, offset, whence, &newOffset);
  if (err > 0) {
    return -err;
  }
  return newOffset;
}

int _wasmfs_rename(const char* oldpath, const char* newpath) {
  return __syscall_renameat(
    AT_FDCWD, (intptr_t)oldpath, AT_FDCWD, (intptr_t)newpath);
}

int _wasmfs_read(int fd, void* buf, size_t count) {
  __wasi_iovec_t iovs[1];
  iovs[0].buf = (uint8_t*)buf;
  iovs[0].buf_len = count;

  __wasi_size_t numBytes;
  __wasi_errno_t err = __wasi_fd_read(fd, iovs, 1, &numBytes);
  if (err) {
    return -err;
  }
  return numBytes;
}

int _wasmfs_pread(int fd, void* buf, size_t count, off_t offset) {
  __wasi_iovec_t iovs[1];
  iovs[0].buf = (uint8_t*)buf;
  iovs[0].buf_len = count;

  __wasi_size_t numBytes;
  __wasi_errno_t err = __wasi_fd_pread(fd, iovs, 1, offset, &numBytes);
  if (err) {
    return -err;
  }
  return numBytes;
}

int _wasmfs_truncate(const char* path, off_t length) {
  return __syscall_truncate64((intptr_t)path, length);
}

int _wasmfs_ftruncate(int fd, off_t length) {
  return __syscall_ftruncate64(fd, length);
}

int _wasmfs_close(int fd) { return __wasi_fd_close(fd); }

void* _wasmfs_mmap(size_t length, int prot, int flags, int fd, off_t offset) {
  return (void*)__syscall_mmap2(0, length, prot, flags, fd, offset);
}

int _wasmfs_msync(void* addr, size_t length, int flags) {
  return __syscall_msync((intptr_t)addr, length, flags);
}

int _wasmfs_munmap(void* addr, size_t length) {
  return __syscall_munmap((intptr_t)addr, length);
}

int _wasmfs_utime(const char* path, double atime_ms, double mtime_ms) {
  struct timespec times[2];
  times[0].tv_sec = (long)atime_ms / 1000;
  times[0].tv_nsec = ((long)atime_ms % 1000) * 1000000;
  times[1].tv_sec = (long)mtime_ms / 1000;
  times[1].tv_nsec = ((long)mtime_ms % 1000) * 1000000;

  return __syscall_utimensat(AT_FDCWD, (intptr_t)path, (intptr_t)times, 0);
}

int _wasmfs_stat(const char* path, struct stat* statBuf) {
  return __syscall_stat64((intptr_t)path, (intptr_t)statBuf);
}

int _wasmfs_lstat(const char* path, struct stat* statBuf) {
  return __syscall_lstat64((intptr_t)path, (intptr_t)statBuf);
}

// The legacy JS API requires a mountpoint to already exist, so  WasmFS will
// attempt to remove the target directory if it exists before replacing it with
// a mounted directory.
int _wasmfs_mount(const char* path, ::backend_t created_backend) {
  int err = __syscall_rmdir((intptr_t)path);

  // The legacy JS API mount requires the directory to already exist, but we
  // will also allow it to be missing.
  if (err && err != -ENOENT) {
    return err;
  }

  return wasmfs_create_directory(path, 0777, created_backend);
}

// Helper method that identifies what a path is:
//   ENOENT - if nothing exists there
//   EISDIR - if it is a directory
//   EEXIST - if it is a normal file
int _wasmfs_identify(const char* path) {
  struct stat file;
  int err = 0;
  err = stat(path, &file);
  if (err < 0) {
    return ENOENT;
  }
  if (S_ISDIR(file.st_mode)) {
    return EISDIR;
  }
  return EEXIST;
}

struct wasmfs_readdir_state {
  int i;
  int nentries;
  struct dirent** entries;
};

struct wasmfs_readdir_state* _wasmfs_readdir_start(const char* path) {
  struct dirent** entries;
  int nentries = scandir(path, &entries, NULL, alphasort);
  if (nentries == -1) {
    return NULL;
  }
  struct wasmfs_readdir_state* state =
    (struct wasmfs_readdir_state*)malloc(sizeof(*state));
  if (state == NULL) {
    return NULL;
  }
  state->i = 0;
  state->nentries = nentries;
  state->entries = entries;
  return state;
}

const char* _wasmfs_readdir_get(struct wasmfs_readdir_state* state) {
  if (state->i < state->nentries) {
    return state->entries[state->i++]->d_name;
  }
  return NULL;
}

void _wasmfs_readdir_finish(struct wasmfs_readdir_state* state) {
  for (int i = 0; i < state->nentries; i++) {
    free(state->entries[i]);
  }
  free(state->entries);
  free(state);
}

char* _wasmfs_get_cwd(void) {
  // TODO: PATH_MAX is 4K atm, so it might be good to reduce this somehow.
  static thread_local char* path = (char*)malloc(PATH_MAX);
  return getcwd(path, PATH_MAX);
}

} // extern "C"
