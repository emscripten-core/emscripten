#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main() {
  int vals[] = {
    _PC_LINK_MAX,
    _PC_MAX_CANON,
    _PC_MAX_INPUT,
    _PC_NAME_MAX,
    _PC_PATH_MAX,
    _PC_PIPE_BUF,
    _PC_REC_MIN_XFER_SIZE,
    _PC_REC_XFER_ALIGN,
    _PC_ALLOC_SIZE_MIN,
    _PC_CHOWN_RESTRICTED,
    _PC_NO_TRUNC,
    _PC_2_SYMLINKS,
    _PC_VDISABLE,
    _PC_SYNC_IO,
    _PC_ASYNC_IO,
    _PC_PRIO_IO,
    _PC_SOCK_MAXBUF,
    _PC_REC_INCR_XFER_SIZE,
    _PC_REC_MAX_XFER_SIZE,
    _PC_SYMLINK_MAX,
    _PC_FILESIZEBITS
  };
  const char* names[] = {
    "_PC_LINK_MAX",
    "_PC_MAX_CANON",
    "_PC_MAX_INPUT",
    "_PC_NAME_MAX",
    "_PC_PATH_MAX",
    "_PC_PIPE_BUF",
    "_PC_REC_MIN_XFER_SIZE",
    "_PC_REC_XFER_ALIGN",
    "_PC_ALLOC_SIZE_MIN",
    "_PC_CHOWN_RESTRICTED",
    "_PC_NO_TRUNC",
    "_PC_2_SYMLINKS",
    "_PC_VDISABLE",
    "_PC_SYNC_IO",
    "_PC_ASYNC_IO",
    "_PC_PRIO_IO",
    "_PC_SOCK_MAXBUF",
    "_PC_REC_INCR_XFER_SIZE",
    "_PC_REC_MAX_XFER_SIZE",
    "_PC_SYMLINK_MAX",
    "_PC_FILESIZEBITS"
  };

  for (int i = 0; i < sizeof vals / sizeof vals[0]; i++) {
    printf("%s: %ld\n", names[i], pathconf("/", vals[i]));
    printf("errno: %d\n\n", errno);
    errno = 0;
  }

  printf("(invalid): %ld\n", pathconf("/", -123));
  printf("errno: %d\n", errno);

  return 0;
}
