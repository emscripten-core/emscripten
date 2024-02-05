//========================================================================
//
// FixedPoint.h
//
// Fixed point type, with C++ operators.
//
// Copyright 2004 Glyph & Cog, LLC
//
//========================================================================

#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include "poppler/poppler-config.h"

#if USE_FIXEDPOINT

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include <stdlib.h>
#include "gtypes.h"

#define fixptShift 16
#define fixptMaskL ((1 << fixptShift) - 1)
#define fixptMaskH (~fixptMaskL)

typedef long long FixPtInt64;

class FixedPoint {
public:

  FixedPoint() { val = 0; }
  FixedPoint(const FixedPoint &x) { val = x.val; }
  FixedPoint(double x) { val = (int)(x * (1 << fixptShift) + 0.5); }
  FixedPoint(int x) { val = x << fixptShift; }
  FixedPoint(long x) { val = x << fixptShift; }

  operator float()
    { return (float) val * ((float)1 / (float)(1 << fixptShift)); }
  operator double()
    { return (double) val * (1.0 / (double)(1 << fixptShift)); }
  operator int()
    { return val >> fixptShift; }

  int getRaw() { return val; }

  FixedPoint operator =(FixedPoint x) { val = x.val; return *this; }

  int operator ==(FixedPoint x) { return val == x.val; }
  int operator ==(double x) { return *this == (FixedPoint)x; }
  int operator ==(int x) { return *this == (FixedPoint)x; }
  int operator ==(long x) { return *this == (FixedPoint)x; }

  int operator !=(FixedPoint x) { return val != x.val; }
  int operator !=(double x) { return *this != (FixedPoint)x; }
  int operator !=(int x) { return *this != (FixedPoint)x; }
  int operator !=(long x) { return *this != (FixedPoint)x; }

  int operator <(FixedPoint x) { return val < x.val; }
  int operator <(double x) { return *this < (FixedPoint)x; }
  int operator <(int x) { return *this < (FixedPoint)x; }
  int operator <(long x) { return *this < (FixedPoint)x; }

  int operator <=(FixedPoint x) { return val <= x.val; }
  int operator <=(double x) { return *this <= (FixedPoint)x; }
  int operator <=(int x) { return *this <= (FixedPoint)x; }
  int operator <=(long x) { return *this <= (FixedPoint)x; }

  int operator >(FixedPoint x) { return val > x.val; }
  int operator >(double x) { return *this > (FixedPoint)x; }
  int operator >(int x) { return *this > (FixedPoint)x; }
  int operator >(long x) { return *this > (FixedPoint)x; }

  int operator >=(FixedPoint x) { return val >= x.val; }
  int operator >=(double x) { return *this >= (FixedPoint)x; }
  int operator >=(int x) { return *this >= (FixedPoint)x; }
  int operator >=(long x) { return *this >= (FixedPoint)x; }

  FixedPoint operator -() { return make(-val); }

  FixedPoint operator +(FixedPoint x) { return make(val + x.val); }
  FixedPoint operator +(double x) { return *this + (FixedPoint)x; }
  FixedPoint operator +(int x) { return *this + (FixedPoint)x; }
  FixedPoint operator +(long x) { return *this + (FixedPoint)x; }

  FixedPoint operator +=(FixedPoint x) { val = val + x.val; return *this; }
  FixedPoint operator +=(double x) { return *this += (FixedPoint)x; }
  FixedPoint operator +=(int x) { return *this += (FixedPoint)x; }
  FixedPoint operator +=(long x) { return *this += (FixedPoint)x; }

  FixedPoint operator -(FixedPoint x) { return make(val - x.val); }
  FixedPoint operator -(double x) { return *this - (FixedPoint)x; }
  FixedPoint operator -(int x) { return *this - (FixedPoint)x; }
  FixedPoint operator -(long x) { return *this - (FixedPoint)x; }

  FixedPoint operator -=(FixedPoint x) { val = val - x.val; return *this; }
  FixedPoint operator -=(double x) { return *this -= (FixedPoint)x; }
  FixedPoint operator -=(int x) { return *this -= (FixedPoint)x; }
  FixedPoint operator -=(long x) { return *this -= (FixedPoint)x; }

  FixedPoint operator *(FixedPoint x) { return make(mul(val, x.val)); }
  FixedPoint operator *(double x) { return *this * (FixedPoint)x; }
  FixedPoint operator *(int x) { return *this * (FixedPoint)x; }
  FixedPoint operator *(long x) { return *this * (FixedPoint)x; }

  FixedPoint operator *=(FixedPoint x) { val = mul(val, x.val); return *this; }
  FixedPoint operator *=(double x) { return *this *= (FixedPoint)x; }
  FixedPoint operator *=(int x) { return *this *= (FixedPoint)x; }
  FixedPoint operator *=(long x) { return *this *= (FixedPoint)x; }

  FixedPoint operator /(FixedPoint x) { return make(div(val, x.val)); }
  FixedPoint operator /(double x) { return *this / (FixedPoint)x; }
  FixedPoint operator /(int x) { return *this / (FixedPoint)x; }
  FixedPoint operator /(long x) { return *this / (FixedPoint)x; }

  FixedPoint operator /=(FixedPoint x) { val = div(val, x.val); return *this; }
  FixedPoint operator /=(double x) { return *this /= (FixedPoint)x; }
  FixedPoint operator /=(int x) { return *this /= (FixedPoint)x; }
  FixedPoint operator /=(long x) { return *this /= (FixedPoint)x; }

  static FixedPoint abs(FixedPoint x) { return make(::abs(x.val)); }

  static int floor(FixedPoint x) { return x.val >> fixptShift; }

  static int ceil(FixedPoint x)
    { return (x.val & fixptMaskL) ? ((x.val >> fixptShift) + 1)
	                          : (x.val >> fixptShift); }

  static int round(FixedPoint x)
    { return (x.val + (1 << (fixptShift - 1))) >> fixptShift; }

  static FixedPoint sqrt(FixedPoint x);

  static FixedPoint pow(FixedPoint x, FixedPoint y);

  // Compute *result = x/y; return false if there is an underflow or
  // overflow.
  static GBool divCheck(FixedPoint x, FixedPoint y, FixedPoint *result);

private:

  static FixedPoint make(int valA) { FixedPoint x; x.val = valA; return x; }

  static int mul(int x, int y);
  static int div(int x, int y);

  int val;			// 16.16 fixed point
};

#endif // USE_FIXEDPOINT

#endif
