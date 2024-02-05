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

  printf("posix_fadvise: %d\n", posix_fadvise(f, 3, 2, POSIX_FADV_DONTNEED));
  printf("errno: %d\n", errno);
  printf("\n");
  errno = 0;

  printf("posix_fallocate: %d\n", posix_fallocate(f, 3, 2));
  printf("errno: %d\n", errno);
  stat("/test", &s);
  assert(s.st_size == 6);
  memset(&s, 0, sizeof s);
  printf("\n");
  errno = 0;

  printf("posix_fallocate2: %d\n", posix_fallocate(f, 3, 7));
  printf("errno: %d\n", errno);
  stat("/test", &s);
  printf("st_size: %lld\n", s.st_size);
  memset(&s, 0, sizeof s);

  return 0;
}
