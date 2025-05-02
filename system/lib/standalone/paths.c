#define _GNU_SOURCE
#include "paths.h"

#include <dirent.h>
#include <fcntl.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "lock.h"

/// A name and file descriptor pair.
typedef struct preopen {
  /// The path prefix associated with the file descriptor.
  const char* prefix;

  /// The file descriptor.
  __wasi_fd_t fd;

  /// Device ID of device containing the file.
  __wasi_device_t dev;

  /// File serial number.
  __wasi_inode_t ino;
} preopen;

/// A simple growable array of `preopen`.
static preopen* preopens;
static size_t num_preopens;
static size_t preopen_capacity;

/// cwd handling
static bool cwd_is_root = true;
static __wasi_fd_t cwd_fd;
static bool cwd_fd_from_preopen;

/// Access to the cwd above must be protected.
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
    const preopen* pre = &preopens[i];
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
static bool resize_preopens(void) {
  size_t start_capacity = 4;
  size_t old_capacity = preopen_capacity;
  size_t new_capacity = old_capacity == 0 ? start_capacity : old_capacity * 2;

  preopen* old_preopens = preopens;
  preopen* new_preopens = calloc(sizeof(preopen), new_capacity);
  if (new_preopens == NULL) {
    return false;
  }

  memcpy(new_preopens, old_preopens, num_preopens * sizeof(preopen));
  preopens = new_preopens;
  preopen_capacity = new_capacity;
  free(old_preopens);

  assert_invariants();
  return true;
}

// Normalize an absolute path. Removes leading `/` and leading `./`, so the
// first character is the start of a directory name. This works because our
// process always starts with a working directory of `/`. Additionally translate
// `.` to the empty string.
static const char* strip_prefixes(const char* path) {
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
static bool register_preopened_fd(__wasi_fd_t fd,
                                  const char* relprefix,
                                  __wasi_device_t dev,
                                  __wasi_inode_t ino) {
  // Check preconditions.
  assert_invariants();
  assert(fd != AT_FDCWD);
  assert(fd != -1);
  assert(relprefix != NULL);

  if (num_preopens == preopen_capacity && !resize_preopens()) {
    return false;
  }

  char* prefix = strdup(strip_prefixes(relprefix));
  if (prefix == NULL) {
    return false;
  }

  preopens[num_preopens++] = (preopen){prefix, fd, dev, ino};

  assert_invariants();
  return true;
}

/// Are the `prefix_len` bytes pointed to by `prefix` a prefix of `path`?
static bool
prefix_matches(const char* prefix, size_t prefix_len, const char* path) {
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

static bool resolve_path_unlocked(bool* need_unlock,
                                  int* resolved_dirfd,
                                  const char** path_ptr) {
  const char* path = *path_ptr;

  if (*resolved_dirfd != AT_FDCWD && path[0] != '/') {
    *need_unlock = false;
    return true;
  }

  bool is_absolute = path[0] == '/';

  // Strip leading `/` characters, the prefixes we're mataching won't have
  // them.
  while (*path == '/')
    path++;

  if (!cwd_is_root && !is_absolute) {
    assert(*resolved_dirfd == AT_FDCWD);

    *need_unlock = !cwd_fd_from_preopen;
    *resolved_dirfd = cwd_fd;
    *path_ptr = path;
    return true;
  }

  // Search through the preopens table. Iterate in reverse so that more
  // recently added preopens take precedence over less recently addded ones.
  size_t match_len = 0;
  int fd = -1;
  for (size_t i = num_preopens; i > 0; --i) {
    const preopen* pre = &preopens[i - 1];
    const char* prefix = pre->prefix;
    size_t len = strlen(prefix);

    // If we haven't had a match yet, or the candidate path is longer than
    // our current best match's path, and the candidate path is a prefix of
    // the requested path, take that as the new best path.
    if ((fd == -1 || len > match_len) && prefix_matches(prefix, len, path)) {
      fd = pre->fd;
      match_len = len;
    }
  }

  if (fd == -1) {
    return false;
  }

  // The relative path is the substring after the portion that was matched.
  const char* computed = path + match_len;

  // Omit leading slashes in the relative path.
  while (*computed == '/')
    ++computed;

  // *at syscalls don't accept empty relative paths, so use "." instead.
  if (*computed == '\0')
    computed = ".";

  *need_unlock = false;
  *resolved_dirfd = fd;
  *path_ptr = computed;
  return true;
}

bool __paths_resolve_path(bool* need_unlock,
                          int* resolved_dirfd,
                          const char** path_ptr) {
  // fast path
  if (*resolved_dirfd != AT_FDCWD && (*path_ptr)[0] != '/') {
    *need_unlock = false;
    return true;
  }

  LOCK(lock);
  bool ret = resolve_path_unlocked(need_unlock, resolved_dirfd, path_ptr);
  if (!*need_unlock) {
    UNLOCK(lock);
  }
  return ret;
}

void __paths_unlock() { UNLOCK(lock); }

static void change_cwd_with_fd_unlocked(int newfd,
                                        __wasi_device_t dev,
                                        __wasi_inode_t ino) {
  if (cwd_is_root) {
    cwd_is_root = false;
  } else {
    if (!cwd_fd_from_preopen) {
      __wasi_fd_close(cwd_fd);
    }
  }

  for (size_t i = 0; i < num_preopens; ++i) {
    const preopen* pre = &preopens[i];

    if (pre->dev == dev && pre->ino == ino) {
      __wasi_fd_close(newfd);

      if (*pre->prefix == '\0') {
        cwd_is_root = true;
      } else {
        cwd_fd = pre->fd;
        cwd_fd_from_preopen = true;
      }

      return;
    }
  }

  cwd_fd = newfd;
  cwd_fd_from_preopen = false;
}

static int change_cwd_unlocked(int dirfd, const char* path) {
  __wasi_errno_t error;

  int newdir = openat(dirfd, path, O_DIRECTORY | O_SEARCH);
  if (newdir == -1) {
    error = errno;
    goto out;
  }

  __wasi_filestat_t sb;
  error = __wasi_fd_filestat_get(newdir, &sb);
  if (error != __WASI_ERRNO_SUCCESS) {
    __wasi_fd_close(newdir);
    goto out;
  }

  change_cwd_with_fd_unlocked(newdir, sb.dev, sb.ino);
  error = __WASI_ERRNO_SUCCESS;

out:
  return error;
}

__wasi_errno_t __paths_chdir(const char* path) {
  __wasi_errno_t error;

  LOCK(lock);

  int dirfd = AT_FDCWD;

  bool need_unlock;
  bool ret = resolve_path_unlocked(&need_unlock, &dirfd, &path);
  if (!ret) {
    error = __WASI_ERRNO_NOENT;
    goto out;
  }
  (void)need_unlock;

  error = change_cwd_unlocked(dirfd, path);

out:
  UNLOCK(lock);
  return error;
}

__wasi_errno_t __paths_fchdir(int fd) {
  if (fd == AT_FDCWD) {
    return EBADF;
  }

  LOCK(lock);
  __wasi_errno_t error = change_cwd_unlocked(fd, ".");
  UNLOCK(lock);
  return error;
}

struct buf {
  char* buf;
  size_t size;
  size_t capa;
};

static __wasi_errno_t buf_prepend(struct buf* buf, const char* str) {
  size_t str_length = strlen(str);
  size_t newsize = buf->size + str_length;
  if (newsize > buf->capa) {
    size_t newcapa = buf->capa != 0 ? buf->capa : 16;
    while (newcapa < newsize) {
      newcapa *= 2;
    }

    char* newbuf = realloc(buf->buf, newcapa);
    if (!newbuf) {
      return errno;
    }
    buf->buf = newbuf;
    buf->capa = newcapa;
  }

  memmove(&buf->buf[str_length], &buf->buf[0], buf->size);
  memcpy(&buf->buf[0], str, str_length);
  buf->size = newsize;

  return __WASI_ERRNO_SUCCESS;
}

static bool buf_empty(const struct buf* buf) { return buf->size == 0; }

static __wasi_errno_t
calculate_cwd_path(char** out_buf, size_t* out_size, __wasi_fd_t fd) {
  __wasi_errno_t error;
  struct buf buf = {NULL, 0, 0};

  int parent_for_search = -1;

  for (;;) {
    __wasi_device_t dev;
    __wasi_inode_t ino;
    {
      __wasi_filestat_t sb;
      error = __wasi_fd_filestat_get(fd, &sb);
      if (error != __WASI_ERRNO_SUCCESS) {
        goto out;
      }
      dev = sb.dev;
      ino = sb.ino;
    }

    {
      int new_parent = openat(fd, "..", O_DIRECTORY | O_SEARCH);
      if (new_parent == -1) {
        error = errno;
        goto out;
      }

      if (parent_for_search != -1) {
        __wasi_fd_close(parent_for_search);
      }
      parent_for_search = new_parent;
    }

    int parent = openat(parent_for_search, ".", O_DIRECTORY | O_RDONLY);
    if (parent == -1) {
      error = errno;
      goto out;
    }

    __wasi_device_t parent_dev;
    __wasi_inode_t parent_ino;
    {
      __wasi_filestat_t sb;
      error = __wasi_fd_filestat_get(parent, &sb);
      if (error != __WASI_ERRNO_SUCCESS) {
        __wasi_fd_close(parent);
        goto out;
      }
      parent_dev = sb.dev;
      parent_ino = sb.ino;
    }

    if (parent_dev != dev) {
      error = __WASI_ERRNO_NOENT;
      __wasi_fd_close(parent);
      goto out;
    }

    DIR* parentdir = fdopendir(parent);
    if (!parentdir) {
      error = errno;
      __wasi_fd_close(parent);
      goto out;
    }

    errno = 0;
    struct dirent* dent;
    bool found = false;
    while ((dent = readdir(parentdir)) != NULL) {
      if (dent->d_ino == ino) {
        if (!buf_empty(&buf)) {
          error = buf_prepend(&buf, "/");
          if (error != __WASI_ERRNO_SUCCESS) {
            closedir(parentdir);
            goto out;
          }
        }

        error = buf_prepend(&buf, dent->d_name);
        if (error != __WASI_ERRNO_SUCCESS) {
          closedir(parentdir);
          goto out;
        }

        found = true;
        break;
      }
    }
    error = errno;
    closedir(parentdir);
    if (error != __WASI_ERRNO_SUCCESS) {
      goto out;
    }
    if (!found) {
      error = __WASI_ERRNO_NOENT;
      goto out;
    }

    for (size_t i = 0; i < num_preopens; ++i) {
      const preopen* pre = &preopens[i];
      if (parent_dev == pre->dev && parent_ino == pre->ino) {
        // We have reached a root.

        error = buf_prepend(&buf, "/");
        if (error != __WASI_ERRNO_SUCCESS) {
          goto out;
        }

        error = buf_prepend(&buf, pre->prefix);
        if (error != __WASI_ERRNO_SUCCESS) {
          goto out;
        }

        if (*pre->prefix != '\0') {
          error = buf_prepend(&buf, "/");
        }
        goto out;
      }
    }

    fd = parent_for_search;
  }

out:
  if (parent_for_search != -1) {
    __wasi_fd_close(parent_for_search);
  }
  if (error != __WASI_ERRNO_SUCCESS) {
    free(buf.buf);
  } else {
    *out_buf = buf.buf;
    *out_size = buf.size;
  }
  return error;
}

__wasi_errno_t __paths_getcwd(char* buf, size_t* size) {
  __wasi_errno_t error;

  LOCK(lock);
  if (cwd_is_root) {
    if (*size < 2) {
      error = __WASI_ERRNO_RANGE;
      goto out;
    }
    buf[0] = '/';
    buf[1] = '\0';
    *size = 2;
    error = __WASI_ERRNO_SUCCESS;
    goto out;
  }

  if (cwd_fd_from_preopen) {
    for (size_t i = 0; i < num_preopens; ++i) {
      const preopen* pre = &preopens[i];
      if (pre->fd == cwd_fd) {
        size_t cwd_len = strlen(pre->prefix);
        if (1 + cwd_len + 1 > *size) {
          error = __WASI_ERRNO_RANGE;
          goto out;
        }

        buf[0] = '/';
        strcpy(&buf[1], pre->prefix);
        *size = 1 + cwd_len + 1;

        error = __WASI_ERRNO_SUCCESS;
        goto out;
      }
    }
    error = __WASI_ERRNO_NOENT;
    assert(false);
  } else {
    char* cwd;
    size_t cwd_size;

    error = calculate_cwd_path(&cwd, &cwd_size, cwd_fd);
    if (error != __WASI_ERRNO_SUCCESS) {
      goto out;
    }

    if (cwd_size + 1 > *size) {
      free(cwd);
      error = __WASI_ERRNO_RANGE;
      goto out;
    }

    memcpy(buf, cwd, cwd_size);
    free(cwd);
    buf[cwd_size] = '\0';
    *size = cwd_size + 1;

    error = __WASI_ERRNO_SUCCESS;
  }

out:
  UNLOCK(lock);
  return error;
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
        char* prefix = malloc(prestat.u.dir.pr_name_len + 1);
        if (prefix == NULL)
          goto software;

        // TODO: Remove the cast on `path` once the witx is updated with
        // char8 support.
        ret = __wasi_fd_prestat_dir_name(
          fd, (uint8_t*)prefix, prestat.u.dir.pr_name_len);
        if (ret != __WASI_ERRNO_SUCCESS)
          goto oserr;
        prefix[prestat.u.dir.pr_name_len] = '\0';

        __wasi_filestat_t fsb_cur;
        ret = __wasi_path_filestat_get(fd, 0, ".", 1, &fsb_cur);
        if (ret != __WASI_ERRNO_SUCCESS)
          goto oserr;

        if (!register_preopened_fd(fd, prefix, fsb_cur.dev, fsb_cur.ino))
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
