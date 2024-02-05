/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char* file = "/foo.txt";

off_t getPos(int fd) {
  errno = 0;
  off_t pos = lseek(fd, 0, SEEK_CUR);
  assert(pos != (off_t)-1);
  assert(errno == 0);
  return pos;
}

void setPos(int fd, off_t pos) {
  errno = 0;
  pos = lseek(fd, pos, SEEK_SET);
  assert(pos != (off_t)-1);
  assert(errno == 0);
}

int main() {
  int fd = open(file, O_RDWR | O_CREAT | O_EXCL | O_APPEND, 0777);
  assert(fd > 0);

  off_t pos = getPos(fd);
  assert(pos == 0);

  ssize_t nwritten = write(fd, "hello", 5);
  assert(nwritten == 5);

  pos = getPos(fd);
  assert(pos == 5);

  setPos(fd, 0);
  pos = getPos(fd);
  assert(pos == 0);

  nwritten = write(fd, "", 0);
  assert(nwritten == 0);

  pos = getPos(fd);
  assert(pos == 5);

  setPos(fd, 0);
  pos = getPos(fd);
  assert(pos == 0);

  nwritten = write(fd, ", world!", 8);
  assert(nwritten == 8);

  pos = getPos(fd);
  assert(pos == 13);

  setPos(fd, 42);
  pos = getPos(fd);
  assert(pos == 42);

  nwritten = write(fd, "!!", 2);
  assert(nwritten == 2);

  pos = getPos(fd);
  assert(pos == 15);

  setPos(fd, 0);

  char buf[100] = {};
  ssize_t nread = read(fd, buf, 100);
  assert(nread == 15);
  assert(strcmp(buf, "hello, world!!!") == 0);

  printf("ok\n");
}
