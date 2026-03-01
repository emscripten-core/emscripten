/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

int main() {
  int fd = open("testfile", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  assert(fd >= 0);

  char buf0[16] = "a=1\n";
  char buf1[] = "b=2\n";
  struct iovec iov[] = {{.iov_base = buf0, .iov_len = 4},
                        {.iov_base = buf1, .iov_len = 4}};
  ssize_t nwritten = pwritev(fd, iov, 2, 0);
  assert(nwritten == 8);
  close(fd);

  int fd2 = open("teStfIle", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
  assert(fd >= 0);

  char buf3[16] = "a=3\n";
  char buf4[] = "b=4\n";
  struct iovec iov2[] = {{.iov_base = buf3, .iov_len = 4},
                        {.iov_base = buf4, .iov_len = 4}};
  ssize_t nwritten2 = pwritev(fd2, iov2, 2, 0);
  assert(nwritten2 == 8);
  close(fd2);

  int fd3 = open("testfile", O_RDONLY);
  size_t nread = read(fd, buf0, sizeof(buf0));
  assert(nread == 8);
  assert(strcmp(buf0, "a=3\nb=4\n") == 0);
  close(fd);

  printf("success\n");
  return 0;
}
