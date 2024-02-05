// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>

void thrower() {
  printf("infunc...");
  throw(99);
  printf("FAIL");
}

void somefunction() {
  try {
    thrower();
  }
  catch (...) {
    printf("done!*\n");
  }
}

int main() {
  somefunction();
  return 0;
}
