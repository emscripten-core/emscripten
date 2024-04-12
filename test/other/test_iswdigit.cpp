/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <cctype>
#include <cwctype>

int main() {
  using namespace std;
  printf("%d ", isdigit('0'));
  printf("%d ", iswdigit(L'0'));
  return 0;
}
