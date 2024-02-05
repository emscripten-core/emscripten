/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <fcntl.h>
typedef int (*fp_t)(int, int, ...);
int main(int argc, char **argv) {
  fp_t fp = &fcntl;
  if (argc == 1337) fp = (fp_t) & main;
  (*fp)(0, F_GETFL);
  (*fp)(0, F_SETSIG, 5);
  printf("waka\n");
  return 0;
}
