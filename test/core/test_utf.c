/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>
#include <stdlib.h>

int main() {
  char *c = "μ†ℱ ╋ℯ╳╋ 😇";
  printf("%d %d %d %d %s\n", c[0] & 0xff, c[1] & 0xff, c[2] & 0xff, c[3] & 0xff,
         c);
  emscripten_run_script(
      "var cheez = _malloc(100);"
      "Module.stringToUTF8(\"μ†ℱ ╋ℯ╳╋ 😇\", cheez, 100);"
      "out([UTF8ToString(cheez), Module.getValue(cheez, "
      "'i8')&0xff, Module.getValue(cheez+1, 'i8')&0xff, "
      "Module.getValue(cheez+2, 'i8')&0xff, Module.getValue(cheez+3, "
      "'i8')&0xff].join(','));"
      "_free(cheez);"
      );
}
