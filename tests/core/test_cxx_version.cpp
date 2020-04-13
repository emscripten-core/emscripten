/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main(int argc, const char *argv[]) {
  printf("__cplusplus: %ld\n", __cplusplus);
  return 0;
}
