// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>

int main() {
  {
    volatile double d = 17179870521;
    EM_ASM({
      out('|' + $0 + '|')
    }, int(d));
  }
  {
    // unsigned
    volatile double d = 4294967295.0;
    EM_ASM({
      out('|' + ($0 >>> 0) + '|')
    }, unsigned(d));
  }
}

