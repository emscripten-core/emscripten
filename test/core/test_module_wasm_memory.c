/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

int main() {
  assert(EM_ASM_INT({
    return Module['wasmMemory'] === wasmMemory && wasmMemory.buffer === HEAP8.buffer;
  }));
  puts("success");
}
