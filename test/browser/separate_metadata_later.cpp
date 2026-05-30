// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE extern "C" void finish() {
  REPORT_RESULT(1);
}

int main() {
  EM_ASM({
    setTimeout(function() {
      // hijack run blocker logic to see when the metadata is loaded ok.
      var real = Module["addRunBlocker"];
      Module["addRunBlocker"] = function(promise) {
        real(promise);
        promise.then(function() {
          Module["_finish"]();
        });
      };
      function loadChildScript(name, then) {
        var js = document.createElement("script");
        if (then) js.onload = then;
        js.src = name;
        document.body.appendChild(js);
      }
      loadChildScript("more.js");
    }, 1);
  });
}

