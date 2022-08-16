/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/console.h>
#include <stdio.h>

void hi_world() {
  puts("hi world");
}

int main() {
  hi_world();

  // Also test an indirect call. Taking the function by reference makes us go
  // through more code paths that could have bugs, like relocatable code needing
  // to make wrapper functions for function pointers, which need signatures for
  // an imported function from JS.
  void (*func)(const char*) = &emscripten_console_log;
  func("indirect import");

  return 0;
}
