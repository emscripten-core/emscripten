/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>
#include <fenv.h>

const char *mode(int mode)
{
  switch(mode)
  {
    case FE_DOWNWARD: return "FE_DOWNWARD";
    case FE_TONEAREST: return "FE_TONEAREST";
    case FE_TOWARDZERO: return "FE_TOWARDZERO";
    case FE_UPWARD: return "FE_UPWARD";
    default: return "Unknown";
  }
}

#define NUMELEMS(x) (sizeof((x)) / sizeof((x)[0]))

int main() {
  printf("Initial rounding mode is %s\n", mode(fegetround()));

  // The functions round() and roundf() are specced to always round *away* from zero (negative numbers down, positive numbers up)
  // fesetround() mode dictates what to do when rounding halfway numbers (-0.5, 0.5, 1.5, 2.5, ...) in functions rint(), rintf(), lrint(), lrintf(), llrint() and llrintf():
  // FE_DOWNWARD always rounds down to the smaller integer.
  // FE_TONEAREST performs banker's rounding (always round towards the even number)
  // FE_TOWARDZERO rounds negative numbers up, and positive numbers down
  // FE_UPWARD always rounds up to the next higher integer.
//  const int modes[] = { FE_DOWNWARD, FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD }; // TODO: Currently only supported mode is FE_TONEAREST.
  const int modes[] = { FE_TONEAREST };
  const double interestingDoubles[] = { -4.5,  -3.6,  -3.5,  -3.4,  -2.6,  -2.5,  -2.4,  -1.5,  -0.5,  0, 0.5,  1.4,  1.5,  1.6,  2.5,  3.5,  4.5 };
  const float interestingFloats[] =   { -4.5f, -3.6f, -3.5f, -3.4f, -2.6f, -2.5f, -2.4f, -1.5f, -0.5f, 0, 0.5f, 1.4f, 1.5f, 1.6f, 2.5f, 3.5f, 4.5f };

  for (int i = 0; i < NUMELEMS(modes); ++i)
  {
    int ret = fesetround(modes[i]);
    int modeAfter = fegetround();
    printf("fesetround(%s) returned %d, fegetround() afterwards returns %s. Test results:\n", mode(modes[i]), ret, mode(modeAfter));

    for (int j = 0; j < NUMELEMS(interestingDoubles); ++j)
      printf("%s: round(%.1f)=%.1f\n", mode(modeAfter), interestingDoubles[j], round(interestingDoubles[j]));

    for (int j = 0; j < NUMELEMS(interestingFloats); ++j)
      printf("%s: roundf(%.1f)=%.1f\n", mode(modeAfter), interestingFloats[j], roundf(interestingFloats[j]));

    for (int j = 0; j < NUMELEMS(interestingDoubles); ++j)
      printf("%s: rint(%.1f)=%.1f\n", mode(modeAfter), interestingDoubles[j], rint(interestingDoubles[j]));

    for (int j = 0; j < NUMELEMS(interestingDoubles); ++j)
      printf("%s: rintf(%.1f)=%.1f\n", mode(modeAfter), interestingFloats[j], rint(interestingFloats[j]));

    for (int j = 0; j < NUMELEMS(interestingDoubles); ++j)
      printf("%s: lrint(%.1f)=%ld\n", mode(modeAfter), interestingDoubles[j], lrint(interestingDoubles[j]));

    for (int j = 0; j < NUMELEMS(interestingDoubles); ++j)
      printf("%s: lrintf(%.1f)=%ld\n", mode(modeAfter), interestingFloats[j], lrintf(interestingFloats[j]));

    for (int j = 0; j < NUMELEMS(interestingDoubles); ++j)
      printf("%s: llrint(%.1f)=%lld\n", mode(modeAfter), interestingDoubles[j], llrint(interestingDoubles[j]));

    for (int j = 0; j < NUMELEMS(interestingDoubles); ++j)
      printf("%s: llrintf(%.1f)=%lld\n", mode(modeAfter), interestingFloats[j], llrintf(interestingFloats[j]));
  }

  double param, fractpart, intpart;

  param = 3.14159265;
  fractpart = modf (param , &intpart);
  printf ("%f = %f + %f\n", param, intpart, fractpart);

  param = -3.14159265;
  fractpart = modf (param , &intpart);
  printf ("%f = %f + %f\n", param, intpart, fractpart);
  return 0;
}
