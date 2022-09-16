/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
int main()
{
  long long a = 0x2b00505c10;
  long long b = a >> 29;
  long long c = a >> 32;
  long long d = a >> 34;
  printf("*%lld,%lld,%lld,%lld*\n", a, b, c, d);
  unsigned long long ua = 0x2b00505c10;
  unsigned long long ub = ua >> 29;
  unsigned long long uc = ua >> 32;
  unsigned long long ud = ua >> 34;
  printf("*%lld,%lld,%lld,%lld*\n", ua, ub, uc, ud);

  long long x = 0x0000def123450789ULL; // any bigger than this, and we
  long long y = 0x00020ef123456089ULL; // start to run into the double precision limit!
  printf("*%lld,%lld,%lld,%lld,%lld*\n", x, y, x | y, x & y, x ^ y);

  printf("*");
  long long z = 13;
  int n = 0;
  while (z > 1) {
    printf("%.2f,", (float)z); // these must be integers!
    z = z >> 1;
    n++;
  }
  printf("*%d*\n", n);
  return 0;
}
