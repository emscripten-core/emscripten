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
  // Memory size starts at 64MB, max is 130MB and growth step is 1MB.
  // We should get exactly to 130MB.
  for (int i = 0; 1; i++) {
    printf("%d\n", i);
    volatile int sink = (int)malloc(MB);
    if (!sink) {
      printf("failed at %d\n", i);
      // i ==  64 > growth to 65MB failed
      // i == 129 > growth to 130MB failed
      // i == 130 > growth to 131MB failed
      assert(i >= 129);
      break;
    }
    assert(i < 131); // the wasm mem max limit, we must not get there
  }
  printf("grew memory ok.\n");
}

