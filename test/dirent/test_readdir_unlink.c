/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  if (mkdir("test", 0777) != 0) {
    printf("Unable to create dir 'test'\n");
    return 1;
  }

  for (int i = 0; i < 10; i++) {
    char path[10];
    snprintf(path, 10, "test/%d", i);
    int fd = open(path, O_CREAT, 0777);
    if (fd < 0) {
      printf("Unable to create file '%s'\n", path);
      return 1;
    }
    close(fd);
  }

  DIR* dir = opendir("test");
  if (!dir) {
    printf("Unable to open dir 'test'\n");
    return 1;
  }

  struct dirent* dirent;

  while ((dirent = readdir(dir)) != NULL) {
    if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
      continue;
    }
    char path[10];
    snprintf(path, 10, "test/%s", dirent->d_name);
    if (unlink(path)) {
      printf("Unable to unlink '%s'\n", path);
      return 1;
    }
    printf("Unlinked '%s'\n", path);

    // Add a new file in the middle. This will not be visited (although it would
    // be valid to visit it).
    if (strcmp(dirent->d_name, "5") == 0) {
      int fd = open("test/X", O_CREAT, 0777);
      if (fd < 0) {
        printf("Unable to create file 'test/X'\n");
        return 1;
      }
      close(fd);
    }
  }

  closedir(dir);

  // The new file should still exist.
  if (access("test/X", F_OK) != 0) {
    printf("Expected file 'test/X' to exist\n");
    return 1;
  }
  if (unlink("test/X")) {
    printf("Unable to unlink 'test/X'\n");
    return 1;
  }

  if (rmdir("test")) {
    printf("Unable to remove dir 'test'\n");
    return 1;
  }

  printf("success\n");

  return 0;
}
