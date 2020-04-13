// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stddef.h>
#include <emscripten.h>

__attribute__((aligned(16))) volatile char aligned;

int main() {
  assert((((size_t) &aligned) % 16) == 0);
#if __wasm__
  EM_ASM({
    out('tempDoublePtr alignment: 0.');
  });
#else
  EM_ASM({
    // whether the char was properly aligned affects tempDoublePtr, which is after the static aligns
    out('tempDoublePtr: ' + tempDoublePtr + '.');
    out('tempDoublePtr alignment: ' + (tempDoublePtr % 16) + '.');
  });
#endif
}

