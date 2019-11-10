/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>

void waka(int x, int y, int z) {
  EM_ASM({
    out('received ' + [$0, $1, $2] + '.');
  }, x, y, z);
}

int main() {
  EM_ASM({
#if EXPORTED
    // test for additional things being exported
    assert(Module['addFunction']);
    assert(Module['lengthBytesUTF8']);
    // the main test here
    Module['dynCall']('viii', $0, [1, 4, 9]);
#else
    dynCall('viii', $0, [1, 4, 9]);
#endif
  }, &waka);
}

