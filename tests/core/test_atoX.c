/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
  printf("%d*", atoi(""));
  printf("%d*", atoi("a"));
  printf("%d*", atoi(" b"));
  printf("%d*", atoi(" c "));
  printf("%d*", atoi("6"));
  printf("%d*", atoi(" 5"));
  printf("%d*", atoi("4 "));
  printf("%d*", atoi("3 6"));
  printf("%d*", atoi(" 3 7"));
  printf("%d*", atoi("9 d"));
  printf("%d\n", atoi(" 8 e"));
  printf("%d*", (int)atol(""));
  printf("%d*", (int)atol("a"));
  printf("%d*", (int)atol(" b"));
  printf("%d*", (int)atol(" c "));
  printf("%d*", (int)atol("6"));
  printf("%d*", (int)atol(" 5"));
  printf("%d*", (int)atol("4 "));
  printf("%d*", (int)atol("3 6"));
  printf("%d*", (int)atol(" 3 7"));
  printf("%d*", (int)atol("9 d"));
  printf("%d\n", (int)atol(" 8 e"));
  printf("%lld*", atoll("6294967296"));
  printf("%lld*", atoll(""));
  printf("%lld*", atoll("a"));
  printf("%lld*", atoll(" b"));
  printf("%lld*", atoll(" c "));
  printf("%lld*", atoll("6"));
  printf("%lld*", atoll(" 5"));
  printf("%lld*", atoll("4 "));
  printf("%lld*", atoll("3 6"));
  printf("%lld*", atoll(" 3 7"));
  printf("%lld*", atoll("9 d"));
  printf("%lld\n", atoll(" 8 e"));
  return 0;
}
