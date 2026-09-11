// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Recursion 200 deep with a filled frame per level, suspending at the leaf;
// every level verifies its frame on the way back up. Two such fibers run
// concurrently.

#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/eventloop.h>
#include <emscripten/stack.h>
#include <stdio.h>
#include <stdlib.h>

#define DEPTH 200

__attribute__((noinline)) static int recurse(int pattern, int depth) {
  volatile unsigned char frame[64];
  for (int i = 0; i < 64; i++) {
    frame[i] = (unsigned char)(pattern + depth + i);
  }
  int ok;
  if (depth == DEPTH) {
    emscripten_sleep(pattern);
    ok = 1;
  } else {
    ok = recurse(pattern, depth + 1);
  }
  for (int i = 0; i < 64; i++) {
    if (frame[i] != (unsigned char)(pattern + depth + i)) {
      return 0;
    }
  }
  return ok;
}

EMSCRIPTEN_KEEPALIVE int fiber(int pattern) {
  int ok = recurse(pattern, 0);
  printf("fiber %d: %s\n", pattern, ok ? "intact" : "CORRUPT");
  return ok;
}

EMSCRIPTEN_KEEPALIVE uintptr_t stack_pointer(void) {
  return emscripten_stack_get_current();
}

EMSCRIPTEN_KEEPALIVE void finish(int ok, int sp_balanced) {
  printf("sp balanced: %d\n", sp_balanced);
  if (ok && sp_balanced) {
    printf("done\n");
  }
  emscripten_runtime_keepalive_pop();
  exit(0);
}

EM_JS(void, start, (), {
  (async () => {
    var sp = Module['_stack_pointer']();
    var a = Module['_fiber'](1);
    var b = Module['_fiber'](20);
    var ok = (await a) & (await b);
    callUserCallback(() => Module['_finish'](ok, Module['_stack_pointer']() == sp));
  })();
});

int main() {
  emscripten_runtime_keepalive_push();
  EM_ASM({ setTimeout(start, 0); });
  return 0;
}
