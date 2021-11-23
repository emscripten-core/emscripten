/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>

int result = 0;

void EMSCRIPTEN_KEEPALIVE success() {
  printf("success? %d\n", result);
  assert(result == 10);
}

void EMSCRIPTEN_KEEPALIVE later() {
  printf("later, now force an exit\n");
  result += 10;
  emscripten_force_exit(result);
}

int main() {
  atexit(success);

  EM_ASM({
    setTimeout(function() {
      Module._later();
    }, 1000);
  });

  printf("exit, but still alive\n");
  emscripten_exit_with_live_runtime();
  __builtin_trap();
  return 0;
}

