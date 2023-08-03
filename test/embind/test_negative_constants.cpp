// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

#define NEGATIVE_FLOAT_NUM -3.1416
#define NEGATIVE_INT_NUM -10
const float negative_float_num = -3.1416;
const double negative_double_num = -2.7182818;
const int negative_int_num = -10;

EMSCRIPTEN_BINDINGS(constants) {
  emscripten::constant("NEGATIVE_FLOAT_NUM", NEGATIVE_FLOAT_NUM);
  emscripten::constant("NEGATIVE_INT_NUM", NEGATIVE_INT_NUM);
  emscripten::constant("negative_float_num", negative_float_num);
  emscripten::constant("negative_double_num", negative_double_num);
  emscripten::constant("negative_int_num", negative_int_num);
}

int main() {
    EM_ASM(
      out("NEGATIVE_FLOAT_NUM = " + Module['NEGATIVE_FLOAT_NUM']);
      out("NEGATIVE_INT_NUM = " + Module['NEGATIVE_INT_NUM']);
      out("negative_float_num = " + Module['negative_float_num']);
      out("negative_double_num = " + Module['negative_double_num']);
      out("negative_int_num = " + Module['negative_int_num']);
    );
}
