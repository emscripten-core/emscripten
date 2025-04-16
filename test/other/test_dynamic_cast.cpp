/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

struct Support {
  virtual void f() { printf("f()\n"); }
};

struct Derived : Support {};

int main() {
  Support* p = new Derived;
  dynamic_cast<Derived*>(p)->f();
  delete p;
  return 0;
}
