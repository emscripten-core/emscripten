/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <exception>

class Foo {
  public:
    ~Foo() {
      printf("in destructor: %d\n", std::uncaught_exceptions());
    }
};

int main() {
  try {
    Foo f;
    try {
      throw std::exception();
    } catch (...) {
      printf("in inner catch: %d\n", std::uncaught_exceptions());
      throw;
    }
  } catch (...) {
    printf("in outer catch: %d\n", std::uncaught_exceptions());
  }
}
