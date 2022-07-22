/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>

double testVu, testVv, testWu, testWv;

void Test(double _testVu, double _testVv, double _testWu, double _testWv) {
  testVu = _testVu;
  testVv = _testVv;
  testWu = _testWu;
  testWv = _testWv;
  printf("BUG?\n");
  printf("Display: Vu=%f  Vv=%f  Wu=%f  Wv=%f\n", testVu, testVv, testWu,
         testWv);
}

int main(void) {
  double v1 = 465.1;
  double v2 = 465.2;
  double v3 = 160.3;
  double v4 = 111.4;
  Test(v1, v2, v3, v4);
  return 0;
}
