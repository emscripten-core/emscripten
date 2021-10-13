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
#include <sys/stat.h>
#include <unistd.h>

int main() {
  struct stat s;
  int fd = open("dev/stdout/", O_RDONLY);
  fstat(fd, &s);
  assert(fd >= 0);

  printf("%i\n", s.st_dev);

  assert(s.st_dev);
  assert(s.st_ino == 3);
  assert(s.st_nlink);
  assert(s.st_rdev);
  assert(s.st_blksize == 4096);
  assert(s.st_uid == 0);
  assert(s.st_gid == 0);

  return 0;
}
