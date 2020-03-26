//========================================================================
//
// SplashClip.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2010 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHCLIP_H
#define SPLASHCLIP_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"
#include "SplashMath.h"
#include "SplashXPathScanner.h"

class SplashPath;
class SplashXPath;
class SplashBitmap;

//------------------------------------------------------------------------

enum SplashClipResult {
  splashClipAllInside,
  splashClipAllOutside,
  splashClipPartial
};

//------------------------------------------------------------------------
// SplashClip
//------------------------------------------------------------------------

class SplashClip {
public:

  // Create a clip, for the given rectangle.
  SplashClip(SplashCoord x0, SplashCoord y0,
	     SplashCoord x1, SplashCoord y1,
	     GBool antialiasA);

  // Copy a clip.
  SplashClip *copy() { return new SplashClip(this); }

  ~SplashClip();

  // Reset the clip to a rectangle.
  void resetToRect(SplashCoord x0, SplashCoord y0,
		   SplashCoord x1, SplashCoord y1);

  // Intersect the clip with a rectangle.
  SplashError clipToRect(SplashCoord x0, SplashCoord y0,
			 SplashCoord x1, SplashCoord y1);

  // Interesect the clip with <path>.
  SplashError clipToPath(SplashPath *path, SplashCoord *matrix,
			 SplashCoord flatness, GBool eo);

  // Returns true if (<x>,<y>) is inside the clip.
  GBool test(int x, int y)
  {
    int i;

    // check the rectangle
    if (x < xMinI || x > xMaxI || y < yMinI || y > yMaxI) {
      return gFalse;
    }

    // check the paths
    if (antialias) {
      for (i = 0; i < length; ++i) {
        if (!scanners[i]->test(x * splashAASize, y * splashAASize)) {
	  return gFalse;
        }
      }
    } else {
      for (i = 0; i < length; ++i) {
        if (!scanners[i]->test(x, y)) {
	  return gFalse;
        }
      }
    }

    return gTrue;
  }

  // Tests a rectangle against the clipping region.  Returns one of:
  //   - splashClipAllInside if the entire rectangle is inside the
  //     clipping region, i.e., all pixels in the rectangle are
  //     visible
  //   - splashClipAllOutside if the entire rectangle is outside the
  //     clipping region, i.e., all the pixels in the rectangle are
  //     clipped
  //   - splashClipPartial if the rectangle is part inside and part
  //     outside the clipping region
  SplashClipResult testRect(int rectXMin, int rectYMin,
			    int rectXMax, int rectYMax);

  // Similar to testRect, but tests a horizontal span.
  SplashClipResult testSpan(int spanXMin, int spanXMax, int spanY);

  // Clips an anti-aliased line by setting pixels to zero.  On entry,
  // all non-zero pixels are between <x0> and <x1>.  This function
  // will update <x0> and <x1>.
  void clipAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y);

  // Get the rectangle part of the clip region, in integer coordinates.
  int getXMinI() { return xMinI; }
  int getXMaxI() { return xMaxI; }
  int getYMinI() { return yMinI; }
  int getYMaxI() { return yMaxI; }

  // Get the number of arbitrary paths used by the clip region.
  int getNumPaths() { return length; }

protected:

  SplashClip(SplashClip *clip);
  void grow(int nPaths);

  GBool antialias;
  SplashCoord xMin, yMin, xMax, yMax;
  int xMinI, yMinI, xMaxI, yMaxI;
  SplashXPath **paths;
  Guchar *flags;
  SplashXPathScanner **scanners;
  int length, size;
};

#endif
