// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <iostream>

struct NodeInfo {  // structure that we want to transmit to our shaders
  float x;
  float y;
  float s;
  float c;
};
const int nbNodes = 100;
NodeInfo* data = new NodeInfo[nbNodes];  // our data that will be transmitted
                                         // using float texture.

template <int i>
void printText(const char (&text)[i]) {
  std::cout << text << std::endl;
}

int main() {
  printText("some string constant");
  return 0;
}
