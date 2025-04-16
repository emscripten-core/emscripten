/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>
int main() {
  char str[] = "memmove can be very useful....!";
  memmove(str + 20, str + 15, 11);
  puts(str);
  return 0;
}
