//========================================================================
//
// SplashXPathScanner.h
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

#ifndef SPLASHXPATHSCANNER_H
#define SPLASHXPATHSCANNER_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"

class SplashXPath;
class SplashBitmap;
struct SplashIntersect;

//------------------------------------------------------------------------
// SplashXPathScanner
//------------------------------------------------------------------------

class SplashXPathScanner {
public:

  // Create a new SplashXPathScanner object.  <xPathA> must be sorted.
  SplashXPathScanner(SplashXPath *xPathA, GBool eoA,
		     int clipYMin, int clipYMax);

  ~SplashXPathScanner();

  // Return the path's bounding box.
  void getBBox(int *xMinA, int *yMinA, int *xMaxA, int *yMaxA)
    { *xMinA = xMin; *yMinA = yMin; *xMaxA = xMax; *yMaxA = yMax; }

  // Return the path's bounding box.
  void getBBoxAA(int *xMinA, int *yMinA, int *xMaxA, int *yMaxA);

  // Returns true if at least part of the path was outside the
  // clipYMin/clipYMax bounds passed to the constructor.
  GBool hasPartialClip() { return partialClip; }

  // Return the min/max x values for the span at <y>.
  void getSpanBounds(int y, int *spanXMin, int *spanXMax);

  // Returns true if (<x>,<y>) is inside the path.
  GBool test(int x, int y);

  // Returns true if the entire span ([<x0>,<x1>], <y>) is inside the
  // path.
  GBool testSpan(int x0, int x1, int y);

  // Returns the next span inside the path at <y>.  If <y> is
  // different than the previous call to getNextSpan, this returns the
  // first span at <y>; otherwise it returns the next span (relative
  // to the previous call to getNextSpan).  Returns false if there are
  // no more spans at <y>.
  GBool getNextSpan(int y, int *x0, int *x1);

  // Renders one anti-aliased line into <aaBuf>.  Returns the min and
  // max x coordinates with non-zero pixels in <x0> and <x1>.
  void renderAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y,
    GBool adjustVertLine = gFalse);

  // Clips an anti-aliased line by setting pixels to zero.  On entry,
  // all non-zero pixels are between <x0> and <x1>.  This function
  // will update <x0> and <x1>.
  void clipAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y);

private:

  void computeIntersections();
  void addIntersection(double segYMin, double segYMax,
		       Guint segFlags,
		       int y, int x0, int x1);

  SplashXPath *xPath;
  GBool eo;
  int xMin, yMin, xMax, yMax;
  GBool partialClip;

  SplashIntersect *allInter;	// array of intersections
  int allInterLen;		// number of intersections in <allInter>
  int allInterSize;		// size of the <allInter> array
  int *inter;			// indexes into <allInter> for each y value
  int interY;			// current y value - used by getNextSpan
  int interIdx;			// current index into <inter> - used by
				//   getNextSpan 
  int interCount;		// current EO/NZWN counter - used by
				//   getNextSpan
};

#endif
