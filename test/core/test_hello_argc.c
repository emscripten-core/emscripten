/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main(int argc, char** argv) {
  printf("hello, world! %d\n", argc);
  // Skip arg 0 because it will be an absolute path that varies between
  // machines.
  for (int i = 1; i < argc; i++) {
    printf("%d: %s\n", i, argv[i]);
  }
  return 0;
}
