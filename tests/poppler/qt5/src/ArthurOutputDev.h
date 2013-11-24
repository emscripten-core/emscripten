//========================================================================
//
// ArthurOutputDev.h
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2005 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Pino Toscano <pino@kde.org>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef ARTHUROUTPUTDEV_H
#define ARTHUROUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "OutputDev.h"
#include "GfxState.h"

#include <QtGui/QPainter>

class GfxState;
class GfxPath;
class Gfx8BitFont;
struct GfxRGB;

class SplashFont;
class SplashFontEngine;
struct SplashGlyphBitmap;

//------------------------------------------------------------------------
// ArthurOutputDev - Qt 4 QPainter renderer
//------------------------------------------------------------------------

class ArthurOutputDev: public OutputDev {
public:
  /**
   * Describes how fonts are distorted (aka hinted) to fit the pixel grid.
   * More hinting means sharper edges and less adherence to the true letter shapes.
   */
  enum FontHinting {
    NoHinting = 0, ///< Font shapes are left unchanged
    SlightHinting, ///< Font shapes are distorted vertically only
    FullHinting ///< Font shapes are distorted horizontally and vertically
  };

  // Constructor.
  ArthurOutputDev(QPainter *painter );

  // Destructor.
  virtual ~ArthurOutputDev();

  void setFontHinting(FontHinting hinting) { m_fontHinting = hinting; }

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
  virtual void startPage(int pageNum, GfxState *state, XRef *xref);

  // End a page.
  virtual void endPage();

  //----- save/restore graphics state
  virtual void saveState(GfxState *state);
  virtual void restoreState(GfxState *state);

  //----- update graphics state
  virtual void updateAll(GfxState *state);
  virtual void updateCTM(GfxState *state, double m11, double m12,
			 double m21, double m22, double m31, double m32);
  virtual void updateLineDash(GfxState *state);
  virtual void updateFlatness(GfxState *state);
  virtual void updateLineJoin(GfxState *state);
  virtual void updateLineCap(GfxState *state);
  virtual void updateMiterLimit(GfxState *state);
  virtual void updateLineWidth(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);
  virtual void updateFillOpacity(GfxState *state);
  virtual void updateStrokeOpacity(GfxState *state);

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  //   virtual void drawString(GfxState *state, GooString *s);
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen);
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);
  virtual void endType3Char(GfxState *state);
  virtual void endTextObject(GfxState *state);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 GBool interpolate, int *maskColors, GBool inlineImg);

  //----- Type 3 font operators
  virtual void type3D0(GfxState *state, double wx, double wy);
  virtual void type3D1(GfxState *state, double wx, double wy,
		       double llx, double lly, double urx, double ury);

  //----- special access

  // Called to indicate that a new PDF document has been loaded.
  void startDoc(XRef *xrefA);
 
  GBool isReverseVideo() { return gFalse; }
  
private:
  QPainter *m_painter;
  FontHinting m_fontHinting;
  QFont m_currentFont;
  QPen m_currentPen;
  QBrush m_currentBrush;
  GBool m_needFontUpdate;		// set when the font needs to be updated
  SplashFontEngine *m_fontEngine;
  SplashFont *m_font;		// current font
  XRef *xref;			// xref table for current document
};

#endif
