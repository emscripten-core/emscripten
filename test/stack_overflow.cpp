// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>

#include <emscripten/em_asm.h>
#include <emscripten/em_macros.h>

EM_JS_DEPS(main, "$stringToUTF8OnStack");

void __attribute__((noinline)) InteropString(char *staticBuffer) {
  char *string = (char*)EM_ASM_PTR({
    var str = "hello, this is a string! ";
#if ONE_BIG_STRING
    // double it until it is bigger than the stack
    for (var i = 0; i < 15; ++i) {
      str = str + str;
    }
    stringToUTF8OnStack(str);
#else
    // allocate as many times as we need to overflow
    for (var i = 0; i < 1024 * 1024; i++) {
      stringToUTF8OnStack(str);
    }
    abort("we should never get here!");
#endif
  });
}

int main() {
  // Make C side consume a large portion of the stack, before bumping the rest with C++<->JS interop.
  char staticBuffer[512288] = {};
  InteropString(staticBuffer);
  int stringLength = strlen(staticBuffer);
  printf("Got string: %s\n", staticBuffer);
  printf("Received a string of length %d.\n", stringLength);
  return 0;
}
