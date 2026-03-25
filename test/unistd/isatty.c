/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
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

#ifndef EXPECT_STDIN
#define EXPECT_STDIN 1
#endif

#ifndef EXPECT_STDOUT
#define EXPECT_STDOUT 1
#endif

int main() {
  printf("EXPECT_STDIN: %d\n", EXPECT_STDIN);
  printf("EXPECT_STDOUT: %d\n", EXPECT_STDOUT);

  assert(isatty(0) == EXPECT_STDIN);
  assert(isatty(1) == EXPECT_STDOUT);

  int err, fd;

  assert(isatty(-1) == 0);
  assert(errno == EBADF);

  fd = open("/dev/stdin", O_RDONLY);
  assert(fd >= 0);
  assert(isatty(fd) == EXPECT_STDIN);

  fd = open("/dev/stdout", O_RDONLY);
  assert(fd >= 0);
  assert(isatty(fd) == EXPECT_STDOUT);

  fd = open("/dev/null", O_RDONLY);
  assert(fd >= 0);
  assert(isatty(fd) == 0);

  fd = open("/dev", O_RDONLY);
  assert(fd >= 0);
  assert(isatty(fd) == 0);

  puts("success");

  return EXIT_SUCCESS;
}
