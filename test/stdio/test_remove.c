// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <errno.h>
#include <stdio.h>
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

void test() {
  int err;

  err = remove("dir/file");
  assert(!err);

  err = remove("file");
  assert(!err);

  // should fail, folder is not empty
  err = remove("dir");
  assert(err);

  err = remove("dir/subdir");
  assert(!err);

  err = remove("dir");
  assert(!err);

  printf("success\n");
}

int main() {
  setup();
  test();
  return 0;
}
