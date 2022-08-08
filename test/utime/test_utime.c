/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

void setup() {
  mkdir("writeable", 0777);
  mkdir("unwriteable", 0111);
}

void cleanup() {
  rmdir("writeable");
  rmdir("unwriteable");
}

void test() {
  struct stat s;
  // currently, the most recent timestamp is shared for atime,
  // ctime and mtime. using unique values for each in the test
  // will fail
  struct utimbuf t = {1000000000, 1000000000};

  errno = 0;
  int rv = utime("writeable", &t);
  assert(rv == 0);
  assert(!errno);
  memset(&s, 0, sizeof s);
  rv = stat("writeable", &s);
  assert(rv == 0);
  assert(s.st_atime == t.actime);
  assert(s.st_mtime == t.modtime);

  // NULL sets atime and mtime to current time.
  time_t now = time(NULL);
  rv = utime("writeable", NULL);
  assert(rv == 0);
  memset(&s, 0, sizeof s);
  stat("writeable", &s);
  assert(s.st_atime == s.st_mtime);
  time_t diff = s.st_atime - now;
  if (llabs(diff) > 5) {
    fprintf(stderr, "st_atime: %" PRId64 ", now: %" PRId64 ", diff: %" PRId64 "\n ", s.st_atime, now, diff);
    assert(llabs(diff) <= 5);
  }

  // write permissions aren't checked when setting node
  // attributes unless the user uid isn't the owner (so
  // therefor, this should work fine)
  rv = utime("unwriteable", &t);
  assert(rv == 0);
  assert(!errno);
  memset(&s, 0, sizeof s);
  rv = stat("unwriteable", &s);
  assert(rv == 0);
  assert(s.st_atime == t.actime);
  assert(s.st_mtime == t.modtime);

  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);
  setup();
  test();
  return EXIT_SUCCESS;
}
