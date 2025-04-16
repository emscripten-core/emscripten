/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include <emscripten.h>

int main() {
  EM_ASM({
    Module['prints'] = [];

    var real = out;

    out = function(x) {
      real(x);
      Module['prints'].push(x);
    }
  });
  printf("hello, world!\n");
  EM_ASM({
    if (Module['prints'].length !== 1) throw 'bad length ' + Module['prints'].length;
    if (Module['prints'][0] !== 'hello, world!') throw 'bad contents: ' + Module['prints'][0];
  });
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
  return 0;
}

