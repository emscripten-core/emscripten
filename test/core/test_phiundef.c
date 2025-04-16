/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>

static int state;

struct my_struct {
  union {
    struct {
      unsigned char a;
      unsigned char b;
    } c;
    unsigned int d;
  } e;
  unsigned int f;
};

int main(int argc, char **argv) {
  struct my_struct r;

  state = 0;

  for (int i = 0; i < argc + 10; i++) {
    if (state % 2 == 0)
      r.e.c.a = 3;
    else
      printf("%d\n", r.e.c.a);
    state++;
  }
  return 0;
}
