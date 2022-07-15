// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <errno.h>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  create_file("file", "abcdef", 0777);
  mkdir("dir", 0777);
  create_file("dir/file", "abcdef", 0777);
  mkdir("dir/subdir", 0777);
}

void cleanup() {
  // make sure we get it all regardless of anything failing
  unlink("file");
  unlink("dir/file");
  rmdir("dir/subdir");
  rmdir("dir");
}

void test() {
  int err;
  
  err = std::remove("dir/file");
  assert(!err);

  err = std::remove("file");
  assert(!err);

  // should fail, folder is not empty
  err = std::remove("dir");
  assert(err);

  err = std::remove("dir/subdir");
  assert(!err);

  err = std::remove("dir");
  assert(!err);

  std::cout << "success\n";
}

int main() {
  atexit(cleanup);
  setup();
  test();
  return 0;
}
