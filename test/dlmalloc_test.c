/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Emscripten tests

#include <assert.h>
#include <emscripten/console.h>
#include <stdlib.h>

int main(int ac, char **av) {
  int NUM = ac > 1 ? atoi(av[1]) : 0;
  int REPS = ac > 2 ? atoi(av[2]) : 0;
  int c1 = 0, c2 = 0;
  for (int x = 0; x < REPS; x++) {
    char* allocations[NUM];
    for (int i = 0; i < NUM/2; i++) {
      allocations[i] = (char*)malloc((11*i)%1024 + x);
      assert(allocations[i]);
      if (i > 10 && i%4 == 1 && allocations[i-10]) {
        free(allocations[i-10]);
        allocations[i-10] = NULL;
      }
    }
    for (int i = NUM/2; i < NUM; i++) {
      allocations[i] = (char*)malloc(1024*(i+1));
      assert(allocations[i]);
      if (i > 10 && i%4 != 1 && allocations[i-10]) {
        free(allocations[i-10]);
        allocations[i-10] = NULL;
      }
    }
    char* first = allocations[0];
    for (int i = 0; i < NUM; i++) {
      if (allocations[i]) {
        free(allocations[i]);
      }
    }
    char *last = (char*)malloc(512); // should be identical, as we free'd it all
    char *newer = (char*)malloc(512); // should be different
    c1 += first == last;
    c2 += first == newer;
  }
  emscripten_console_logf("*%d,%d*\n", c1, c2);
}
