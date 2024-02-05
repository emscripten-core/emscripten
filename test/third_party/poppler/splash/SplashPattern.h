//========================================================================
//
// SplashPattern.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2010 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHPATTERN_H
#define SPLASHPATTERN_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"

class SplashScreen;

//------------------------------------------------------------------------
// SplashPattern
//------------------------------------------------------------------------

class SplashPattern {
public:

  SplashPattern();

  virtual SplashPattern *copy() = 0;

  virtual ~SplashPattern();

  // Return the color value for a specific pixel.
  virtual GBool getColor(int x, int y, SplashColorPtr c) = 0;

  // Returns true if this pattern object will return the same color
  // value for all pixels.
  virtual GBool isStatic() = 0;

private:
};

//------------------------------------------------------------------------
// SplashSolidColor
//------------------------------------------------------------------------

class SplashSolidColor: public SplashPattern {
public:

  SplashSolidColor(SplashColorPtr colorA);

  virtual SplashPattern *copy() { return new SplashSolidColor(color); }

  virtual ~SplashSolidColor();

  virtual GBool getColor(int x, int y, SplashColorPtr c);

  virtual GBool isStatic() { return gTrue; }

private:

  SplashColor color;
};

//------------------------------------------------------------------------
// SplashGouraudColor (needed for gouraudTriangleShadedFill)
//------------------------------------------------------------------------

class SplashGouraudColor: public SplashPattern {
public:

  virtual GBool isParameterized() = 0;

  virtual int getNTriangles() = 0;

  virtual  void getTriangle(int i, double *x0, double *y0, double *color0,
                            double *x1, double *y1, double *color1,
                            double *x2, double *y2, double *color2) = 0;

  virtual void getParameterizedColor(double t, SplashColorMode mode, SplashColorPtr c) = 0;
};

#endif
