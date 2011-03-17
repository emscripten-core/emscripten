//========================================================================
//
// SplashScreen.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
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
#include "SplashMath.h"
#include "SplashScreen.h"

static SplashScreenParams defaultParams = {
  splashScreenDispersed,	// type
  2,				// size
  2,				// dotRadius
  1.0,				// gamma
  0.0,				// blackThreshold
  1.0				// whiteThreshold
};

//------------------------------------------------------------------------

struct SplashScreenPoint {
  int x, y;
  int dist;
};

static int cmpDistances(const void *p0, const void *p1) {
  return ((SplashScreenPoint *)p0)->dist - ((SplashScreenPoint *)p1)->dist;
}

//------------------------------------------------------------------------
// SplashScreen
//------------------------------------------------------------------------

// If <clustered> is true, this generates a 45 degree screen using a
// circular dot spot function.  DPI = resolution / ((size / 2) *
// sqrt(2)).  If <clustered> is false, this generates an optimal
// threshold matrix using recursive tesselation.  Gamma correction
// (gamma = 1 / 1.33) is also computed here.
SplashScreen::SplashScreen(SplashScreenParams *params) {

  if (!params) {
    params = &defaultParams;
  }
  
  screenParams = params;
  mat = NULL;
  size = 0;
  maxVal = 0;
  minVal = 0;
}

void SplashScreen::createMatrix()
{
  Guchar u, black, white;
  int i;
  
  SplashScreenParams *params = screenParams;

  switch (params->type) {

  case splashScreenDispersed:
    // size must be a power of 2
    for (size = 1; size < params->size; size <<= 1) ;
    mat = (Guchar *)gmallocn(size * size, sizeof(Guchar));
    buildDispersedMatrix(size/2, size/2, 1, size/2, 1);
    break;

  case splashScreenClustered:
    // size must be even
    size = (params->size >> 1) << 1;
    if (size < 2) {
      size = 2;
    }
    mat = (Guchar *)gmallocn(size * size, sizeof(Guchar));
    buildClusteredMatrix();
    break;

  case splashScreenStochasticClustered:
    // size must be at least 2*r
    if (params->size < 2 * params->dotRadius) {
      size = 2 * params->dotRadius;
    } else {
      size = params->size;
    }
    mat = (Guchar *)gmallocn(size * size, sizeof(Guchar));
    buildSCDMatrix(params->dotRadius);
    break;
  }

  // do gamma correction and compute minVal/maxVal
  minVal = 255;
  maxVal = 0;
  black = splashRound((SplashCoord)255.0 * params->blackThreshold);
  if (black < 1) {
    black = 1;
  }
  int whiteAux = splashRound((SplashCoord)255.0 * params->whiteThreshold);
  if (whiteAux > 255) {
    white = 255;
  } else {
    white = whiteAux;
  }
   for (i = 0; i < size * size; ++i) {
    u = splashRound((SplashCoord)255.0 *
		    splashPow((SplashCoord)mat[i] / 255.0, params->gamma));
    if (u < black) {
      u = black;
    } else if (u >= white) {
      u = white;
    }
    mat[i] = u;
    if (u < minVal) {
      minVal = u;
    } else if (u > maxVal) {
      maxVal = u;
    }
  }
}

void SplashScreen::buildDispersedMatrix(int i, int j, int val,
					int delta, int offset) {
  if (delta == 0) {
    // map values in [1, size^2] --> [1, 255]
    mat[i * size + j] = 1 + (254 * (val - 1)) / (size * size - 1);
  } else {
    buildDispersedMatrix(i, j,
			 val, delta / 2, 4*offset);
    buildDispersedMatrix((i + delta) % size, (j + delta) % size,
			 val + offset, delta / 2, 4*offset);
    buildDispersedMatrix((i + delta) % size, j,
			 val + 2*offset, delta / 2, 4*offset);
    buildDispersedMatrix((i + 2*delta) % size, (j + delta) % size,
			 val + 3*offset, delta / 2, 4*offset);
  }
}

void SplashScreen::buildClusteredMatrix() {
  SplashCoord *dist;
  SplashCoord u, v, d;
  Guchar val;
  int size2, x, y, x1, y1, i;

  size2 = size >> 1;

  // initialize the threshold matrix
  for (y = 0; y < size; ++y) {
    for (x = 0; x < size; ++x) {
      mat[y * size + x] = 0;
    }
  }

  // build the distance matrix
  dist = (SplashCoord *)gmallocn(size * size2, sizeof(SplashCoord));
  for (y = 0; y < size2; ++y) {
    for (x = 0; x < size2; ++x) {
      if (x + y < size2 - 1) {
	u = (SplashCoord)x + 0.5 - 0;
	v = (SplashCoord)y + 0.5 - 0;
      } else {
	u = (SplashCoord)x + 0.5 - (SplashCoord)size2;
	v = (SplashCoord)y + 0.5 - (SplashCoord)size2;
      }
      dist[y * size2 + x] = u*u + v*v;
    }
  }
  for (y = 0; y < size2; ++y) {
    for (x = 0; x < size2; ++x) {
      if (x < y) {
	u = (SplashCoord)x + 0.5 - 0;
	v = (SplashCoord)y + 0.5 - (SplashCoord)size2;
      } else {
	u = (SplashCoord)x + 0.5 - (SplashCoord)size2;
	v = (SplashCoord)y + 0.5 - 0;
      }
      dist[(size2 + y) * size2 + x] = u*u + v*v;
    }
  }

  // build the threshold matrix
  minVal = 1;
  maxVal = 0;
  x1 = y1 = 0; // make gcc happy
  for (i = 0; i < size * size2; ++i) {
    d = -1;
    for (y = 0; y < size; ++y) {
      for (x = 0; x < size2; ++x) {
	if (mat[y * size + x] == 0 &&
	    dist[y * size2 + x] > d) {
	  x1 = x;
	  y1 = y;
	  d = dist[y1 * size2 + x1];
	}
      }
    }
    // map values in [0, 2*size*size2-1] --> [1, 255]
    val = 1 + (254 * (2*i)) / (2*size*size2 - 1);
    mat[y1 * size + x1] = val;
    val = 1 + (254 * (2*i+1)) / (2*size*size2 - 1);
    if (y1 < size2) {
      mat[(y1 + size2) * size + x1 + size2] = val;
    } else {
      mat[(y1 - size2) * size + x1 + size2] = val;
    }
  }

  gfree(dist);
}

// Compute the distance between two points on a toroid.
int SplashScreen::distance(int x0, int y0, int x1, int y1) {
  int dx0, dx1, dx, dy0, dy1, dy;

  dx0 = abs(x0 - x1);
  dx1 = size - dx0;
  dx = dx0 < dx1 ? dx0 : dx1;
  dy0 = abs(y0 - y1);
  dy1 = size - dy0;
  dy = dy0 < dy1 ? dy0 : dy1;
  return dx * dx + dy * dy;
}

// Algorithm taken from:
// Victor Ostromoukhov and Roger D. Hersch, "Stochastic Clustered-Dot
// Dithering" in Color Imaging: Device-Independent Color, Color
// Hardcopy, and Graphic Arts IV, SPIE Vol. 3648, pp. 496-505, 1999.
void SplashScreen::buildSCDMatrix(int r) {
  SplashScreenPoint *dots, *pts;
  int dotsLen, dotsSize;
  char *tmpl;
  char *grid;
  int *region, *dist;
  int x, y, xx, yy, x0, x1, y0, y1, i, j, d, iMin, dMin, n;

  //~ this should probably happen somewhere else
  srand(123);

  // generate the random space-filling curve
  pts = (SplashScreenPoint *)gmallocn(size * size, sizeof(SplashScreenPoint));
  i = 0;
  for (y = 0; y < size; ++y) {
    for (x = 0; x < size; ++x) {
      pts[i].x = x;
      pts[i].y = y;
      ++i;
    }
  }
  for (i = 0; i < size * size; ++i) {
    j = i + (int)((double)(size * size - i) *
		  (double)rand() / ((double)RAND_MAX + 1.0));
    x = pts[i].x;
    y = pts[i].y;
    pts[i].x = pts[j].x;
    pts[i].y = pts[j].y;
    pts[j].x = x;
    pts[j].y = y;
  }

  // construct the circle template
  tmpl = (char *)gmallocn((r+1)*(r+1), sizeof(char));
  for (y = 0; y <= r; ++y) {
    for (x = 0; x <= r; ++x) {
      tmpl[y*(r+1) + x] = (x * y <= r * r) ? 1 : 0;
    }
  }

  // mark all grid cells as free
  grid = (char *)gmallocn(size * size, sizeof(char));
  for (y = 0; y < size; ++y) {
    for (x = 0; x < size; ++x) {
      grid[y*size + x] = 0;
    }
  }

  // walk the space-filling curve, adding dots
  dotsLen = 0;
  dotsSize = 32;
  dots = (SplashScreenPoint *)gmallocn(dotsSize, sizeof(SplashScreenPoint));
  for (i = 0; i < size * size; ++i) {
    x = pts[i].x;
    y = pts[i].y;
    if (!grid[y*size + x]) {
      if (dotsLen == dotsSize) {
	dotsSize *= 2;
	dots = (SplashScreenPoint *)greallocn(dots, dotsSize,
					      sizeof(SplashScreenPoint));
      }
      dots[dotsLen++] = pts[i];
      for (yy = 0; yy <= r; ++yy) {
	y0 = (y + yy) % size;
	y1 = (y - yy + size) % size;
	for (xx = 0; xx <= r; ++xx) {
	  if (tmpl[yy*(r+1) + xx]) {
	    x0 = (x + xx) % size;
	    x1 = (x - xx + size) % size;
	    grid[y0*size + x0] = 1;
	    grid[y0*size + x1] = 1;
	    grid[y1*size + x0] = 1;
	    grid[y1*size + x1] = 1;
	  }
	}
      }
    }
  }

  gfree(tmpl);
  gfree(grid);

  // assign each cell to a dot, compute distance to center of dot
  region = (int *)gmallocn(size * size, sizeof(int));
  dist = (int *)gmallocn(size * size, sizeof(int));
  for (y = 0; y < size; ++y) {
    for (x = 0; x < size; ++x) {
      iMin = 0;
      dMin = distance(dots[0].x, dots[0].y, x, y);
      for (i = 1; i < dotsLen; ++i) {
	d = distance(dots[i].x, dots[i].y, x, y);
	if (d < dMin) {
	  iMin = i;
	  dMin = d;
	}
      }
      region[y*size + x] = iMin;
      dist[y*size + x] = dMin;
    }
  }

  // compute threshold values
  for (i = 0; i < dotsLen; ++i) {
    n = 0;
    for (y = 0; y < size; ++y) {
      for (x = 0; x < size; ++x) {
	if (region[y*size + x] == i) {
	  pts[n].x = x;
	  pts[n].y = y;
	  pts[n].dist = distance(dots[i].x, dots[i].y, x, y);
	  ++n;
	}
      }
    }
    qsort(pts, n, sizeof(SplashScreenPoint), &cmpDistances);
    for (j = 0; j < n; ++j) {
      // map values in [0 .. n-1] --> [255 .. 1]
      mat[pts[j].y * size + pts[j].x] = 255 - (254 * j) / (n - 1);
    }
  }

  gfree(pts);
  gfree(region);
  gfree(dist);

  gfree(dots);
}

SplashScreen::SplashScreen(SplashScreen *screen) {
  screenParams = screen->screenParams;
  size = screen->size;
  mat = (Guchar *)gmallocn(size * size, sizeof(Guchar));
  memcpy(mat, screen->mat, size * size * sizeof(Guchar));
  minVal = screen->minVal;
  maxVal = screen->maxVal;
}

SplashScreen::~SplashScreen() {
  gfree(mat);
}

int SplashScreen::test(int x, int y, Guchar value) {
  int xx, yy;
  
  if (mat == NULL) createMatrix();

  if (value < minVal) {
    return 0;
  }
  if (value >= maxVal) {
    return 1;
  }
  if ((xx = x % size) < 0) {
    xx = -xx;
  }
  if ((yy = y % size) < 0) {
    yy = -yy;
  }
  return value < mat[yy * size + xx] ? 0 : 1;
}

GBool SplashScreen::isStatic(Guchar value) {
  if (mat == NULL) createMatrix();
  
  return value < minVal || value >= maxVal;
}
