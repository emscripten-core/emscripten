/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

EM_JS(void, throwException, (void), {
  throw new Error("crash");
});

int main() {
  printf("crashing during main\n");
  throwException();
  return 0;
}
