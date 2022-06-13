/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  // Set up test directories.
  assert(mkdir("working", 0777) != -1);
  assert(mkdir("/working/test", 0777) != -1);

  // Try to print the root directory.
  char cwd[100];
  char* ret;
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir: %s\n", cwd);

  // Try to switch to /working.
  chdir("working");
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir: %s\n", cwd);

  // Try to switch back to the root directory.
  chdir("/");
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir: %s\n", cwd);

  // Try to switch to a subdirectory of working.
  chdir("/working/test");
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir: %s\n", cwd);

  // Try to switch to a non-existent relative path from subdirectory.
  // Changing to working in /working/test should fail.
  chdir("working");
  printf("Errno: %s\n", strerror(errno));
  assert(errno == ENOENT);
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir is still: %s\n", cwd);

  // Try to switch back to absolute path /working.
  errno = 0;
  chdir("/working");
  printf("Errno: %s\n", strerror(errno));
  assert(errno == 0);
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir is now: %s\n", cwd);

  // Try to switch to a non-existent absolute path.
  errno = 0;
  chdir("/foobar");
  printf("Errno: %s\n", strerror(errno));
  assert(errno == ENOENT);
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir is still: %s\n", cwd);

  // Try to switch to /dev.
  chdir("/dev");
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir: %s\n", cwd);

  // Try to change cwd to a file.
  chdir("/dev/stdout");
  printf("Errno: %s\n", strerror(errno));
  assert(errno == ENOTDIR);
  ret = getcwd(cwd, sizeof(cwd));
  assert(ret == cwd);
  printf("Current working dir is still: %s\n", cwd);

  // Try to pass a size of 0.
  errno = 0;
  getcwd(cwd, 0);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == EINVAL);

  // Try to write to a buffer of size 1.
  errno = 0;
  char smallBuffer[1];
  getcwd(smallBuffer, 1);
  assert(errno == ERANGE);
  return 0;
}
