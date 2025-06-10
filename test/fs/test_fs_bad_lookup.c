// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//--------------------------------------------------------------------------
// Helper to create an empty file with the given path.
void touch(const char* path, const mode_t mode) {
  printf("Touching file: %s with mode=%o\n", path, mode);

  int fd = open(path, O_CREAT | O_WRONLY, mode);
  if (fd == -1) {
    int error = errno;
    printf("Failed to touch file using open: %s; %s\n", path, strerror(errno));
  } else {
    close(fd);
  }
}

//--------------------------------------------------------------------------
// Stats the given path and prints the mode.  Returns true if the path
// exists; false otherwise.
bool exists(const char* path) {
  struct stat path_stat;
  if (lstat(path, &path_stat) != 0) {
    int error = errno;
    if (error == ENOENT) {
      // Only bother logging if something other than the path not existing
      // went wrong.
      printf("Failed to lstat path: %s; %s", path, strerror(error));
    }
    return false;
  }

  printf("Mode for path=%s: %o\n", path, path_stat.st_mode);
  return true;
}

int main() {
  touch("file1", 0667);
  if (!exists("file1")) {
    printf("Failed to create path: file1\n");
    return 1;
  }
  if (exists("file1/dir")) {
    printf("Path should not exists: file1/dir\n");
    return 1;
  }

  touch("file2", 0676);
  if (!exists("file2")) {
    printf("Failed to create path: file2\n");
    return 1;
  }
  if (exists("file2/dir")) {
    printf("Path should not exists: file2/dir\n");
    return 1;
  }

  touch("file3", 0766);
  if (!exists("file3")) {
    printf("Failed to create path: file3\n");
    return 1;
  }
  if (exists("file3/dir")) {
    printf("Path should not exists: file3/dir\n");
    return 1;
  }

  printf("ok.\n");
  return 0;
}

