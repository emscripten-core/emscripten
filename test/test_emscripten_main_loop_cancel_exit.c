/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

void loop(void) {
  emscripten_cancel_main_loop();
  exit(0);
}

int main(void) {
  emscripten_set_main_loop(loop, -1, 0);
  return 99;
}
