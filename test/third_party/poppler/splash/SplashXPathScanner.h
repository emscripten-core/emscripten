//========================================================================
//
// SplashXPathScanner.h
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
  SplashXPathScanner(SplashXPath *xPathA, GBool eoA);

  ~SplashXPathScanner();

  // Return the path's bounding box.
  void getBBox(int *xMinA, int *yMinA, int *xMaxA, int *yMaxA)
    { *xMinA = xMin; *yMinA = yMin; *xMaxA = xMax; *yMaxA = yMax; }

  // Return the path's bounding box.
  void getBBoxAA(int *xMinA, int *yMinA, int *xMaxA, int *yMaxA);

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
  void renderAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y);

  // Clips an anti-aliased line by setting pixels to zero.  On entry,
  // all non-zero pixels are between <x0> and <x1>.  This function
  // will update <x0> and <x1>.
  void clipAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y);

private:

  void computeIntersections(int y);

  SplashXPath *xPath;
  GBool eo;
  int xMin, yMin, xMax, yMax;

  int interY;			// current y value
  int interIdx;			// current index into <inter> - used by
				//   getNextSpan 
  int interCount;		// current EO/NZWN counter - used by
				//   getNextSpan
  int xPathIdx;			// current index into <xPath> - used by
				//   computeIntersections
  SplashIntersect *inter;	// intersections array for <interY>
  int interLen;			// number of intersections in <inter>
  int interSize;		// size of the <inter> array
};

#endif
