// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// A promising export entered synchronously from inside another fiber's import
// call, in three shapes: the inner fiber suspends and outlives the outer, the
// inner completes synchronously, and both suspend.

#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/eventloop.h>
#include <emscripten/stack.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAME 2048

static int verify(volatile unsigned char* buf, int n, unsigned char pattern) {
  for (int i = 0; i < n; i++) {
    if (buf[i] != (unsigned char)(pattern + i)) {
      return 0;
    }
  }
  return 1;
}

__attribute__((noinline)) static void scribble(int pattern) {
  volatile unsigned char buf[2 * FRAME];
  for (int i = 0; i < 2 * FRAME; i++) {
    buf[i] = (unsigned char)~pattern;
  }
  (void)buf[2 * FRAME - 1];
}

// Plain import: starts the inner fiber synchronously.
EM_JS(void, start_inner, (int pattern, int ms), {
  Module['inner'].push(Module['_inner'](pattern, ms));
});

EMSCRIPTEN_KEEPALIVE int inner(int pattern, int ms) {
  volatile unsigned char buf[FRAME];
  for (int i = 0; i < FRAME; i++) {
    buf[i] = (unsigned char)(pattern + i);
  }
  if (ms >= 0) {
    emscripten_sleep(ms);
  }
  scribble(pattern);
  int ok = verify(buf, FRAME, pattern);
  printf("inner %d: %s\n", pattern, ok ? "intact" : "CORRUPT");
  return ok;
}

EMSCRIPTEN_KEEPALIVE int outer(int pattern, int inner_ms, int outer_ms) {
  volatile unsigned char buf[FRAME];
  for (int i = 0; i < FRAME; i++) {
    buf[i] = (unsigned char)(pattern + i);
  }
  start_inner(pattern + 100, inner_ms);
  if (outer_ms >= 0) {
    emscripten_sleep(outer_ms);
  }
  scribble(pattern);
  int ok = verify(buf, FRAME, pattern);
  printf("outer %d: %s\n", pattern, ok ? "intact" : "CORRUPT");
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
    Module['inner'] = [];
    var ok = 1;
    // Inner suspends, outer finishes first.
    ok &= await Module['_outer'](1, 5, -1);
    ok &= await Module['inner'].pop();
    // Inner completes synchronously.
    ok &= await Module['_outer'](2, -1, -1);
    ok &= await Module['inner'].pop();
    // Both suspend, inner's base is inside outer's region.
    ok &= await Module['_outer'](3, 10, 1);
    ok &= await Module['inner'].pop();
    callUserCallback(() => Module['_finish'](ok, Module['_stack_pointer']() == sp));
  })();
});

int main() {
  emscripten_runtime_keepalive_push();
  EM_ASM({ setTimeout(start, 0); });
  return 0;
}
