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
// Copyright (C) 2009-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2011 Andrea Canciani <ranma42@gmail.com>
// Copyright (C) 2011 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Albert Astals Cid <aacid@kde.org>
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
#include "GlobalParams.h"

class PDFDoc;
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

class SplashUnivariatePattern: public SplashPattern {
public:

  SplashUnivariatePattern(SplashColorMode colorMode, GfxState *state, GfxUnivariateShading *shading);

  virtual ~SplashUnivariatePattern();

  virtual GBool getColor(int x, int y, SplashColorPtr c);

  virtual GBool testPosition(int x, int y);

  virtual GBool isStatic() { return gFalse; }

  virtual GBool getParameter(double xs, double ys, double *t) = 0;

  virtual GfxUnivariateShading *getShading() { return shading; }

protected:
  Matrix ictm;
  double t0, t1, dt;
  GfxUnivariateShading *shading;
  GfxState *state;
  SplashColorMode colorMode;
};

class SplashAxialPattern: public SplashUnivariatePattern {
public:

  SplashAxialPattern(SplashColorMode colorMode, GfxState *state, GfxAxialShading *shading);

  virtual SplashPattern *copy() { return new SplashAxialPattern(colorMode, state, (GfxAxialShading *) shading); }

  virtual ~SplashAxialPattern();

  virtual GBool getParameter(double xs, double ys, double *t);

private:
  double x0, y0, x1, y1;
  double dx, dy, mul;
};

// see GfxState.h, GfxGouraudTriangleShading
class SplashGouraudPattern: public SplashGouraudColor {
public:

  SplashGouraudPattern(GBool bDirectColorTranslation, GfxState *state, GfxGouraudTriangleShading *shading, SplashColorMode mode);

  virtual SplashPattern *copy() { return new SplashGouraudPattern(bDirectColorTranslation, state, shading, mode); }

  virtual ~SplashGouraudPattern();

  virtual GBool getColor(int x, int y, SplashColorPtr c) { return gFalse; }

  virtual GBool testPosition(int x, int y) { return gFalse; }

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
  SplashColorMode mode;
};

// see GfxState.h, GfxRadialShading
class SplashRadialPattern: public SplashUnivariatePattern {
public:

  SplashRadialPattern(SplashColorMode colorMode, GfxState *state, GfxRadialShading *shading);

  virtual SplashPattern *copy() { return new SplashRadialPattern(colorMode, state, (GfxRadialShading *) shading); }

  virtual ~SplashRadialPattern();

  virtual GBool getParameter(double xs, double ys, double *t);

private:
  double x0, y0, r0, dx, dy, dr;
  double a, inva;
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
		  GBool allowAntialiasA = gTrue,
		  SplashThinLineMode thinLineMode = splashThinLineDefault,
      GBool overprintPreviewA = globalParams->getOverprintPreview());

  // Destructor.
  virtual ~SplashOutputDev();

  //----- get info about output device

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill() { return gTrue; }

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
  virtual GBool useShadedFills(int type)
  { return (type >= 2 && type <= 5) ? gTrue : gFalse; }

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return bitmapTopDown ^ bitmapUpsideDown; }

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
  virtual void updateStrokeAdjust(GfxState *state);
  virtual void updateFillColorSpace(GfxState *state);
  virtual void updateStrokeColorSpace(GfxState *state);
  virtual void updateFillColor(GfxState *state);
  virtual void updateStrokeColor(GfxState *state);
  virtual void updateBlendMode(GfxState *state);
  virtual void updateFillOpacity(GfxState *state);
  virtual void updateStrokeOpacity(GfxState *state);
  virtual void updateFillOverprint(GfxState *state);
  virtual void updateStrokeOverprint(GfxState *state);
  virtual void updateOverprintMode(GfxState *state);
  virtual void updateTransfer(GfxState *state);

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);
  virtual GBool tilingPatternFill(GfxState *state, Gfx *gfx, Catalog *catalog, Object *str,
				  double *pmat, int paintType, int tilingType, Dict *resDict,
				  double *mat, double *bbox,
				  int x0, int y0, int x1, int y1,
				  double xStep, double yStep);
  virtual GBool axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax);
  virtual GBool radialShadedFill(GfxState *state, GfxRadialShading *shading, double tMin, double tMax);
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
  virtual void endTextObject(GfxState *state);

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool interpolate, GBool inlineImg);
  virtual void setSoftMaskFromImageMask(GfxState *state,
					Object *ref, Stream *str,
					int width, int height, GBool invert,
					GBool inlineImg, double *baseMatrix);
  virtual void unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix);
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

  //----- Type 3 font operators
  virtual void type3D0(GfxState *state, double wx, double wy);
  virtual void type3D1(GfxState *state, double wx, double wy,
		       double llx, double lly, double urx, double ury);

  //----- transparency groups and soft masks
  virtual GBool checkTransparencyGroup(GfxState *state, GBool knockout);
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
  void startDoc(PDFDoc *docA);
 
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

  // Set this flag to true to generate an upside-down bitmap (useful
  // for Windows BMP files).
  void setBitmapUpsideDown(GBool f) { bitmapUpsideDown = f; }

  // Get the Splash object.
  Splash *getSplash() { return splash; }

  // Get the modified region.
  void getModRegion(int *xMin, int *yMin, int *xMax, int *yMax);

  // Clear the modified region.
  void clearModRegion();

  SplashFont *getCurrentFont() { return font; }

  // If <skipTextA> is true, don't draw horizontal text.
  // If <skipRotatedTextA> is true, don't draw rotated (non-horizontal) text.
  void setSkipText(GBool skipHorizTextA, GBool skipRotatedTextA)
    { skipHorizText = skipHorizTextA; skipRotatedText = skipRotatedTextA; }

  int getNestCount() { return nestCount; }

#if 1 //~tmp: turn off anti-aliasing temporarily
  virtual GBool getVectorAntialias();
  virtual void setVectorAntialias(GBool vaa);
#endif

  void setFreeTypeHinting(GBool enable, GBool enableSlightHinting);

protected:
  void doUpdateFont(GfxState *state);

private:
  GBool univariateShadedFill(GfxState *state, SplashUnivariatePattern *pattern, double tMin, double tMax);

  void setupScreenParams(double hDPI, double vDPI);
  SplashPattern *getColor(GfxGray gray);
  SplashPattern *getColor(GfxRGB *rgb);
#if SPLASH_CMYK
  SplashPattern *getColor(GfxCMYK *cmyk);
  SplashPattern *getColor(GfxColor *deviceN);
#endif
  void setOverprintMask(GfxColorSpace *colorSpace, GBool overprintFlag,
			int overprintMode, GfxColor *singleColor, GBool grayIndexed = gFalse);
  SplashPath *convertPath(GfxState *state, GfxPath *path,
			  GBool dropEmptySubpaths);
  void drawType3Glyph(GfxState *state, T3FontCache *t3Font,
		      T3FontCacheTag *tag, Guchar *data);
  static GBool imageMaskSrc(void *data, SplashColorPtr line);
  static GBool imageSrc(void *data, SplashColorPtr colorLine,
			Guchar *alphaLine);
  static GBool alphaImageSrc(void *data, SplashColorPtr line,
			     Guchar *alphaLine);
  static GBool maskedImageSrc(void *data, SplashColorPtr line,
			      Guchar *alphaLine);
  static GBool tilingBitmapSrc(void *data, SplashColorPtr line,
			     Guchar *alphaLine);

  GBool keepAlphaChannel;	// don't fill with paper color, keep alpha channel

  SplashColorMode colorMode;
  int bitmapRowPad;
  GBool bitmapTopDown;
  GBool bitmapUpsideDown;
  GBool allowAntialias;
  GBool vectorAntialias;
  GBool overprintPreview;
  GBool enableFreeTypeHinting;
  GBool enableSlightHinting;
  GBool reverseVideo;		// reverse video mode
  SplashColor paperColor;	// paper color
  SplashScreenParams screenParams;
  GBool skipHorizText;
  GBool skipRotatedText;

  PDFDoc *doc;			// the current document
  XRef *xref;       // the xref of the current document

  SplashBitmap *bitmap;
  Splash *splash;
  SplashFontEngine *fontEngine;

  T3FontCache *			// Type 3 font cache
    t3FontCache[splashOutT3FontCacheSize];
  int nT3Fonts;			// number of valid entries in t3FontCache
  T3GlyphStack *t3GlyphStack;	// Type 3 glyph context stack
  GBool haveT3Dx;		// set after seeing a d0/d1 operator

  SplashFont *font;		// current font
  GBool needFontUpdate;		// set when the font needs to be updated
  SplashPath *textClipPath;	// clipping path built with text object

  SplashTransparencyGroup *	// transparency group stack
    transpGroupStack;
  SplashBitmap *maskBitmap; // for image masks in pattern colorspace
  int nestCount;
};

#endif
