/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE *fp;
  int c;

  malloc(20000000);  // Enlarge memory

  fp = fopen("test.txt", "r");
  int nChars = 0;
  while ((c = fgetc(fp)) != EOF)
  {
    putchar(c);
    ++nChars;
  }
  assert(nChars == 15);
  return 0;
}
