/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main() {
  const int MB = 1024 * 1024;
  // Memory size starts at 64MB, and max is 100. allocate enough
  // to prove we can grow. 70 is enough to prove we can grow,
  // higher can prove we stop at the right point.
  for (int i = 0; 1; i++) {
    printf("%d\n", i);
    volatile long sink = (long)malloc(MB);
    if (!sink) {
      printf("failed at %d\n", i);
      assert(i > 70);
      break;
    }
    assert(i <= 100); // the wasm mem max limit, we must not get there
  }
  printf("grew memory ok.\n");
}

