/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

#include <emscripten/emscripten.h>
#include <emscripten/em_js.h>

EM_JS_DEPS(deps, "$UTF8ToString,$stringToUTF8OnStack,$getValue");

int main() {
  char *c = "μ†ℱ ╋ℯ╳╋ 😇";
  printf("%hhu %hhu %hhu %hhu %s\n", c[0], c[1], c[2], c[3], c);
  EM_ASM({
    var cheez = stringToUTF8OnStack("μ†ℱ ╋ℯ╳╋ 😇");
    out(UTF8ToString(cheez), getValue(cheez+0, 'i8')&0xff,
                             getValue(cheez+1, 'i8')&0xff,
                             getValue(cheez+2, 'i8')&0xff,
                             getValue(cheez+3, 'i8')&0xff);
  });
}
