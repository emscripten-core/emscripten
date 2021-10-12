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
#include <unistd.h>

int main() {
  int fd = open("dev/stdout/", O_RDONLY);

  dprintf(fd, "WORKING\n");

  // Close open file
  close(fd);

  // Attempt to write to a non-existent fd.
  dprintf(fd, "FAILURE\n");

  return 0;
}
