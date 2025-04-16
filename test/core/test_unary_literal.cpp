// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <cstdio>

class UnaryTest {
public:
  static const UnaryTest STATIC_INSTANCE;
  static const double STATIC_PROPERTY;

  double p;

  UnaryTest(double p);
};

const UnaryTest UnaryTest::STATIC_INSTANCE(-1);
const double UnaryTest::STATIC_PROPERTY(-1);
const double STATIC_DOUBLE(-1);

UnaryTest::UnaryTest(double inP) : p(inP) { }

int main(int, char**){
  const double t1 = -1;
  double t2 = -1;

  double p1 = -UnaryTest::STATIC_INSTANCE.p;
  double p2 = -UnaryTest::STATIC_PROPERTY;
  double p3 = -STATIC_DOUBLE;
  double p4 = -t1;
  double p5 = -t2;

  printf("%.2f\n", p1);
  printf("%.2f\n", p2);
  printf("%.2f\n", p3);
  printf("%.2f\n", p4);
  printf("%.2f\n", p5);

  return 0;
}
