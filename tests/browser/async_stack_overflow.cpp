// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main() {
  int x = EM_ASM_INT({
    window.onerror = function(e) {
      var message = e.toString();
      var success = message.indexOf("unreachable") >= 0 || // firefox
                    message.indexOf("Script error.") >= 0; // chrome
      if (success && !Module.reported) {
        Module.reported = true;
        console.log("reporting success");
        // manually REPORT_RESULT; we shouldn't call back into native code at this point
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "http://localhost:8888/report_result?0");
        xhr.onload = xhr.onerror = function() {
          window.close();
        };
        xhr.send();
      }
    };
    return 0;
  });

  int y = x * x;
  int z = y * y;
  x++;
  y++;
  z++;
  emscripten_sleep(1);
  // We should not get here - the unwind will fail as the stack is too small
  printf("We should not get here %d %d %d\n", x, y, z);

  REPORT_RESULT(1);

  return 0;
}
