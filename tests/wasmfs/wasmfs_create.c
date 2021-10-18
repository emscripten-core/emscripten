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
  // Test creating a new file and writing and reading from it.
  int fd = open("/test", O_RDWR | O_CREAT);
  const char* msg = "Test\n";
  write(fd, msg, strlen(msg));
  char buf[100];
  read(fd, buf, sizeof(buf));
  printf("%s", buf);
  close(fd);

  // Try to create an existing file with O_EXCL and O_CREAT
  errno = 0;
  int fd2 = open("/dev/stdin", O_RDWR | O_CREAT | O_EXCL);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == EEXIST);

  // Try to open a file with O_DIRECTORY
  errno = 0;
  int fd3 = open("/dev/stdin", O_RDWR | O_DIRECTORY);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == ENOTDIR);

  // Try to open a directory with O_DIRECTORY
  errno = 0;
  int fd4 = open("/dev", O_RDONLY | O_DIRECTORY);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == 0);

  return 0;
}
