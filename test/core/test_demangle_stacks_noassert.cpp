// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>

namespace NameSpace {
class Class {
 public:
  __attribute__((noinline))
  void Aborter(double x, char y, int *z) {
    volatile int w = 1;
    if (w) {
      abort();
    }
  }
};
}

int main(int argc, char **argv) {
  NameSpace::Class c;
  c.Aborter(1.234, 'a', NULL);
  return 0;
}
