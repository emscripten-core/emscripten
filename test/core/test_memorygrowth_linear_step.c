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
#include "emscripten/heap.h"

int get_memory_size() {
  return emscripten_get_heap_size();
}

int main() {
  // STACK_SIZE=1Mb
  // INITIAL_MEMORY=32Mb
  // MAXIMUM_MEMORY=64Mb
  // MEMORY_GROWTH_LINEAR_STEP=1Mb

  // Because the stack is 1Mb, the first increase will take place
  // when i = 30, which attempts to grow the memory to 32Mb + 1Mb of stack > 32Mb
  // We should get to exactly 64MB at i == 62

  const int MB = 1024 * 1024;
  const int initialMemory = get_memory_size();
  int printedOnce = 0;

  for (int i = 0; 1; i++) {
    printf("%d %d %d\n", i, get_memory_size(), get_memory_size() / MB);
    volatile long sink = (long)malloc(MB);

    if (!sink) {
      printf("failed at %d %d %d\n", i, get_memory_size(), get_memory_size() / MB);
      // i ==  63 > growth to 65MB failed
      // i == 128 > growth to 130MB failed
      // i == 129 > growth to 131MB failed
      assert(i == 62);
      break;
    }

    if (get_memory_size() > initialMemory && !printedOnce) {
      printf("memory growth started at %d %d %d\n", i, get_memory_size(), get_memory_size() / MB);
      printedOnce = 1;
      assert(i == 30);
    }

    assert(i < 64); // the wasm mem max limit, we must not get there
  }

  printf("grew memory ok.\n");
}
