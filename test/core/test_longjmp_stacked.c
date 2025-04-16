/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

intptr_t bottom, top;

int run(int y) {
  // confuse stack
  char *s = (char *)alloca(100);
  memset(s, 1, 100);
  s[y] = y;
  s[y / 2] = y * 2;
  volatile int x = s[y];
  top = (intptr_t)alloca(4);
  if (x <= 2) return x;
  jmp_buf buf;
  printf("setjmp of %d\n", x);
  if (setjmp(buf) == 0) {
    printf("going\n");
    x += run(x / 2);
    longjmp(buf, 1);
  }
  printf("back\n");
  return x / 2;
}

int main(int argc, char **argv) {
  int sum = 0;
  for (int i = 0; i < argc * 2; i++) {
    bottom = (intptr_t)alloca(4);
    sum += run(10);
    // scorch the earth
    if (bottom < top) {
      memset((void *)bottom, 1, top - bottom);
    } else {
      memset((void *)top, 1, bottom - top);
    }
  }
  printf("%d\n", sum);
  return sum;
}
