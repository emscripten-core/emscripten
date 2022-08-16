//========================================================================
//
// PreScanOutputDev.h
//
// Copyright 2005 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 William Bader <williambader@hotmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PRESCANOUTPUTDEV_H
#define PRESCANOUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "GfxState.h"
#include "OutputDev.h"

//------------------------------------------------------------------------
// PreScanOutputDev
//------------------------------------------------------------------------

class PreScanOutputDev: public OutputDev {
public:

  // Constructor.
  PreScanOutputDev();

  // Destructor.
  virtual ~PreScanOutputDev();

  //----- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gTrue; }

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  virtual void beginStringOp(GfxState *state);
  virtual void endStringOp(GfxState *state);
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);
  virtual void endType3Char(GfxState *state);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);
  virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       Stream *maskStr, int maskWidth, int maskHeight,
			       GBool maskInvert, GBool maskInterpolate);
  virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
				   int width, int height,
				   GfxImageColorMap *colorMap,
				   GBool interpolate,
				   Stream *maskStr,
				   int maskWidth, int maskHeight,
				   GfxImageColorMap *maskColorMap,
				   GBool maskInterpolate);

  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(GfxState *state, double *bbox,
				      GfxColorSpace *blendingColorSpace,
				      GBool isolated, GBool knockout,
				      GBool forSoftMask);
	virtual void paintTransparencyGroup(GfxState *state, double *bbox);
	virtual void setSoftMask(GfxState *state, double *bbox, GBool alpha,
			   Function *transferFunc, GfxColor *backdropColor);

  //----- special access

  // Returns true if the operations performed since the last call to
  // clearStats() are all monochrome (black or white).
  GBool isMonochrome() { return mono; }

  // Returns true if the operations performed since the last call to
  // clearStats() are all gray.
  GBool isGray() { return gray; }

  // Returns true if the operations performed since the last call to
  // clearStats() included any transparency.
  GBool usesTransparency() { return transparency; }

  // Returns true if the operations performed since the last call to
  // clearStats() are all rasterizable by GDI calls in GDIOutputDev.
  GBool isAllGDI() { return gdi; }

  // Returns true if the operations performed since the last call to
  // clearStats() processed a feature that PSOutputDev does not implement.
  GBool hasLevel1PSBug() { return level1PSBug; }

  // Clear the stats used by the above functions.
  void clearStats();

private:

  void check(GfxColorSpace *colorSpace, GfxColor *color,
	     double opacity, GfxBlendMode blendMode);

  GBool mono;
  GBool gray;
  GBool transparency;
  GBool gdi;
  PSLevel level;		// PostScript level (1, 2, separation)
  GBool level1PSBug;		// gTrue if it uses a feature not supported in PSOutputDev
};

#endif
