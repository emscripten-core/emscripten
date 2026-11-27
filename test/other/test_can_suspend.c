// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>

// With ASYNCIFY=1 any export can suspend, so only an async operation already
// in flight prevents it.
#define JSPI (emscripten_has_asyncify() == 2)

EM_ASYNC_JS(void, sleep_ms, (int ms), {
  await new Promise((resolve) => setTimeout(resolve, ms));
});

EM_JS(void, throw_error, (), {
  throw new Error('expected');
});

EMSCRIPTEN_KEEPALIVE int plain(void) {
  return emscripten_can_suspend();
}

EMSCRIPTEN_KEEPALIVE void plain_throws(void) {
  assert(emscripten_can_suspend() == !JSPI);
  throw_error();
}

// Checks that its stack frame survives other activations running meanwhile.
EMSCRIPTEN_KEEPALIVE void promising_inner(int ms) {
#ifdef OVERFLOW
  // Overflows its stack into the guard region below it.
  volatile char buf[24 * 1024];
#else
  volatile char buf[256];
#endif
  for (int i = 0; i < sizeof(buf); i++) buf[i] = ms;
  assert(emscripten_can_suspend());
  sleep_ms(ms);
  for (int i = 0; i < sizeof(buf); i++) assert(buf[i] == ms);
  assert(emscripten_can_suspend());
}

EMSCRIPTEN_KEEPALIVE void clobber(void) {
  volatile char buf[4096];
  for (int i = 0; i < sizeof(buf); i++) buf[i] = -1;
}

EM_JS(int, call_plain, (), {
  return _plain();
});

EM_JS(int, call_plain_throws, (), {
  try {
    _plain_throws();
  } catch (e) {
    return e.message == 'expected';
  }
  return 0;
});

EM_JS(void, call_promising_inner, (int ms, int delay), {
  if (delay < 0) _promising_inner(ms);
  else setTimeout(() => _promising_inner(ms), delay);
});

EM_JS(void, clobber_later, (int delay), {
  setTimeout(() => _clobber(), delay);
});

// Returns while the inner activation is still suspended.
__attribute__((noinline)) void start_inner(int ms) {
  volatile char pad[64];
  pad[0] = 0;
  call_promising_inner(ms, -1);
}

EM_JS(void, plain_from_js_timer, (int expected), {
  setTimeout(() => {
    if (_plain() != expected) throw new Error('unexpected can_suspend from timer');
  });
});

void during_suspend(void* arg) {
  assert(!emscripten_can_suspend());
}

void after_complete(void* arg) {
  assert(emscripten_can_suspend() == !JSPI);
  printf("done\n");
}

int main() {
#ifdef OVERFLOW
  start_inner(0);
  return 0;
#endif
  assert(emscripten_can_suspend());

  assert(call_plain() == !JSPI);
  assert(emscripten_can_suspend());

  assert(call_plain_throws());
  assert(emscripten_can_suspend());

  sleep_ms(0);
  assert(emscripten_can_suspend());

  if (JSPI) {
    // The inner activation suspends, returning control here.
    start_inner(10);
    clobber();
    assert(emscripten_can_suspend());
    // These complete out of order with this one and each other, using more
    // stacks than are kept for reuse.
    for (int i = 0; i < 6; i++) {
      call_promising_inner(30 + i % 2 * 10, i);
    }
    clobber_later(35);
  }

  emscripten_set_timeout(during_suspend, 0, NULL);
  plain_from_js_timer(0);
  sleep_ms(20);
  clobber();
  assert(emscripten_can_suspend());

  plain_from_js_timer(!JSPI);
  emscripten_set_timeout(after_complete, 60, NULL);
  return 0;
}
