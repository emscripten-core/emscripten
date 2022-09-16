/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
int main() {
  char szToupr[] = "hello, ";
  char szTolwr[] = "EMSCRIPTEN";
  strupr(szToupr);
  strlwr(szTolwr);
  puts(szToupr);
  puts(szTolwr);
  return 0;
}
