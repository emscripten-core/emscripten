/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <stdint.h>
#include <stdio.h>

int waka(int w, long long xy, int z) {
  // xy should be 0xffff_ffff_0000_0004
  int x = (int) xy;  // should be 4
  int y = xy >> 32;  // should be -1
  EM_ASM({
    out('received ' + [$0, $1, $2, $3] + '.');
  }, w, x, y, z);
  return 42;
}

EM_JS_DEPS(main, "$dynCall");

int main() {
  EM_ASM({
    // Note that these would need to use BigInts if the file were built with
    // -sWASM_BIGINT
#if DIRECT
    console.log('Received ' + dynCall_iiji($0, 1, 4, 0xffffffff, 9));
    return;
#endif
#if DYNAMIC_SIG
    console.log('Received ' + dynCall('iiji', $0, [1, 4, 0xffffffff, 9]));
    return;
#endif
#if EXPORTED
    console.log('Received ' + Module['dynCall_iiji']($0, 1, 4, 0xffffffff, 9));
    return;
#endif
#if EXPORTED_DYNAMIC_SIG
    console.log('Received ' + Module['dynCall']('iiji', $0, [1, 4, 0xffffffff, 9]));
    return;
#endif
#if FROM_OUTSIDE
    eval("console.log('Received ' + Module['dynCall_iiji'](" + $0 + ", 1, 4, 0xffffffff, 9))");
    return;
#endif
    throw "no test mode";
  }, &waka);
}

