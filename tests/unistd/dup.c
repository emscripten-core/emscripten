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
#include <emscripten.h>
#include <assert.h>
#include <string.h>


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

  printf("DUP2 err\n");
  f = dup2(-2, -2);
  printf("f: %d\n", f == -1);
  printf("errno: %d\n", errno);
  printf("close(f): %d\n", close(f));
  printf("\n");
  errno = 0;

  printf("DUP2 pipe\n");
  int p[2];
  pipe(p);
  int g = dup2(p[0], 7);
  int rtn = write(p[1], "abc", 3);
  assert(rtn == 3);
  char buf[5] = {0};
  rtn = read(g, buf, 5);
  assert(rtn == 3);
  printf("buf: %s\n", buf);
  assert(strcmp(buf, "abc") == 0);
  printf("\n");

  printf("DUP2 shared seek position\n");
  int fd1 = open("./blah.txt", O_RDWR | O_CREAT | O_EXCL, 0600);
  int fd2 = dup(fd1);
  int n = write(fd1, "abcabc\n", 7);
  assert(n == 7);
  assert(lseek(fd1, 0, SEEK_CUR) == 7);
  assert(lseek(fd2, 0, SEEK_CUR) == 7);

  return 0;
}
