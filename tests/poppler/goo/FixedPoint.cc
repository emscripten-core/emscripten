//========================================================================
//
// FixedPoint.cc
//
// Fixed point type, with C++ operators.
//
// Copyright 2004 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#if USE_FIXEDPOINT

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "FixedPoint.h"

#define ln2 ((FixedPoint)0.69314718)

#define ln2 ((FixedPoint)0.69314718)

FixedPoint FixedPoint::sqrt(FixedPoint x) {
  FixedPoint y0, y1, z;

  if (x.val <= 0) {
    y1.val = 0;
  } else {
    y1.val = x.val == 1 ? 2 : x.val >> 1;
    do {
      y0.val = y1.val;
      z = x / y0;
      y1.val = (y0.val + z.val) >> 1;
    } while (::abs(y0.val - y1.val) > 1);
  }
  return y1;
}

FixedPoint FixedPoint::pow(FixedPoint x, FixedPoint y) {
  FixedPoint t, t2, lnx0, lnx, z0, z;
  int d, n, i;

  if (y.val <= 0) {
    z.val = 0;
  } else {
    // y * ln(x)
    t = (x - 1) / (x + 1);
    t2 = t * t;
    d = 1;
    lnx = 0;
    do {
      lnx0 = lnx;
      lnx += t / d;
      t *= t2;
      d += 2;
    } while (::abs(lnx.val - lnx0.val) > 2);
    lnx.val <<= 1;
    t = y * lnx;
    // exp(y * ln(x))
    n = floor(t / ln2);
    t -= ln2 * n;
    t2 = t;
    d = 1;
    i = 1;
    z = 1;
    do {
      z0 = z;
      z += t2 / d;
      t2 *= t;
      ++i;
      d *= i;
    } while (::abs(z.val - z0.val) > 2 && d < (1 << fixptShift));
    if (n >= 0) {
      z.val <<= n;
    } else if (n < 0) {
      z.val >>= -n;
    }
  }
  return z;
}

int FixedPoint::mul(int x, int y) {
  FixPtInt64 z;

  z = ((FixPtInt64)x * y) >> fixptShift;
  if (z > 0x7fffffffLL) {
    return 0x7fffffff;
  } else if (z < -0x80000000LL) {
    return 0x80000000;
  } else {
    return (int)z;
  }
}

int FixedPoint::div(int x, int y) {
  FixPtInt64 z;

  z = ((FixPtInt64)x << fixptShift) / y;
  if (z > 0x7fffffffLL) {
    return 0x7fffffff;
  } else if (z < -0x80000000LL) {
    return 0x80000000;
  } else {
    return (int)z;
  }
}

GBool FixedPoint::divCheck(FixedPoint x, FixedPoint y, FixedPoint *result) {
  FixPtInt64 z;

  z = ((FixPtInt64)x.val << fixptShift) / y.val;
  if ((z == 0 && x != 0) ||
      z >= ((FixPtInt64)1 << 31) || z < -((FixPtInt64)1 << 31)) {
    return gFalse;
  }
  result->val = z;
  return gTrue;
}

GBool FixedPoint::checkDet(FixedPoint m11, FixedPoint m12,
                          FixedPoint m21, FixedPoint m22,
                          FixedPoint epsilon) {
  FixPtInt64 det, e;

  det = (FixPtInt64)m11.val * (FixPtInt64)m22.val
        - (FixPtInt64)m12.val * (FixPtInt64)m21.val;
  e = (FixPtInt64)epsilon.val << fixptShift;
  // NB: this comparison has to be >= not > because epsilon can be
  // truncated to zero as a fixed point value.
  return det >= e || det <= -e;
}

#endif // USE_FIXEDPOINT
