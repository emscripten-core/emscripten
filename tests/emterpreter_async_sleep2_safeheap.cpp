// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int result = 0;

extern "C" {

void fix() {
  result = 17;
}

void EMSCRIPTEN_KEEPALIVE callback() {
  EM_ASM({
    out('callback...');
    Module['dynCall_v']($0);
    out('callback fixed.');
  }, (int)&fix);
}

}

int main(void) {
  EM_ASM({
    setTimeout(function() {
      Module['_callback']();
    }, 1);
  });
  printf("Sleep:\n");
  emscripten_sleep(1000);
  printf("Done!\n");
  REPORT_RESULT(result);
  return 0;
}

