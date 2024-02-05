/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

typedef void (*funcptr)(int, int);
typedef funcptr (*funcptrfunc)(intptr_t);

funcptr __attribute__((noinline)) getIt(intptr_t x) { return (funcptr)x; }

int main(int argc, char **argv) {
  funcptrfunc fpf = argc < 100 ? getIt : NULL;
  printf("*%p*\n", fpf(argc));
  return 0;
}
