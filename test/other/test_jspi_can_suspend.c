// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>

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
  assert(!emscripten_can_suspend());
  throw_error();
}

EMSCRIPTEN_KEEPALIVE void promising_inner(void) {
  assert(emscripten_can_suspend());
  sleep_ms(10);
  assert(emscripten_can_suspend());
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

EM_JS(void, call_promising_inner, (), {
  _promising_inner();
});

EM_JS(void, plain_from_js_timer, (), {
  setTimeout(() => {
    if (_plain() !== 0) throw new Error('plain export from timer can suspend');
  });
});

void during_suspend(void* arg) {
  assert(!emscripten_can_suspend());
}

void after_complete(void* arg) {
  assert(!emscripten_can_suspend());
  printf("done\n");
}

int main() {
  assert(emscripten_can_suspend());

  assert(call_plain() == 0);
  assert(emscripten_can_suspend());

  assert(call_plain_throws());
  assert(emscripten_can_suspend());

  sleep_ms(0);
  assert(emscripten_can_suspend());

  // The inner activation suspends, returning control here.
  call_promising_inner();
  assert(emscripten_can_suspend());

  emscripten_set_timeout(during_suspend, 0, NULL);
  plain_from_js_timer();
  sleep_ms(20);
  assert(emscripten_can_suspend());

  plain_from_js_timer();
  emscripten_set_timeout(after_complete, 10, NULL);
  return 0;
}
