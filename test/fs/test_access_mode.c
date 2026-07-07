/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Verify that the kernel enforces the open file access mode on read/write, as
// POSIX requires: writing to an O_RDONLY descriptor and reading from an
// O_WRONLY descriptor must fail with EBADF. This should behave identically
// across all of our filesystems (MEMFS, WASMFS, NODEFS, NODERAWFS).

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
  const char* msg = "hello";
  const size_t len = strlen(msg);
  char buf[16];

  // Create a file with some content.
  int fd = open("testfile", O_WRONLY | O_CREAT | O_TRUNC, 0666);
  assert(fd >= 0);
  assert(write(fd, msg, len) == (ssize_t)len);
  assert(close(fd) == 0);

  // O_RDONLY: reads are allowed, writes must fail with EBADF.
  fd = open("testfile", O_RDONLY);
  assert(fd >= 0);

  errno = 0;
  assert(write(fd, msg, len) == -1);
  assert(errno == EBADF);

  errno = 0;
  assert(pwrite(fd, msg, len, 0) == -1);
  assert(errno == EBADF);

  errno = 0;
  memset(buf, 0, sizeof buf);
  assert(read(fd, buf, sizeof buf) == (ssize_t)len);
  assert(errno == 0);
  assert(strcmp(buf, msg) == 0);
  assert(close(fd) == 0);

  // O_WRONLY: writes are allowed, reads must fail with EBADF.
  fd = open("testfile", O_WRONLY);
  assert(fd >= 0);

  errno = 0;
  assert(read(fd, buf, sizeof buf) == -1);
  assert(errno == EBADF);

  errno = 0;
  assert(pread(fd, buf, sizeof buf, 0) == -1);
  assert(errno == EBADF);

  errno = 0;
  assert(write(fd, msg, len) == (ssize_t)len);
  assert(errno == 0);
  assert(close(fd) == 0);

  // O_RDWR: both reads and writes are allowed.
  fd = open("testfile", O_RDWR);
  assert(fd >= 0);
  errno = 0;
  memset(buf, 0, sizeof buf);
  assert(read(fd, buf, sizeof buf) == (ssize_t)len);
  assert(write(fd, msg, len) == (ssize_t)len);
  assert(errno == 0);
  assert(close(fd) == 0);

  printf("done\n");
  return 0;
}
