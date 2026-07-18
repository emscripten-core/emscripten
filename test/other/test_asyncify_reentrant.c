// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten/em_js.h>
#include <emscripten/emscripten.h>

EM_ASYNC_JS(void, sleep_ms, (int ms), {
  await new Promise((resolve) => setTimeout(resolve, ms));
});

// A promising export whose stack frame must survive suspension while other
// promising calls use the same stack region.
EMSCRIPTEN_KEEPALIVE int work(int val, int ms) {
  volatile int buf[64];
  for (int i = 0; i < 64; i++) buf[i] = val + i;
  sleep_ms(ms);
  for (int i = 0; i < 64; i++) assert(buf[i] == val + i);
  return val;
}

// Consumes enough stack to reach into the region used by a suspended work()
// call if its stack were not saved away.
EMSCRIPTEN_KEEPALIVE int work_big(int val, int ms) {
  volatile int buf[2048];
  for (int i = 0; i < 2048; i++) buf[i] = val + i;
  sleep_ms(ms);
  for (int i = 0; i < 2048; i++) assert(buf[i] == val + i);
  return val;
}

// A promising entry made mid-stack: main -> foo -> bar -> JS -> work, with
// live allocations in the outer frames occupying the initial stack space.
// The suspended work call resumes on the empty stack after the outer frames
// have been saved away, restoring its (self-contained) frame relocated.
EM_JS(void, start_work_from_js, (), {
  globalThis.workPromise = _work(70000, 15);
});

EM_ASYNC_JS(int, wait_work, (), {
  return globalThis.workPromise;
});

__attribute__((noinline)) void bar(void) {
  volatile int buf[128];
  for (int i = 0; i < 128; i++) buf[i] = 2 * i;
  start_work_from_js();
  for (int i = 0; i < 128; i++) assert(buf[i] == 2 * i);
}

__attribute__((noinline)) void foo(void) {
  volatile int buf[128];
  for (int i = 0; i < 128; i++) buf[i] = 3 * i;
  bar();
  for (int i = 0; i < 128; i++) assert(buf[i] == 3 * i);
}

EM_ASYNC_JS(void, run_overlapping, (), {
  // Overlapping promising calls, each entered while the others are suspended.
  const results = await Promise.all([_work(100, 30), _work(2000, 20), _work(30000, 10)]);
  if (results[0] != 100 || results[1] != 2000 || results[2] != 30000) {
    throw new Error('bad results: ' + results);
  }
  // Enter and complete a large call after an earlier call has completed,
  // while another remains suspended.
  const a = _work(100, 10);
  const b = _work(2000, 30);
  await a;
  const c = _work_big(50000, 0);
  const rest = await Promise.all([b, c]);
  if (rest[0] != 2000 || rest[1] != 50000) {
    throw new Error('bad results: ' + rest);
  }
});

int main() {
  // Suspension within a plain call chain under a promising export.
  assert(work(42, 10) == 42);
  run_overlapping();
  foo();
  assert(wait_work() == 70000);
  printf("done\n");
  return 0;
}
