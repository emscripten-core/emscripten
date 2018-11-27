/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

typedef struct {
  int (*f)(void *);
  void *d;
  char s[16];
} LMEXFunctionStruct;

int f(void *user) { return 0; }

static LMEXFunctionStruct const a[] = {{f, (void *)(int)'a', "aa"}};

int main() {
  printf("ok\n");
  return a[0].f(a[0].d);
}
