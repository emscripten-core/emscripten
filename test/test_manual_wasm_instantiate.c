// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten/em_asm.h>

int main() {
  printf("in main\n");
  int result = EM_ASM_INT({return Module.testWasmInstantiationSucceeded;});
  assert(result);
  return 0;
}
