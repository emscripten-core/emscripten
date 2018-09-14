/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#if __cplusplus != 199711L
#error By default, if no -std is specified, emscripten should be compiling with -std=c++03!
#endif

int main(int argc, const char *argv[]) {
  printf("Hello world!\n");
  return 0;
}
