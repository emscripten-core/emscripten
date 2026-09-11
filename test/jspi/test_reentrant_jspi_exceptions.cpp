// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Error paths: a rejecting import caught inside the fiber after suspension, a
// rejection escaping the promising export after a suspension (the case-2 stack
// reset on the throw path), and C++ throws with and without a suspension. A
// second fiber is kept suspended throughout so that a leaked region would be
// noticed.

#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/eventloop.h>
#include <emscripten/stack.h>
#include <stdexcept>
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

EM_ASYNC_JS(void, reject_after_tick, (), {
  await new Promise((resolve) => setTimeout(resolve, 1));
  throw new Error('rejected');
});

extern "C" {

// Kept suspended while the error cases run.
EMSCRIPTEN_KEEPALIVE int bystander(int pattern) {
  volatile unsigned char buf[FRAME];
  for (int i = 0; i < FRAME; i++) {
    buf[i] = (unsigned char)(pattern + i);
  }
  emscripten_sleep(50);
  scribble(pattern);
  int ok = verify(buf, FRAME, pattern);
  printf("bystander: %s\n", ok ? "intact" : "CORRUPT");
  return ok;
}

// Destructors are the landing pads that run in the fiber's frames while a JS
// rejection unwinds through them (it cannot be caught by C++).
struct Verifier {
  volatile unsigned char* buf;
  int pattern;
  ~Verifier() {
    scribble(pattern);
    printf("fiber %d unwinding: %s\n", pattern,
           verify(buf, FRAME, pattern) ? "intact" : "CORRUPT");
  }
};

// mode 0: import rejection unwinds through the fiber's frames
// mode 1: import rejection escapes the export from a nested frame
// mode 2: C++ throw without any suspension, caught
// mode 3: C++ throw after a suspension, escapes the export
EMSCRIPTEN_KEEPALIVE int fiber(int pattern, int mode) {
  volatile unsigned char buf[FRAME];
  for (int i = 0; i < FRAME; i++) {
    buf[i] = (unsigned char)(pattern + i);
  }
  if (mode == 0) {
    Verifier v = {buf, pattern};
    reject_after_tick();
  } else if (mode == 1) {
    reject_after_tick();
  } else if (mode == 2) {
    try {
      throw std::runtime_error("sync");
    } catch (const std::exception& e) {
      scribble(pattern);
      int ok = verify(buf, FRAME, pattern);
      printf("fiber %d caught %s: %s\n", pattern, e.what(), ok ? "intact" : "CORRUPT");
      return ok;
    }
  } else {
    emscripten_sleep(1);
    scribble(pattern);
    if (!verify(buf, FRAME, pattern)) {
      printf("fiber %d: CORRUPT before throw\n", pattern);
    }
    throw std::runtime_error("after suspend");
  }
  return 0;
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

}

EM_JS(void, start, (), {
  (async () => {
    var sp = Module['_stack_pointer']();
    var bystander = Module['_bystander'](7);
    async function expectReject(p, what) {
      try {
        await p;
        console.log(what, 'resolved unexpectedly');
        return 0;
      } catch (e) {
        console.log(what, 'rejected with', e instanceof WebAssembly.Exception ? 'WebAssembly.Exception' : e.message);
        return 1;
      }
    }
    var ok = 1;
    ok &= await expectReject(Module['_fiber'](1, 0), 'unwound rejection');
    ok &= await expectReject(Module['_fiber'](2, 1), 'escaping rejection');
    ok &= await Module['_fiber'](3, 2);
    ok &= await expectReject(Module['_fiber'](4, 3), 'throw after suspend');
    // A fresh fiber after the error exits.
    ok &= await Module['_fiber'](5, 2);
    ok &= await bystander;
    callUserCallback(() => Module['_finish'](ok, Module['_stack_pointer']() == sp));
  })();
});

int main() {
  emscripten_runtime_keepalive_push();
  EM_ASM({ setTimeout(start, 0); });
  return 0;
}
