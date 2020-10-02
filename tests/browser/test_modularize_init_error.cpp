/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>

class ThrowsOnConstruction {
 public:
  ThrowsOnConstruction() {
    EM_ASM({
      throw "intentional error to test rejection";
    });
  }
} throws_;

int main() {
  return 0;
}
