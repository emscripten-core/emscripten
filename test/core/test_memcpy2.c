/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
int main() {
  char buffer[256];
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      for (int k = 0; k < 35; k++) {
        for (int t = 0; t < 256; t++) buffer[t] = t;
        char *dest = buffer + i + 128;
        char *src = buffer + j;
        // printf("%d, %d, %d\n", i, j, k);
        assert(memcpy(dest, src, k) == dest);
        assert(memcmp(dest, src, k) == 0);
      }
    }
  }
  printf("ok.\n");
  return 0;
}
