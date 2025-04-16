/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main() {
  struct stat s;
  int f = open("/test", O_RDWR, 0777);
  assert(f);

  int err = posix_fadvise(f, 3, 2, POSIX_FADV_DONTNEED);
  printf("posix_fadvise: %d\n", err);
  assert(err == 0);
  printf("\n");

  err = posix_fallocate(f, 3, 2);
  printf("posix_fallocate: %d\n", err);
  assert(err == 0);
  printf("errno: %d\n", errno);
  stat("/test", &s);
  assert(s.st_size == 6);
  memset(&s, 0, sizeof s);
  printf("\n");

  err = posix_fallocate(f, 3, 7);
  printf("posix_fallocate 2: %d\n", err);
  assert(err == 0);
  stat("/test", &s);
  printf("st_size: %lld\n", s.st_size);
  printf("\n");

  err = posix_fallocate(f, -1, 7);
  printf("posix_fallocate 3: %s\n", strerror(err));
  printf("\n");

  err = posix_fallocate(f, 3, -1);
  printf("posix_fallocate 4: %s\n", strerror(err));

  return 0;
}
