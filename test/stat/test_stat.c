/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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

  // printf("size of: %lu\n", sizeof(struct stat));
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

// #if WASMFS
//   EM_ASM(
//     var stats = FS.stat("folder");
//     console.log("Folder: " + stats);
//   );
// #else
//   EM_ASM(
//     var stats = FS.stat("folder");
//     console.log(stats);
//     assert(stats.dev);
//     assert(stats.ino);
//   );
// #endif
  
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

  printf("- stdev: %u\n", s.st_dev);
  printf("- stnlink: %lu\n", s.st_nlink);
  printf("- stino: %llu\n", s.st_ino);
  printf("- stmode: %u\n", s.st_mode);
  printf("- st_atime: %llu\n", s.st_atime);
  printf("- st_mtime: %llu\n", s.st_mtime);
  printf("- st_ctime: %llu\n", s.st_ctime);
  printf("- st_blksize: %d\n", s.st_blksize);
  printf("- st_blocks: %d\n", s.st_blocks);
  printf("- st_gid: %u\n", s.st_gid);
  printf("- st_rdev: %u\n", s.st_rdev);
  printf("- st_size: %llu\n", s.st_size);
  printf("- st_uid: %u\n", s.st_uid);

  // printf("p stdev: %lu\n", (unsigned long)&s.st_dev);
  // printf("p stnlink: %lu\n", (unsigned long)&s.st_nlink);
  // printf("p stino: %lu\n", (unsigned long)&s.st_ino);
  // printf("p stmode: %lu\n", (unsigned long) &s.st_mode);
  // printf("p st_atime: %lu\n", (unsigned long) &s.st_atime);
  // printf("p st_mtime: %lu\n", (unsigned long) &s.st_mtime);
  // printf("p st_ctime: %lu\n", (unsigned long) &s.st_ctime);
  // printf("p st_blksize: %lu\n", (unsigned long) &s.st_blksize);
  // printf("p st_blocks: %lu\n", (unsigned long) &s.st_blocks);
  // printf("p st_gid: %lu\n", (unsigned long) &s.st_gid);
  // printf("p st_rdev: %lu\n", (unsigned long) &s.st_rdev);
  // printf("p st_size: %lu\n", (unsigned long) &s.st_size);
  // printf("p st_uid: %lu\n", (unsigned long) &s.st_uid);

  // printf("sizes: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %lu, %lu, %lu, %lu, %d\n",
  // sizeof(s.st_dev), sizeof(s.st_mode), sizeof(s.st_nlink),
  // sizeof(s.st_uid), sizeof(s.st_gid), sizeof(s.st_rdev), sizeof(s.st_size),
  // sizeof(s.st_blksize), sizeof(s.st_blocks), sizeof(s.st_atime), sizeof(s.st_atimensec),
  // sizeof(s.st_mtime), sizeof(s.st_mtimensec), 
  // sizeof(s.st_ctime), sizeof(s.st_ctimensec), (s.st_ino));
  
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

  EM_ASM(
    var stats = FS.stat("folder/file");
    console.log("before chmod stats: %o", stats);
    assert(stats.dev == 1);
    assert(stats.ino);
    assert(stats.mode == 33279);
    assert(stats.nlink == 1);
    assert(stats.rdev == 0);
    assert(stats.size == 6);
    assert(stats.atime);
    assert(stats.mtime);
    assert(stats.ctime);

    FS.symlink("file", "folder/symlinkfile");
    FS.chmod("folder/file", 0o000);

    var stats = FS.stat("folder/file");
    // console.log("recv stats: %o", stats);
    assert(stats.dev == 1);
    assert(stats.ino);
    assert(stats.mode);
    assert(stats.nlink);
    assert(stats.rdev == 0);
    assert(stats.size == 6);
    assert(stats.atime);
    assert(stats.mtime);
    assert(stats.ctime);

    var original_mode = stats.mode;

    var linkStats = FS.lstat("folder/symlinkfile");
    console.log("Link: ", linkStats);
    assert(linkStats.dev == 1);
    assert(linkStats.ino);
    assert(linkStats.mode != original_mode);
    assert(linkStats.nlink);
    assert(linkStats.rdev == 0);
    assert(linkStats.size == 4);
    assert(linkStats.atime);
    assert(linkStats.mtime);
    assert(linkStats.ctime);

    FS.chmod("folder/file", 0o777);
  );

  lstat("folder/symlinkfile", &s);
  printf("Link- stdev: %u\n", s.st_dev);
  printf("Link- stnlink: %lu\n", s.st_nlink);
  printf("Link- stino: %llu\n", s.st_ino);
  printf("Link- stmode: %u\n", s.st_mode);
  printf("Link- st_atime: %llu\n", s.st_atime);
  printf("Link- st_mtime: %llu\n", s.st_mtime);
  printf("Link- st_ctime: %llu\n", s.st_ctime);
  printf("Link- st_blksize: %d\n", s.st_blksize);
  printf("Link- st_blocks: %d\n", s.st_blocks);
  printf("Link- st_gid: %u\n", s.st_gid);
  printf("Link- st_rdev: %u\n", s.st_rdev);
  printf("Link- st_size: %llu\n", s.st_size);
  printf("Link- st_uid: %u\n", s.st_uid);

  EM_ASM(
    FS.stat("nonexistent");

    FS.lstat("nonexistent");
  );

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
