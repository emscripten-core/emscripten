// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <exception>

int main() {
  std::exception e;
  try {
    throw e;
  }
  catch (std::exception e) {
    printf("what? %s\n", e.what());
    printf("caught std::exception\n");
  }
  catch (int x) {
    throw std::bad_exception();
  }
  return 0;
}
