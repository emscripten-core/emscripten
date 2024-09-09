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
#include <unistd.h>
#include <time.h>
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
  create_file("folder/file", "dracones", 0777);
  symlink("file", "folder/file-link");
}

void check_times(int fd, struct timespec* expected, int tolerance) {
  struct stat s;
  int rtn = fstatat(fd, "", &s, AT_EMPTY_PATH);
  assert(rtn == 0);
  printf("atime: tv_sec=%lld tv_nsec=%ld\n", s.st_atim.tv_sec, s.st_atim.tv_nsec);
  printf("mtime: tv_sec=%lld tv_nsec=%ld\n", s.st_mtim.tv_sec, s.st_mtim.tv_nsec);
  printf("expected atime: tv_sec=%lld tv_nsec=%ld\n", expected[0].tv_sec, expected[0].tv_nsec);
  printf("expected mtime: tv_sec=%lld tv_nsec=%ld\n", expected[1].tv_sec, expected[1].tv_nsec);
  if (tolerance) {
    assert(llabs(expected[0].tv_sec - s.st_atim.tv_sec) <= tolerance);
    assert(llabs(expected[1].tv_sec - s.st_mtim.tv_sec) <= tolerance);
  } else {
    assert(expected[0].tv_sec == s.st_atim.tv_sec);
    assert(expected[1].tv_sec == s.st_mtim.tv_sec);
  }
}

void test() {
  int err;
  struct stat s;

  int fd = open("folder/file", O_RDONLY);
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
  assert(s.st_size == 8);
  assert(s.st_ctime);
#if defined(__EMSCRIPTEN__) && !defined(NODERAWFS)
  assert(s.st_blksize == 4096);
  assert(s.st_blocks == 1);
#endif

  struct timespec times[2];
  times[0].tv_sec = s.st_atim.tv_sec;
  times[0].tv_nsec = s.st_atim.tv_nsec;
  times[1].tv_sec = s.st_mtim.tv_sec;
  times[1].tv_nsec = s.st_mtim.tv_nsec;

  // set the timestampe to the current value
  err = futimens(fd, times);
  assert(!err);
  check_times(fd, times, 0);

  // UTIME_OMIT means that the timeval is ignored, so
  // this call should do nothing.
  printf("check double UTIME_OMIT...\n");
  struct timespec newtimes[2];
  newtimes[0].tv_sec = 42;
  newtimes[0].tv_nsec = UTIME_OMIT;
  newtimes[1].tv_sec = 42;
  newtimes[1].tv_nsec = UTIME_OMIT;
  err = futimens(fd, newtimes);
  assert(!err);
  check_times(fd, times, 0);

  // Setting just one of the two times to UTIME_OMIT means
  // the other should be honored.
  printf("check single UTIME_OMIT...\n");
  newtimes[0].tv_sec = 41;
  newtimes[0].tv_nsec = UTIME_OMIT;
  newtimes[1].tv_sec = 42;
  newtimes[1].tv_nsec = 88;
  err = futimens(fd, newtimes);
  assert(!err);

#if defined(__EMSCRIPTEN__) && !defined(WASMFS) && !defined(NODERAWFS)
  // The original emscripten FS (in JS) only supports a single timestamp so both
  // mtime and atime will always be the same.
  times[0].tv_sec = 42;
  times[0].tv_nsec = 88;
#endif
  times[1].tv_sec = 42;
  times[1].tv_nsec = 88;
  check_times(fd, times, 0);

  // UTIME_NOW means use the current date and ignore the seconds value
  printf("check single UTIME_NOW...\n");
  newtimes[0].tv_sec = 99;
  newtimes[0].tv_nsec = UTIME_NOW;
  newtimes[1].tv_sec = 99;
  newtimes[1].tv_nsec = UTIME_NOW;
  err = futimens(fd, newtimes);
  assert(!err);

  struct timespec now;
  err = clock_gettime(CLOCK_REALTIME, &now);
  printf("now: %lld %ld\n", now.tv_sec, now.tv_nsec);
  assert(!err);
  times[0].tv_sec = now.tv_sec;
  times[0].tv_nsec = now.tv_nsec;
  times[1].tv_sec = now.tv_sec;
  times[1].tv_nsec = now.tv_nsec;
  check_times(fd, times, 1);

  close(fd);

  puts("success");
}

int main() {
  setup();
  test();
  return 0;
}
