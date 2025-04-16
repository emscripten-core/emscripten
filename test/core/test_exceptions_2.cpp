// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdexcept>
#include <stdio.h>

typedef void (*FuncPtr)();

void ThrowException() { throw std::runtime_error("catch me!"); }

FuncPtr ptr = ThrowException;

int main() {
  try {
    ptr();
  }
  catch (...) {
    printf("Exception caught successfully!\n");
  }
  return 0;
}
