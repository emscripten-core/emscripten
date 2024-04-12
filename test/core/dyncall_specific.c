/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <stdint.h>
#include <stdio.h>

int waka(int w, long long xy, int z) {
#ifdef WASM_BIGINT
  // With WASM_BIGINT things are straightforward: the 64-bit value just arrives
  // with the expected value of 4.
  assert(w == 1);
  assert(xy == 4);
  assert(z == 9);
#else
  // xy should be 0xffff_ffff_0000_0004
  int x = (int) xy;
  int y = xy >> 32;
  assert(w == 1);
  assert(x == 4);
  assert(y == -1);
  assert(z == 9);
#endif
  return 42;
}

EM_JS_DEPS(main, "$dynCall");

int main() {
  EM_ASM({

#ifdef WASM_BIGINT

#if DIRECT
    console.log('Received ' + dynCall_iiji($0, 1, BigInt(4), 9));
    return;
#endif
#if DYNAMIC_SIG
    console.log('Received ' + dynCall('iiji', $0, [1, BigInt(4), 9]));
    return;
#endif
#if EXPORTED
    console.log('Received ' + Module['dynCall_iiji']($0, 1, BigInt(4), 9));
    return;
#endif
#if EXPORTED_DYNAMIC_SIG
    console.log('Received ' + Module['dynCall']('iiji', $0, [1, BigInt(4), 9]));
    return;
#endif
#if FROM_OUTSIDE
    eval("console.log('Received ' + Module['dynCall_iiji'](" + $0 + ", 1, BigInt(4), 9))");
    return;
#endif

#else // WASM_BIGINT

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

#endif

    // We should have run the test and returned before we get here.
    throw "no test mode";
  }, &waka);
}

