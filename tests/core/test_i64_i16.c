/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdint.h>
#include <stdio.h>
int main(int argc, char** argv) {
  int y = -133;
  int64_t x = ((int64_t)((short)(y))) * (100 + argc);
  if (x > 0)
    printf(">0\n");
  else
    printf("<=0\n");
}
