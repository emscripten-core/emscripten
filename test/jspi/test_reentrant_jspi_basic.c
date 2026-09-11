// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Two promising exports started back to back each keep a filled 4 KiB frame
// across a suspension. Without REENTRANT_JSPI the second fiber's frames land
// on top of the first one's.

#include <assert.h>
#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/eventloop.h>
#include <emscripten/stack.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME 4096

static int verify(volatile unsigned char* buf, int n, unsigned char pattern) {
  for (int i = 0; i < n; i++) {
    if (buf[i] != (unsigned char)(pattern + i)) {
      return 0;
    }
  }
  return 1;
}

// Pushes and dirties a frame covering where a concurrently suspended fiber's
// frame sits without REENTRANT_JSPI.
__attribute__((noinline)) static void scribble(int pattern) {
  volatile unsigned char buf[2 * FRAME];
  for (int i = 0; i < 2 * FRAME; i++) {
    buf[i] = (unsigned char)~pattern;
  }
  (void)buf[2 * FRAME - 1];
}

EMSCRIPTEN_KEEPALIVE int fiber(int pattern, int ms) {
  volatile unsigned char buf[FRAME];
  for (int i = 0; i < FRAME; i++) {
    buf[i] = (unsigned char)(pattern + i);
  }
  emscripten_sleep(ms);
  scribble(pattern);
  int ok = verify(buf, FRAME, pattern);
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
    // a wakes first and runs while b is still suspended.
    var a = Module['_fiber'](1, 1);
    var b = Module['_fiber'](2, 20);
    var ok = (await a) & (await b);
    callUserCallback(() => Module['_finish'](ok, Module['_stack_pointer']() == sp));
  })();
});

int main() {
  // Start from outside any fiber so that the two fibers are siblings.
  emscripten_runtime_keepalive_push();
  EM_ASM({ setTimeout(start, 0); });
  return 0;
}
