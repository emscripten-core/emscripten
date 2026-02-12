/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <string.h>

int main() {
  // Test boolean, int, and string settings
  printf("INVOKE_RUN: %ld\n", emscripten_get_compiler_setting("INVOKE_RUN"));
  assert((unsigned)emscripten_get_compiler_setting("ASSERTIONS") <= 2);
  printf("CLOSURE_WARNINGS: %s\n", (char*)emscripten_get_compiler_setting("CLOSURE_WARNINGS"));

  // Internal setting should not be visible.
  const char* embind = (char*)emscripten_get_compiler_setting("EMBIND");
  printf("EMBIND: %s\n", embind);
  assert(strstr(embind, "invalid compiler setting") != NULL);

  // EMSCRIPTEN_VERSION should be allowed though..
  const char* version = (char*)emscripten_get_compiler_setting("EMSCRIPTEN_VERSION");
  printf("EMSCRIPTEN_VERSION: %s\n", version);
  assert(strstr(version, "invalid compiler setting") == NULL);
}

