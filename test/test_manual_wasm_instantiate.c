// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <emscripten/em_asm.h>
#include <emscripten.h>

EM_ASYNC_JS(int, getResult, (), {
  return Module.testWasmInstantiationSucceeded;
});

int main() {
  printf("main: before getResult\n");
  int result = getResult();
  printf("testWasmInstantiationSucceeded: %d\n",result);
  return 0;
}
