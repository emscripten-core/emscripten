/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include <emscripten.h>

int main() {
  printf("hello, world!\n");
  int result = EM_ASM_INT({
    return Module.sawAsyncCompilation | 0;
  });
  printf("sawAsyncCompilation => %d\n", result);
  return result;
}

