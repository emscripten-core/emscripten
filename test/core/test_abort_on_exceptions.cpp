/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/eventloop.h>

EM_JS(void, throwException, (void), {
  throw new Error("crash");
});

EMSCRIPTEN_KEEPALIVE extern "C" void cFunc() {
  printf("crashing\n");
  throwException();
}

void embindFunc() {
  printf("shouldn't run\n");
}

EMSCRIPTEN_BINDINGS(test_abort_on_exception) {
  emscripten::function("embindFunc", &embindFunc);
}

int main() {
  emscripten_runtime_keepalive_push();
  return 0;
}
