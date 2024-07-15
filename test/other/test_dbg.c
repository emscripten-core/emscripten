/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten/console.h>

int main() {
  printf("hello, world!\n");
#ifndef NDEBUG
  // This symbol is only available in debug builds (i.e. -sASSERTIONS)
  emscripten_dbg("native dbg message");
  emscripten_dbgn("hello world!", 5);
  emscripten_dbgf("formatted: %d", 42);
#endif
  return 0;
}
