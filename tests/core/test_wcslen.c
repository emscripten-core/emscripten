/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <wchar.h>

int main() {
  const wchar_t* wstr = L"Hello";

  printf("wcslen: %lu\n", wcslen(wstr));

  return 0;
}
