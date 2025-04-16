//========================================================================
//
// SplashState.h
//
//========================================================================

#ifndef SPLASHSTATE_H
#define SPLASHSTATE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"

class SplashPattern;
class SplashScreen;
class SplashClip;
class SplashBitmap;

//------------------------------------------------------------------------
// line cap values
//------------------------------------------------------------------------

#define splashLineCapButt       0
#define splashLineCapRound      1
#define splashLineCapProjecting 2

//------------------------------------------------------------------------
// line join values
//------------------------------------------------------------------------

#define splashLineJoinMiter     0
#define splashLineJoinRound     1
#define splashLineJoinBevel     2

//------------------------------------------------------------------------
// SplashState
//------------------------------------------------------------------------

class SplashState {
public:

  // Create a new state object, initialized with default settings.
  SplashState(int width, int height, GBool vectorAntialias,
	      SplashScreenParams *screenParams);
  SplashState(int width, int height, GBool vectorAntialias,
	      SplashScreen *screenA);

  // Copy a state object.
  SplashState *copy() { return new SplashState(this); }

  ~SplashState();

  // Set the stroke pattern.  This does not copy <strokePatternA>.
  void setStrokePattern(SplashPattern *strokePatternA);

  // Set the fill pattern.  This does not copy <fillPatternA>.
  void setFillPattern(SplashPattern *fillPatternA);

  // Set the screen.  This does not copy <screenA>.
  void setScreen(SplashScreen *screenA);

  // Set the line dash pattern.  This copies the <lineDashA> array.
  void setLineDash(SplashCoord *lineDashA, int lineDashLengthA,
		   SplashCoord lineDashPhaseA);

  // Set the soft mask bitmap.
  void setSoftMask(SplashBitmap *softMaskA);

private:

  SplashState(SplashState *state);

  SplashCoord matrix[6];
  SplashPattern *strokePattern;
  SplashPattern *fillPattern;
  SplashScreen *screen;
  SplashBlendFunc blendFunc;
  SplashCoord strokeAlpha;
  SplashCoord fillAlpha;
  SplashCoord lineWidth;
  int lineCap;
  int lineJoin;
  SplashCoord miterLimit;
  SplashCoord flatness;
  SplashCoord *lineDash;
  int lineDashLength;
  SplashCoord lineDashPhase;
  GBool strokeAdjust;
  SplashClip *clip;
  SplashBitmap *softMask;
  GBool deleteSoftMask;
  GBool inNonIsolatedGroup;

  SplashState *next;		// used by Splash class

  friend class Splash;
};

#endif
