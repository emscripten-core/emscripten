// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <assert.h>
#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>

// test file operations after main() exits

#define NAME "file.cpp"

EMSCRIPTEN_KEEPALIVE
void finish() {
  EM_ASM({
    var printed = Module['extraSecretBuffer'].split('Iteration').length - 1;
    console.log(printed);
    assert(printed == 5, 'should have printed 5 iterations');
  });
  printf("Test passed.\n");
  emscripten_force_exit(0);
}

EMSCRIPTEN_KEEPALIVE
void looper() {
  // exiting main should not cause any weirdness with file opening
  printf("Iteration\n");
  FILE* f = fopen("/dev/stdin", "rb");
  if (!f) {
    printf("Test failed.\n");
    emscripten_force_exit(1);
  }
  fclose(f);
}

EM_JS_DEPS(deps, "$safeSetTimeout");

int main() {
  EM_ASM({
    (function() {
      // exiting main should not cause any weirdness with printing
      var realPrint = out;
      Module['extraSecretBuffer'] = "";
      out = function(x) {
        Module['extraSecretBuffer'] += x;
        realPrint(x);
      };
    })();
  });
  printf("Start\n");
  FILE* f = fopen(NAME, "wb");
  fclose(f);
  printf("Looping...\n");
  EM_ASM({
    out('js');
    var counter = 0;
    function looper() {
      out('js looping');
      Module['_looper']();
      counter++;
      if (counter < 5) {
        out('js queueing');
        // Use safeSetTimeout here so that `looper` get called via
        // `callUserCallback` which gracefully handles things like `exit`.
        safeSetTimeout(looper, 1);
      } else {
        out('js finishing');
        safeSetTimeout(Module['_finish'], 1);
      }
    }
    looper();
  });

  emscripten_runtime_keepalive_push();
  return 99;
}
