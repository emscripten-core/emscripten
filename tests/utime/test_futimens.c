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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  mkdir("folder", 0777);
  create_file("folder/file", "dracones", 0777);
  symlink("file", "folder/file-link");
}

void test() {
  int err;
  struct stat s;

  int fd = open("folder/file", O_RDONLY);
  assert(fd >= 0);

  // empty path - with AT_EMPTY_PATH (stat's the fd itself, a file)
  memset(&s, 0, sizeof(s));
  err = fstatat(fd, "", &s, AT_EMPTY_PATH);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISREG(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 8);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  struct timespec origTimes[2];
  origTimes[0].tv_sec = (time_t)s.st_atime;
  origTimes[0].tv_nsec = origTimes[0].tv_sec * 1000;
  origTimes[1].tv_sec = (time_t)s.st_mtime;
  origTimes[1].tv_nsec = origTimes[1].tv_sec * 1000;
  err = futimens(fd, origTimes);
  assert(!err);

  close(fd);

  puts("success");
}

int main() {
  setup();
  test();
  return 0;
}
