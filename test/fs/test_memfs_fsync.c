/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int main() {
  int fd, rtn;

  // We first make sure the file doesn't currently exist.
  // We then write a file, call fsync, and close the file,
  // to make sure synchronous calls to resume does not throw.

  struct stat st;

  // a file whose contents are just 'az'
  rtn = stat("/wakaka.txt", &st);
  assert(rtn == -1 && errno == ENOENT);

  fd = open("/wakaka.txt", O_RDWR | O_CREAT, 0666);
  assert(fd >= 0);

  rtn = write(fd, "az", 2);
  assert(rtn == 2);

  rtn = fsync(fd);
  assert(rtn == 0);

  rtn = close(fd);
  assert(rtn == 0);

  return 0;
}
