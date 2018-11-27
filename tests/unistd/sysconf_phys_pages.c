/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main() {
  printf("%ld, ", sysconf(_SC_PHYS_PAGES));
  printf("errno: %d\n", errno);
  return 0;
}
