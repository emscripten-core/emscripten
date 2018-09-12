/*
 * Copyright 2011 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <unistd.h>

int main() {
  char src[] = "abcdefg";
  char dst[10] = {0};
  swab(src, dst, 5);

  printf("before: %s\n", src);
  printf("after: %s\n", dst);

  return 0;
}
