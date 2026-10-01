/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int fd = creat("foo.txt", 0777);
  assert(fd > 0);

  int err = fdatasync(fd);
  assert(err == 0);

  close(fd);

  errno = 0;
  err = fdatasync(42);
  assert(err == -1);
  assert(errno == EBADF);

  printf("ok\n");
}
