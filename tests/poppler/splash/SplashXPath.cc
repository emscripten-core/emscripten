//========================================================================
//
// SplashXPath.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
// Copyright (C) 2010, 2011 Albert Astals Cid <aacid@kde.org>
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
#include "SplashPath.h"
#include "SplashXPath.h"

//------------------------------------------------------------------------

struct SplashXPathPoint {
  SplashCoord x, y;
};

struct SplashXPathAdjust {
  int firstPt, lastPt;		// range of points
  GBool vert;			// vertical or horizontal hint
  SplashCoord x0a, x0b,		// hint boundaries
              xma, xmb,
              x1a, x1b;
  SplashCoord x0, x1, xm;	// adjusted coordinates
};

//------------------------------------------------------------------------

// Transform a point from user space to device space.
inline void SplashXPath::transform(SplashCoord *matrix,
				   SplashCoord xi, SplashCoord yi,
				   SplashCoord *xo, SplashCoord *yo) {
  //                          [ m[0] m[1] 0 ]
  // [xo yo 1] = [xi yi 1] *  [ m[2] m[3] 0 ]
  //                          [ m[4] m[5] 1 ]
  *xo = xi * matrix[0] + yi * matrix[2] + matrix[4];
  *yo = xi * matrix[1] + yi * matrix[3] + matrix[5];
}

//------------------------------------------------------------------------
// SplashXPath
//------------------------------------------------------------------------

SplashXPath::SplashXPath(SplashPath *path, SplashCoord *matrix,
			 SplashCoord flatness, GBool closeSubpaths,
			 GBool adjustLines, int linePosI) {
  SplashPathHint *hint;
  SplashXPathPoint *pts;
  SplashXPathAdjust *adjusts, *adjust;
  SplashCoord x0, y0, x1, y1, x2, y2, x3, y3, xsp, ysp;
  SplashCoord adj0, adj1;
  int curSubpath, i, j;

  // transform the points
  pts = (SplashXPathPoint *)gmallocn(path->length, sizeof(SplashXPathPoint));
  for (i = 0; i < path->length; ++i) {
    transform(matrix, path->pts[i].x, path->pts[i].y, &pts[i].x, &pts[i].y);
  }

  // set up the stroke adjustment hints
  if (path->hints) {
    adjusts = (SplashXPathAdjust *)gmallocn(path->hintsLength,
					    sizeof(SplashXPathAdjust));
    for (i = 0; i < path->hintsLength; ++i) {
      hint = &path->hints[i];
      if (hint->ctrl0 + 1 >= path->length || hint->ctrl1 + 1 >= path->length) {
	gfree(adjusts);
	adjusts = NULL;
	break;
      }
      x0 = pts[hint->ctrl0    ].x;    y0 = pts[hint->ctrl0    ].y;
      x1 = pts[hint->ctrl0 + 1].x;    y1 = pts[hint->ctrl0 + 1].y;
      x2 = pts[hint->ctrl1    ].x;    y2 = pts[hint->ctrl1    ].y;
      x3 = pts[hint->ctrl1 + 1].x;    y3 = pts[hint->ctrl1 + 1].y;
      if (x0 == x1 && x2 == x3) {
	adjusts[i].vert = gTrue;
	adj0 = x0;
	adj1 = x2;
      } else if (y0 == y1 && y2 == y3) {
	adjusts[i].vert = gFalse;
	adj0 = y0;
	adj1 = y2;
      } else {
	gfree(adjusts);
	adjusts = NULL;
	break;
      }
      if (adj0 > adj1) {
	x0 = adj0;
	adj0 = adj1;
	adj1 = x0;
      }
      adjusts[i].x0a = adj0 - 0.01;
      adjusts[i].x0b = adj0 + 0.01;
      adjusts[i].xma = (SplashCoord)0.5 * (adj0 + adj1) - 0.01;
      adjusts[i].xmb = (SplashCoord)0.5 * (adj0 + adj1) + 0.01;
      adjusts[i].x1a = adj1 - 0.01;
      adjusts[i].x1b = adj1 + 0.01;
      // rounding both edge coordinates can result in lines of
      // different widths (e.g., adj=10.1, adj1=11.3 --> x0=10, x1=11;
      // adj0=10.4, adj1=11.6 --> x0=10, x1=12), but it has the
      // benefit of making adjacent strokes/fills line up without any
      // gaps between them
      x0 = splashRound(adj0);
      x1 = splashRound(adj1);
      if (x1 == x0) {
        if (adjustLines) {
          // the adjustment moves thin lines (clip rectangle with
          // empty width or height) out of clip area, here we need
          // a special adjustment:
          x0 = linePosI;
          x1 = x0 + 1;
        } else {
          x1 = x1 + 1;
        }
      }
      adjusts[i].x0 = (SplashCoord)x0;
      adjusts[i].x1 = (SplashCoord)x1 - 0.01;
      adjusts[i].xm = (SplashCoord)0.5 * (adjusts[i].x0 + adjusts[i].x1);
      adjusts[i].firstPt = hint->firstPt;
      adjusts[i].lastPt = hint->lastPt;
    }

  } else {
    adjusts = NULL;
  }

  // perform stroke adjustment
  if (adjusts) {
    for (i = 0, adjust = adjusts; i < path->hintsLength; ++i, ++adjust) {
      for (j = adjust->firstPt; j <= adjust->lastPt; ++j) {
	strokeAdjust(adjust, &pts[j].x, &pts[j].y);
      }
    }
    gfree(adjusts);
  }

  segs = NULL;
  length = size = 0;

  x0 = y0 = xsp = ysp = 0; // make gcc happy
  adj0 = adj1 = 0; // make gcc happy
  curSubpath = 0;
  i = 0;
  while (i < path->length) {

    // first point in subpath - skip it
    if (path->flags[i] & splashPathFirst) {
      x0 = pts[i].x;
      y0 = pts[i].y;
      xsp = x0;
      ysp = y0;
      curSubpath = i;
      ++i;

    } else {

      // curve segment
      if (path->flags[i] & splashPathCurve) {
	x1 = pts[i].x;
	y1 = pts[i].y;
	x2 = pts[i+1].x;
	y2 = pts[i+1].y;
	x3 = pts[i+2].x;
	y3 = pts[i+2].y;
	addCurve(x0, y0, x1, y1, x2, y2, x3, y3,
		 flatness,
		 (path->flags[i-1] & splashPathFirst),
		 (path->flags[i+2] & splashPathLast),
		 !closeSubpaths &&
		   (path->flags[i-1] & splashPathFirst) &&
		   !(path->flags[i-1] & splashPathClosed),
		 !closeSubpaths &&
		   (path->flags[i+2] & splashPathLast) &&
		   !(path->flags[i+2] & splashPathClosed));
	x0 = x3;
	y0 = y3;
	i += 3;

      // line segment
      } else {
	x1 = pts[i].x;
	y1 = pts[i].y;
	addSegment(x0, y0, x1, y1);
	x0 = x1;
	y0 = y1;
	++i;
      }

      // close a subpath
      if (closeSubpaths &&
	  (path->flags[i-1] & splashPathLast) &&
	  (pts[i-1].x != pts[curSubpath].x ||
	   pts[i-1].y != pts[curSubpath].y)) {
	addSegment(x0, y0, xsp, ysp);
      }
    }
  }

  gfree(pts);
}

// Apply the stroke adjust hints to point <pt>: (*<xp>, *<yp>).
void SplashXPath::strokeAdjust(SplashXPathAdjust *adjust,
			       SplashCoord *xp, SplashCoord *yp) {
  SplashCoord x, y;

  if (adjust->vert) {
    x = *xp;
    if (x > adjust->x0a && x < adjust->x0b) {
      *xp = adjust->x0;
    } else if (x > adjust->xma && x < adjust->xmb) {
      *xp = adjust->xm;
    } else if (x > adjust->x1a && x < adjust->x1b) {
      *xp = adjust->x1;
    }
  } else {
    y = *yp;
    if (y > adjust->x0a && y < adjust->x0b) {
      *yp = adjust->x0;
    } else if (y > adjust->xma && y < adjust->xmb) {
      *yp = adjust->xm;
    } else if (y > adjust->x1a && y < adjust->x1b) {
      *yp = adjust->x1;
    }
  }
}

SplashXPath::SplashXPath(SplashXPath *xPath) {
  length = xPath->length;
  size = xPath->size;
  segs = (SplashXPathSeg *)gmallocn(size, sizeof(SplashXPathSeg));
  memcpy(segs, xPath->segs, length * sizeof(SplashXPathSeg));
}

SplashXPath::~SplashXPath() {
  gfree(segs);
}

// Add space for <nSegs> more segments
void SplashXPath::grow(int nSegs) {
  if (length + nSegs > size) {
    if (size == 0) {
      size = 32;
    }
    while (size < length + nSegs) {
      size *= 2;
    }
    segs = (SplashXPathSeg *)greallocn(segs, size, sizeof(SplashXPathSeg));
  }
}

void SplashXPath::addCurve(SplashCoord x0, SplashCoord y0,
			   SplashCoord x1, SplashCoord y1,
			   SplashCoord x2, SplashCoord y2,
			   SplashCoord x3, SplashCoord y3,
			   SplashCoord flatness,
			   GBool first, GBool last, GBool end0, GBool end1) {
  SplashCoord *cx = new SplashCoord[(splashMaxCurveSplits + 1) * 3];
  SplashCoord *cy = new SplashCoord[(splashMaxCurveSplits + 1) * 3];
  int *cNext = new int[splashMaxCurveSplits + 1];
  SplashCoord xl0, xl1, xl2, xr0, xr1, xr2, xr3, xx1, xx2, xh;
  SplashCoord yl0, yl1, yl2, yr0, yr1, yr2, yr3, yy1, yy2, yh;
  SplashCoord dx, dy, mx, my, d1, d2, flatness2;
  int p1, p2, p3;

#if USE_FIXEDPOINT
  flatness2 = flatness;
#else
  flatness2 = flatness * flatness;
#endif

  // initial segment
  p1 = 0;
  p2 = splashMaxCurveSplits;

  *(cx + p1 * 3 + 0) = x0;
  *(cx + p1 * 3 + 1) = x1;
  *(cx + p1 * 3 + 2) = x2;
  *(cx + p2 * 3 + 0) = x3;

  *(cy + p1 * 3 + 0) = y0;
  *(cy + p1 * 3 + 1) = y1;
  *(cy + p1 * 3 + 2) = y2;
  *(cy + p2 * 3 + 0) = y3;

  *(cNext + p1) = p2;

  while (p1 < splashMaxCurveSplits) {

    // get the next segment
    xl0 = *(cx + p1 * 3 + 0);
    xx1 = *(cx + p1 * 3 + 1);
    xx2 = *(cx + p1 * 3 + 2);

    yl0 = *(cy + p1 * 3 + 0);
    yy1 = *(cy + p1 * 3 + 1);
    yy2 = *(cy + p1 * 3 + 2);

    p2 = *(cNext + p1);

    xr3 = *(cx + p2 * 3 + 0);
    yr3 = *(cy + p2 * 3 + 0);

    // compute the distances from the control points to the
    // midpoint of the straight line (this is a bit of a hack, but
    // it's much faster than computing the actual distances to the
    // line)
    mx = (xl0 + xr3) * 0.5;
    my = (yl0 + yr3) * 0.5;
#if USE_FIXEDPOINT
    d1 = splashDist(xx1, yy1, mx, my);
    d2 = splashDist(xx2, yy2, mx, my);
#else
    dx = xx1 - mx;
    dy = yy1 - my;
    d1 = dx*dx + dy*dy;
    dx = xx2 - mx;
    dy = yy2 - my;
    d2 = dx*dx + dy*dy;
#endif    

    // if the curve is flat enough, or no more subdivisions are
    // allowed, add the straight line segment
    if (p2 - p1 == 1 || (d1 <= flatness2 && d2 <= flatness2)) {
      addSegment(xl0, yl0, xr3, yr3);
      p1 = p2;

    // otherwise, subdivide the curve
    } else {
      xl1 = (xl0 + xx1) * 0.5;
      yl1 = (yl0 + yy1) * 0.5;
      xh = (xx1 + xx2) * 0.5;
      yh = (yy1 + yy2) * 0.5;
      xl2 = (xl1 + xh) * 0.5;
      yl2 = (yl1 + yh) * 0.5;
      xr2 = (xx2 + xr3) * 0.5;
      yr2 = (yy2 + yr3) * 0.5;
      xr1 = (xh + xr2) * 0.5;
      yr1 = (yh + yr2) * 0.5;
      xr0 = (xl2 + xr1) * 0.5;
      yr0 = (yl2 + yr1) * 0.5;
      // add the new subdivision points
      p3 = (p1 + p2) / 2;

      *(cx + p1 * 3 + 1) = xl1;
      *(cx + p1 * 3 + 2) = xl2;

      *(cy + p1 * 3 + 1) = yl1;
      *(cy + p1 * 3 + 2) = yl2;

      *(cNext + p1) = p3;

      *(cx + p3 * 3 + 0) = xr0;
      *(cx + p3 * 3 + 1) = xr1;
      *(cx + p3 * 3 + 2) = xr2;

      *(cy + p3 * 3 + 0) = yr0;
      *(cy + p3 * 3 + 1) = yr1;
      *(cy + p3 * 3 + 2) = yr2;

      *(cNext + p3) = p2;
    }
  }

  delete [] cx;
  delete [] cy;
  delete [] cNext;
}

void SplashXPath::addSegment(SplashCoord x0, SplashCoord y0,
			     SplashCoord x1, SplashCoord y1) {
  grow(1);
  segs[length].x0 = x0;
  segs[length].y0 = y0;
  segs[length].x1 = x1;
  segs[length].y1 = y1;
  segs[length].flags = 0;
  if (y1 == y0) {
    segs[length].dxdy = segs[length].dydx = 0;
    segs[length].flags |= splashXPathHoriz;
    if (x1 == x0) {
      segs[length].flags |= splashXPathVert;
    }
  } else if (x1 == x0) {
    segs[length].dxdy = segs[length].dydx = 0;
    segs[length].flags |= splashXPathVert;
  } else {
#if USE_FIXEDPOINT
    if (FixedPoint::divCheck(x1 - x0, y1 - y0, &segs[length].dxdy)) {
      segs[length].dydx = (SplashCoord)1 / segs[length].dxdy;
    } else {
      segs[length].dxdy = segs[length].dydx = 0;
      if (splashAbs(x1 - x0) > splashAbs(y1 - y0)) {
	segs[length].flags |= splashXPathHoriz;
      } else {
	segs[length].flags |= splashXPathVert;
      }
    }
#else
    segs[length].dxdy = (x1 - x0) / (y1 - y0);
    segs[length].dydx = (SplashCoord)1 / segs[length].dxdy;
#endif
  }
  if (y0 > y1) {
    segs[length].flags |= splashXPathFlip;
  }
  ++length;
}

struct cmpXPathSegsFunctor {
  bool operator()(const SplashXPathSeg &seg0, const SplashXPathSeg &seg1) {
    SplashCoord x0, y0, x1, y1;

    if (seg0.flags & splashXPathFlip) {
      x0 = seg0.x1;
      y0 = seg0.y1;
    } else {
      x0 = seg0.x0;
      y0 = seg0.y0;
    }
    if (seg1.flags & splashXPathFlip) {
      x1 = seg1.x1;
      y1 = seg1.y1;
    } else {
      x1 = seg1.x0;
      y1 = seg1.y0;
    }
    return (y0 != y1) ? (y0 < y1) : (x0 < x1);
  }
};

void SplashXPath::aaScale() {
  SplashXPathSeg *seg;
  int i;

  for (i = 0, seg = segs; i < length; ++i, ++seg) {
    seg->x0 *= splashAASize;
    seg->y0 *= splashAASize;
    seg->x1 *= splashAASize;
    seg->y1 *= splashAASize;
  }
}

void SplashXPath::sort() {
  std::sort(segs, segs + length, cmpXPathSegsFunctor());
}
