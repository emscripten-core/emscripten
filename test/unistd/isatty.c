/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  int err;

  err = isatty(-1);
  assert(!err);
  assert(errno == EBADF);

  err = isatty(open("/dev/stdin", O_RDONLY));
  assert(err == 1);

  err = isatty(open("/dev/null", O_RDONLY));
  assert(!err);

  err = isatty(open("/dev", O_RDONLY));
  assert(!err);

  puts("success");

  return EXIT_SUCCESS;
}
