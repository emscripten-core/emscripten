/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <string>
#include <sstream>
#include <iostream>

int main() {
  std::string mystring("1 2 3");
  std::istringstream is(mystring);
  int one, two, three;

  is >> one >> two >> three;

  printf("%i %i %i\n", one, two, three);
}
