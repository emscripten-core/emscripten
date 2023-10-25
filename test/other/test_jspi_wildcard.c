// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

EM_ASYNC_JS(int, test, (), {
  const promise1 = Module._async1();
  assert(promise1 instanceof Promise);
  await promise1;
  const promise2 = Module._async2();
  assert(promise2 instanceof Promise);
  await promise2;
  assert(!(Module._sync() instanceof Promise));
});

EMSCRIPTEN_KEEPALIVE int async1() {
  emscripten_sleep(0);
  return 99;
}

EMSCRIPTEN_KEEPALIVE int async2() {
  emscripten_sleep(0);
  return 99;
}

EMSCRIPTEN_KEEPALIVE int sync() {
  return 99;
}

int main() {
  test();
  printf("done\n");
  return 0;
}
