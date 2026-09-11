// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// N fibers each suspend K times with different delays so that they resume in
// a permutation of their start order, verifying their frames every round.

#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/eventloop.h>
#include <emscripten/stack.h>
#include <stdio.h>
#include <stdlib.h>

#define N 8
#define K 5
#define FRAME 1024

static int verify(volatile unsigned char* buf, int n, unsigned char pattern) {
  for (int i = 0; i < n; i++) {
    if (buf[i] != (unsigned char)(pattern + i)) {
      return 0;
    }
  }
  return 1;
}

__attribute__((noinline)) static void scribble(int pattern) {
  volatile unsigned char buf[4 * FRAME];
  for (int i = 0; i < 4 * FRAME; i++) {
    buf[i] = (unsigned char)~pattern;
  }
  (void)buf[4 * FRAME - 1];
}

EMSCRIPTEN_KEEPALIVE int fiber(int n) {
  volatile unsigned char buf[FRAME];
  for (int i = 0; i < FRAME; i++) {
    buf[i] = (unsigned char)(n + i);
  }
  for (int round = 0; round < K; round++) {
    // Fiber n sleeps (n * 7 + round * 3) % 11 ms: a different order each round.
    emscripten_sleep((n * 7 + round * 3) % 11);
    scribble(n);
    if (!verify(buf, FRAME, n)) {
      printf("fiber %d round %d: CORRUPT\n", n, round);
      return 0;
    }
  }
  return 1;
}

EMSCRIPTEN_KEEPALIVE uintptr_t stack_pointer(void) {
  return emscripten_stack_get_current();
}

EMSCRIPTEN_KEEPALIVE void finish(int ok, int sp_balanced) {
  printf("all intact: %d, sp balanced: %d\n", ok, sp_balanced);
  if (ok && sp_balanced) {
    printf("done\n");
  }
  emscripten_runtime_keepalive_pop();
  exit(0);
}

EM_JS(void, start, (), {
  (async () => {
    var sp = Module['_stack_pointer']();
    var fibers = [];
    for (var n = 1; n <= 8; n++) {
      fibers.push(Module['_fiber'](n));
    }
    var ok = (await Promise.all(fibers)).every((r) => r);
    callUserCallback(() => Module['_finish'](ok, Module['_stack_pointer']() == sp));
  })();
});

int main() {
  emscripten_runtime_keepalive_push();
  EM_ASM({ setTimeout(start, 0); });
  return 0;
}
