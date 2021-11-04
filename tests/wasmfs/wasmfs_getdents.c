/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

void print(struct dirent d, int fd) {
  for (;;) {
    int nread = getdents(fd, &d, sizeof(d));
    if (nread == 0) {
      break;
    }
    printf("d.d_name = %s\n", d.d_name);
    printf("d.d_off = %lld\n", d.d_off);
    printf("d.d_reclen = %hu\n", d.d_reclen);
    printf("d.d_type = %hhu\n\n", d.d_type);
  }
}

int main() {
  // Set up test directories.
  assert(mkdir("working", 0777) != -1);
  assert(mkdir("/working/test", 0777) != -1);

  struct dirent d;

  // Try opening the directory that was just created.
  int fd = open("/working", O_RDONLY | O_DIRECTORY);
  printf("------------- Reading from /working Directory -------------\n");
  print(d, fd);

  // Try reading an invalid fd.
  errno = 0;
  getdents(-1, &d, sizeof(d));
  printf("Errno: %s\n", strerror(errno));
  assert(errno == EBADF);

  // Try passing in a size that is too small.
  // Existing file system doesn't catch this error.
  // https://man7.org/linux/man-pages/man2/getdents.2.html#ERRORS
  errno = 0;
  getdents(fd, &d, sizeof(d) - 1);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == EINVAL);

  // Try to read from a file.
  int fileFd = open("/dev/stdout", O_RDONLY);
  getdents(fileFd, &d, sizeof(d));
  printf("Errno: %s\n\n", strerror(errno));
  assert(errno == ENOTDIR);

  // Try opening the root directory and read its contents.
  fd = open("/", O_RDONLY | O_DIRECTORY);

  printf("------------- Reading from root Directory -------------\n");
  print(d, fd);

  // Try opening the dev directory and read its contents.
  fd = open("/dev", O_RDONLY | O_DIRECTORY);
  printf("------------- Reading from /dev Directory -------------\n");
  print(d, fd);
}
