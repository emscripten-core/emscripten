/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
extern "C" void waka(int x, int y, int z) {
  EM_ASM({
    out("ok.");
  });
}

int main() {
  EM_ASM({
    ccall("waka");
  });
}

