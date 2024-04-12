/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main() {
  // CLOEXEC is not supported
  assert(ioctl(STDOUT_FILENO, FIOCLEX, NULL) == -1);
  assert(errno == EINVAL);

  puts("success");
  return 0;
}
