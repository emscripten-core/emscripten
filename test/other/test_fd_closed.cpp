// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() {
  const char *filename = "test.dat";

  // Create a file
  int fd = open(filename, O_CREAT|O_WRONLY);
  if (fd == -1) {
    return 1;
  }
  // Write to it
  if (write(fd, "XXXX", 4) != 4) {
    return 2;
  }
  // Close it
  if (close(fd)) {
    return 3;
  }
  // This write should fail
  if (write(fd, "YYYY", 4) != -1) {
    return 4;
  }
  // The error number is EBADF
  if (errno != EBADF) {
    return 5;
  }

  printf("ok\n");

  return 0;
}
