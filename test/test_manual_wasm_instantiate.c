// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <emscripten/em_asm.h>
#include <stdio.h>

EM_ASYNC_JS(int, getResult, (), {
  return Module.testWasmInstantiationSucceeded;
});

int main() {
  printf("in main\n");
  int result = getResult();
  assert(result);
  return 0;
}
