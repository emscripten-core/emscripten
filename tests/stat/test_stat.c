#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  struct utimbuf t = {1200000000, 1200000000};
  
  mkdir("folder", 0777);
  create_file("folder/file", "abcdef", 0777);
  symlink("file", "folder/file-link");

  utime("folder/file", &t);
  utime("folder", &t);
}

void cleanup() {
  unlink("folder/file");
  unlink("folder/file-link");
  rmdir("folder");
}

void test() {
  int err;
  struct stat s;

  // stat a folder
  memset(&s, 0, sizeof(s));
  err = stat("folder", &s);
  assert(!err);
  assert(s.st_dev);
  assert(s.st_ino);
  assert(S_ISDIR(s.st_mode));
  assert(s.st_nlink);
  assert(s.st_rdev == 0);
  assert(s.st_size);
  assert(s.st_atime == 1200000000);
  assert(s.st_mtime == 1200000000);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
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
  assert(s.st_atime == 1200000000);
  assert(s.st_mtime == 1200000000);
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
  assert(s.st_atime == 1200000000);
  assert(s.st_mtime == 1200000000);
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
  // mac uses makedev(3, 2) for /dev/null
  assert(s.st_rdev == makedev(1, 3));
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
  assert(s.st_atime == 1200000000);
  assert(s.st_mtime == 1200000000);
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
  assert(s.st_atime != 1200000000);  // should NOT match the utime call we did for dir/file
  assert(s.st_mtime != 1200000000);
  assert(s.st_ctime);
#ifdef __EMSCRIPTEN__
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
