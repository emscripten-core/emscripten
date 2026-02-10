/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <iostream>
template <int i>
void printText(const char (&text)[i]) {
  std::cout << text << "\n";
}
int main() {
  printText("some string constant");
  return 0;
}
