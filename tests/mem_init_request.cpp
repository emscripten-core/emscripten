// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
  const char *hello = "hello world";
  putc('g', stdout);
  putc('o', stdout);
  putc('!', stdout);
  assert(strchr(hello, 'l') == hello + 2);
  assert(strchr(hello, 'w') == hello + 6);
  putc('d', stdout);
  putc('o', stdout);
  putc('n', stdout);
  putc('e', stdout);
  return 0;
}

