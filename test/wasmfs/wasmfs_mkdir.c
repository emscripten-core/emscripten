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

// FIXME: Merge with unlink test

int main() {
  // Try to make a directory under the root directory.
  errno = 0;
  mkdir("/working", 0777);
  mkdir("/foobar", 01777);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == 0);

  // Check that the file type is correct on mode (0777 = S_IRWXUGO)
  int fdStat = open("/working", O_RDONLY | O_DIRECTORY);
  struct stat directory;
  fstat(fdStat, &directory);
  assert((directory.st_mode & S_IFMT) == S_IFDIR);
  assert(directory.st_mode == (S_IRWXUGO | S_IFDIR));
  close(fdStat);

  // Check that the file type is correct on mode (01777 = S_ISVTX | S_IRWXUGO)
  int fdStat2 = open("/foobar", O_RDONLY | O_DIRECTORY);
  struct stat directory2;
  fstat(fdStat2, &directory2);
  assert(directory2.st_mode == (S_IRWXUGO | S_ISVTX | S_IFDIR));
  close(fdStat2);

  // Try to create a file in the same directory.
  int fd = open("/working/test", O_RDWR | O_CREAT, 0777);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == 0);

  // Try to read and write to the same file.
  const char* msg = "Test\n";
  errno = 0;
  write(fd, msg, strlen(msg));
  assert(errno == 0);
  lseek(fd, 0, SEEK_SET);
  char buf[100] = {};
  errno = 0;
  read(fd, buf, sizeof(buf));
  assert(errno == 0);
  printf("%s", buf);
  close(fd);

  // In Linux and WasmFS, an empty pathname returns ENOENT.
  errno = 0;
  mkdir("", 0777);
  assert(errno == ENOENT);

  // Try to make the root directory.
  errno = 0;
  mkdir("/", 0777);
#ifdef WASMFS
  assert(errno == EEXIST);
#else
  assert(errno == EINVAL);
#endif

  // Try to make a directory that exists already.
  errno = 0;
  mkdir("/dev", 0777);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == EEXIST);

  // Try to make a directory with a path component that is not a directory.
  errno = 0;
  mkdir("/dev/stdout/fake-directory", 0777);
  // TODO: This may have to change when access modes are implemented, depending
  // on if we check access mode before file type.
#ifdef WASMFS
  assert(errno == ENOTDIR);
#else
  assert(errno == EACCES);
#endif

  // Try to make a directory with a path component that does not exist.
  errno = 0;
  mkdir("/dev/false-path/fake-directory", 0777);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == ENOENT);

  // Try to make a directory under the `working` directory.
  errno = 0;
  mkdir("/working/new-directory", 0777);
  printf("Errno: %s\n", strerror(errno));
  assert(errno == 0);

  // Try to make a directory with a name that is longer than WASMFS_NAME_MAX.
  // In Linux, creating a directory with a name that is longer than 255
  // characters returns ENAMETOOLONG.
  errno = 0;
  mkdir("/working/"
        "0000000001000000000200000000030000000004000000000500000000060000000007"
        "0000000008000000000900000000000000000001000000000200000000030000000004"
        "0000000005000000000600000000070000000008000000000900000000000000000001"
        "0000000002000000000300000000040000000005123456",
        0777);
#ifdef WASMFS
  assert(errno == ENAMETOOLONG);
#endif

  return 0;
}
