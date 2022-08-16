/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int err = mkdir("/working", 0777);
  assert(err == 0);

  int fd = creat("/working/foo.txt", 0777);
  assert(fd > 0);

  err = fchown(fd, 0, 0);
  assert(err == 0);

  close(fd);

  err = chown("/working/foo.txt", 0, 0);
  assert(err == 0);

  err = lchown("/working/foo.txt", 0, 0);
  assert(err == 0);

  printf("ok\n");
}
