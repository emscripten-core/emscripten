/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <ctype.h>
#include <wctype.h>

int main() {
  printf("%d ", isdigit('0'));
  printf("%d\n", iswdigit(L'0'));
  return 0;
}
