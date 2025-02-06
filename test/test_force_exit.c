/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/eventloop.h>

bool done = false;

void success() {
  printf("atexit: done=%d\n", done);
  assert(done);
}

void EMSCRIPTEN_KEEPALIVE later() {
  printf("later, now force an exit\n");
  done = true;
  emscripten_force_exit(0);
}

EM_JS_DEPS(deps, "$callUserCallback")

int main() {
  atexit(success);

  EM_ASM({
    // Use callUserCallback here so that ExitStatus is handled correctly
    setTimeout(() => callUserCallback(Module._later), 1);
  });

  printf("exit, but still alive\n");
  emscripten_exit_with_live_runtime();
  __builtin_trap();
  return 99;
}

