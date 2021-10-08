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

static void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  int err;
#ifdef NODERAWFS
  err = mkdir("testtmp", 0777);
  chdir("testtmp");
  // create directories to mimic mounted root
  err = mkdir("tmp", 0777);
  err = mkdir("proc", 0777);
  err = mkdir("home", 0777);
  err = mkdir("dev", 0777);
#endif
  err = mkdir("nocanread", 0111);
  assert(!err);
  err = mkdir("foobar", 0777);
  assert(!err);
  create_file("foobar/file.txt", "ride into the danger zone", 0666);
}

void cleanup() {
  rmdir("nocanread");
  unlink("foobar/file.txt");
  rmdir("foobar");
#ifndef NODERAWFS
  rmdir("tmp");
  rmdir("proc");
  rmdir("home");
  rmdir("dev");
  chdir("..");
  rmdir("testtmp");
#endif
}

void test() {
  int err;
  DIR *dir;
  // check bad opendir input
  dir = opendir("noexist");
  assert(!dir);
  assert(errno == ENOENT);

  

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

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
  return EXIT_SUCCESS;
}
