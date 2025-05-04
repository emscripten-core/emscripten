/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <malloc.h>
#include <syscall_arch.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <emscripten.h>
#include <emscripten/heap.h>
#include <emscripten/console.h>
#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

#include "../src/dirent/__dirent.h"
#include "lock.h"
#include "emscripten_internal.h"
#include "paths.h"

/*
 * WASI support code. These are compiled with the program, and call out
 * using wasi APIs, which can be provided either by a wasi VM or by our
 * emitted JS.
 */

// libc

void _abort_js(void) {
  __builtin_trap();
  /* Beyond this point should be unreachable. */
  _Exit(117);
}

_Static_assert(CLOCK_REALTIME == __WASI_CLOCKID_REALTIME, "must match");
_Static_assert(CLOCK_MONOTONIC == __WASI_CLOCKID_MONOTONIC, "must match");
_Static_assert(CLOCK_PROCESS_CPUTIME_ID == __WASI_CLOCKID_PROCESS_CPUTIME_ID, "must match");
_Static_assert(CLOCK_THREAD_CPUTIME_ID == __WASI_CLOCKID_THREAD_CPUTIME_ID, "must match");

static void wasi_filestat_to_stat(const __wasi_filestat_t* in,
                                  struct stat* out) {
  *out = (struct stat){
    .st_dev = in->dev,
    .st_ino = in->ino,
    .st_nlink = in->nlink,
    .st_size = in->size,
    .st_atim = __wasi_timestamp_to_timespec(in->atim),
    .st_mtim = __wasi_timestamp_to_timespec(in->mtim),
    .st_ctim = __wasi_timestamp_to_timespec(in->ctim),
  };

  // Convert file type to legacy types encoded in st_mode.
  switch (in->filetype) {
    case __WASI_FILETYPE_BLOCK_DEVICE:
      out->st_mode |= S_IFBLK;
      break;
    case __WASI_FILETYPE_CHARACTER_DEVICE:
      out->st_mode |= S_IFCHR;
      break;
    case __WASI_FILETYPE_DIRECTORY:
      out->st_mode |= S_IFDIR;
      break;
    case __WASI_FILETYPE_REGULAR_FILE:
      out->st_mode |= S_IFREG;
      break;
    case __WASI_FILETYPE_SOCKET_DGRAM:
    case __WASI_FILETYPE_SOCKET_STREAM:
      out->st_mode |= S_IFSOCK;
      break;
    case __WASI_FILETYPE_SYMBOLIC_LINK:
      out->st_mode |= S_IFLNK;
      break;
  }
}

static __wasi_fdflags_t fdflags_to_wasi_fdflags(int flags) {
  __wasi_fdflags_t fs_flags = 0;
  if (flags & O_APPEND) {
    fs_flags |= __WASI_FDFLAGS_APPEND;
  }
  if (flags & O_DSYNC) {
    fs_flags |= __WASI_FDFLAGS_DSYNC;
  }
  if (flags & O_NONBLOCK) {
    fs_flags |= __WASI_FDFLAGS_NONBLOCK;
  }
  if (flags & O_RSYNC) {
    fs_flags |= __WASI_FDFLAGS_RSYNC;
  }
  if (flags & O_SYNC) {
    fs_flags |= __WASI_FDFLAGS_SYNC;
  }
  return fs_flags;
}

// mmap support is nonexistent. TODO: emulate simple mmaps using
// stdio + malloc, which is slow but may help some things?

// Mark these as weak so that wasmfs does not collide with it. That is, if
// wasmfs is in use, we want to use that and not this.
weak int _mmap_js(size_t length,
                  int prot,
                  int flags,
                  int fd,
                  off_t offset,
                  int* allocated,
                  void** addr) {
  return -ENOSYS;
}

weak int _munmap_js(
  intptr_t addr, size_t length, int prot, int flags, int fd, off_t offset) {
  return -ENOSYS;
}

weak int __syscall_openat(int dirfd, intptr_t path, int flags, ...) {
  const char* resolved_path = (const char*)path;

  if (!strcmp(resolved_path, "/dev/stdin")) {
    return STDIN_FILENO;
  }
  if (!strcmp(resolved_path, "/dev/stdout")) {
    return STDOUT_FILENO;
  }
  if (!strcmp(resolved_path, "/dev/stderr")) {
    return STDERR_FILENO;
  }

  bool unlock;
  if (!__paths_resolve_path(&unlock, &dirfd, &resolved_path)) {
    return -ENOENT;
  }

  __wasi_errno_t error;

  // Compute rights corresponding with the access modes provided.
  // Attempt to obtain all rights, except the ones that contradict the
  // access mode provided to openat().
  __wasi_rights_t max =
    ~(__WASI_RIGHTS_FD_DATASYNC | __WASI_RIGHTS_FD_READ |
      __WASI_RIGHTS_FD_WRITE | __WASI_RIGHTS_FD_ALLOCATE |
      __WASI_RIGHTS_FD_READDIR | __WASI_RIGHTS_FD_FILESTAT_SET_SIZE);
  {
    int accmode = flags & O_ACCMODE;
    if (accmode == O_RDONLY || accmode == O_RDWR || accmode == O_WRONLY) {
      if (accmode == O_RDONLY || accmode == O_RDWR) {
        max |= __WASI_RIGHTS_FD_READ | __WASI_RIGHTS_FD_READDIR;
      }
      if (accmode == O_WRONLY || accmode == O_RDWR) {
        max |= __WASI_RIGHTS_FD_DATASYNC | __WASI_RIGHTS_FD_WRITE |
               __WASI_RIGHTS_FD_ALLOCATE |
               __WASI_RIGHTS_FD_FILESTAT_SET_SIZE;
      }
    } else if (accmode == O_EXEC || accmode == O_SEARCH) {
      // Do nothing.
    } else {
      error = EINVAL;
      goto out;
    }
  }

  // Ensure that we can actually obtain the minimal rights needed.
  __wasi_fdstat_t fsb_cur;
  error = __wasi_fd_fdstat_get(dirfd, &fsb_cur);
  if (error != __WASI_ERRNO_SUCCESS) {
    goto out;
  }

  // Path lookup properties.
  __wasi_lookupflags_t lookup_flags = 0;
  if ((flags & O_NOFOLLOW) == 0) {
    lookup_flags |= __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW;
  }

  // Open file with appropriate rights.
  __wasi_fdflags_t fs_flags = fdflags_to_wasi_fdflags(flags);

  __wasi_oflags_t oflags = 0;
  if (flags & O_CREAT) {
    oflags |= __WASI_OFLAGS_CREAT;
  }
  if (flags & O_DIRECTORY) {
    oflags |= __WASI_OFLAGS_DIRECTORY;
  }
  if (flags & O_EXCL) {
    oflags |= __WASI_OFLAGS_EXCL;
  }
  if (flags & O_TRUNC) {
    oflags |= __WASI_OFLAGS_TRUNC;
  }

  __wasi_rights_t fs_rights_base = max & fsb_cur.fs_rights_inheriting;
  __wasi_rights_t fs_rights_inheriting = fsb_cur.fs_rights_inheriting;
  __wasi_fd_t newfd;

  error = __wasi_path_open(dirfd,
                           lookup_flags,
                           resolved_path,
                           strlen(resolved_path),
                           oflags,
                           fs_rights_base,
                           fs_rights_inheriting,
                           fs_flags,
                           &newfd);

out:
  if (unlock) {
    __paths_unlock();
  }
  if (error != __WASI_ERRNO_SUCCESS) {
    return -error;
  }
  return newfd;
}

weak int __syscall_ioctl(int fd, int op, ...) {
  return -ENOSYS;
}

weak int __syscall_fcntl64(int fd, int cmd, ...) {
  switch (cmd) {
    case F_GETFD:
      // Act as if the close-on-exec flag is always set.
      return FD_CLOEXEC;
    case F_SETFD:
      // The close-on-exec flag is ignored.
      return 0;
    case F_GETFL: {
      // Obtain the flags and the rights of the descriptor.
      __wasi_fdstat_t fds;
      __wasi_errno_t error = __wasi_fd_fdstat_get(fd, &fds);
      if (error != 0) {
        return -error;
      }

      int oflags = 0;
      if (fds.fs_flags & __WASI_FDFLAGS_APPEND) {
        oflags |= O_APPEND;
      }
      if (fds.fs_flags & __WASI_FDFLAGS_DSYNC) {
        oflags |= O_DSYNC;
      }
      if (fds.fs_flags & __WASI_FDFLAGS_NONBLOCK) {
        oflags |= O_NONBLOCK;
      }
      if (fds.fs_flags & __WASI_FDFLAGS_RSYNC) {
        oflags |= O_RSYNC;
      }
      if (fds.fs_flags & __WASI_FDFLAGS_SYNC) {
        oflags |= O_SYNC;
      }

      // Roughly approximate the access mode by converting the rights.
      if ((fds.fs_rights_base &
           (__WASI_RIGHTS_FD_READ | __WASI_RIGHTS_FD_READDIR)) != 0) {
        if ((fds.fs_rights_base & __WASI_RIGHTS_FD_WRITE) != 0)
          oflags |= O_RDWR;
        else
          oflags |= O_RDONLY;
      } else if ((fds.fs_rights_base & __WASI_RIGHTS_FD_WRITE) != 0) {
        oflags |= O_WRONLY;
      } else {
        _Static_assert(O_SEARCH == O_EXEC, "");
        oflags |= O_SEARCH;
      }
      return oflags;
    }
    case F_SETFL: {
      // Set new file descriptor flags.
      va_list ap;
      va_start(ap, cmd);
      int flags = va_arg(ap, int);
      va_end(ap);

      __wasi_fdflags_t fs_flags = fdflags_to_wasi_fdflags(flags);

      __wasi_errno_t error = __wasi_fd_fdstat_set_flags(fd, fs_flags);
      if (error != 0) {
        return -error;
      }
      return 0;
    }
    default:
      return -EINVAL;
  }
}

weak int __syscall_fstat64(int fd, intptr_t buf) {
  __wasi_filestat_t sb;
  __wasi_errno_t error = __wasi_fd_filestat_get(fd, &sb);
  if (error != __WASI_ERRNO_SUCCESS) {
    return -error;
  }

  wasi_filestat_to_stat(&sb, (struct stat*)buf);
  return 0;
}

weak int __syscall_stat64(intptr_t path, intptr_t buf) {
  return __syscall_newfstatat(AT_FDCWD, path, buf, 0);
}

weak int __syscall_dup(int fd) {
  return -ENOSYS;
}

weak int __syscall_mkdirat(int dirfd, intptr_t path, int mode) {
  const char* resolved_path = (const char*)path;

  bool unlock;
  if (!__paths_resolve_path(&unlock, &dirfd, &resolved_path)) {
    return -ENOENT;
  }

  __wasi_errno_t error =
    __wasi_path_create_directory(dirfd, resolved_path, strlen(resolved_path));

  if (unlock) {
    __paths_unlock();
  }
  if (error != __WASI_ERRNO_SUCCESS) {
    return -error;
  }
  return 0;
}

weak int __syscall_newfstatat(int dirfd, intptr_t path, intptr_t buf, int flags) {
  // Convert flags to WASI.
  __wasi_lookupflags_t lookup_flags = 0;
  if ((flags & AT_SYMLINK_NOFOLLOW) == 0) {
    lookup_flags |= __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW;
  }

  const char* resolved_path = (const char*)path;

  bool unlock;
  if (!__paths_resolve_path(&unlock, &dirfd, &resolved_path)) {
    return -ENOENT;
  }

  __wasi_filestat_t fsb_cur;
  __wasi_errno_t error = __wasi_path_filestat_get(
    dirfd, lookup_flags, resolved_path, strlen(resolved_path), &fsb_cur);

  if (unlock) {
    __paths_unlock();
  }
  if (error != __WASI_ERRNO_SUCCESS) {
    return -error;
  }

  wasi_filestat_to_stat(&fsb_cur, (struct stat*)buf);
  return 0;
}

weak int __syscall_lstat64(intptr_t path, intptr_t buf) {
  return __syscall_newfstatat(AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
}

weak int __syscall_symlinkat(intptr_t target_arg, int newdirfd, intptr_t linkpath) {
  const char* resolved_linkpath = (const char*)linkpath;

  bool unlock;
  if (!__paths_resolve_path(&unlock, &newdirfd, &resolved_linkpath)) {
    return -ENOENT;
  }

  const char* target = (const char*)target_arg;

  __wasi_errno_t error = __wasi_path_symlink(target,
                                             strlen(target),
                                             newdirfd,
                                             resolved_linkpath,
                                             strlen(resolved_linkpath));

  if (unlock) {
    __paths_unlock();
  }
  if (error != __WASI_ERRNO_SUCCESS) {
    return -error;
  }
  return 0;
}

weak int __syscall_chdir(intptr_t path_arg) {
  const char* path = (const char*)path_arg;
  if (*path == 0) {
    return -ENOENT;
  }

  __wasi_errno_t error = __paths_chdir(path);
  if (error != 0) {
    return -error;
  }

  return 0;
}

weak int __syscall_fchdir(int fd) {
  __wasi_errno_t error = __paths_fchdir(fd);
  if (error != 0) {
    return -error;
  }

  return 0;
}

weak int __syscall_getcwd(intptr_t buf, size_t size) {
  __wasi_errno_t error = __paths_getcwd((char*)buf, &size);
  if (error != 0) {
    return -error;
  }

  return (int)size;
}

weak int __syscall_getdents64(int fd, intptr_t dirp, size_t count) {
  __wasi_errno_t error;
  intptr_t dirpointer = dirp;
  struct dirent *de = (void *)dirpointer;

  // Check if the result buffer is too small.
  if (count / sizeof(struct dirent) == 0) {
    return -EINVAL;
  }

  __wasi_dirent_t entry;

  // Create new buffer size to save same amount of __wasi_dirent_t as dirp records.
  size_t buffer_size = (count / sizeof(struct dirent)) * (sizeof(entry) + 256);
  char *buffer = malloc(buffer_size);
  if (buffer == NULL) {
    return -errno;
  }

  size_t buffer_processed = buffer_size;
  size_t buffer_used = buffer_size;
  size_t dirent_processed = 0;

  // We assume `dirp` always points to the buffer of a `DIR*`.
  __wasi_dircookie_t cookie =
    ((DIR*)((char*)dirpointer - offsetof(DIR, buf)))->tell;

  for (;;) {
    // Extract the next dirent header.
    size_t buffer_left = buffer_used - buffer_processed;
    if (buffer_left < sizeof(__wasi_dirent_t)) {
      // End-of-file.
      if (buffer_used < buffer_size) {
        break;
      }

      goto read_entries;
    }
    __wasi_dirent_t entry;
    memcpy(&entry, buffer + buffer_processed, sizeof(entry));

    size_t entry_size = sizeof(__wasi_dirent_t) + entry.d_namlen;
    if (entry.d_namlen == 0) {
      // Invalid pathname length. Skip the entry.
      buffer_processed += entry_size;
      continue;
    }

    // The entire entry must be present in buffer space. If not, read
    // the entry another time. Ensure that the read buffer is large
    // enough to fit at least this single entry.
    if (buffer_left < entry_size) {
      while (buffer_size < entry_size) {
        buffer_size *= 2;
      }
      char *new_buffer = realloc(buffer, buffer_size);
      if (new_buffer == NULL) {
        error = errno;
        goto out;
      }
      buffer = new_buffer;
      goto read_entries;
    }

    const char *name = buffer + buffer_processed + sizeof(entry);
    buffer_processed += entry_size;

    // Skip entries that do not fit in the dirent name buffer.
    if (entry.d_namlen > sizeof de->d_name) {
      continue;
    }

    // Skip entries having null bytes in the filename.
    if (memchr(name, '\0', entry.d_namlen) != NULL) {
      continue;
    }

    off_t d_ino = entry.d_ino;
    unsigned char d_type = entry.d_type;

    // Adapted from wasi-libc.
    if (d_ino == 0 && (entry.d_namlen != 2 || memcmp(name, "..", 2) != 0)) {
      __wasi_filestat_t sb;
      error = __wasi_path_filestat_get(fd, 0, name, entry.d_namlen, &sb);
      if (error == __WASI_ERRNO_NOENT) {
        // The file disappeared before we could read it, so skip it.
        continue;
      }
      if (error != __WASI_ERRNO_SUCCESS) {
        goto out;
      }

      // Fill in the inode.
      d_ino = sb.ino;

      // In case someone raced with us and replaced the object with this name
      // with another of a different type, update the type too.
      d_type = sb.filetype;
    }

    de->d_ino = d_ino;

    // Map the right WASI type to dirent type.
    // I could not get the dirent.h import to work to use defines.
    switch (d_type) {
      case __WASI_FILETYPE_UNKNOWN:
        de->d_type = 0;
        break;
      case __WASI_FILETYPE_BLOCK_DEVICE:
        de->d_type = 6;
        break;
      case __WASI_FILETYPE_CHARACTER_DEVICE:
        de->d_type = 2;
        break;
      case __WASI_FILETYPE_DIRECTORY:
        de->d_type = 4;
        break;
      case __WASI_FILETYPE_REGULAR_FILE:
        de->d_type = 8;
        break;
      case __WASI_FILETYPE_SOCKET_DGRAM:
        de->d_type = 12;
        break;
      case __WASI_FILETYPE_SOCKET_STREAM:
        de->d_type = 12;
        break;
      case __WASI_FILETYPE_SYMBOLIC_LINK:
        de->d_type = 10;
        break;
      default:
        de->d_type = 0;
        break;
    }

    de->d_off = entry.d_next;
    de->d_reclen = sizeof(struct dirent);
    memcpy(de->d_name, name, entry.d_namlen);
    de->d_name[entry.d_namlen] = '\0';
    cookie = entry.d_next;
    dirent_processed = dirent_processed + sizeof(struct dirent);

    // Can't fit more in my buffer.
    if (dirent_processed + sizeof(struct dirent) > count) {
      break;
    }

    // Set entry to next entry in memory.
    dirpointer = dirpointer + sizeof(struct dirent);
    de = (void*)(dirpointer);

    continue;

  read_entries:
    // Load more directory entries and continue.
    error = __wasi_fd_readdir(
      fd, (uint8_t*)buffer, buffer_size, cookie, &buffer_used);
    if (error != __WASI_ERRNO_SUCCESS) {
      goto out;
    }
    buffer_processed = 0;
  }

  error = __WASI_ERRNO_SUCCESS;

out:
  if (error != __WASI_ERRNO_SUCCESS) {
    free(buffer);
    return -error;
  }
  return dirent_processed;
}

// Emscripten additions

size_t emscripten_get_heap_max() {
  // In standalone mode we don't have any wasm instructions to access the max
  // memory size so the best we can do (without calling an import) is return
  // the current heap size.
  return emscripten_get_heap_size();
}

int emscripten_resize_heap(size_t size) {
#if defined(EMSCRIPTEN_MEMORY_GROWTH)
  size_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
  assert(old_size < size);
  ssize_t diff = (size - old_size + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE;
  size_t result = __builtin_wasm_memory_grow(0, diff);
  if (result != (size_t)-1) {
#if !defined(EMSCRIPTEN_PURE_WASI)
    // Success, update JS (see https://github.com/WebAssembly/WASI/issues/82)
    emscripten_notify_memory_growth(0);
#endif
    return 1;
  }
#endif
  return 0;
}

// Call clock_gettime with a particular clock and return the result in ms.
static double clock_gettime_ms(clockid_t clock) {
  struct timespec ts;
  if (clock_gettime(clock, &ts)) {
    return 0;
  }
  return (double)ts.tv_sec * 1000 + (double)ts.tv_nsec / 1000000;
}

weak double emscripten_get_now(void) {
  return clock_gettime_ms(CLOCK_MONOTONIC);
}

weak double emscripten_date_now(void) {
  return clock_gettime_ms(CLOCK_REALTIME);
}

// C++ ABI

#if EMSCRIPTEN_NOCATCH
// When exception catching is disabled, we stub out calls to `__cxa_throw`.
// Otherwise, `__cxa_throw` will be imported from the host.
void __cxa_throw(void* ptr, void* type, void* destructor) {
  abort();
}
#endif

// WasmFS integration. We stub out file preloading and such, that are not
// expected to work anyhow.

size_t _wasmfs_get_num_preloaded_files() { return 0; }

size_t _wasmfs_get_num_preloaded_dirs() { return 0; }

int _wasmfs_get_preloaded_file_size(int index) { return 0; }

int _wasmfs_get_preloaded_file_mode(int index) { return 0; }

void _wasmfs_copy_preloaded_file_data(int index, void* buffer) {}

void _wasmfs_get_preloaded_parent_path(int index, void* buffer) {}

void _wasmfs_get_preloaded_child_path(int index, void* buffer) {}

void _wasmfs_get_preloaded_path_name(int index, void* buffer) {}

// Import the VM's fd_write under a different name. Then we can interpose in
// between it and WasmFS's fd_write. That is, libc calls fd_write, which WasmFS
// implements. And WasmFS will forward actual writing to stdout/stderr to the
// VM's fd_write. (This allows WasmFS to do work in the middle, for example, it
// could support embedded files and other functionality.)
__attribute__((import_module("wasi_snapshot_preview1"),
               import_name("fd_write"))) __wasi_errno_t
imported__wasi_fd_write(__wasi_fd_t fd,
                        const __wasi_ciovec_t* iovs,
                        size_t iovs_len,
                        __wasi_size_t* nwritten);

// Write a buffer + a newline.
static void wasi_writeln_n(__wasi_fd_t fd, const char* buffer, size_t len) {
  struct __wasi_ciovec_t iovs[2];
  iovs[0].buf = (uint8_t*)buffer;
  iovs[0].buf_len = len;
  iovs[1].buf = (uint8_t*)"\n";
  iovs[1].buf_len = 1;
  __wasi_size_t nwritten;
  imported__wasi_fd_write(fd, iovs, 2, &nwritten);
}

static void wasi_writeln(__wasi_fd_t fd, const char* buffer) {
  return wasi_writeln_n(fd, buffer, strlen(buffer));
}

weak void emscripten_out(const char* text) { wasi_writeln(1, text); }

weak void emscripten_err(const char* text) { wasi_writeln(2, text); }

weak void emscripten_dbg(const char* text) { wasi_writeln(2, text); }

weak void emscripten_outn(const char* text, size_t len) {
  wasi_writeln_n(1, text, len);
}

weak void emscripten_errn(const char* text, size_t len) {
  wasi_writeln_n(2, text, len);
}

weak void emscripten_dbgn(const char* text, size_t len) {
  wasi_writeln_n(2, text, len);
}

__attribute__((import_module("wasi_snapshot_preview1"),
               import_name("fd_read"))) __wasi_errno_t
imported__wasi_fd_read(__wasi_fd_t fd,
                        const __wasi_ciovec_t* iovs,
                        size_t iovs_len,
                        __wasi_size_t* nread);

int _wasmfs_stdin_get_char(void) {
  char c;
  struct __wasi_ciovec_t iov;
  iov.buf = (uint8_t*)&c;
  iov.buf_len = 1;
  __wasi_size_t nread;
  imported__wasi_fd_read(0, &iov, 1, &nread);
  if (nread == 0) {
    return -1;
  }
  return c;
}

// In the non-standalone build we define this helper function in JS to avoid
// signture mismatch issues.
// See: https://github.com/emscripten-core/posixtestsuite/issues/6
void __call_sighandler(sighandler_t handler, int sig) {
  handler(sig);
}

int _setitimer_js(int which, double timeout) {
  // There is no API to let us set timers in standalone mode atm. Return an
  // error.
  errno = ENOTSUP;
  return -1;
}

weak uintptr_t emscripten_stack_snapshot(void) {
  return 0;
}

weak uint32_t emscripten_stack_unwind_buffer(uintptr_t pc,
                                             uintptr_t* buffer,
                                             uint32_t depth) {
  return 0;
}

weak const char* emscripten_pc_get_function(uintptr_t pc) {
  return NULL;
}

weak const char* emscripten_pc_get_file(uintptr_t pc) {
  return NULL;
}

weak int emscripten_pc_get_line(uintptr_t pc) {
  return 0;
}

weak int emscripten_pc_get_column(uintptr_t pc) {
  return 0;
}

weak void* emscripten_return_address(int level) {
  return NULL;
}

weak int _emscripten_sanitizer_use_colors(void) {
  return 1;
}

weak char* _emscripten_sanitizer_get_option(const char* name) {
  return strdup("");
}

weak void _emscripten_get_progname(char* buf, int length) {
  strncpy(buf, "<unknown>", length);
}

weak void _emscripten_runtime_keepalive_clear() {}
