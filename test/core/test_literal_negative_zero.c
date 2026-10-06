/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>

float XXXf = -0.0f;
double XXXd = -0.0;

struct x {
  float f;
  double d;
};

struct x xx[] = {
  -0x0p+0,
  -0x0p+0,
};

int main(int argc, char ** argv) {
  float YYYf = -0.0f;
  float YYYd = -0.0;

  printf("%.2f\n", XXXf);
  printf("%.2f\n", XXXd);
  printf("%.2f\n", YYYf);
  printf("%.2f\n", YYYd);
  printf("%.2f\n", xx->f);
  printf("%.2f\n", xx->d);
}
