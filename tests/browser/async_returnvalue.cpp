// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

#include <emscripten.h>

// A "sync" tunnel that adds 1.
extern "C" int sync_tunnel(int);

int main() {
#ifdef BAD
  EM_ASM({
    window.onerror = function(err) {
      assert(err.toString().indexOf("wakan") > 0, "expect good error message");
      // manually REPORT_RESULT; we shoudln't call back into native code at this point
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "http://localhost:8888/report_result?0");
      xhr.onload = xhr.onerror = function() {
        window.close();
      };
      xhr.send();
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
  REPORT_RESULT(1);
#else
  // Success!
  REPORT_RESULT(0);
#endif

  return 0;
}

