/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

int main() {
  volatile int x = 0;
  printf("setjmp:%d\n", setjmp(buf));
  x++;
  printf("x:%d\n", x);
  if (x < 4) longjmp(buf, x * 2);
  return 0;
}
