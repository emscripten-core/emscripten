#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  wasmfs_create_directory("/opfs", 0777, wasmfs_create_opfs_backend());
  EM_ASM({ run_test(); });
  emscripten_exit_with_live_runtime();
}

const char* file = "/opfs/data";

// Each of these functions returns:
//   0: failure with EACCES (or other expected error code)
//   1: success
//   2: other error

static int try_open(int flags) {
  int fd = open(file, flags);
  if (fd >= 0) {
    int err = close(fd);
    assert(err == 0);
    return 1;
  }
  if (errno == EACCES) {
    return 0;
  }
  emscripten_console_error(strerror(errno));
  return 2;
}


EMSCRIPTEN_KEEPALIVE
int try_open_wronly(void) {
  return try_open(O_WRONLY);
}

EMSCRIPTEN_KEEPALIVE
int try_open_rdwr(void) {
  return try_open(O_RDWR);
}

EMSCRIPTEN_KEEPALIVE
int try_open_rdonly(void) {
  return try_open(O_RDONLY);
}

EMSCRIPTEN_KEEPALIVE
int try_truncate(void) {
  int err = truncate(file, 42);
  if (err == 0) {
    return 1;
  }
  if (errno == EIO) {
    return 0;
  }
  emscripten_console_error(strerror(errno));
  return 2;
}

EMSCRIPTEN_KEEPALIVE
int try_unlink(void) {
  int err = unlink(file);
  if (err == 0) {
    return 1;
  }
  if (errno == EIO) {
    return 0;
  }
  emscripten_console_error(strerror(errno));
  return 2;
}

EMSCRIPTEN_KEEPALIVE
int try_oob_read(void) {
  int fd = open(file, O_RDWR);
  if (fd < 0) {
    emscripten_outf("fd %d", fd);
    return 2;
  }
  char buf;
  int nread = pread(fd, &buf, 1, (off_t)-1ll);
  if (nread > 0) {
    close(fd);
    return 1;
  }
  if (errno == EINVAL) {
    close(fd);
    return 0;
  }
  emscripten_outf("errno %d", errno);
  close(fd);
  return 2;
}

EMSCRIPTEN_KEEPALIVE
int try_oob_write(void) {
  int fd = open(file, O_RDWR);
  if (fd < 0) {
    emscripten_console_error(strerror(errno));
    return 2;
  }
  char buf = 0;
  int nread = pwrite(fd, &buf, 1, (off_t)-1ll);
  if (nread > 0) {
    close(fd);
    return 1;
  }
  if (errno == EINVAL) {
    close(fd);
    return 0;
  }
  emscripten_console_error(strerror(errno));
  close(fd);
  return 2;
}

EMSCRIPTEN_KEEPALIVE
int try_rename_dir(void) {
  int err = mkdir("/opfs/dir1", 0666);
  if (err != 0) {
    return 2;
  }
  err = rename("/opfs/dir1", "/opfs/dir2");
  if (err == 0) {
    return 1;
  }
  if (errno == EBUSY) {
    rmdir("/opfs/dir1");
    return 0;
  }
  emscripten_console_error(strerror(errno));
  rmdir("/opfs/dir1");
  return 2;
}

EMSCRIPTEN_KEEPALIVE
void report_result(int result) {
  EM_ASM({ out(new Error().stack); });
#ifdef REPORT_RESULT
  REPORT_RESULT(result);
#else
  if (result != 0) {
    abort();
  }
#endif
}
