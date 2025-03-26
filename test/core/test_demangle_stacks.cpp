// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>

namespace NameSpace {
class Class {
 public:
  __attribute__((noinline))
  void Aborter(double x, char y, int *z) {
    volatile int w = 1;
    if (w) {
      if (EM_ASM_INT({
        out(jsStackTrace());
      }) == 999999) {
        // Add a fake call (that never happens in practice) to avoid the
        // binaryen optimizer from inlining this method.
        Aborter(x, y, z);
      }
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
