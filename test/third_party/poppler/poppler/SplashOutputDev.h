//========================================================================
//
// SplashOutputDev.h
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
// Copyright (C) 2005 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2009, 2010 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHOUTPUTDEV_H
#define SPLASHOUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "splash/SplashTypes.h"
#include "splash/SplashPattern.h"
#include "poppler-config.h"
#include "OutputDev.h"
#include "GfxState.h"

class Gfx8BitFont;
class SplashBitmap;
class Splash;
class SplashPath;
class SplashFontEngine;
class SplashFont;
class T3FontCache;
struct T3FontCacheTag;
struct T3GlyphStack;
struct SplashTransparencyGroup;

//------------------------------------------------------------------------
// Splash dynamic pattern
//------------------------------------------------------------------------

class SplashAxialPattern: public SplashPattern {
public:

  SplashAxialPattern(SplashColorMode colorMode, GfxState *state, GfxAxialShading *shading);

  virtual SplashPattern *copy() { return new SplashAxialPattern(colorMode, state, shading); }

  virtual ~SplashAxialPattern();

  virtual GBool getColor(int x, int y, SplashColorPtr c);

  virtual GBool isStatic() { return gFalse; }

private:
  Matrix ictm;
  double x0, y0, x1, y1;
  double dx, dy, mul;
  double t0, t1;
  GfxAxialShading *shading;
  GfxState *state;
  SplashColorMode colorMode;
  double *bbox;
};

// see GfxState.h, GfxGouraudTriangleShading
class SplashGouraudPattern: public SplashGouraudColor {
public:

  SplashGouraudPattern(GBool bDirectColorTranslation, GfxState *state, GfxGouraudTriangleShading *shading);

  virtual SplashPattern *copy() { return new SplashGouraudPattern(bDirectColorTranslation, state, shading); }

  virtual ~SplashGouraudPattern();

  virtual GBool getColor(int x, int y, SplashColorPtr c) { return gFalse; }

  virtual GBool isStatic() { return gFalse; }

  virtual GBool isParameterized() { return shading->isParameterized(); }
  virtual int getNTriangles() { return shading->getNTriangles(); }
  virtual  void getTriangle(int i, double *x0, double *y0, double *color0,
                            double *x1, double *y1, double *color1,
                            double *x2, double *y2, double *color2) 
  { return shading->getTriangle(i, x0, y0, color0, x1, y1, color1, x2, y2, color2); }

  virtual void getParameterizedColor(double t, SplashColorMode mode, SplashColorPtr c);

private:
  GfxGouraudTriangleShading *shading;
  GfxState *state;
  GBool bDirectColorTranslation;
};

//------------------------------------------------------------------------

// number of Type 3 fonts to cache
#define splashOutT3FontCacheSize 8

//------------------------------------------------------------------------
// SplashOutputDev
//------------------------------------------------------------------------

class SplashOutputDev: public OutputDev {
public:

  // Constructor.
  SplashOutputDev(SplashColorMode colorModeA, int bitmapRowPadA,
		  GBool reverseVideoA, SplashColorPtr paperColorA,
		  GBool bitmapTopDownA = gTrue,
		  GBool allowAntialiasA = gTrue);

  // Destructor.
  virtual ~SplashOutputDev();

  //----- get info about output device

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
  virtual GBool useShadedFills(int type)
  { return (type == 2 || type == 4 || type == 5 ) ? gTrue : gFalse; }

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gTrue; }

  // This device now supports text in pattern colorspace!
  virtual GBool supportTextCSPattern(GfxState *state)
  	{ return state->getFillColorSpace()->getMode() == csPattern; }

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

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
  virtual void updateStrokeAdjust(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);
  virtual void updateBlendMode(GfxState *state);
  virtual void updateFillOpacity(GfxState *state);
  virtual void updateStrokeOpacity(GfxState *state);

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);
  virtual GBool axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax);
  virtual GBool gouraudTriangleShadedFill(GfxState *state, GfxGouraudTriangleShading *shading);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);
  virtual void clipToStrokePath(GfxState *state);

  //----- text drawing
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen);
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);
  virtual void endType3Char(GfxState *state);
  virtual void beginTextObject(GfxState *state);
  virtual GBool deviceHasTextClip(GfxState *state) { return textClipPath && haveCSPattern; }
  virtual void endTextObject(GfxState *state);

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
  // If current colorspace ist pattern,
  // need this device special handling for masks in pattern colorspace?
  // Default is false
  virtual GBool fillMaskCSPattern(GfxState * state)
  	{ return state->getFillColorSpace()->getMode() == csPattern; }
  virtual void endMaskClip(GfxState * /*state*/);

  //----- Type 3 font operators
  virtual void type3D0(GfxState *state, double wx, double wy);
  virtual void type3D1(GfxState *state, double wx, double wy,
		       double llx, double lly, double urx, double ury);

  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(GfxState *state, double *bbox,
				      GfxColorSpace *blendingColorSpace,
				      GBool isolated, GBool knockout,
				      GBool forSoftMask);
  virtual void endTransparencyGroup(GfxState *state);
  virtual void paintTransparencyGroup(GfxState *state, double *bbox);
  virtual void setSoftMask(GfxState *state, double *bbox, GBool alpha,
			   Function *transferFunc, GfxColor *backdropColor);
  virtual void clearSoftMask(GfxState *state);

  //----- special access

  // Called to indicate that a new PDF document has been loaded.
  void startDoc(XRef *xrefA);
 
  void setPaperColor(SplashColorPtr paperColorA);

  GBool isReverseVideo() { return reverseVideo; }
  void setReverseVideo(GBool reverseVideoA) { reverseVideo = reverseVideoA; }

  // Get the bitmap and its size.
  SplashBitmap *getBitmap() { return bitmap; }
  int getBitmapWidth();
  int getBitmapHeight();

  // Returns the last rasterized bitmap, transferring ownership to the
  // caller.
  SplashBitmap *takeBitmap();

  // Get the Splash object.
  Splash *getSplash() { return splash; }

  // Get the modified region.
  void getModRegion(int *xMin, int *yMin, int *xMax, int *yMax);

  // Clear the modified region.
  void clearModRegion();

  // Set the Splash fill color.
  void setFillColor(int r, int g, int b);

  SplashFont *getCurrentFont() { return font; }

#if 1 //~tmp: turn off anti-aliasing temporarily
  virtual GBool getVectorAntialias();
  virtual void setVectorAntialias(GBool vaa);
#endif

  void setFreeTypeHinting(GBool enable);

private:

  void setupScreenParams(double hDPI, double vDPI);
#if SPLASH_CMYK
  SplashPattern *getColor(GfxGray gray, GfxRGB *rgb, GfxCMYK *cmyk);
#else
  SplashPattern *getColor(GfxGray gray, GfxRGB *rgb);
#endif
  SplashPath *convertPath(GfxState *state, GfxPath *path);
  void doUpdateFont(GfxState *state);
  void drawType3Glyph(T3FontCache *t3Font,
		      T3FontCacheTag *tag, Guchar *data);
  static GBool imageMaskSrc(void *data, SplashColorPtr line);
  static GBool imageSrc(void *data, SplashColorPtr colorLine,
			Guchar *alphaLine);
  static GBool alphaImageSrc(void *data, SplashColorPtr line,
			     Guchar *alphaLine);
  static GBool maskedImageSrc(void *data, SplashColorPtr line,
			      Guchar *alphaLine);

  GBool haveCSPattern;		// set if text has been drawn with a
							//   clipping render mode because of pattern colorspace
  GBool keepAlphaChannel;	// don't fill with paper color, keep alpha channel

  SplashColorMode colorMode;
  int bitmapRowPad;
  GBool bitmapTopDown;
  GBool allowAntialias;
  GBool vectorAntialias;
  GBool enableFreeTypeHinting;
  GBool reverseVideo;		// reverse video mode
  SplashColor paperColor;	// paper color
  SplashScreenParams screenParams;

  XRef *xref;			// xref table for current document

  SplashBitmap *bitmap;
  Splash *splash;
  SplashFontEngine *fontEngine;

  T3FontCache *			// Type 3 font cache
    t3FontCache[splashOutT3FontCacheSize];
  int nT3Fonts;			// number of valid entries in t3FontCache
  T3GlyphStack *t3GlyphStack;	// Type 3 glyph context stack

  SplashFont *font;		// current font
  GBool needFontUpdate;		// set when the font needs to be updated
  SplashPath *textClipPath;	// clipping path built with text object

  SplashTransparencyGroup *	// transparency group stack
    transpGroupStack;
  SplashBitmap *maskBitmap; // for image masks in pattern colorspace
};

#endif
