//========================================================================
//
// SplashXPath.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHXPATH_H
#define SPLASHXPATH_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"

class SplashPath;
struct SplashXPathAdjust;

//------------------------------------------------------------------------

#define splashMaxCurveSplits (1 << 10)

//------------------------------------------------------------------------
// SplashXPathSeg
//------------------------------------------------------------------------

struct SplashXPathSeg {
  SplashCoord x0, y0;		// first endpoint
  SplashCoord x1, y1;		// second endpoint
  SplashCoord dxdy;		// slope: delta-x / delta-y
  SplashCoord dydx;		// slope: delta-y / delta-x
  Guint flags;
};

#define splashXPathHoriz   0x01 // segment is vertical (y0 == y1)
				//   (dxdy is undef)
#define splashXPathVert    0x02 // segment is horizontal (x0 == x1)
				//   (dydx is undef)
#define splashXPathFlip	   0x04	// y0 > y1

//------------------------------------------------------------------------
// SplashXPath
//------------------------------------------------------------------------

class SplashXPath {
public:

  // Expands (converts to segments) and flattens (converts curves to
  // lines) <path>.  Transforms all points from user space to device
  // space, via <matrix>.  If <closeSubpaths> is true, closes all open
  // subpaths.
  SplashXPath(SplashPath *path, SplashCoord *matrix,
	      SplashCoord flatness, GBool closeSubpaths,
	      GBool adjustLines = gFalse, int linePosI = 0);

  // Copy an expanded path.
  SplashXPath *copy() { return new SplashXPath(this); }

  ~SplashXPath();

  // Multiply all coordinates by splashAASize, in preparation for
  // anti-aliased rendering.
  void aaScale();

  // Sort by upper coordinate (lower y), in y-major order.
  void sort();

protected:

  SplashXPath(SplashXPath *xPath);
  void transform(SplashCoord *matrix, SplashCoord xi, SplashCoord yi,
		 SplashCoord *xo, SplashCoord *yo);
  void strokeAdjust(SplashXPathAdjust *adjust,
		    SplashCoord *xp, SplashCoord *yp);
  void grow(int nSegs);
  void addCurve(SplashCoord x0, SplashCoord y0,
		SplashCoord x1, SplashCoord y1,
		SplashCoord x2, SplashCoord y2,
		SplashCoord x3, SplashCoord y3,
		SplashCoord flatness,
		GBool first, GBool last, GBool end0, GBool end1);
  void addSegment(SplashCoord x0, SplashCoord y0,
		  SplashCoord x1, SplashCoord y1);

  SplashXPathSeg *segs;
  int length, size;		// length and size of segs array

  friend class SplashXPathScanner;
  friend class SplashClip;
  friend class Splash;
};

#endif
