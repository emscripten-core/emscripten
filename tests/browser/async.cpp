// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>
#include <assert.h>

int main() {
  // infinite main loop, turned async via asyncify
  int counter = 0;
  while (1) {
    printf("frame: %d\n", ++counter);
    emscripten_sleep(100);
    if (counter == 10) {
      return 0;
    }
  }
  __builtin_trap();
}

