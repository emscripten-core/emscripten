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
#include <unistd.h>
#include <sys/stat.h>

void setup() {
  int err;
  err = mkdir("nocanread", 0111);
  assert(!err);
  err = mkdir("foobar", 0777);
  assert(!err);
}

void cleanup() {
  rmdir("nocanread");
  rmdir("foobar");
}

void test() {
  int err;
  DIR *dir;
  dir = opendir("nocanread");
  assert(!dir);
  assert(errno == EACCES);
  puts("success");
}

int main() {
  atexit(cleanup);
  signal(SIGABRT, cleanup);

  setup();
  test();
  cleanup();

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
  return EXIT_SUCCESS;
}
