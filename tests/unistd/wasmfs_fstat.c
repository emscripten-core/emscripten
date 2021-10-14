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

// FIXME: Individual test to verify fstat in isolation. May get merged with others later.

int main() {
  // Test opening a file and calling fstat.
  struct stat file;
  int fd = open("/dev/stdout/", O_RDONLY);
  fstat(fd, &file);
  assert(fd >= 0);

  printf("%i\n", file.st_dev);

  assert(file.st_size == 0);
  assert(file.st_dev);
  assert(file.st_ino == 3);
  assert(file.st_nlink);
  assert(file.st_uid == 0);
  assert(file.st_gid == 0);
  assert(file.st_rdev);
  assert(file.st_blocks == 0);
  assert(file.st_blksize == 4096);
  assert(file.st_atim.tv_sec == 0);
  assert(file.st_mtim.tv_sec == 0);
  assert(file.st_ctim.tv_sec == 0);

  // Test opening a directory and calling fstat.
  struct stat directory;
  int fd2 = open("/dev", O_RDONLY);
  fstat(fd2, &directory);
  assert(fd2 >= 0);

  printf("%i\n", directory.st_dev);

  assert(directory.st_size == 4096);
  assert(directory.st_dev);
  assert(directory.st_ino == 4);
  assert(directory.st_nlink);
  assert(directory.st_uid == 0);
  assert(directory.st_gid == 0);
  assert(directory.st_rdev);
  assert(directory.st_blocks == 8);
  assert(directory.st_blksize == 4096);
  assert(directory.st_atim.tv_sec == 0);
  assert(directory.st_mtim.tv_sec == 0);
  assert(directory.st_ctim.tv_sec == 0);

  return 0;
}
