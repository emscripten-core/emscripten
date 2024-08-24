/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <iostream>

int main() {
  using namespace std;
  use_facet<num_put<char> >(cout.getloc()).put(cout, cout, '0', 3.14159265);
  std::cout << "\n";
}
