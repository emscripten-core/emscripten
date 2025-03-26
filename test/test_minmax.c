/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <assert.h>
#include <float.h>
#include <stdio.h>

union fi {
  float f;
  unsigned int i;
};

unsigned int Float2Int(float f) {
  union fi a;
  a.f = f;
  return a.i;
}

float Fminf(float x, float y) {
  if (x < y || !(y == y))
    return x;
  else if (y < x || !(x == x))
    return y;
  else if (x != 0.f)
    return x;
  else
    return Float2Int(x) == 0 ? y : x;
}

float Fmaxf(float x, float y) {
  if (x > y || !(y == y))
    return x;
  else if (y > x || !(x == x))
    return y;
  else if (x != 0.f)
    return x;
  else
    return Float2Int(x) == 0 ? x : y;
}

void TestMin(float a, float b, float ret) {
  float x = Fminf(a, b);
  if (x != ret)
    printf("min(%f, %f) returned %f, should return ret: %f\n", a, b, x, ret);
  assert(Float2Int(x) == Float2Int(ret));
  float y = Fminf(b, a);
  if (y != ret)
    printf("min(%f, %f) returned %f, should return ret: %f\n", b, a, y, ret);
  assert(Float2Int(y) == Float2Int(ret));
}

void TestMax(float a, float b, float ret) {
  float x = Fmaxf(a, b);
  if (x != ret)
    printf("max(%f, %f) returned %f, should return ret: %f\n", a, b, x, ret);
  assert(Float2Int(x) == Float2Int(ret));
  float y = Fmaxf(b, a);
  if (y != ret)
    printf("max(%f, %f) returned %f, should return ret: %f\n", b, a, x, ret);
  assert(Float2Int(y) == Float2Int(ret));
}

int main() {
  TestMin(-INFINITY, INFINITY, -INFINITY);
  TestMax(-INFINITY, INFINITY, INFINITY);

  TestMin(-INFINITY, 1.f, -INFINITY);
  TestMax(-INFINITY, 1.f, 1.f);

  TestMin(INFINITY, 1.f, 1.f);
  TestMax(INFINITY, 1.f, INFINITY);

  TestMin(-INFINITY, NAN, -INFINITY);
  TestMax(-INFINITY, NAN, -INFINITY);
  assert(Float2Int(0.0f) != Float2Int(-0.0f));

  if (NAN == NAN)
    printf("NAN == NAN\n");
  if (NAN != NAN)
    printf("NAN != NAN\n");
  TestMin(-0.0f, 0.0f, -0.0f);
  TestMax(-0.0f, 0.0f, 0.0f);
  printf("Success!\n");
}
