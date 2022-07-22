//========================================================================
//
// SplashState.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2009 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <string.h>
#include "goo/gmem.h"
#include "SplashPattern.h"
#include "SplashScreen.h"
#include "SplashClip.h"
#include "SplashBitmap.h"
#include "SplashState.h"

//------------------------------------------------------------------------
// SplashState
//------------------------------------------------------------------------

// number of components in each color mode
int splashColorModeNComps[] = {
  1, 1, 3, 3, 4
#if SPLASH_CMYK
  ,4
#endif
};

SplashState::SplashState(int width, int height, GBool vectorAntialias,
			 SplashScreenParams *screenParams) {
  SplashColor color;

  matrix[0] = 1;  matrix[1] = 0;
  matrix[2] = 0;  matrix[3] = 1;
  matrix[4] = 0;  matrix[5] = 0;
  memset(&color, 0, sizeof(SplashColor));
  strokePattern = new SplashSolidColor(color);
  fillPattern = new SplashSolidColor(color);
  screen = new SplashScreen(screenParams);
  blendFunc = NULL;
  strokeAlpha = 1;
  fillAlpha = 1;
  lineWidth = 0;
  lineCap = splashLineCapButt;
  lineJoin = splashLineJoinMiter;
  miterLimit = 10;
  flatness = 1;
  lineDash = NULL;
  lineDashLength = 0;
  lineDashPhase = 0;
  strokeAdjust = gFalse;
  clip = new SplashClip(0, 0, width - 0.001, height - 0.001, vectorAntialias);
  softMask = NULL;
  deleteSoftMask = gFalse;
  inNonIsolatedGroup = gFalse;
  next = NULL;
}

SplashState::SplashState(int width, int height, GBool vectorAntialias,
			 SplashScreen *screenA) {
  SplashColor color;

  matrix[0] = 1;  matrix[1] = 0;
  matrix[2] = 0;  matrix[3] = 1;
  matrix[4] = 0;  matrix[5] = 0;
  memset(&color, 0, sizeof(SplashColor));
  strokePattern = new SplashSolidColor(color);
  fillPattern = new SplashSolidColor(color);
  screen = screenA->copy();
  blendFunc = NULL;
  strokeAlpha = 1;
  fillAlpha = 1;
  lineWidth = 0;
  lineCap = splashLineCapButt;
  lineJoin = splashLineJoinMiter;
  miterLimit = 10;
  flatness = 1;
  lineDash = NULL;
  lineDashLength = 0;
  lineDashPhase = 0;
  strokeAdjust = gFalse;
  clip = new SplashClip(0, 0, width - 0.001, height - 0.001, vectorAntialias);
  softMask = NULL;
  deleteSoftMask = gFalse;
  inNonIsolatedGroup = gFalse;
  next = NULL;
}

SplashState::SplashState(SplashState *state) {
  memcpy(matrix, state->matrix, 6 * sizeof(SplashCoord));
  strokePattern = state->strokePattern->copy();
  fillPattern = state->fillPattern->copy();
  screen = state->screen->copy();
  blendFunc = state->blendFunc;
  strokeAlpha = state->strokeAlpha;
  fillAlpha = state->fillAlpha;
  lineWidth = state->lineWidth;
  lineCap = state->lineCap;
  lineJoin = state->lineJoin;
  miterLimit = state->miterLimit;
  flatness = state->flatness;
  if (state->lineDash) {
    lineDashLength = state->lineDashLength;
    lineDash = (SplashCoord *)gmallocn(lineDashLength, sizeof(SplashCoord));
    memcpy(lineDash, state->lineDash, lineDashLength * sizeof(SplashCoord));
  } else {
    lineDash = NULL;
    lineDashLength = 0;
  }
  lineDashPhase = state->lineDashPhase;
  strokeAdjust = state->strokeAdjust;
  clip = state->clip->copy();
  softMask = state->softMask;
  deleteSoftMask = gFalse;
  inNonIsolatedGroup = state->inNonIsolatedGroup;
  next = NULL;
}

SplashState::~SplashState() {
  delete strokePattern;
  delete fillPattern;
  delete screen;
  gfree(lineDash);
  delete clip;
  if (deleteSoftMask && softMask) {
    delete softMask;
  }
}

void SplashState::setStrokePattern(SplashPattern *strokePatternA) {
  delete strokePattern;
  strokePattern = strokePatternA;
}

void SplashState::setFillPattern(SplashPattern *fillPatternA) {
  delete fillPattern;
  fillPattern = fillPatternA;
}

void SplashState::setScreen(SplashScreen *screenA) {
  delete screen;
  screen = screenA;
}

void SplashState::setLineDash(SplashCoord *lineDashA, int lineDashLengthA,
			      SplashCoord lineDashPhaseA) {
  gfree(lineDash);
  lineDashLength = lineDashLengthA;
  if (lineDashLength > 0) {
    lineDash = (SplashCoord *)gmallocn(lineDashLength, sizeof(SplashCoord));
    memcpy(lineDash, lineDashA, lineDashLength * sizeof(SplashCoord));
  } else {
    lineDash = NULL;
  }
  lineDashPhase = lineDashPhaseA;
}

void SplashState::setSoftMask(SplashBitmap *softMaskA) {
  if (deleteSoftMask) {
    delete softMask;
  }
  softMask = softMaskA;
  deleteSoftMask = gTrue;
}
