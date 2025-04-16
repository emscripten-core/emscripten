/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten/emscripten.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

int main() {
  EM_ASM(
    FS.writeFile('testfile', 'a=1\nb=2\n');
  );

  int fd = open("testfile", O_RDONLY);
  assert(fd >= 0);

  char buf0[16] = {0};
  char buf1[16] = {0};
  struct iovec iov[] = {{.iov_base = buf0, .iov_len = 4},
                        {.iov_base = buf1, .iov_len = 4}};
  ssize_t nread = preadv(fd, iov, 2, 0);
  assert(nread == 8);
  assert(strcmp(buf0, "a=1\n") == 0);
  assert(strcmp(buf1, "b=2\n") == 0);

  close(fd);

  printf("success\n");
  return 0;
}
