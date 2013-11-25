//========================================================================
//
// SplashXPathScanner.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
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
#include <algorithm>
#include "goo/gmem.h"
#include "SplashMath.h"
#include "SplashXPath.h"
#include "SplashBitmap.h"
#include "SplashXPathScanner.h"

//------------------------------------------------------------------------

struct SplashIntersect {
  int y;
  int x0, x1;			// intersection of segment with [y, y+1)
  int count;			// EO/NZWN counter increment
};

struct cmpIntersectFunctor {
  bool operator()(const SplashIntersect &i0, const SplashIntersect &i1) {
    return (i0.y != i1.y) ? (i0.y < i1.y) : (i0.x0 < i1.x0);
  }
};

//------------------------------------------------------------------------
// SplashXPathScanner
//------------------------------------------------------------------------

SplashXPathScanner::SplashXPathScanner(SplashXPath *xPathA, GBool eoA,
				       int clipYMin, int clipYMax) {
  SplashXPathSeg *seg;
  SplashCoord xMinFP, yMinFP, xMaxFP, yMaxFP;
  int i;

  xPath = xPathA;
  eo = eoA;
  partialClip = gFalse;

  // compute the bbox
  if (xPath->length == 0) {
    xMin = yMin = 1;
    xMax = yMax = 0;
  } else {
    seg = &xPath->segs[0];
    if (seg->x0 <= seg->x1) {
      xMinFP = seg->x0;
      xMaxFP = seg->x1;
    } else {
      xMinFP = seg->x1;
      xMaxFP = seg->x0;
    }
    if (seg->flags & splashXPathFlip) {
      yMinFP = seg->y1;
      yMaxFP = seg->y0;
    } else {
      yMinFP = seg->y0;
      yMaxFP = seg->y1;
    }
    for (i = 1; i < xPath->length; ++i) {
      seg = &xPath->segs[i];
      if (seg->x0 < xMinFP) {
	xMinFP = seg->x0;
      } else if (seg->x0 > xMaxFP) {
	xMaxFP = seg->x0;
      }
      if (seg->x1 < xMinFP) {
	xMinFP = seg->x1;
      } else if (seg->x1 > xMaxFP) {
	xMaxFP = seg->x1;
      }
      if (seg->flags & splashXPathFlip) {
	if (seg->y0 > yMaxFP) {
	  yMaxFP = seg->y0;
	}
      } else {
	if (seg->y1 > yMaxFP) {
	  yMaxFP = seg->y1;
	}
      }
    }
    xMin = splashFloor(xMinFP);
    xMax = splashFloor(xMaxFP);
    yMin = splashFloor(yMinFP);
    yMax = splashFloor(yMaxFP);
    if (clipYMin > yMin) {
      yMin = clipYMin;
      partialClip = gTrue;
    }
    if (clipYMax < yMax) {
      yMax = clipYMax;
      partialClip = gTrue;
    }
  }

  allInter = NULL;
  inter = NULL;
  computeIntersections();
  interY = yMin - 1;
}

SplashXPathScanner::~SplashXPathScanner() {
  gfree(inter);
  gfree(allInter);
}

void SplashXPathScanner::getBBoxAA(int *xMinA, int *yMinA,
				   int *xMaxA, int *yMaxA) {
  *xMinA = xMin / splashAASize;
  *yMinA = yMin / splashAASize;
  *xMaxA = xMax / splashAASize;
  *yMaxA = yMax / splashAASize;
}

void SplashXPathScanner::getSpanBounds(int y, int *spanXMin, int *spanXMax) {
  int interBegin, interEnd, xx, i;

  if (y < yMin || y > yMax) {
    interBegin = interEnd = 0;
  } else {
    interBegin = inter[y - yMin];
    interEnd = inter[y - yMin + 1];
  }
  if (interBegin < interEnd) {
    *spanXMin = allInter[interBegin].x0;
    xx = allInter[interBegin].x1;
    for (i = interBegin + 1; i < interEnd; ++i) {
      if (allInter[i].x1 > xx) {
	xx = allInter[i].x1;
      }
    }
    *spanXMax = xx;
  } else {
    *spanXMin = xMax + 1;
    *spanXMax = xMax;
  }
}

GBool SplashXPathScanner::test(int x, int y) {
  int interBegin, interEnd, count, i;

  if (y < yMin || y > yMax) {
    return gFalse;
  }
  interBegin = inter[y - yMin];
  interEnd = inter[y - yMin + 1];
  count = 0;
  for (i = interBegin; i < interEnd && allInter[i].x0 <= x; ++i) {
    if (x <= allInter[i].x1) {
      return gTrue;
    }
    count += allInter[i].count;
  }
  return eo ? (count & 1) : (count != 0);
}

GBool SplashXPathScanner::testSpan(int x0, int x1, int y) {
  int interBegin, interEnd, count, xx1, i;

  if (y < yMin || y > yMax) {
    return gFalse;
  }
  interBegin = inter[y - yMin];
  interEnd = inter[y - yMin + 1];
  count = 0;
  for (i = interBegin; i < interEnd && allInter[i].x1 < x0; ++i) {
    count += allInter[i].count;
  }

  // invariant: the subspan [x0,xx1] is inside the path
  xx1 = x0 - 1;
  while (xx1 < x1) {
    if (i >= interEnd) {
      return gFalse;
    }
    if (allInter[i].x0 > xx1 + 1 &&
	!(eo ? (count & 1) : (count != 0))) {
      return gFalse;
    }
    if (allInter[i].x1 > xx1) {
      xx1 = allInter[i].x1;
    }
    count += allInter[i].count;
    ++i;
  }

  return gTrue;
}

GBool SplashXPathScanner::getNextSpan(int y, int *x0, int *x1) {
  int interEnd, xx0, xx1;

  if (y < yMin || y > yMax) {
    return gFalse;
  }
  if (interY != y) {
    interY = y;
    interIdx = inter[y - yMin];
    interCount = 0;
  }
  interEnd = inter[y - yMin + 1];
  if (interIdx >= interEnd) {
    return gFalse;
  }
  xx0 = allInter[interIdx].x0;
  xx1 = allInter[interIdx].x1;
  interCount += allInter[interIdx].count;
  ++interIdx;
  while (interIdx < interEnd &&
	 (allInter[interIdx].x0 <= xx1 ||
	  (eo ? (interCount & 1) : (interCount != 0)))) {
    if (allInter[interIdx].x1 > xx1) {
      xx1 = allInter[interIdx].x1;
    }
    interCount += allInter[interIdx].count;
    ++interIdx;
  }
  *x0 = xx0;
  *x1 = xx1;
  return gTrue;
}

void SplashXPathScanner::computeIntersections() {
  SplashXPathSeg *seg;
  SplashCoord segXMin, segXMax, segYMin, segYMax, xx0, xx1;
  int x, y, y0, y1, i;

  if (yMin > yMax) {
    return;
  }

  // build the list of all intersections
  allInterLen = 0;
  allInterSize = 16;
  allInter = (SplashIntersect *)gmallocn(allInterSize,
					 sizeof(SplashIntersect));
  for (i = 0; i < xPath->length; ++i) {
    seg = &xPath->segs[i];
    if (seg->flags & splashXPathFlip) {
      segYMin = seg->y1;
      segYMax = seg->y0;
    } else {
      segYMin = seg->y0;
      segYMax = seg->y1;
    }
    if (seg->flags & splashXPathHoriz) {
      y = splashFloor(seg->y0);
      if (y >= yMin && y <= yMax) {
	addIntersection(segYMin, segYMax, seg->flags,
			y, splashFloor(seg->x0), splashFloor(seg->x1));
      }
    } else if (seg->flags & splashXPathVert) {
      y0 = splashFloor(segYMin);
      if (y0 < yMin) {
	y0 = yMin;
      }
      y1 = splashFloor(segYMax);
      if (y1 > yMax) {
	y1 = yMax;
      }
      x = splashFloor(seg->x0);
      for (y = y0; y <= y1; ++y) {
	addIntersection(segYMin, segYMax, seg->flags, y, x, x);
      }
    } else {
      if (seg->x0 < seg->x1) {
	segXMin = seg->x0;
	segXMax = seg->x1;
      } else {
	segXMin = seg->x1;
	segXMax = seg->x0;
      }
      y0 = splashFloor(segYMin);
      if (y0 < yMin) {
	y0 = yMin;
      }
      y1 = splashFloor(segYMax);
      if (y1 > yMax) {
	y1 = yMax;
      }
      // this loop could just add seg->dxdy to xx1 on each iteration,
      // but that introduces numerical accuracy problems
      xx1 = seg->x0 + ((SplashCoord)y0 - seg->y0) * seg->dxdy;
      for (y = y0; y <= y1; ++y) {
	xx0 = xx1;
	xx1 = seg->x0 + ((SplashCoord)(y + 1) - seg->y0) * seg->dxdy;
	// the segment may not actually extend to the top and/or bottom edges
	if (xx0 < segXMin) {
	  xx0 = segXMin;
	} else if (xx0 > segXMax) {
	  xx0 = segXMax;
	}
	if (xx1 < segXMin) {
	  xx1 = segXMin;
	} else if (xx1 > segXMax) {
	  xx1 = segXMax;
	}
	addIntersection(segYMin, segYMax, seg->flags, y,
			splashFloor(xx0), splashFloor(xx1));
      }
    }
  }
  std::sort(allInter, allInter + allInterLen, cmpIntersectFunctor());

  // build the list of y pointers
  inter = (int *)gmallocn(yMax - yMin + 2, sizeof(int));
  i = 0;
  for (y = yMin; y <= yMax; ++y) {
    inter[y - yMin] = i;
    while (i < allInterLen && allInter[i].y <= y) {
      ++i;
    }
  }
  inter[yMax - yMin + 1] = i;
}

void SplashXPathScanner::addIntersection(double segYMin, double segYMax,
					 Guint segFlags,
					 int y, int x0, int x1) {
  if (allInterLen == allInterSize) {
    allInterSize *= 2;
    allInter = (SplashIntersect *)greallocn(allInter, allInterSize,
					    sizeof(SplashIntersect));
  }
  allInter[allInterLen].y = y;
  if (x0 < x1) {
    allInter[allInterLen].x0 = x0;
    allInter[allInterLen].x1 = x1;
  } else {
    allInter[allInterLen].x0 = x1;
    allInter[allInterLen].x1 = x0;
  }
  if (segYMin <= y &&
      (SplashCoord)y < segYMax &&
      !(segFlags & splashXPathHoriz)) {
    allInter[allInterLen].count = eo ? 1
                                     : (segFlags & splashXPathFlip) ? 1 : -1;
  } else {
    allInter[allInterLen].count = 0;
  }
  ++allInterLen;
}

void SplashXPathScanner::renderAALine(SplashBitmap *aaBuf,
				      int *x0, int *x1, int y, GBool adjustVertLine) {
  int xx0, xx1, xx, xxMin, xxMax, yy, interEnd;
  Guchar mask;
  SplashColorPtr p;

  memset(aaBuf->getDataPtr(), 0, aaBuf->getRowSize() * aaBuf->getHeight());
  xxMin = aaBuf->getWidth();
  xxMax = -1;
  if (yMin <= yMax) {
    if (splashAASize * y < yMin) {
      interIdx = inter[0];
    } else if (splashAASize * y > yMax) {
      interIdx = inter[yMax - yMin + 1];
    } else {
      interIdx = inter[splashAASize * y - yMin];
    }
    for (yy = 0; yy < splashAASize; ++yy) {
      if (splashAASize * y + yy < yMin) {
	interEnd = inter[0];
      } else if (splashAASize * y + yy > yMax) {
	interEnd = inter[yMax - yMin + 1];
      } else {
	interEnd = inter[splashAASize * y + yy - yMin + 1];
      }
      interCount = 0;
      while (interIdx < interEnd) {
	xx0 = allInter[interIdx].x0;
	xx1 = allInter[interIdx].x1;
	interCount += allInter[interIdx].count;
	++interIdx;
	while (interIdx < interEnd &&
	       (allInter[interIdx].x0 <= xx1 ||
		(eo ? (interCount & 1) : (interCount != 0)))) {
	  if (allInter[interIdx].x1 > xx1) {
	    xx1 = allInter[interIdx].x1;
	  }
	  interCount += allInter[interIdx].count;
	  ++interIdx;
	}
	if (xx0 < 0) {
	  xx0 = 0;
	}
	++xx1;
	if (xx1 > aaBuf->getWidth()) {
	  xx1 = aaBuf->getWidth();
	}
	// set [xx0, xx1) to 1
	if (xx0 < xx1) {
	  xx = xx0;
	  p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx >> 3);
	  if (xx & 7) {
	    mask = adjustVertLine ? 0xff : 0xff >> (xx & 7);
	    if (!adjustVertLine && (xx & ~7) == (xx1 & ~7)) {
	      mask &= (Guchar)(0xff00 >> (xx1 & 7));
	    }
	    *p++ |= mask;
	    xx = (xx & ~7) + 8;
	  }
	  for (; xx + 7 < xx1; xx += 8) {
	    *p++ |= 0xff;
	  }
	  if (xx < xx1) {
	    *p |= adjustVertLine ? 0xff : (Guchar)(0xff00 >> (xx1 & 7));
	  }
	}
	if (xx0 < xxMin) {
	  xxMin = xx0;
	}
	if (xx1 > xxMax) {
	  xxMax = xx1;
	}
      }
    }
  }
  if (xxMin > xxMax) {
    xxMin = xxMax;
  }
  *x0 = xxMin / splashAASize;
  *x1 = (xxMax - 1) / splashAASize;
}

void SplashXPathScanner::clipAALine(SplashBitmap *aaBuf,
				    int *x0, int *x1, int y) {
  int xx0, xx1, xx, yy, interEnd;
  Guchar mask;
  SplashColorPtr p;

  for (yy = 0; yy < splashAASize; ++yy) {
    xx = *x0 * splashAASize;
    if (yMin <= yMax) {
      if (splashAASize * y + yy < yMin) {
	interIdx = interEnd = inter[0];
      } else if (splashAASize * y + yy > yMax) {
	interIdx = interEnd = inter[yMax - yMin + 1];
      } else {
	interIdx = inter[splashAASize * y + yy - yMin];
	if (splashAASize * y + yy > yMax) {
	  interEnd = inter[yMax - yMin + 1];
	} else {
	  interEnd = inter[splashAASize * y + yy - yMin + 1];
	}
      }
      interCount = 0;
      while (interIdx < interEnd && xx < (*x1 + 1) * splashAASize) {
	xx0 = allInter[interIdx].x0;
	xx1 = allInter[interIdx].x1;
	interCount += allInter[interIdx].count;
	++interIdx;
	while (interIdx < interEnd &&
	       (allInter[interIdx].x0 <= xx1 ||
		(eo ? (interCount & 1) : (interCount != 0)))) {
	  if (allInter[interIdx].x1 > xx1) {
	    xx1 = allInter[interIdx].x1;
	  }
	  interCount += allInter[interIdx].count;
	  ++interIdx;
	}
	if (xx0 > aaBuf->getWidth()) {
	  xx0 = aaBuf->getWidth();
	}
	// set [xx, xx0) to 0
	if (xx < xx0) {
	  p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx >> 3);
	  if (xx & 7) {
	    mask = (Guchar)(0xff00 >> (xx & 7));
	    if ((xx & ~7) == (xx0 & ~7)) {
	      mask |= 0xff >> (xx0 & 7);
	    }
	    *p++ &= mask;
	    xx = (xx & ~7) + 8;
	  }
	  for (; xx + 7 < xx0; xx += 8) {
	    *p++ = 0x00;
	  }
	  if (xx < xx0) {
	    *p &= 0xff >> (xx0 & 7);
	  }
	}
	if (xx1 >= xx) {
	  xx = xx1 + 1;
	}
      }
    }
    xx0 = (*x1 + 1) * splashAASize;
    if (xx0 > aaBuf->getWidth()) xx0 = aaBuf->getWidth();
    // set [xx, xx0) to 0
    if (xx < xx0) {
      p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx >> 3);
      if (xx & 7) {
	mask = (Guchar)(0xff00 >> (xx & 7));
	if ((xx & ~7) == (xx0 & ~7)) {
	  mask &= 0xff >> (xx0 & 7);
	}
	*p++ &= mask;
	xx = (xx & ~7) + 8;
      }
      for (; xx + 7 < xx0; xx += 8) {
	*p++ = 0x00;
      }
      if (xx < xx0) {
	*p &= 0xff >> (xx0 & 7);
      }
    }
  }
}
