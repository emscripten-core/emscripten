/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Test RAF is actually used (and not setTimeout etc.)

#include <assert.h>
#include <stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE extern "C" void good() {
  printf("good!\n");
  emscripten_force_exit(0);
}

void bad() {
  printf("bad!\n");
  assert(false);
}

int main() {
  EM_ASM({
    requestAnimationFrame = function() {
      Module._good();
    };
  });
  emscripten_set_main_loop(bad, 0, 1);
  return 99;
}

