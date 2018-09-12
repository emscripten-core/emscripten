/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main() {
  FILE* f1 = fopen("s", "r");
  if (f1 == NULL) {
    printf("file 's' not found!\n");
  }

  FILE* f2 = fopen("", "r");
  if (f2 == NULL) {
    printf("file '' not found!\n");
  }
  return 0;
}
