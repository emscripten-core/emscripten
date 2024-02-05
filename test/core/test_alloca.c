/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv) {
  char *pc, *pc2;
  assert(argc == 1);
  pc = (char *)alloca(4+argc);
  assert(((uintptr_t)pc) % 4 == 0);
  pc2 = (char *)alloca(4+argc);
  assert(((uintptr_t)pc2) % 4 == 0);
  printf("z:%p*%p*\n", pc, pc2);
  return 0;
}
