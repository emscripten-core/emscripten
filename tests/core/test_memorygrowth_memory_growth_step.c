/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "emscripten.h"

int get_TOTAL_MEMORY() {
  return EM_ASM_INT({ return HEAP8.length });
}

int main() {
  const int MB = 1024 * 1024;
  // Memory size starts at 64MB, max is 130MB and growth step is 1MB.
  // We should get exactly to 130MB.
  for (int i = 0; 1; i++) {
    printf("%d %d %d\n", i, get_TOTAL_MEMORY(), get_TOTAL_MEMORY() / 65536);
    volatile int sink = (int)malloc(MB);
    if (!sink) {
      printf("failed at %d %d %d\n", i, get_TOTAL_MEMORY(), get_TOTAL_MEMORY() / 65536);
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

