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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// FIXME: Merge with other existing close and open tests.

int main() {
  // Test writing to a file with a trailing slash.
  int fd = open("/dev/stdout/", O_WRONLY);

  dprintf(fd, "WORKING WITH TRAILING BACKSLASH\n");

  // Close open file
  close(fd);

  // Test writing to a file with no trailing backslash.
  int fd2 = open("/dev/stdout", O_WRONLY);

  dprintf(fd2, "WORKING WITHOUT TRAILING BACKSLASH\n");

  // Check that the file type is correct on mode.
  struct stat file;
  fstat(fd2, &file);

  assert((file.st_mode & S_IFMT) == S_IFCHR);
  // The JS file system assigns S_IWUGO | S_IRUGO | S_IFCHR to devices.
#ifdef WASMFS
  assert(file.st_mode == (S_IWUGO | S_IFCHR));
#else
  assert(file.st_mode == (S_IWUGO | S_IRUGO | S_IFCHR));
#endif

  // Close open file
  close(fd2);

  // Attempt to write to a non-existent fd.
  errno = 0;
  dprintf(fd, "SHOULD NOT PRINT\n");
  assert(errno == EBADF);
  printf("Errno: %s\n", strerror(errno));

  // Attempt to open and then read/write to a directory.
  int fd3 = open("/dev", O_RDONLY | O_DIRECTORY);

  // Check that the file type is correct on mode.
  struct stat dir;
  fstat(fd3, &dir);

  assert((dir.st_mode & S_IFMT) == S_IFDIR);
  // The JS file system assigns write access to /dev.
#ifdef WASMFS
  assert(dir.st_mode == (S_IRUGO | S_IXUGO | S_IFDIR));
#else
  assert(dir.st_mode == (S_IWUGO | S_IRUGO | S_IXUGO | S_IFDIR));
#endif

  const char* msg = "Test\n";

  errno = 0;
  write(fd3, msg, strlen(msg));
  // TODO: Change to assert(errno == EBADF) when access mode checking is added.

  char buf[100];

  errno = 0;
  read(fd3, buf, sizeof(buf));
  assert(errno == EISDIR);
  printf("Errno: %s\n", strerror(errno));

  errno = 0;
  // Attempt to open a non-existent file path.
  int fd4 = open("/foo", O_RDWR);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == ENOENT);

  errno = 0;
  // Attempt to open a file path with a file intermediary.
  int fd5 = open("/dev/stdout/foo", O_RDWR);
  printf("Errno: %s\n", strerror(errno));
  // Both errors are valid, but in WasmFS, ENOTDIR is returned first.
#ifdef WASMFS
  assert(errno == ENOTDIR);
#else
  assert(errno == ENOENT);
#endif

  errno = 0;
  // Attempt to open and write to the root directory.
  int fd6 = open("/", O_RDONLY);
  write(fd6, msg, strlen(msg));
  printf("Errno: %s\n", strerror(errno));
  // In Linux and WasmFS one can obtain a fd to a directory.
#ifdef WASMFS
  assert(errno == EISDIR);
#else
  assert(errno == EBADF);
#endif

  errno = 0;
  // Attempt to open a blank path.
  int fd7 = open("", O_RDONLY);
  assert(errno == ENOENT);

  return 0;
}
