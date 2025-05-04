/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include <assert.h>
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

  if (!__paths_resolve_path(&dirfd, &resolved_path)) {
    return -ENOENT;
  }

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
      return -EINVAL;
    }
  }

  // Ensure that we can actually obtain the minimal rights needed.
  __wasi_fdstat_t fsb_cur;
  __wasi_errno_t error = __wasi_fd_fdstat_get(dirfd, &fsb_cur);
  if (error != __WASI_ERRNO_SUCCESS) {
    return -error;
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

  error = __wasi_path_open(dirfd, lookup_flags, resolved_path, strlen(resolved_path),
                           oflags,
                           fs_rights_base, fs_rights_inheriting, fs_flags,
                           &newfd);
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
  return -ENOSYS;
}

weak int __syscall_stat64(intptr_t path, intptr_t buf) {
  return __syscall_newfstatat(AT_FDCWD, path, buf, 0);
}

weak int __syscall_dup(int fd) {
  return -ENOSYS;
}

weak int __syscall_mkdirat(int dirfd, intptr_t path, int mode) {
  const char* resolved_path = (const char*)path;

  if (!__paths_resolve_path(&dirfd, &resolved_path)) {
    return -ENOENT;
  }

  __wasi_errno_t error = __wasi_path_create_directory(dirfd, resolved_path, strlen(resolved_path));
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

  if (!__paths_resolve_path(&dirfd, &resolved_path)) {
    return -ENOENT;
  }

  __wasi_filestat_t fsb_cur;
  __wasi_errno_t error = __wasi_path_filestat_get(
    dirfd, lookup_flags, resolved_path, strlen(resolved_path), &fsb_cur);
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

  if (!__paths_resolve_path(&newdirfd, &resolved_linkpath)) {
    return -ENOENT;
  }

  const char* target = (const char*)target_arg;

  __wasi_errno_t error = __wasi_path_symlink(target,
                                             strlen(target),
                                             newdirfd,
                                             resolved_linkpath,
                                             strlen(resolved_linkpath));
  if (error != __WASI_ERRNO_SUCCESS) {
    return -error;
  }
  return 0;
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
