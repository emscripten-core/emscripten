// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <string>
#include <iostream>
#include <stdexcept>

int main() {
  std::string s;
  try {
    std::cout << s.at(0) << std::endl;
  } catch (const std::out_of_range& e) {
    std::cout << "Exception caught:" << std::endl;
    std::cout << e.what() << std::endl;
  }
  return 0;
}

