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

// FIXME: Merge this standalone test back into dup.c after new FS can support
// it.

int main() {
  int f = 1;

  printf("DUP\n");
  int f2 = dup(f);

  // Print to stdout and print to fd = 3
  assert(f2 == 3);
  dprintf(f, "STDOUT\n");
  dprintf(f2, "CAN PRINT TO STDOUT WITH fd = 3\n");

  // Try calling dup with an invalid fd
  errno = 0;
  int f_invalid = dup(-1);
  assert(f_invalid == -1);
  assert(errno == EBADF);

  printf("DUP2\n");
  int f3 = 5;
  int f4 = dup2(f, f3);

  // Print to stdout and print to fd = 4
  assert(f3 == 5);
  assert(f4 == 5);
  dprintf(f, "STDOUT\n");
  dprintf(f3, "CAN PRINT TO STDOUT WITH fd = 5\n");
  dprintf(f4, "CAN PRINT TO STDOUT WITH fd = 5\n");

  // Try printing to a non-existent fd
  errno = 0;
  dprintf(10, "FD DOES NOT EXIST\n");
  assert(errno == EBADF);

  // Try setting newfd == oldfd
  int f5 = dup2(f4, 5);
  assert(f5 == 5);
  dprintf(f5, "CAN PRINT TO STDOUT WITH fd = 5\n");

  // Try calling dup2 with an invalid newfd
  errno = 0;
  f5 = dup2(f4, -1);
  assert(f5 == -1);
  // This error is not reported in the JS filesystem.
  assert(errno == EBADF);

  // Try calling dup2 with an invalid oldfd
  errno = 0;
  int f6 = dup2(-1, f5);
  assert(f6 == -1);
  // This error is not reported in the JS filesystem.
  assert(errno == EBADF);

  // Try assigning a large fd
  int f7 = 4069;
  int f8 = dup2(f4, f7);

  dprintf(f8, "CAN PRINT TO STDOUT WITH f8 = 4069\n");

  errno = 0;
  int f9 = dup(-1);
  // This error is not reported in the JS filesystem.
  assert(f9 == -1);
  assert(errno == EBADF);

  off_t offset;

  errno = 0;
  printf("DUP\n");
  mkdir("working", 0700);
  f = open("working/file", O_RDWR | O_CREAT);
  f2 = open("working/file", O_RDONLY);
  f3 = dup(f);
  printf("errno: %d\n", errno);
  printf("f: %d\n", f != f2 && f != f3);
  printf("f2,f3: %d\n", f2 != f3);

  // dup()ed file descriptors should share all flags and even seek position
  offset = lseek(f3, 0, SEEK_CUR);
  printf("1. f3 offset was %d.    Should be 0\n", (int)offset);
  offset = lseek(f, 1, SEEK_SET);
  printf("2. f  offset set to %d. Should be 1\n", (int)offset);
  offset = lseek(f2, 2, SEEK_SET);
  printf("3. f2 offset set to %d. Should be 2\n", (int)offset);
  offset = lseek(f, 0, SEEK_CUR);
  printf("4. f  offset now is %d. Should be 1\n", (int)offset);
  offset = lseek(f2, 0, SEEK_CUR);
  printf("5. f2 offset now is %d. Should be 2\n", (int)offset);
  offset = lseek(f3, 0, SEEK_CUR);
  printf("6. f3 offset now is %d. Should be 1 (and not 0)\n", (int)offset);
  offset = lseek(f3, 3, SEEK_SET);
  printf("7. f3 offset set to %d. Should be 3\n", (int)offset);
  offset = lseek(f, 0, SEEK_CUR);
  printf("8. f  offset now is %d. Should be 3 (and not 1)\n", (int)offset);

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
  // f3 is identical to f2, so this will error.
  printf("close(f3): %d\n", close(f3));
  errno = 0;

  return 0;
}
