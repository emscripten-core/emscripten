// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>

struct WithBitField {
    unsigned x:2;
};

enum SomeEnum { SIXTY = 60 };

int main(int argc, char **argv) {
  // Ensure that a subset of default argument conversions is held.
  // Promotions of arrays, function/member pointers and objects implicitly
  // convertible to numbers are excluded because they will not be translated
  // to corresponding JS objects.
#define TEST_TYPE(type, value) EM_ASM({out(#type, Number($0));}, (type)(value));
  TEST_TYPE(int*, 0);
  TEST_TYPE(float, 1.5f);
  TEST_TYPE(double, 2.5);

  TEST_TYPE(char, 10);
  TEST_TYPE(signed char, -10);
  TEST_TYPE(unsigned char, 10);

  TEST_TYPE(short, -20);
  TEST_TYPE(signed short, -20);
  TEST_TYPE(unsigned short, 20);

  TEST_TYPE(int, -30);
  TEST_TYPE(signed int, -30);
  TEST_TYPE(unsigned int, 30);

  TEST_TYPE(long, -40);
  TEST_TYPE(signed long, -40);
  TEST_TYPE(unsigned long, 40);

  struct WithBitField w;
  w.x = 3;
  EM_ASM({ out('bit field', $0); }, w.x);

#ifdef __cplusplus
  TEST_TYPE(bool, true);
  TEST_TYPE(wchar_t, 50);
#else
  EM_ASM({out('bool 1')});
  EM_ASM({out('wchar_t 50')});
#endif

  TEST_TYPE(enum SomeEnum, SIXTY);
  return 0;
}
