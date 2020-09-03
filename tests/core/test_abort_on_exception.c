/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

EM_JS(void, throwException, (void), {
	throw new Error("crash");
});

EMSCRIPTEN_KEEPALIVE void crash() {
  printf("crashing\n");
  throwException();
}

int main() {
  return 0;
} 
