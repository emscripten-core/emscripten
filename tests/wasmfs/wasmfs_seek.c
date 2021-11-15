/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

// FIXME: The existing io.c file implements devices, which haven't been
// implemented in the new FS yet.
// This test simply implements the seek portion.

int main() {

  char readBuffer[256] = {0};
  char writeBuffer[] = "writeme";

  int f = open("/file", O_RDWR | O_CREAT);
  const char* msg = "1234567890";
  write(f, msg, strlen(msg));
  lseek(f, 0, SEEK_SET);

  printf("read from file: %zd\n", read(f, readBuffer, sizeof readBuffer));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("pread past end of file: %zd\n",
         pread(f, readBuffer, sizeof readBuffer, 999999999));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 3, SEEK_SET));
  printf("errno: %d\n\n", errno);
  printf("partial read from file: %zd\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, -2, SEEK_END));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("partial read from end of file: %zd\n", read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("before bad seek: %lld\n", lseek(f, 0, SEEK_CUR));
  printf("errno: %d\n", errno);
  errno = 0;

  printf("bad seek: %lld\n", lseek(f, -15, SEEK_CUR));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("after bad seek: %lld\n", lseek(f, 0, SEEK_CUR));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("partial read from before start of file: %zd\n",
         read(f, readBuffer, 3));
  printf("data: %s\n", readBuffer);
  memset(readBuffer, 0, sizeof readBuffer);
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 0, SEEK_SET));
  printf("write to start of file: %zd\n", write(f, writeBuffer, 3));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 0, SEEK_END));
  printf("write to end of file: %zd\n", write(f, writeBuffer, 3));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("seek: %lld\n", lseek(f, 10, SEEK_END));
  printf("write after end of file: %zd\n",
         write(f, writeBuffer, sizeof writeBuffer));
  printf("errno: %d\n\n", errno);
  errno = 0;

  printf("pwrite to the middle of file: %zd\n",
         pwrite(f, writeBuffer + 2, 3, 17));
  printf("errno: %d\n", errno);
  printf("seek: %lld\n\n", lseek(f, 0, SEEK_CUR));
  errno = 0;

  printf("pwrite past end of file: %zd\n", pwrite(f, writeBuffer, 5, 32));
  printf("errno: %d\n", errno);
  printf("seek: %lld\n\n", lseek(f, 0, SEEK_CUR));
  errno = 0;

  ssize_t bytesRead;
  printf("seek: %lld\n", lseek(f, 0, SEEK_SET));
  printf("read after write: %zd\n",
         bytesRead = read(f, readBuffer, sizeof readBuffer));
  printf("errno: %d\n", errno);
  errno = 0;
  printf("final: ");
  for (ssize_t i = 0; i < bytesRead; i++) {
    if (readBuffer[i] == 0) {
      printf("\\0");
    } else {
      printf("%c", readBuffer[i]);
    }
  }
  printf("\n");
  return 0;
}
