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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>


void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  mkdir("folder", 0777);
  create_file("folder/file", "abcdef", 0777);
  symlink("file", "folder/file-link");
}

void test() {
  int err;
  struct stat s;

  int fd = open(".", O_RDONLY);
  assert(fd >= 0);

  // missing file
  err = fstatat(fd, "does_not_exist", &s, 0);
  assert(err == -1);
  printf("errno: %d\n", errno);
  assert(errno == ENOENT);

  // stat a folder
  memset(&s, 0, sizeof(s));
  err = fstatat(fd, "folder", &s, 0);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISDIR(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  // WasmFS correctly counts 512B blocks, but MEMFS counts 4kb blocks.
#ifdef WASMFS
  assert(s.st_blocks == 8);
#else
  assert(s.st_blocks == 1);
#endif
#endif

  // stat a file
  memset(&s, 0, sizeof(s));
  err = fstatat(fd, "folder/file", &s, 0);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISREG(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 6);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  close(fd);

  fd = open("folder", O_RDONLY);
  assert(fd >= 0);

  // missing file
  err = fstatat(fd, "does_not_exist", &s, 0);
  assert(err == -1);
  assert(errno == ENOENT);

  // stat a file
  memset(&s, 0, sizeof(s));
  err = fstatat(fd, "file", &s, 0);
  assert(!err);

  // empty path - not allowed
  err = fstatat(fd, "", &s, 0);
  assert(err == -1);
  assert(errno == ENOENT);

  // empty path - with AT_EMPTY_PATH (stat's the fd itself, a directory in this case)
  memset(&s, 0, sizeof(s));
  err = fstatat(fd, "", &s, AT_EMPTY_PATH);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISDIR(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
#ifdef WASMFS
  assert(s.st_blocks == 8);
#else
  assert(s.st_blocks == 1);
#endif
#endif

  close(fd);

  fd = open("folder/file", O_RDONLY);
  assert(fd >= 0);

  // empty path - with AT_EMPTY_PATH (stat's the fd itself, a file)
  memset(&s, 0, sizeof(s));
  err = fstatat(fd, "", &s, AT_EMPTY_PATH);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISREG(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 6);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  close(fd);

  // lstat a link - with AT_FDCWD and AT_SYMLINK_NOFOLLOW.
  // Also test that AT_NO_AUTOMOUNT is ignored.
  memset(&s, 0, sizeof(s));
  err = fstatat(AT_FDCWD, "folder/file-link", &s, AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISLNK(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 4);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  puts("success");
}

int main() {
  setup();
  test();
  return 0;
}
