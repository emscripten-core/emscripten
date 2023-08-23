// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#define PI 3.1416
#define EULER 2.7182818
const float pi = 3.1416;
const double euler = 2.7182818;

EMSCRIPTEN_BINDINGS(constants) {
  emscripten::constant("PI", PI);
  emscripten::constant("EULER", EULER);
  emscripten::constant("pi", pi);
  emscripten::constant("euler", euler);
}

int main() {
  EM_ASM(
    out("PI (as double) = " + Module['PI']);
    out("EULER = " + Module['EULER']);
    out("pi (as float) = " + Module['pi']);
    out("euler = " + Module['euler']);
  );
}
