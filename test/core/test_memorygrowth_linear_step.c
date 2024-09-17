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

  // INITIAL_MEMORY=64Mb
  // STACK_SIZE=1Mb
  // MAXIMUM_MEMORY=130Mb
  // MEMORY_GROWTH_LINEAR_STEP=1Mb

  // Because the stack is 1Mb, the first increase will take place
  // in i = 63, which attempts to grow the heap to 64Mb + 1Mb of stack > 64Mb INITIAL_MEMORY
  // We should get exactly to 130MB at i == 128

  const int MB = 1024 * 1024;
  const int totalMemory = get_memory_size();
  int printedOnce = 0;

  for (int i = 0; 1; i++) {

    printf("%d %d %d\n", i, get_memory_size(), get_memory_size() / MB);
    volatile long sink = (long)malloc(MB);

    if (!sink) {
      printf("failed at %d %d %d\n", i, get_memory_size(), get_memory_size() / MB);
      // i ==  63 > growth to 65MB failed
      // i == 128 > growth to 130MB failed
      // i == 129 > growth to 131MB failed
      assert(i >= 128);
      break;
    }

    if (get_memory_size() > totalMemory && !printedOnce) {
      printf("memory growth started at %d %d %d\n", i, get_memory_size(), get_memory_size() / MB);
      printedOnce = 1;
    }

    assert(i < 130); // the wasm mem max limit, we must not get there
  }

  printf("grew memory ok.\n");
}
