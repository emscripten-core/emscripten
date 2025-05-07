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
  printf("%hhu %hhu %hhu %hhu %s\n", c[0], c[1], c[2], c[3], c);
  emscripten_run_script(
    "var cheez = Module.stringToUTF8OnStack(\"μ†ℱ ╋ℯ╳╋ 😇\");"
    "out(UTF8ToString(cheez), Module.getValue(cheez+0, 'i8')&0xff, "
    "                         Module.getValue(cheez+1, 'i8')&0xff, "
    "                         Module.getValue(cheez+2, 'i8')&0xff, "
    "                         Module.getValue(cheez+3, 'i8')&0xff);");
}
