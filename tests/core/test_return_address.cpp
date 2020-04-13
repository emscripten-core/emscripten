/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <cassert>
#include <cstdio>

extern "C" void *emscripten_return_address(int level);

void func(void) {
  assert(emscripten_return_address(0) != 0);
  assert(emscripten_return_address(50) == 0);
}

// We need to take these two arguments or clang can potentially generate
// a main function that takes two arguments and calls our main, messing up
// the stack trace and breaking this test.
int main(int argc, char **argv) {
  assert(emscripten_return_address(50) == 0);
  func();
  std::puts("passed");
}
