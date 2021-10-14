/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// FIXME: Merge with other existing close and open tests.

int main() {
  int fd = open("/dev/stdout/", O_RDONLY);

  dprintf(fd, "WORKING\n");

  // Close open file
  close(fd);

  // Attempt to write to a non-existent fd.
  errno = 0;
  dprintf(fd, "SHOULD NOT PRINT\n");
  assert(errno == EBADF);
  printf("Errno: %s\n", strerror(errno));

  // Attempt to open and then read/write to a directory.
  int fd2 = open("/dev", O_RDONLY);

  const char* msg = "Test\n";

  errno = 0;
  int result = write(fd2, msg, strlen(msg));
  assert(errno == EISDIR);
  printf("Errno: %s\n", strerror(errno));

  char buf[100];

  errno = 0;
  int bytes = read(fd2, buf, sizeof(buf));
  assert(errno == EISDIR);
  printf("Errno: %s\n", strerror(errno));

  errno = 0;
  // Attempt to open a non-existent file path.
  int fd3 = open("/nonsense", O_RDONLY);
  assert(errno == EINVAL);
  printf("Errno: %s\n", strerror(errno));

  return 0;
}
