/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  EM_ASM(
    FS.close(FS.open('test.txt', 'w'));
  );

  int fd = open("test.txt", O_CREAT, 0444);
  assert(fd > 0);
  close(fd);

  int fd1 = open("test.txt", O_WRONLY);
  int fd2 = dup(fd1);
  int fd3 = fcntl(fd1, F_DUPFD_CLOEXEC, 0);

  assert(fd1 == 3);
  assert(fd2 == 4);
  assert(fd3 == 5);
  assert(close(fd1) == 0);
  assert(write(fd2, "abcdef", 6) == 6);
  assert(close(fd2) == 0);
  assert(write(fd3, "ghijkl", 6) == 6);
  assert(close(fd3) == 0);
  printf("success\n");
  return 0;
}
