/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int f, f2, f3;

  printf("DUP\n");
  f = open("/", O_RDONLY);
  f2 = open("/", O_RDONLY);
  f3 = dup(f);
  printf("errno: %d\n", errno);
  printf("f: %d\n", f != f2 && f != f3);
  printf("f2,f3: %d\n", f2 != f3);
  printf("close(f1): %d\n", close(f));
  printf("close(f2): %d\n", close(f2));
  printf("close(f3): %d\n", close(f3));
  printf("\n");
  errno = 0;

  printf("DUP2\n");
  f = open("/", O_RDONLY);
  f2 = open("/", O_RDONLY);
  f3 = dup2(f, f2);
  printf("errno: %d\n", errno);
  printf("f: %d\n", f != f2 && f != f3);
  printf("f2,f3: %d\n", f2 == f3);
  printf("close(f1): %d\n", close(f));
  printf("close(f2): %d\n", close(f2));
  printf("close(f3): %d\n", close(f3));
  printf("\n");
  errno = 0;

  return 0;
}
