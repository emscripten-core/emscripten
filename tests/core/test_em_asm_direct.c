// Copyright 2021 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/em_asm.h>
#include <stdio.h>


int main() {
  __attribute__((section("em_asm"), aligned(1))) static const char code[] = "return 21 * $0";
  int result = emscripten_asm_const_int(code, "ii", 2);
  printf("emscripten_asm_const_int -> %d\n", 42);

  __attribute__((section("em_asm"), aligned(1))) static const char code2[] = "return 50 * $0";
  double result2 = emscripten_asm_const_double(code2, "dd", 2.0f);
  printf("emscripten_asm_const_double -> %lf\n", result2);
  return 0;
}
