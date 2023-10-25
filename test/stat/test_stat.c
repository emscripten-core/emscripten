/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// For LFS functions (e.g. stat64)
#define _GNU_SOURCE 1

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <emscripten/emscripten.h>

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

#define TEST_TIME (1ll << 34)
void setup() {
  struct utimbuf t = {TEST_TIME, TEST_TIME};

  mkdir("folder", 0777);
  create_file("folder/file", "abcdef", 0777);
  symlink("file", "folder/file-link");

  utime("folder/file", &t);
  utime("folder", &t);
}

void cleanup() {
  rmdir("folder/subdir");
  unlink("folder/file");
  unlink("folder/file-link");
  rmdir("folder");
}

void test() {
  int err;
  struct stat s;
  struct utimbuf t = {TEST_TIME, TEST_TIME};

  // non-existent
  err = stat("does_not_exist", &s);
  assert(err == -1);
  assert(errno == ENOENT);

  // test stat64 LFS functions
  struct stat64 s64;
  err = stat("does_not_exist", &s64);
  assert(err == -1);
  assert(errno == ENOENT);

  // stat a folder
  memset(&s, 0, sizeof(s));
  err = stat("folder", &s);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISDIR(s.st_mode));
  assert(s.st_nlink);
#ifndef WASMFS
  assert(s.st_rdev == 0);
#endif
  assert(s.st_size);
  printf("TEST_TIME: %llx\n", TEST_TIME);
  printf("s.st_atime: %llx\n", s.st_atime);
  printf("s.st_mtime: %llx\n", s.st_mtime);
  assert(s.st_atime == TEST_TIME);
  assert(s.st_mtime == TEST_TIME);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
#ifdef WASMFS
  assert(s.st_blocks == 8);
#else
  assert(s.st_blocks == 1);
#endif
#endif
  // stat a file
  memset(&s, 0, sizeof(s));
  err = stat("folder/file", &s);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISREG(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 6);
  assert(s.st_atime == TEST_TIME);
  assert(s.st_mtime == TEST_TIME);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  // fstat a file (should match file stat from above)
  memset(&s, 0, sizeof(s));
  err = fstat(open("folder/file", O_RDONLY), &s);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISREG(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 6);
  assert(s.st_atime == TEST_TIME);
  assert(s.st_mtime == TEST_TIME);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  // stat a device
  memset(&s, 0, sizeof(s));
  err = stat("/dev/null", &s);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISCHR(s.st_mode));
  assert(s.st_nlink);
#ifndef __APPLE__
#ifndef WASMFS
  // mac uses makedev(3, 2) for /dev/null
  // WasmFS doesn't report a meaningful st_rdev.
  assert(s.st_rdev == makedev(1, 3));
#endif
#endif
  assert(!s.st_size);
  assert(s.st_atime);
  assert(s.st_mtime);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 0);
#endif

  // stat a link (should match the file stat from above)
  memset(&s, 0, sizeof(s));
  err = stat("folder/file-link", &s);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISREG(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 6);
  assert(s.st_atime == TEST_TIME);
  assert(s.st_mtime == TEST_TIME);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  // lstat a link (should NOT match the file stat from above)
  memset(&s, 0, sizeof(s));
  err = lstat("folder/file-link", &s);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISLNK(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size == 4);  // strlen("file")
  assert(s.st_atime != TEST_TIME);  // should NOT match the utime call we did for dir/file
  assert(s.st_mtime != TEST_TIME);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  // create and unlink files inside a directory and check that mtime updates
  mkdir("folder/subdir", 0777);
  utime("folder/subdir", &t);
  create_file("folder/subdir/file", "abcdef", 0777);
  err = stat("folder/subdir", &s);
  assert(s.st_mtime != TEST_TIME);
  utime("folder/subdir", &t);
  unlink("folder/subdir/file");
  err = stat("folder/subdir", &s);
  assert(s.st_mtime != TEST_TIME);

  chmod("folder/file", 0666);
  EM_ASM(
    var stats = FS.stat("folder/file");
    assert(stats.dev == 1);
    assert(stats.ino);
    assert(stats.mode == 0o100666);
    assert(stats.nlink);
    assert(stats.rdev == 0);
    assert(stats.size == 6);
    assert(stats.atime);
    assert(stats.mtime);
    assert(stats.ctime);
  );

  symlink("folder/file", "folder/symlinkfile");

  EM_ASM(
    var linkStats = FS.lstat("folder/symlinkfile");
    assert(linkStats.dev == 1);
    assert(linkStats.ino);
#if WASMFS
    assert(linkStats.mode == 0o120000);
#else
    assert(linkStats.mode == 0o120777);
#endif
    assert(linkStats.nlink);
    assert(linkStats.rdev == 0);
    assert(linkStats.size == 11);
    assert(linkStats.atime);
    assert(linkStats.mtime);
    assert(linkStats.ctime);

    var ex;
    try {
      FS.stat("nonexistent");
    } catch (err) {
      ex = err;
    }
    assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);

    try {
      FS.lstat("nonexistent");
    } catch (err) {
      ex = err;
    }
    assert(ex.name === "ErrnoError" && ex.errno === 44 /* ENOENT */);
  );
  chmod("folder/file", 0777);

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
