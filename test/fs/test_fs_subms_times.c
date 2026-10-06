/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A file time set with utimensat() with a sub-millisecond part reads back from
 * stat() and fstat() (utimensat keeps 10 microseconds, so allow for that).
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define SEC 1700000000
#define NSEC 123450000 // a multiple of 10 microseconds

static void check(const char* what, struct timespec t) {
  printf("%s: %lld.%09ld\n", what, (long long)t.tv_sec, t.tv_nsec);
  assert(t.tv_sec == SEC);
  assert(labs(t.tv_nsec - NSEC) < 10000);
}

int main() {
  const char* path = "subms.txt";
  int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  assert(fd >= 0);
  assert(write(fd, "x", 1) == 1);

  struct timespec times[2] = {{SEC, NSEC}, {SEC, NSEC}};
  assert(utimensat(AT_FDCWD, path, times, 0) == 0);

  struct stat st;
  assert(stat(path, &st) == 0);
  check("stat atime", st.st_atim);
  check("stat mtime", st.st_mtim);
  assert(fstat(fd, &st) == 0);
  check("fstat mtime", st.st_mtim);

  close(fd);
  puts("done");
  return 0;
}
