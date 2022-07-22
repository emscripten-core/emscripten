// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <limits>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) || defined(__EMSCRIPTEN__)
#define FLOAT_NAN ((float)std::numeric_limits<float>::quiet_NaN())
#define FLOAT_INF ((float)std::numeric_limits<float>::infinity())
#else
#define FLOAT_NAN ((float)NAN)
#define FLOAT_INF ((float)INFINITY)
#endif

#if defined(_MSC_VER) || defined(__EMSCRIPTEN__)
#define DOUBLE_NAN ((double)std::numeric_limits<double>::quiet_NaN())
#define DOUBLE_INF ((double)std::numeric_limits<double>::infinity())
#else
#define DOUBLE_NAN ((double)NAN)
#define DOUBLE_INF ((double)INFINITY)
#endif

#ifdef _MSC_VER
#define NOINLINE
#else
#define NOINLINE __attribute__((noinline))
#endif

float NOINLINE ret_e() { return (float)2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274; }
float NOINLINE ret_minuspi() { return (float)-3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679; }
float NOINLINE val() { return 10.f; }
float NOINLINE val2() { return -10.f; }
float NOINLINE zero() { return 0.f; }
float NOINLINE zero2() { return -0.f; }

double NOINLINE dret_e() { return (double)2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274; }
double NOINLINE dret_minuspi() { return (double)-3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679; }
double NOINLINE dval() { return 10.0; }
double NOINLINE dval2() { return -10.0; }
double NOINLINE dzero() { return 0.0; }
double NOINLINE dzero2() { return -0.0; }

const float e =     ret_e();
const float negpi = ret_minuspi();
const float inf =   FLOAT_INF;
const float negInf = -FLOAT_INF;
const float floatNan = FLOAT_NAN;
const float floatMax = FLT_MAX;
const float floatMin = -FLT_MAX;
const float posValue = val();
const float negValue = val2();
const float posZero = zero();
const float negZero = zero2();

const double de =     dret_e();
const double dnegpi = dret_minuspi();
const double dinf =   DOUBLE_INF;
const double dnegInf = -DOUBLE_INF;
const double doubleNan = DOUBLE_NAN;
const double doubleMax = DBL_MAX;
const double doubleMin = -DBL_MAX;
const double dposValue = dval();
const double dnegValue = dval2();
const double dposZero = dzero();
const double dnegZero = dzero2();

int main()
{
  printf("e: %f\n", e);
  printf("negpi: %f\n", negpi);
  printf("inf: %f\n", inf);
  printf("negInf: %f\n", negInf);
  printf("floatNan: %f\n", floatNan);
  printf("floatMax: %f\n", floatMax);
  printf("floatMin: %f\n", floatMin);
  printf("posValue: %f\n", posValue);
  printf("negValue: %f\n", negValue);
  printf("posZero: %f\n", posZero);
  printf("negZero: %f\n", negZero);

  printf("e: %f\n", de);
  printf("negpi: %f\n", dnegpi);
  printf("inf: %f\n", dinf);
  printf("negInf: %f\n", dnegInf);
  printf("doubleNan: %f\n", doubleNan);
  printf("doubleMax: %f\n", doubleMax);
  printf("doubleMin: %f\n", doubleMin);
  printf("posValue: %f\n", dposValue);
  printf("negValue: %f\n", dnegValue);
  printf("posZero: %f\n", dposZero);
  printf("negZero: %f\n", dnegZero);
}
