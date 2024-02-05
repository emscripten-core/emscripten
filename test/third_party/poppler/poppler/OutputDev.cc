//========================================================================
//
// OutputDev.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2006 Thorkild Stray <thorkild@ifi.uio.no>
// Copyright (C) 2007 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
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

#include <stddef.h>
#include "Object.h"
#include "Stream.h"
#include "GfxState.h"
#include "OutputDev.h"
#include "goo/GooHash.h"

//------------------------------------------------------------------------
// OutputDev
//------------------------------------------------------------------------

void OutputDev::setDefaultCTM(double *ctm) {
  int i;
  double det;

  for (i = 0; i < 6; ++i) {
    defCTM[i] = ctm[i];
  }
  det = 1 / (defCTM[0] * defCTM[3] - defCTM[1] * defCTM[2]);
  defICTM[0] = defCTM[3] * det;
  defICTM[1] = -defCTM[1] * det;
  defICTM[2] = -defCTM[2] * det;
  defICTM[3] = defCTM[0] * det;
  defICTM[4] = (defCTM[2] * defCTM[5] - defCTM[3] * defCTM[4]) * det;
  defICTM[5] = (defCTM[1] * defCTM[4] - defCTM[0] * defCTM[5]) * det;
}

void OutputDev::cvtDevToUser(double dx, double dy, double *ux, double *uy) {
  *ux = defICTM[0] * dx + defICTM[2] * dy + defICTM[4];
  *uy = defICTM[1] * dx + defICTM[3] * dy + defICTM[5];
}

void OutputDev::cvtUserToDev(double ux, double uy, int *dx, int *dy) {
  *dx = (int)(defCTM[0] * ux + defCTM[2] * uy + defCTM[4] + 0.5);
  *dy = (int)(defCTM[1] * ux + defCTM[3] * uy + defCTM[5] + 0.5);
}

void OutputDev::updateAll(GfxState *state) {
  updateLineDash(state);
  updateFlatness(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateMiterLimit(state);
  updateLineWidth(state);
  updateStrokeAdjust(state);
  updateFillColorSpace(state);
  updateFillColor(state);
  updateStrokeColorSpace(state);
  updateStrokeColor(state);
  updateBlendMode(state);
  updateFillOpacity(state);
  updateStrokeOpacity(state);
  updateFillOverprint(state);
  updateStrokeOverprint(state);
  updateTransfer(state);
  updateFont(state);
}

GBool OutputDev::beginType3Char(GfxState *state, double x, double y,
				double dx, double dy,
				CharCode code, Unicode *u, int uLen) {
  return gFalse;
}

void OutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
			      int width, int height, GBool invert,
			      GBool interpolate, GBool inlineImg) {
  int i, j;

  if (inlineImg) {
    str->reset();
    j = height * ((width + 7) / 8);
    for (i = 0; i < j; ++i)
      str->getChar();
    str->close();
  }
}

void OutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			  int width, int height, GfxImageColorMap *colorMap,
			  GBool interpolate, int *maskColors, GBool inlineImg) {
  int i, j;

  if (inlineImg) {
    str->reset();
    j = height * ((width * colorMap->getNumPixelComps() *
		   colorMap->getBits() + 7) / 8);
    for (i = 0; i < j; ++i)
      str->getChar();
    str->close();
  }
}

void OutputDev::drawMaskedImage(GfxState *state, Object *ref, Stream *str,
				int width, int height,
				GfxImageColorMap *colorMap,
				GBool interpolate,
				Stream *maskStr,
				int maskWidth, int maskHeight,
				GBool maskInvert,
				GBool maskInterpolate) {
  drawImage(state, ref, str, width, height, colorMap, interpolate, NULL, gFalse);
}

void OutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
				    int width, int height,
				    GfxImageColorMap *colorMap,
				    GBool interpolate,
				    Stream *maskStr,
				    int maskWidth, int maskHeight,
				    GfxImageColorMap *maskColorMap,
				    GBool maskInterpolate) {
  drawImage(state, ref, str, width, height, colorMap, interpolate, NULL, gFalse);
}

void OutputDev::endMarkedContent(GfxState *state) {
}

void OutputDev::beginMarkedContent(char *name, Dict *properties) {
}

void OutputDev::markPoint(char *name) {
}

void OutputDev::markPoint(char *name, Dict *properties) {
}


#if OPI_SUPPORT
void OutputDev::opiBegin(GfxState *state, Dict *opiDict) {
}

void OutputDev::opiEnd(GfxState *state, Dict *opiDict) {
}
#endif

void OutputDev::startProfile() {
  if (profileHash)
    delete profileHash;

  profileHash = new GooHash (true);
}
 
GooHash *OutputDev::endProfile() {
  GooHash *profile = profileHash;

  profileHash = NULL;

  return profile;
}

