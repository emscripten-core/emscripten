// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main() {
  int x = EM_ASM_INT({
    window.disableErrorReporting = true;
    window.onerror = async (e) => {
      var message = e.toString();
      var success = message.indexOf("unreachable") >= 0 || // firefox
                    message.indexOf("Script error.") >= 0; // chrome
      if (success && !Module.reported) {
        Module.reported = true;
        console.log("reporting success");
        // manually REPORT_RESULT; we shouldn't call back into native code at this point
        await fetch("http://localhost:8888/report_result?0");
        window.close();
      }
    };
    return 0;
  });

  emscripten_sleep(1);

  // We should not get here - the unwind will fail as we did now all the right
  // functions - this function should be instrumented, but will not be.
  puts("We should not get here!");
  REPORT_RESULT(1);

  return 0;
}
