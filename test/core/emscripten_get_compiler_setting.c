/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

int main() {
  printf("INVOKE_RUN: %ld\n", emscripten_get_compiler_setting("INVOKE_RUN"));
  assert((unsigned)emscripten_get_compiler_setting("OPT_LEVEL") <= 3);
  assert((unsigned)emscripten_get_compiler_setting("DEBUG_LEVEL") <= 4);
  printf("EMSCRIPTEN_VERSION: %s\n", (char*)emscripten_get_compiler_setting("EMSCRIPTEN_VERSION"));
}

