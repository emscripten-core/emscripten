//========================================================================
//
// SplashClip.cc
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
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include "goo/gmem.h"
#include "SplashErrorCodes.h"
#include "SplashPath.h"
#include "SplashXPath.h"
#include "SplashXPathScanner.h"
#include "SplashBitmap.h"
#include "SplashClip.h"

//------------------------------------------------------------------------
// SplashClip.flags
//------------------------------------------------------------------------

#define splashClipEO       0x01	// use even-odd rule

//------------------------------------------------------------------------
// SplashClip
//------------------------------------------------------------------------

SplashClip::SplashClip(SplashCoord x0, SplashCoord y0,
		       SplashCoord x1, SplashCoord y1,
		       GBool antialiasA) {
  antialias = antialiasA;
  if (x0 < x1) {
    xMin = x0;
    xMax = x1;
  } else {
    xMin = x1;
    xMax = x0;
  }
  if (y0 < y1) {
    yMin = y0;
    yMax = y1;
  } else {
    yMin = y1;
    yMax = y0;
  }
  xMinI = splashFloor(xMin);
  yMinI = splashFloor(yMin);
  xMaxI = splashCeil(xMax) - 1;
  yMaxI = splashCeil(yMax) - 1;
  paths = NULL;
  flags = NULL;
  scanners = NULL;
  length = size = 0;
}

SplashClip::SplashClip(SplashClip *clip) {
  int yMinAA, yMaxAA;
  int i;

  antialias = clip->antialias;
  xMin = clip->xMin;
  yMin = clip->yMin;
  xMax = clip->xMax;
  yMax = clip->yMax;
  xMinI = clip->xMinI;
  yMinI = clip->yMinI;
  xMaxI = clip->xMaxI;
  yMaxI = clip->yMaxI;
  length = clip->length;
  size = clip->size;
  paths = (SplashXPath **)gmallocn(size, sizeof(SplashXPath *));
  flags = (Guchar *)gmallocn(size, sizeof(Guchar));
  scanners = (SplashXPathScanner **)
                 gmallocn(size, sizeof(SplashXPathScanner *));
  for (i = 0; i < length; ++i) {
    paths[i] = clip->paths[i]->copy();
    flags[i] = clip->flags[i];
    if (antialias) {
      yMinAA = yMinI * splashAASize;
      yMaxAA = (yMaxI + 1) * splashAASize - 1;
    } else {
      yMinAA = yMinI;
      yMaxAA = yMaxI;
    }
    scanners[i] = new SplashXPathScanner(paths[i], flags[i] & splashClipEO,
					 yMinAA, yMaxAA);
  }
}

SplashClip::~SplashClip() {
  int i;

  for (i = 0; i < length; ++i) {
    delete paths[i];
    delete scanners[i];
  }
  gfree(paths);
  gfree(flags);
  gfree(scanners);
}

void SplashClip::grow(int nPaths) {
  if (length + nPaths > size) {
    if (size == 0) {
      size = 32;
    }
    while (size < length + nPaths) {
      size *= 2;
    }
    paths = (SplashXPath **)greallocn(paths, size, sizeof(SplashXPath *));
    flags = (Guchar *)greallocn(flags, size, sizeof(Guchar));
    scanners = (SplashXPathScanner **)
                   greallocn(scanners, size, sizeof(SplashXPathScanner *));
  }
}

void SplashClip::resetToRect(SplashCoord x0, SplashCoord y0,
			     SplashCoord x1, SplashCoord y1) {
  int i;

  for (i = 0; i < length; ++i) {
    delete paths[i];
    delete scanners[i];
  }
  gfree(paths);
  gfree(flags);
  gfree(scanners);
  paths = NULL;
  flags = NULL;
  scanners = NULL;
  length = size = 0;

  if (x0 < x1) {
    xMin = x0;
    xMax = x1;
  } else {
    xMin = x1;
    xMax = x0;
  }
  if (y0 < y1) {
    yMin = y0;
    yMax = y1;
  } else {
    yMin = y1;
    yMax = y0;
  }
  xMinI = splashFloor(xMin);
  yMinI = splashFloor(yMin);
  xMaxI = splashCeil(xMax) - 1;
  yMaxI = splashCeil(yMax) - 1;
}

SplashError SplashClip::clipToRect(SplashCoord x0, SplashCoord y0,
				   SplashCoord x1, SplashCoord y1) {
  if (x0 < x1) {
    if (x0 > xMin) {
      xMin = x0;
      xMinI = splashFloor(xMin);
    }
    if (x1 < xMax) {
      xMax = x1;
      xMaxI = splashCeil(xMax) - 1;
    }
  } else {
    if (x1 > xMin) {
      xMin = x1;
      xMinI = splashFloor(xMin);
    }
    if (x0 < xMax) {
      xMax = x0;
      xMaxI = splashCeil(xMax) - 1;
    }
  }
  if (y0 < y1) {
    if (y0 > yMin) {
      yMin = y0;
      yMinI = splashFloor(yMin);
    }
    if (y1 < yMax) {
      yMax = y1;
      yMaxI = splashCeil(yMax) - 1;
    }
  } else {
    if (y1 > yMin) {
      yMin = y1;
      yMinI = splashFloor(yMin);
    }
    if (y0 < yMax) {
      yMax = y0;
      yMaxI = splashCeil(yMax) - 1;
    }
  }
  return splashOk;
}

SplashError SplashClip::clipToPath(SplashPath *path, SplashCoord *matrix,
				   SplashCoord flatness, GBool eo) {
  SplashXPath *xPath;
  int yMinAA, yMaxAA;

  xPath = new SplashXPath(path, matrix, flatness, gTrue);

  // check for an empty path
  if (xPath->length == 0) {
    xMax = xMin - 1;
    yMax = yMin - 1;
    xMaxI = splashCeil(xMax) - 1;
    yMaxI = splashCeil(yMax) - 1;
    delete xPath;

  // check for a rectangle
  } else if (xPath->length == 4 &&
	     ((xPath->segs[0].x0 == xPath->segs[0].x1 &&
	       xPath->segs[0].x0 == xPath->segs[1].x0 &&
	       xPath->segs[0].x0 == xPath->segs[3].x1 &&
	       xPath->segs[2].x0 == xPath->segs[2].x1 &&
	       xPath->segs[2].x0 == xPath->segs[1].x1 &&
	       xPath->segs[2].x0 == xPath->segs[3].x0 &&
	       xPath->segs[1].y0 == xPath->segs[1].y1 &&
	       xPath->segs[1].y0 == xPath->segs[0].y1 &&
	       xPath->segs[1].y0 == xPath->segs[2].y0 &&
	       xPath->segs[3].y0 == xPath->segs[3].y1 &&
	       xPath->segs[3].y0 == xPath->segs[0].y0 &&
	       xPath->segs[3].y0 == xPath->segs[2].y1) ||
	      (xPath->segs[0].y0 == xPath->segs[0].y1 &&
	       xPath->segs[0].y0 == xPath->segs[1].y0 &&
	       xPath->segs[0].y0 == xPath->segs[3].y1 &&
	       xPath->segs[2].y0 == xPath->segs[2].y1 &&
	       xPath->segs[2].y0 == xPath->segs[1].y1 &&
	       xPath->segs[2].y0 == xPath->segs[3].y0 &&
	       xPath->segs[1].x0 == xPath->segs[1].x1 &&
	       xPath->segs[1].x0 == xPath->segs[0].x1 &&
	       xPath->segs[1].x0 == xPath->segs[2].x0 &&
	       xPath->segs[3].x0 == xPath->segs[3].x1 &&
	       xPath->segs[3].x0 == xPath->segs[0].x0 &&
	       xPath->segs[3].x0 == xPath->segs[2].x1))) {
    clipToRect(xPath->segs[0].x0, xPath->segs[0].y0,
	       xPath->segs[2].x0, xPath->segs[2].y0);
    delete xPath;

  } else {
    grow(1);
    if (antialias) {
      xPath->aaScale();
    }
    xPath->sort();
    paths[length] = xPath;
    flags[length] = eo ? splashClipEO : 0;
    if (antialias) {
      yMinAA = yMinI * splashAASize;
      yMaxAA = (yMaxI + 1) * splashAASize - 1;
    } else {
      yMinAA = yMinI;
      yMaxAA = yMaxI;
    }
    scanners[length] = new SplashXPathScanner(xPath, eo, yMinAA, yMaxAA);
    ++length;
  }

  return splashOk;
}

SplashClipResult SplashClip::testRect(int rectXMin, int rectYMin,
				      int rectXMax, int rectYMax) {
  // This tests the rectangle:
  //     x = [rectXMin, rectXMax + 1)    (note: rect coords are ints)
  //     y = [rectYMin, rectYMax + 1)
  // against the clipping region:
  //     x = [xMin, xMax)                (note: clipping coords are fp)
  //     y = [yMin, yMax)
  if ((SplashCoord)(rectXMax + 1) <= xMin || (SplashCoord)rectXMin >= xMax ||
      (SplashCoord)(rectYMax + 1) <= yMin || (SplashCoord)rectYMin >= yMax) {
    return splashClipAllOutside;
  }
  if ((SplashCoord)rectXMin >= xMin && (SplashCoord)(rectXMax + 1) <= xMax &&
      (SplashCoord)rectYMin >= yMin && (SplashCoord)(rectYMax + 1) <= yMax &&
      length == 0) {
    return splashClipAllInside;
  }
  return splashClipPartial;
}

SplashClipResult SplashClip::testSpan(int spanXMin, int spanXMax, int spanY) {
  int i;

  // This tests the rectangle:
  //     x = [spanXMin, spanXMax + 1)    (note: span coords are ints)
  //     y = [spanY, spanY + 1)
  // against the clipping region:
  //     x = [xMin, xMax)                (note: clipping coords are fp)
  //     y = [yMin, yMax)
  if ((SplashCoord)(spanXMax + 1) <= xMin || (SplashCoord)spanXMin >= xMax ||
      (SplashCoord)(spanY + 1) <= yMin || (SplashCoord)spanY >= yMax) {
    return splashClipAllOutside;
  }
  if (!((SplashCoord)spanXMin >= xMin && (SplashCoord)(spanXMax + 1) <= xMax &&
	(SplashCoord)spanY >= yMin && (SplashCoord)(spanY + 1) <= yMax)) {
    return splashClipPartial;
  }
  if (antialias) {
    for (i = 0; i < length; ++i) {
      if (!scanners[i]->testSpan(spanXMin * splashAASize,
				 spanXMax * splashAASize + (splashAASize - 1),
				 spanY * splashAASize)) {
	return splashClipPartial;
      }
    }
  } else {
    for (i = 0; i < length; ++i) {
      if (!scanners[i]->testSpan(spanXMin, spanXMax, spanY)) {
	return splashClipPartial;
      }
    }
  }
  return splashClipAllInside;
}

void SplashClip::clipAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y, GBool adjustVertLine) {
  int xx0, xx1, xx, yy, i;
  SplashColorPtr p;

  // zero out pixels with x < xMin
  xx0 = *x0 * splashAASize;
  xx1 = splashFloor(xMin * splashAASize);
  if (xx1 > aaBuf->getWidth()) {
    xx1 = aaBuf->getWidth();
  }
  if (xx0 < xx1) {
    xx0 &= ~7;
    for (yy = 0; yy < splashAASize; ++yy) {
      p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx0 >> 3);
      for (xx = xx0; xx + 7 < xx1; xx += 8) {
	*p++ = 0;
      }
      if (xx < xx1 && !adjustVertLine) {
	*p &= 0xff >> (xx1 & 7);
      }
    }
    *x0 = splashFloor(xMin);
  }

  // zero out pixels with x > xMax
  xx0 = splashFloor(xMax * splashAASize) + 1;
  if (xx0 < 0) {
    xx0 = 0;
  }
  xx1 = (*x1 + 1) * splashAASize;
  if (xx0 < xx1 && !adjustVertLine) {
    for (yy = 0; yy < splashAASize; ++yy) {
      p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx0 >> 3);
      xx = xx0;
      if (xx & 7) {
	*p &= 0xff00 >> (xx & 7);
	xx = (xx & ~7) + 8;
	++p;
      }
      for (; xx < xx1; xx += 8) {
	*p++ = 0;
      }
    }
    *x1 = splashFloor(xMax);
  }

  // check the paths
  for (i = 0; i < length; ++i) {
    scanners[i]->clipAALine(aaBuf, x0, x1, y);
  }
  if (*x0 > *x1) {
    *x0 = *x1;
  }
  if (*x0 < 0) {
    *x0 = 0;
  }
  if ((*x0>>1) >= aaBuf->getRowSize()) {
    xx0 = *x0;
    *x0 = (aaBuf->getRowSize() - 1) << 1;
    if (xx0 & 1) {
      *x0 = *x0 + 1;
    }
  }
  if (*x1 < *x0) {
    *x1 = *x0;
  }
  if ((*x1>>1) >= aaBuf->getRowSize()) {
    xx0 = *x1;
    *x1 = (aaBuf->getRowSize() - 1) << 1;
    if (xx0 & 1) {
      *x1 = *x1 + 1;
    }
  }
}
