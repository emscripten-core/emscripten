#ifdef __wasilibc_unmodified_upstream // WASI has no syscall, dup
#include "stdio_impl.h"
#else
#include <wasi/api.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#ifdef __wasilibc_unmodified_upstream // WASI has no syscall, dup
#else
// Move this below fcntl.h and unistd.h so that the __syscall macro doesn't
// cause trouble.
#include "stdio_impl.h"
#include <errno.h>
#endif

/* The basic idea of this implementation is to open a new FILE,
 * hack the necessary parts of the new FILE into the old one, then
 * close the new FILE. */

/* Locking IS necessary because another thread may provably hold the
 * lock, via flockfile or otherwise, when freopen is called, and in that
 * case, freopen cannot act until the lock is released. */

int __wasilibc_fd_renumber(int fd, int newfd) {
  __wasi_errno_t error = __wasi_fd_renumber(fd, newfd);
  if (error != 0) {
    errno = error;
    return -1;
  }
  return 0;
}

FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict f)
{
  int fl = __fmodeflags(mode);
  FILE *f2;

  FLOCK(f);

  fflush(f);

  if (!filename) {
    if (fl&O_CLOEXEC)
#ifdef __wasilibc_unmodified_upstream // WASI has no syscall
      __syscall(SYS_fcntl, f->fd, F_SETFD, FD_CLOEXEC);
#else
      fcntl(f->fd, F_SETFD, FD_CLOEXEC);
#endif
    fl &= ~(O_CREAT|O_EXCL|O_CLOEXEC);
#ifdef __wasilibc_unmodified_upstream // WASI has no syscall
    if (syscall(SYS_fcntl, f->fd, F_SETFL, fl) < 0)
#else
    if (fcntl(f->fd, F_SETFL, fl) < 0)
#endif
      goto fail;
  } else {
    f2 = fopen(filename, mode);
    if (!f2) goto fail;
    if (f2->fd == f->fd) f2->fd = -1; /* avoid closing in fclose */
#ifdef __wasilibc_unmodified_upstream // WASI has no dup
      else if (__dup3(f2->fd, f->fd, fl&O_CLOEXEC)<0) goto fail2;
#else
      // WASI doesn't have dup3, but does have a way to renumber
      // an existing file descriptor.
    else {
      if (__wasilibc_fd_renumber(f2->fd, f->fd)<0) goto fail2;
      f2->fd = -1; /* avoid closing in fclose */
    }
#endif

    f->flags = (f->flags & F_PERM) | f2->flags;
    f->read = f2->read;
    f->write = f2->write;
    f->seek = f2->seek;
    f->close = f2->close;

    fclose(f2);
  }

  FUNLOCK(f);
  return f;

  fail2:
  fclose(f2);
  fail:
  fclose(f);
  return NULL;
}

weak_alias(freopen, freopen64);
