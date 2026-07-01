// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void touch(const char* fileName) {
  int fd = open(fileName, O_CREAT | O_WRONLY,  S_IRUSR | S_IWUSR | S_IRGRP);
  assert(fd);
  close(fd);
}

void doTest(const char* uniqueDirName, const char* fileName) {
  printf("test in %s: %s\n", uniqueDirName, fileName);

  int ret = mkdir(uniqueDirName, 0777);
  assert(ret == 0);

  char buf[1024];
  sprintf(buf, "%s/%s", uniqueDirName, fileName);
  touch(buf);

  int fd = open(uniqueDirName, O_RDONLY | O_DIRECTORY);
  assert(fd > 0);

  while (1) {
    struct dirent d;
    int nread = getdents(fd, &d, sizeof(d));
    assert(nread != -1);
    if (nread == 0) {
      break;
    }

    if (strcmp(d.d_name, ".") == 0 || strcmp(d.d_name, "..") == 0) {
      continue;
    }

    printf("d.d_name = %s\n", d.d_name);

    // If it needed to be, the name was truncated.
    assert(strlen(d.d_name) <= 255);
  }
}

int main() {
  // Non-ascii file name.
  doTest("test_dir", u8"абвгд");

#ifndef WASMFS // The JS FS truncates filenames automatically, which is incorrect. Wasmfs and Linux do not.
  // File name exceeds the limit of 255 chars and is truncated.
  char longName[300];
  memset(longName, '1', sizeof(longName));
  longName[sizeof(longName) - 1] = '\0';
  doTest("test_dir2", longName);
#endif
}
