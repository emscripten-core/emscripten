#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

  utime("writeable", &t);
  assert(!errno);
  memset(&s, 0, sizeof s);
  stat("writeable", &s);
  assert(s.st_atime == t.actime);
  assert(s.st_mtime == t.modtime);

  // write permissions aren't checked when setting node
  // attributes unless the user uid isn't the owner (so
  // therefor, this should work fine)
  utime("unwriteable", &t);
  assert(!errno);
  memset(&s, 0, sizeof s);
  stat("unwriteable", &s);
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
