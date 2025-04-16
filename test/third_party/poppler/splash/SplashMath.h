//========================================================================
//
// SplashMath.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2009, 2010 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHMATH_H
#define SPLASHMATH_H

#if USE_FIXEDPOINT
#include "goo/FixedPoint.h"
#else
#include <math.h>
#endif
#include "SplashTypes.h"

static inline SplashCoord splashAbs(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::abs(x);
#elif USE_FLOAT
  return fabsf(x);
#else
  return fabs(x);
#endif
}

static inline int splashFloor(SplashCoord x) {
  #if USE_FIXEDPOINT
    return FixedPoint::floor(x);
  #elif USE_FLOAT
    return (int)floorf(x);
  #else
    if (x > 0) return (int)x;
    else return (int)floor(x);
  #endif
}

static inline int splashCeil(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::ceil(x);
#elif USE_FLOAT
  return (int)ceilf(x);
#else
  return (int)ceil(x);
#endif
}

static inline int splashRound(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::round(x);
#else
  return (int)splashFloor(x + 0.5);
#endif
}

static inline SplashCoord splashSqrt(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::sqrt(x);
#elif USE_FLOAT
  return sqrtf(x);
#else
  return sqrt(x);
#endif
}

static inline SplashCoord splashPow(SplashCoord x, SplashCoord y) {
#if USE_FIXEDPOINT
  return FixedPoint::pow(x, y);
#elif USE_FLOAT
  return powf(x, y);
#else
  return pow(x, y);
#endif
}

static inline SplashCoord splashDist(SplashCoord x0, SplashCoord y0,
				     SplashCoord x1, SplashCoord y1) {
  SplashCoord dx, dy;
  dx = x1 - x0;
  dy = y1 - y0;
#if USE_FIXEDPOINT
  // this handles the situation where dx*dx or dy*dy is too large to
  // fit in the 16.16 fixed point format
  SplashCoord dxa, dya;
  dxa = splashAbs(dx);
  dya = splashAbs(dy);
  if (dxa == 0 && dya == 0) {
    return 0;
  } else if (dxa > dya) {
    return dxa * FixedPoint::sqrt(dya / dxa + 1);
  } else {
    return dya * FixedPoint::sqrt(dxa / dya + 1);
  }
#else
  return splashSqrt(dx * dx + dy * dy);
#endif
}

#endif
