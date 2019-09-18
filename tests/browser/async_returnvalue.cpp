// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

#include <emscripten.h>

// A "sync" tunnel that adds 1.
#if USE_EM_JS
EM_JS(int, sync_tunnel, (int value), {
  return Asyncify.handleSleep(function(wakeUp) {
    setTimeout(function() {
      wakeUp(value + 1);
    }, 1);
  });
})
#else
extern "C" int sync_tunnel(int);
#endif

int main() {
#ifdef BAD
  EM_ASM({
    window.onerror = function(e) {
      var success = e.toString().indexOf("import sync_tunnel was not in ASYNCIFY_IMPORTS, but changed the state") > 0;
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
  });
#endif
  int x;
  x = sync_tunnel(0);
  assert(x == 1);
  x = sync_tunnel(1);
  assert(x == 2);
  x = sync_tunnel(2);
  assert(x == 3);
  x = sync_tunnel(42);
  assert(x == 43);
  x = sync_tunnel(-1);
  assert(x == 0);
  x = sync_tunnel(-2);
  assert(x == -1);

#ifdef BAD
  // We should not get here.
  printf("We should not get here\n");
#else
  // Success!
  REPORT_RESULT(0);
#endif

  return 0;
}

