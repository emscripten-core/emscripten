// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>
#include <new>

char* data;

int main() {
  data = new (std::nothrow) char[20 * 1024 * 1024];
  if (data == nullptr) {
    std::cout << "success" << std::endl;
    return 0;
  } else {
    std::cout << "failure" << std::endl;
    return 1;
  }
}
