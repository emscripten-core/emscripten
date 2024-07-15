/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <emscripten/em_asm.h>
#include <emscripten/em_js.h>

EM_JS_DEPS(test, "$setErrNo");

int main() {
  EM_ASM(setErrNo(5));
  printf("errno   : %d\n", errno);
  printf("strerror: %s\n", strerror(errno));
  return 0;
}
