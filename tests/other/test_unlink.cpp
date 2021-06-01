// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  const char *filename = "test.dat";
  const char *dirname = "test";

  // Create a file
  FILE *f = fopen(filename, "wb");
  if (f == NULL) {
    return 1;
  }
  if (fclose(f)) {
    return 1;
  }
  // Check it exists
  if (access(filename, F_OK) != 0) {
    return 1;
  }
  // Delete the file
  if (unlinkat(AT_FDCWD, filename, 0)) {
    return 1;
  }
  // Check that it doesn't exist
  if (access(filename, F_OK) != -1) {
    return 1;
  }

  // Create a directory
  if (mkdir(dirname, 0700)) {
    return 1;
  }
  // Delete the directory
  if (unlinkat(AT_FDCWD, dirname, AT_REMOVEDIR)) {
    return 1;
  }
  // Check that it doesn't exist
  if (access(dirname, F_OK) != -1) {
    return 1;
  }

  printf("ok\n");

  return 0;
}
