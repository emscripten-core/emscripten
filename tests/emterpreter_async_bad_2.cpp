// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>
#include <assert.h>

extern "C" {

void EMSCRIPTEN_KEEPALIVE finish(int result) {
  MAYBE_REPORT_RESULT(1);
}

void __attribute__((noinline)) emterped() {
  // sleep, but non-emterpreted middle() in the way
  emscripten_sleep(1);
}

void __attribute__((noinline)) middle() {
  emterped();
  // this is not an emterpreted function. when we pause emterped(), we can't pause here,
  // so we end up calling the next line
  finish(1);
}

int main() {
  EM_ASM({
    window.onerror = function(err) {
      var str = err.toString();
      assert(err.toString().indexOf("This error happened during an emterpreter-async operation") > 0, "expect good error message (" + str + ')');
      assert(str.indexOf('-12') > 0, '-12 error from emterpreter-async');
      // manually REPORT_RESULT; we can't call back into native code at this point, assertions would trigger
      if (Module.reported) return;
      Module.reported = true;
      reportResultToServer(2);
    };
  });

  printf("call middle..\n");
  middle();
  printf("called middle\n");
}

}

