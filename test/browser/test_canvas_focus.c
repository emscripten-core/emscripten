/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/html5.h>
#include <emscripten/key_codes.h>
#include <emscripten.h>
#include <stdio.h>
#include <string.h>

bool key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  static int i = 0;
  printf("key_callback %d\n", i);
  i++;
  emscripten_force_exit(0);
  return 0;
}

int main() {
  emscripten_set_keypress_callback("#canvas", 0, 1, key_callback);
  EM_ASM({
    Module.canvas.focus();
    simulateKeyEvent("keypress", 38, undefined, undefined, /*target=*/document.activeElement);
  });
  emscripten_exit_with_live_runtime();
  __builtin_trap();
}
