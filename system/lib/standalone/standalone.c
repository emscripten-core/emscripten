/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <malloc.h>
#include <syscall_arch.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <sysexits.h>
#include <sys/ioctl.h>

#include <emscripten.h>
#include <emscripten/heap.h>
#include <emscripten/console.h>
#include <wasi/api.h>
#include <wasi/wasi-helpers.h>

#include "lock.h"
#include "emscripten_internal.h"

/*
 * WASI support code. These are compiled with the program, and call out
 * using wasi APIs, which can be provided either by a wasi VM or by our
 * emitted JS.
 */

// libc

#ifdef NDEBUG
#define REPORT_UNSUPPORTED(action)
#else
#define REPORT_UNSUPPORTED(action) \
  emscripten_console_error("the program tried to " #action ", this is not supported in standalone mode");
#endif

void _abort_js(void) {
  __builtin_trap();
  /* Beyond this point should be unreachable. */
  _Exit(117);
}

_Static_assert(CLOCK_REALTIME == __WASI_CLOCKID_REALTIME, "must match");
_Static_assert(CLOCK_MONOTONIC == __WASI_CLOCKID_MONOTONIC, "must match");
_Static_assert(CLOCK_PROCESS_CPUTIME_ID == __WASI_CLOCKID_PROCESS_CPUTIME_ID, "must match");
_Static_assert(CLOCK_THREAD_CPUTIME_ID == __WASI_CLOCKID_THREAD_CPUTIME_ID, "must match");

#define NSEC_PER_SEC (1000 * 1000 * 1000)

struct timespec __wasi_timestamp_to_timespec(__wasi_timestamp_t timestamp) {
  return (struct timespec){.tv_sec = timestamp / NSEC_PER_SEC,
                           .tv_nsec = timestamp % NSEC_PER_SEC};
}

int clock_getres(clockid_t clk_id, struct timespec *tp) {
  // See https://github.com/bytecodealliance/wasmtime/issues/3714
  if (clk_id > __WASI_CLOCKID_THREAD_CPUTIME_ID || clk_id < 0) {
    errno = EINVAL;
    return -1;
  }
  __wasi_timestamp_t res;
  __wasi_errno_t error = __wasi_clock_res_get(clk_id, &res);
  if (error != __WASI_ERRNO_SUCCESS) {
    return __wasi_syscall_ret(error);
  }
  *tp = __wasi_timestamp_to_timespec(res);
  return 0;
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

/// A name and file descriptor pair.
typedef struct preopen {
  /// The path prefix associated with the file descriptor.
  const char *prefix;

  /// The file descriptor.
  __wasi_fd_t fd;
} preopen;

/// A simple growable array of `preopen`.
static preopen *preopens;
static size_t num_preopens;
static size_t preopen_capacity;

/// Access to the the above preopen must be protected.
static volatile int lock[1];

#ifdef NDEBUG
#define assert_invariants() // assertions disabled
#else
static void assert_invariants(void) {
  assert(num_preopens <= preopen_capacity);
  assert(preopen_capacity == 0 || preopens != NULL);
  assert(preopen_capacity == 0 ||
         preopen_capacity * sizeof(preopen) > preopen_capacity);

  for (size_t i = 0; i < num_preopens; ++i) {
    const preopen *pre = &preopens[i];
    assert(pre->prefix != NULL);
    assert(pre->fd != (__wasi_fd_t)-1);
#ifdef __wasm__
    assert((uintptr_t)pre->prefix <
               (__uint128_t)__builtin_wasm_memory_size(0) * PAGESIZE);
#endif
  }
}
#endif

/// Allocate space for more preopens. Returns 0 on success and -1 on failure.
static int resize(void) {
  LOCK(lock);
  size_t start_capacity = 4;
  size_t old_capacity = preopen_capacity;
  size_t new_capacity = old_capacity == 0 ? start_capacity : old_capacity * 2;

  preopen *old_preopens = preopens;
  preopen *new_preopens = calloc(sizeof(preopen), new_capacity);
  if (new_preopens == NULL) {
    UNLOCK(lock);
    return -1;
  }

  memcpy(new_preopens, old_preopens, num_preopens * sizeof(preopen));
  preopens = new_preopens;
  preopen_capacity = new_capacity;
  free(old_preopens);

  assert_invariants();
  UNLOCK(lock);
  return 0;
}

// Normalize an absolute path. Removes leading `/` and leading `./`, so the
// first character is the start of a directory name. This works because our
// process always starts with a working directory of `/`. Additionally translate
// `.` to the empty string.
static const char *strip_prefixes(const char *path) {
  while (1) {
    if (path[0] == '/') {
      path++;
    } else if (path[0] == '.' && path[1] == '/') {
      path += 2;
    } else if (path[0] == '.' && path[1] == 0) {
      path++;
    } else {
      break;
    }
  }

  return path;
}

/// Register the given preopened file descriptor under the given path.
///
/// This function takes ownership of `prefix`.
static int internal_register_preopened_fd(__wasi_fd_t fd, const char *relprefix) {
  LOCK(lock);

  // Check preconditions.
  assert_invariants();
  assert(fd != AT_FDCWD);
  assert(fd != -1);
  assert(relprefix != NULL);

  if (num_preopens == preopen_capacity && resize() != 0) {
    UNLOCK(lock);
    return -1;
  }

  char *prefix = strdup(strip_prefixes(relprefix));
  if (prefix == NULL) {
    UNLOCK(lock);
    return -1;
  }
  preopens[num_preopens++] = (preopen) { prefix, fd, };

  assert_invariants();
  UNLOCK(lock);
  return 0;
}

/// Are the `prefix_len` bytes pointed to by `prefix` a prefix of `path`?
static bool prefix_matches(const char *prefix, size_t prefix_len, const char *path) {
  // Allow an empty string as a prefix of any relative path.
  if (path[0] != '/' && prefix_len == 0)
    return true;

  // Check whether any bytes of the prefix differ.
  if (memcmp(path, prefix, prefix_len) != 0)
    return false;

  // Ignore trailing slashes in directory names.
  size_t i = prefix_len;
  while (i > 0 && prefix[i - 1] == '/') {
    --i;
  }

  // Match only complete path components.
  char last = path[i];
  return last == '/' || last == '\0';
}

int __standalone_find_abspath(const char *path,
                            const char **abs_prefix,
                            const char **relative_path) {
  // Strip leading `/` characters, the prefixes we're mataching won't have
  // them.
  while (*path == '/')
    path++;
  // Search through the preopens table. Iterate in reverse so that more
  // recently added preopens take precedence over less recently addded ones.
  size_t match_len = 0;
  int fd = -1;
  LOCK(lock);
  for (size_t i = num_preopens; i > 0; --i) {
    const preopen *pre = &preopens[i - 1];
    const char *prefix = pre->prefix;
    size_t len = strlen(prefix);

    // If we haven't had a match yet, or the candidate path is longer than
    // our current best match's path, and the candidate path is a prefix of
    // the requested path, take that as the new best path.
    if ((fd == -1 || len > match_len) &&
        prefix_matches(prefix, len, path))
    {
      fd = pre->fd;
      match_len = len;
      *abs_prefix = prefix;
    }
  }
  UNLOCK(lock);

  if (fd == -1) {
    errno = ENOENT;
    return -1;
  }

  // The relative path is the substring after the portion that was matched.
  const char *computed = path + match_len;

  // Omit leading slashes in the relative path.
  while (*computed == '/')
    ++computed;

  // *at syscalls don't accept empty relative paths, so use "." instead.
  if (*computed == '\0')
    computed = ".";

  *relative_path = computed;

  return fd;
}

// See the documentation in libc.h
int __standalone_register_preopened_fd(int fd, const char *prefix) {
  return internal_register_preopened_fd((__wasi_fd_t)fd, prefix);
}

int __standalone_find_relpath(const char *path,
                            const char **abs_prefix,
                            char **relative_path,
                            size_t relative_path_len) {
  return __standalone_find_abspath(path, abs_prefix, (const char**) relative_path);
}

static int find_relpath2(
  const char *path,
  char **relative,
  size_t *relative_len
) {
  // See comments in `preopens.c` for what this trick is doing.
  const char *abs;
  return __standalone_find_relpath(path, &abs, relative, *relative_len);
}

// Helper to call `__standalone_find_relpath` and return an already-managed
// pointer for the `relative` path. This function is not reentrant since the
// `relative` pointer will point to static data that cannot be reused until
// `relative` is no longer used.
static int find_relpath(const char *path, char **relative) {
  static __thread char *relative_buf = NULL;
  static __thread size_t relative_buf_len = 0;
  int fd = find_relpath2(path, &relative_buf, &relative_buf_len);
  // find_relpath2 can update relative_buf, so assign it after the call
  *relative = relative_buf;
  return fd;
}

// Populate WASI preopens.
__attribute__((constructor(100))) // construct this before user code
static void _standalone_populate_preopens(void) {
  // Skip stdin, stdout, and stderr, and count up until we reach an invalid
  // file descriptor.
  for (__wasi_fd_t fd = 3; fd != 0; ++fd) {
    __wasi_prestat_t prestat;
    __wasi_errno_t ret = __wasi_fd_prestat_get(fd, &prestat);
    if (ret == __WASI_ERRNO_BADF)
      break;
    if (ret != __WASI_ERRNO_SUCCESS)
      goto oserr;
    switch (prestat.pr_type) {
      case __WASI_PREOPENTYPE_DIR: {
        char *prefix = malloc(prestat.u.dir.pr_name_len + 1);
        if (prefix == NULL)
          goto software;

        // TODO: Remove the cast on `path` once the witx is updated with
        // char8 support.
        ret = __wasi_fd_prestat_dir_name(fd, (uint8_t *)prefix,
                                         prestat.u.dir.pr_name_len);
        if (ret != __WASI_ERRNO_SUCCESS)
          goto oserr;
        prefix[prestat.u.dir.pr_name_len] = '\0';

        if (internal_register_preopened_fd(fd, prefix) != 0)
          goto software;
        free(prefix);

        break;
      }
      default:
        break;
    }
  }

  return;
  oserr:
  _Exit(EX_OSERR);
  software:
  _Exit(EX_SOFTWARE);
}

// open(), etc. - we just support the standard streams, with no
// corner case error checking; everything else is not permitted.
// TODO: full file support for WASI, or an option for it
// open()
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

  // Resolve path if fd is AT_FDCWD.
  if (dirfd == AT_FDCWD) {
    char *relative_path;
    dirfd = find_relpath(resolved_path, &relative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (dirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    resolved_path = relative_path;
  }

  // Compute rights corresponding with the access modes provided.
  // Attempt to obtain all rights, except the ones that contradict the
  // access mode provided to openat().
  __wasi_rights_t max =
    ~(__WASI_RIGHTS_FD_DATASYNC | __WASI_RIGHTS_FD_READ |
      __WASI_RIGHTS_FD_WRITE | __WASI_RIGHTS_FD_ALLOCATE |
      __WASI_RIGHTS_FD_READDIR | __WASI_RIGHTS_FD_FILESTAT_SET_SIZE);
  switch (flags & O_ACCMODE) {
    case O_RDONLY:
    case O_RDWR:
    case O_WRONLY:
      if ((flags & O_RDONLY) != 0) {
        max |= __WASI_RIGHTS_FD_READ | __WASI_RIGHTS_FD_READDIR;
      }
      if ((flags & O_WRONLY) != 0) {
        max |= __WASI_RIGHTS_FD_DATASYNC | __WASI_RIGHTS_FD_WRITE |
               __WASI_RIGHTS_FD_ALLOCATE |
               __WASI_RIGHTS_FD_FILESTAT_SET_SIZE;
      }
      break;
    case O_EXEC: // O_EXEC => O_PATH => 010000000
    //case O_SEARCH: O_SEARCH => O_PATH => 010000000, both are the same, so causes errors.
      break;
    default:
      errno = EINVAL;
      return -1;
  }

  // Ensure that we can actually obtain the minimal rights needed.
  __wasi_fdstat_t fsb_cur;
  __wasi_errno_t error = __wasi_fd_fdstat_get(dirfd, &fsb_cur);
  if (error != __WASI_ERRNO_SUCCESS) {
    return __wasi_syscall_ret(error);
  }

  // Path lookup properties.
  __wasi_lookupflags_t lookup_flags = 0;
  if ((flags & O_NOFOLLOW) == 0) {
    lookup_flags |= __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW;
  }

  // Open file with appropriate rights.
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
    return __wasi_syscall_ret(error);
  }

  return newfd;
}

#define __arraycount(x) (sizeof(x) / sizeof((x)[0]))

weak int __syscall_ioctl(int fildes, int request, ...) {
  switch (request) {
    case FIONREAD: {
      // Poll the file descriptor to determine how many bytes can be read.
      __wasi_subscription_t subscriptions[2] = {
        {
          .type = __WASI_EVENTTYPE_FD_READ,
          .u.fd_readwrite.file_descriptor = fildes,
        },
        {
          .type = __WASI_EVENTTYPE_CLOCK,
          .u.clock.id = __WASI_CLOCKID_MONOTONIC,
        },
      };
      __wasi_event_t events[__arraycount(subscriptions)];
      size_t nevents;
      __wasi_errno_t error = __wasi_poll_oneoff(
        subscriptions, events, __arraycount(subscriptions), &nevents);
      if (error != 0) {
        errno = error;
        return -1;
      }

      // Location where result should be written.
      va_list ap;
      va_start(ap, request);
      int *result = va_arg(ap, int *);
      va_end(ap);

      // Extract number of bytes for reading from poll results.
      for (size_t i = 0; i < nevents; ++i) {
        __wasi_event_t *event = &events[i];
        if (event->error != 0) {
          errno = event->error;
          return -1;
        }
        if (event->type == __WASI_EVENTTYPE_FD_READ) {
          *result = event->u.fd_readwrite.nbytes;
          return 0;
        }
      }

      // No data available for reading.
      *result = 0;
      return 0;
    }
    case FIONBIO: {
      // Obtain the current file descriptor flags.
      __wasi_fdstat_t fds;
      __wasi_errno_t error = __wasi_fd_fdstat_get(fildes, &fds);
      if (error != 0) {
        errno = error;
        return -1;
      }

      // Toggle the non-blocking flag based on the argument.
      va_list ap;
      va_start(ap, request);
      if (*va_arg(ap, const int *) != 0)
      fds.fs_flags |= __WASI_FDFLAGS_NONBLOCK;
      else
      fds.fs_flags &= ~__WASI_FDFLAGS_NONBLOCK;
      va_end(ap);

      // Update the file descriptor flags.
      error = __wasi_fd_fdstat_set_flags(fildes, fds.fs_flags);
      if (error != 0) {
        errno = error;
        return -1;
      }
      return 0;
    }
    default:
      // Invalid request.
      errno = EINVAL;
      return -1;
  }
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
        errno = error;
        return -1;
      }

      // Roughly approximate the access mode by converting the rights.
      int oflags = fds.fs_flags;
      if ((fds.fs_rights_base &
           (__WASI_RIGHTS_FD_READ | __WASI_RIGHTS_FD_READDIR)) != 0) {
        if ((fds.fs_rights_base & __WASI_RIGHTS_FD_WRITE) != 0)
          oflags |= O_RDWR;
        else
          oflags |= O_RDONLY;
      } else if ((fds.fs_rights_base & __WASI_RIGHTS_FD_WRITE) != 0) {
        oflags |= O_WRONLY;
      } else {
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

      __wasi_fdflags_t fs_flags = flags & 0xfff;
      __wasi_errno_t error =
        __wasi_fd_fdstat_set_flags(fd, fs_flags);
      if (error != 0) {
        errno = error;
        return -1;
      }
      return 0;
    }
    default:
      errno = EINVAL;
      return -1;
  }
}

weak int __syscall_ftruncate64(int fd, off_t length) {
  if (length < 0) {
    errno = EINVAL;
    return -1;
  }
  __wasi_filesize_t st_size = length;
  __wasi_errno_t error =
    __wasi_fd_filestat_set_size(fd, st_size);
  if (error != 0) {
    errno = error;
    return -1;
  }
  return 0;
}

int rmdirat(int dirfd, intptr_t path) {
  const char* resolved_path = (const char*)path;

  // Resolve path if fd is AT_FDCWD.
  if (dirfd == AT_FDCWD) {
    char *relative_path;
    dirfd = find_relpath(resolved_path, &relative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (dirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    resolved_path = relative_path;
  }

  __wasi_errno_t error = __wasi_path_remove_directory(dirfd, resolved_path, strlen(resolved_path));
  if (error != 0) {
    errno = error;
    return -1;
  }

  return 0;
}

weak int __syscall_rmdir(intptr_t path) {
  return rmdirat(AT_FDCWD, path);
}

weak int __syscall_unlinkat(int dirfd, intptr_t path, int flags) {
  // unlinkat with AT_REMOVEDIR flag is acutally rmdir.
  if ((flags & AT_REMOVEDIR) != 0) {
    return rmdirat(dirfd, path);
  }

  const char* resolved_path = (const char*)path;

  // Resolve path if fd is AT_FDCWD.
  if (dirfd == AT_FDCWD) {
    char *relative_path;
    dirfd = find_relpath(resolved_path, &relative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (dirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    resolved_path = relative_path;
  }

  __wasi_errno_t error = __wasi_path_unlink_file(dirfd, resolved_path, strlen(resolved_path));
  if (error != 0) {
    errno = error;
    return -1;
  }
  return 0;
}

weak int __syscall_pipe(intptr_t fd) {
  REPORT_UNSUPPORTED(__syscall_pipe);
  abort();
  return -ENOSYS;
}

weak int __syscall_renameat(int olddirfd, intptr_t oldpath, int newdirfd, intptr_t newpath) {
  const char* oldpathresolved_path = (const char*)oldpath;

  // Resolve path if fd is AT_FDCWD.
  if (olddirfd == AT_FDCWD) {
    char *oldpathrelative_path;
    olddirfd = find_relpath(oldpathresolved_path, &oldpathrelative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (olddirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    oldpathresolved_path = oldpathrelative_path;
  }

  const char* newpathresolved_path = (const char*)newpath;

  // Resolve path if fd is AT_FDCWD.
  if (newdirfd == AT_FDCWD) {
    char *newpathrelative_path;
    newdirfd = find_relpath(newpathresolved_path, &newpathrelative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (newdirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    newpathresolved_path = newpathrelative_path;
  }

  __wasi_errno_t error = __wasi_path_rename(olddirfd, oldpathresolved_path, strlen(oldpathresolved_path), newdirfd, newpathresolved_path, strlen(newpathresolved_path));
  if (error != 0) {
    errno = error;
    return -1;
  }
  return 0;
}

weak int __syscall_dup(int fd) {
  return -ENOSYS;
}

weak int __syscall_dup3(int fd, int suggestfd, int flags) {
    return -ENOSYS;
}

weak int __syscall_faccessat(int dirfd, intptr_t path, int amode, int flags) {
  const char* resolved_path = (const char*)path;

  // Resolve path if fd is AT_FDCWD.
  if (dirfd == AT_FDCWD) {
    char *relative_path;
    dirfd = find_relpath(resolved_path, &relative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (dirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    resolved_path = relative_path;
  }

  // Validate function parameters.
  if ((amode & ~(F_OK | R_OK | W_OK | X_OK)) != 0 ||
      (flags & ~AT_EACCESS) != 0) {
    errno = EINVAL;
    return -1;
  }

  // Check for target file existence and obtain the file type.
  __wasi_lookupflags_t lookup_flags = __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW;
  __wasi_filestat_t file;
  __wasi_errno_t error =
    __wasi_path_filestat_get(dirfd, lookup_flags, resolved_path, strlen(resolved_path), &file);
  if (error != 0) {
    errno = error;
    return -1;
  }

  // Test whether the requested access rights are present on the
  // directory file descriptor.
  if (amode != 0) {
    __wasi_fdstat_t directory;
    error = __wasi_fd_fdstat_get(dirfd, &directory);
    if (error != 0) {
      errno = error;
      return -1;
    }

    __wasi_rights_t min = 0;
    if ((amode & R_OK) != 0)
      min |= file.filetype == __WASI_FILETYPE_DIRECTORY
             ? __WASI_RIGHTS_FD_READDIR
             : __WASI_RIGHTS_FD_READ;
    if ((amode & W_OK) != 0)
      min |= __WASI_RIGHTS_FD_WRITE;

    if ((min & directory.fs_rights_inheriting) != min) {
      errno = EACCES;
      return -1;
    }
  }
  return 0;
}

static void __wasi_filestat_to_stat(const __wasi_filestat_t *in,
                                  struct stat *out) {
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

weak int __syscall_fstat64(int fd, intptr_t buf) {
  __wasi_filestat_t internal_stat;
  __wasi_errno_t error = __wasi_fd_filestat_get(fd, &internal_stat);
  if (error != __WASI_ERRNO_SUCCESS) {
    return __wasi_syscall_ret(error);
  }
  __wasi_filestat_to_stat(&internal_stat, (struct stat *) buf);
  return 0;
}

weak int __syscall_getdents64(int fd, intptr_t dirp, size_t count) {
  intptr_t dirpointer = dirp;
  struct dirent *de;
  de = (void *)(dirpointer);

  // Check if the result buffer is too small.
  if (count / sizeof(struct dirent) == 0) {
    return -EINVAL;
  }

  __wasi_dirent_t entry;

  // Create new buffer size to save same amount of __wasi_dirent_t as dirp records.
  size_t buffer_size = (count / sizeof(struct dirent)) * (sizeof(entry) + 256);
  char *buffer = malloc(buffer_size);
  if (buffer == NULL) {
    return -1;
  }

  size_t buffer_processed = buffer_size;
  size_t buffer_used = buffer_size;
  size_t dirent_processed = 0;

  // Use the cookie of the previous entries, readdir reuses the buffer so
  // a nonzero de->d_off is the cookie of the last readdir call.
  int i;
  struct dirent *checkde;
  __wasi_dircookie_t cookie = 0;
  for (i = 0; i < (count / sizeof(struct dirent)); ++i) {
    checkde = (void *)(dirpointer + (sizeof(struct dirent) * i));

    // Store cookie if it's bigger than the last known.
    if (checkde->d_off > cookie) {
      cookie = checkde->d_off;
    }

    // Reset cookie to 0 so that this offset isn't going to hunt us in later calls.
    checkde->d_off = 0;
  }

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
        return -1;
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

    de->d_ino = entry.d_ino;

    // Map the right WASI type to dirent type.
    // I could not get the dirent.h import to work to use defines.
    switch (entry.d_type) {
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
    de = (void *)(dirpointer);

    continue;

    read_entries:;
      // Load more directory entries and continue.
      // TODO: Remove the cast on `buffer` once the witx is updated with char8 support.
      __wasi_errno_t error = __wasi_fd_readdir(fd, (uint8_t *)buffer, buffer_size,
                                               cookie, &buffer_used);
      if (error != 0) {
        errno = error;
        return -1;
      }
      buffer_processed = 0;
  }

  return dirent_processed;
}

int __syscall_newfstatat(int dirfd, intptr_t path, intptr_t buf, int flags) {
  // Convert flags to WASI.
  __wasi_lookupflags_t lookup_flags = 0;
  if ((flags & AT_SYMLINK_NOFOLLOW) == 0) {
    lookup_flags |= __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW;
  }

  const char* resolved_path = (const char*)path;

  // Resolve path if fd is AT_FDCWD.
  if (dirfd == AT_FDCWD) {
    char *relative_path;
    dirfd = find_relpath(resolved_path, &relative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (dirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    resolved_path = relative_path;
  }

  __wasi_filestat_t fsb_cur;
  __wasi_errno_t error = __wasi_path_filestat_get(dirfd, lookup_flags, resolved_path, strlen(resolved_path), &fsb_cur);
  if (error != __WASI_ERRNO_SUCCESS) {
    return __wasi_syscall_ret(error);
  }

  __wasi_filestat_to_stat(&fsb_cur, (struct stat *) buf);

  return 0;
}

weak int __syscall_stat64(intptr_t path, intptr_t buf) {
  return __syscall_newfstatat(AT_FDCWD, path, buf, 0);
}

weak int __syscall_lstat64(intptr_t path, intptr_t buf) {
  return __syscall_newfstatat(AT_FDCWD, path, buf, AT_SYMLINK_NOFOLLOW);
}

weak int getentropy(void *buffer, size_t length) {
  return __wasi_syscall_ret(__wasi_random_get(buffer, length));
}

weak int __syscall_getcwd(intptr_t buf, size_t size) {
  // Check if buf points to a bad address.
  if (!buf && size > 0) {
    return -EFAULT;
  }

  // Check if the size argument is zero and buf is not a null pointer.
  if (buf && size == 0) {
    return -EINVAL;
  }

  // We force our CWD to always be /.
  char res[1]="/";
  int len = 2;

  // Check if the size argument is less than the length of the absolute
  // pathname of the working directory, including null terminator.
  if (len >= size) {
    return -ERANGE;
  }

  // Return value is a null-terminated c string.
  strcpy((char*)buf, res);

  return len;
}

weak int __syscall_mkdirat(int dirfd, intptr_t path, int mode) {
  const char* resolved_path = (const char*)path;

  // Resolve path if fd is AT_FDCWD.
  if (dirfd == AT_FDCWD) {
    char *relative_path;
    dirfd = find_relpath(resolved_path, &relative_path);

    // If we can't find a preopen for it, fail as if we can't find the path.
    if (dirfd == -1) {
      errno = ENOENT;
      return -1;
    }

    resolved_path = relative_path;
  }

  __wasi_errno_t error = __wasi_path_create_directory(dirfd, resolved_path, strlen(resolved_path));
  if (error != 0) {
    errno = error;
    return -1;
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
#if defined(EMSCRIPTEN_MEMORY_GROWTH) && !defined(EMSCRIPTEN_PURE_WASI)
  size_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
  assert(old_size < size);
  ssize_t diff = (size - old_size + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE;
  size_t result = __builtin_wasm_memory_grow(0, diff);
  // Its seems v8 has a bug in memory.grow that causes it to return
  // (uint32_t)-1 even with memory64:
  // https://bugs.chromium.org/p/v8/issues/detail?id=13948
  if (result != (uint32_t)-1 && result != (size_t)-1) {
    // Success, update JS (see https://github.com/WebAssembly/WASI/issues/82)
    emscripten_notify_memory_growth(0);
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

weak void __secs_to_zone(long long t, int local, int *isdst, long *offset, long *oppoff, const char **zonename) {
  REPORT_UNSUPPORTED(__secs_to_zone);
}

weak void _tzset_js(long* timezone, int* daylight, char* std_name, char* dst_name) {
  REPORT_UNSUPPORTED(_tzset_js);
}

// C++ ABI

#if EMSCRIPTEN_NOCATCH
// When exception catching is disabled, we stub out calls to `__cxa_throw`.
// Otherwise, `__cxa_throw` will be imported from the host.
void __cxa_throw(void* ptr, void* type, void* destructor) {
  REPORT_UNSUPPORTED(throw an exception);
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

weak void *dlopen(const char *file, int mode) {
  REPORT_UNSUPPORTED(dlopen);
  abort();
}

weak void* __dlsym(void* restrict p, const char* restrict s, void* restrict ra) {
  REPORT_UNSUPPORTED(dlsym);
  abort();
}

weak int system(const char *cmd) {
  REPORT_UNSUPPORTED(system);
  abort();
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
