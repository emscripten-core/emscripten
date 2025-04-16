// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

// Use EMSCRIPTEN_KEEPALIVE to make sure this function ends up in the wasmTable
EMSCRIPTEN_KEEPALIVE void sync1() {
  printf("done\n");
}

EM_JS(int, getFunc, (em_callback_func func), {
  /* this calls updateTableMap, which will call getWasmTableEntry on all functions
     in the wasmTable. If sync1 was registered as asynchronous, that function will fail */
  return Module.addFunction(() => dynCall("v", func), "i");
});

int main() {
  int (*callback)() = (int (*)())getFunc(&sync1);
  callback();
}
