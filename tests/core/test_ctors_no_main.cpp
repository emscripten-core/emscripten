/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>

int foo = 1;

struct A {
  // Use EM_ASM to prevent ctor eval from working.
  A() { foo = EM_ASM_INT({ return 99 }); }
};
A a;

extern "C" int get_foo() {
  return foo;
}
