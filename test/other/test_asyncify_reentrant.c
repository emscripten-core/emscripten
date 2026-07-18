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
  printf("done\n");
  return 0;
}
