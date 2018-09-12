/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

int main() {
  char* err;
  char buffer[200];

  err = strerror(EDOM);
  strerror_r(EWOULDBLOCK, buffer, 200);
  printf("<%s>\n", err);
  printf("<%s>\n", buffer);

  printf("<%d>\n", strerror_r(EWOULDBLOCK, buffer, 0));
  errno = 123;
  printf("<%d>\n", errno);

  return 0;
}
