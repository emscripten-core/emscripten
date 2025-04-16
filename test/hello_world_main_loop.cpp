// Copyright 2020 The Emscripten Authors.  All rights reserved.  Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

void looper() {
  static int frame = 0;
  frame++;
  if (frame == 10) {
    puts("hello, world!");
    emscripten_cancel_main_loop();
  }
}

int main() {
  emscripten_set_main_loop(looper, 0, 0);
}
