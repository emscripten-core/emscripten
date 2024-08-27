/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void create_file(const char *path, const char *buffer, int mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);
  assert(fd >= 0);

  int err = write(fd, buffer, sizeof(char) * strlen(buffer));
  assert(err ==  (sizeof(char) * strlen(buffer)));

  close(fd);
}

void setup() {
  mkdir("folder", 0777);
  create_file("folder/file", "abcdef", 0777);
  symlink("file", "folder/file-link");
}

int main() {
  setup();

  int rc;
  struct statx buf;

  rc = statx(AT_FDCWD, "folder", 0, STATX_ALL, &buf);
  assert(rc == 0);
  assert(S_ISDIR(buf.stx_mode));

  rc = statx(AT_FDCWD, "folder/file", 0, STATX_ALL, &buf);
  assert(rc == 0);
  assert(S_ISREG(buf.stx_mode));

  rc = statx(AT_FDCWD, "folder/file-link", 0, STATX_ALL, &buf);
  assert(rc == 0);
  assert(S_ISREG(buf.stx_mode));

  rc = statx(AT_FDCWD, "folder/file-link", AT_SYMLINK_NOFOLLOW, STATX_ALL, &buf);
  assert(rc == 0);
  assert(S_ISLNK(buf.stx_mode));

  printf("success\n");
  return 0;
}
