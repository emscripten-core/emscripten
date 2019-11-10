/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  char* endptr;
  --argc, ++argv;
  double total = 0.0;
  for (; argc; argc--, argv++) {
    total += strtod(*argv, &endptr);
  }
  printf("total: %g\n", total);
  return 0;
}
