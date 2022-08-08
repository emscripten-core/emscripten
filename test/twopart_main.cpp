// Copyright 2011 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#ifndef __cplusplus
#error "should be compiled as C++"
#endif

// Test that C++ headers are on the include path
#include <vector>

extern "C" void theFunc(const char *str);


int main() {
  theFunc("hello from main");
  return 0;
}

