//========================================================================
//
// Splash.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Marco Pesenti Gritti <mpg@redhat.com>
// Copyright (C) 2007 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASH_H
#define SPLASH_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"
#include "SplashClip.h"
#include "SplashPattern.h"

class SplashBitmap;
struct SplashGlyphBitmap;
class SplashState;
class SplashScreen;
class SplashPath;
class SplashXPath;
class SplashFont;
struct SplashPipe;

//------------------------------------------------------------------------

// Retrieves the next line of pixels in an image mask.  Normally,
// fills in *<line> and returns true.  If the image stream is
// exhausted, returns false.
typedef GBool (*SplashImageMaskSource)(void *data, SplashColorPtr pixel);

// Retrieves the next line of pixels in an image.  Normally, fills in
// *<line> and returns true.  If the image stream is exhausted,
// returns false.
typedef GBool (*SplashImageSource)(void *data, SplashColorPtr colorLine,
				   Guchar *alphaLine);

//------------------------------------------------------------------------

enum SplashPipeResultColorCtrl {
#if SPLASH_CMYK
  splashPipeResultColorNoAlphaBlendCMYK,
#endif
  splashPipeResultColorNoAlphaBlendRGB,
  splashPipeResultColorNoAlphaBlendMono,
  splashPipeResultColorAlphaNoBlendMono,
  splashPipeResultColorAlphaNoBlendRGB,
#if SPLASH_CMYK
  splashPipeResultColorAlphaNoBlendCMYK,
#endif
  splashPipeResultColorAlphaBlendMono,
  splashPipeResultColorAlphaBlendRGB
#if SPLASH_CMYK
  ,
  splashPipeResultColorAlphaBlendCMYK
#endif
};

//------------------------------------------------------------------------
// Splash
//------------------------------------------------------------------------

class Splash {
public:

  // Create a new rasterizer object.
  Splash(SplashBitmap *bitmapA, GBool vectorAntialiasA,
	 SplashScreenParams *screenParams = NULL);
  Splash(SplashBitmap *bitmapA, GBool vectorAntialiasA,
	 SplashScreen *screenA);

  ~Splash();

  //----- state read

  SplashCoord *getMatrix();
  SplashPattern *getStrokePattern();
  SplashPattern *getFillPattern();
  SplashScreen *getScreen();
  SplashBlendFunc getBlendFunc();
  SplashCoord getStrokeAlpha();
  SplashCoord getFillAlpha();
  SplashCoord getLineWidth();
  int getLineCap();
  int getLineJoin();
  SplashCoord getMiterLimit();
  SplashCoord getFlatness();
  SplashCoord *getLineDash();
  int getLineDashLength();
  SplashCoord getLineDashPhase();
  SplashClip *getClip();
  SplashBitmap *getSoftMask();
  GBool getInNonIsolatedGroup();

  //----- state write

  void setMatrix(SplashCoord *matrix);
  void setStrokePattern(SplashPattern *strokeColor);
  void setFillPattern(SplashPattern *fillColor);
  void setScreen(SplashScreen *screen);
  void setBlendFunc(SplashBlendFunc func);
  void setStrokeAlpha(SplashCoord alpha);
  void setFillAlpha(SplashCoord alpha);
  void setLineWidth(SplashCoord lineWidth);
  void setLineCap(int lineCap);
  void setLineJoin(int lineJoin);
  void setMiterLimit(SplashCoord miterLimit);
  void setFlatness(SplashCoord flatness);
  // the <lineDash> array will be copied
  void setLineDash(SplashCoord *lineDash, int lineDashLength,
		   SplashCoord lineDashPhase);
  void setStrokeAdjust(GBool strokeAdjust);
  // NB: uses transformed coordinates.
  void clipResetToRect(SplashCoord x0, SplashCoord y0,
		       SplashCoord x1, SplashCoord y1);
  // NB: uses transformed coordinates.
  SplashError clipToRect(SplashCoord x0, SplashCoord y0,
			 SplashCoord x1, SplashCoord y1);
  // NB: uses untransformed coordinates.
  SplashError clipToPath(SplashPath *path, GBool eo);
  void setSoftMask(SplashBitmap *softMask);
  void setInNonIsolatedGroup(SplashBitmap *alpha0BitmapA,
			     int alpha0XA, int alpha0YA);

  //----- state save/restore

  void saveState();
  SplashError restoreState();

  //----- drawing operations

  // Fill the bitmap with <color>.  This is not subject to clipping.
  void clear(SplashColorPtr color, Guchar alpha = 0x00);

  // Stroke a path using the current stroke pattern.
  SplashError stroke(SplashPath *path);

  // Fill a path using the current fill pattern.
  SplashError fill(SplashPath *path, GBool eo);

  // Fill a path, XORing with the current fill pattern.
  SplashError xorFill(SplashPath *path, GBool eo);

  // Draw a character, using the current fill pattern.
  SplashError fillChar(SplashCoord x, SplashCoord y, int c, SplashFont *font);

  // Draw a glyph, using the current fill pattern.  This function does
  // not free any data, i.e., it ignores glyph->freeData.
  void fillGlyph(SplashCoord x, SplashCoord y,
			SplashGlyphBitmap *glyph);

  // Draws an image mask using the fill color.  This will read <h>
  // lines of <w> pixels from <src>, starting with the top line.  "1"
  // pixels will be drawn with the current fill color; "0" pixels are
  // transparent.  The matrix:
  //    [ mat[0] mat[1] 0 ]
  //    [ mat[2] mat[3] 0 ]
  //    [ mat[4] mat[5] 1 ]
  // maps a unit square to the desired destination for the image, in
  // PostScript style:
  //    [x' y' 1] = [x y 1] * mat
  // Note that the Splash y axis points downward, and the image source
  // is assumed to produce pixels in raster order, starting from the
  // top line.
  SplashError fillImageMask(SplashImageMaskSource src, void *srcData,
			    int w, int h, SplashCoord *mat,
			    GBool glyphMode);

  // Draw an image.  This will read <h> lines of <w> pixels from
  // <src>, starting with the top line.  These pixels are assumed to
  // be in the source mode, <srcMode>.  If <srcAlpha> is true, the
  // alpha values returned by <src> are used; otherwise they are
  // ignored.  The following combinations of source and target modes
  // are supported:
  //    source       target
  //    ------       ------
  //    Mono1        Mono1
  //    Mono8        Mono1   -- with dithering
  //    Mono8        Mono8
  //    RGB8         RGB8
  //    BGR8         BGR8
  //    CMYK8        CMYK8
  // The matrix behaves as for fillImageMask.
  SplashError drawImage(SplashImageSource src, void *srcData,
			SplashColorMode srcMode, GBool srcAlpha,
			int w, int h, SplashCoord *mat);

  // Composite a rectangular region from <src> onto this Splash
  // object.
  SplashError composite(SplashBitmap *src, int xSrc, int ySrc,
			int xDest, int yDest, int w, int h,
			GBool noClip, GBool nonIsolated);

  // Composite this Splash object onto a background color.  The
  // background alpha is assumed to be 1.
  void compositeBackground(SplashColorPtr color);

  // Copy a rectangular region from <src> onto the bitmap belonging to
  // this Splash object.  The destination alpha values are all set to
  // zero.
  SplashError blitTransparent(SplashBitmap *src, int xSrc, int ySrc,
			      int xDest, int yDest, int w, int h);

  //----- misc

  // Construct a path for a stroke, given the path to be stroked, and
  // using the current line parameters.  If <flatten> is true, this
  // function will first flatten the path and handle the linedash.
  SplashPath *makeStrokePath(SplashPath *path, GBool flatten = gTrue);

  // Return the associated bitmap.
  SplashBitmap *getBitmap() { return bitmap; }

  // Get a bounding box which includes all modifications since the
  // last call to clearModRegion.
  void getModRegion(int *xMin, int *yMin, int *xMax, int *yMax)
    { *xMin = modXMin; *yMin = modYMin; *xMax = modXMax; *yMax = modYMax; }

  // Clear the modified region bounding box.
  void clearModRegion();

  // Get clipping status for the last drawing operation subject to
  // clipping.
  SplashClipResult getClipRes() { return opClipRes; }

  // Toggle debug mode on or off.
  void setDebugMode(GBool debugModeA) { debugMode = debugModeA; }

#if 1 //~tmp: turn off anti-aliasing temporarily
  GBool getVectorAntialias() { return vectorAntialias; }
  void setVectorAntialias(GBool vaa) { vectorAntialias = vaa; }
#endif

  // Do shaded fills with dynamic patterns
  SplashError shadedFill(SplashPath *path, GBool hasBBox,
                         SplashPattern *pattern);
  // Draw a gouraud triangle shading.
  GBool gouraudTriangleShadedFill(SplashGouraudColor *shading);

private:

  void pipeInit(SplashPipe *pipe, int x, int y,
		SplashPattern *pattern, SplashColorPtr cSrc,
		SplashCoord aInput, GBool usesShape,
		GBool nonIsolatedGroup);
  void pipeRun(SplashPipe *pipe);
  void pipeSetXY(SplashPipe *pipe, int x, int y);
  void pipeIncX(SplashPipe *pipe);
  void drawPixel(SplashPipe *pipe, int x, int y, GBool noClip);
  void drawAAPixelInit();
  void drawAAPixel(SplashPipe *pipe, int x, int y);
  void drawSpan(SplashPipe *pipe, int x0, int x1, int y, GBool noClip);
  void drawAALine(SplashPipe *pipe, int x0, int x1, int y);
  void transform(SplashCoord *matrix, SplashCoord xi, SplashCoord yi,
		 SplashCoord *xo, SplashCoord *yo);
  void updateModX(int x);
  void updateModY(int y);
  void strokeNarrow(SplashPath *path);
  void strokeWide(SplashPath *path);
  SplashPath *flattenPath(SplashPath *path, SplashCoord *matrix,
			  SplashCoord flatness);
  void flattenCurve(SplashCoord x0, SplashCoord y0,
		    SplashCoord x1, SplashCoord y1,
		    SplashCoord x2, SplashCoord y2,
		    SplashCoord x3, SplashCoord y3,
		    SplashCoord *matrix, SplashCoord flatness2,
		    SplashPath *fPath);
  SplashPath *makeDashedPath(SplashPath *xPath);
  SplashError fillWithPattern(SplashPath *path, GBool eo,
			      SplashPattern *pattern, SplashCoord alpha);
  void fillGlyph2(int x0, int y0, SplashGlyphBitmap *glyph, GBool noclip);
  void dumpPath(SplashPath *path);
  void dumpXPath(SplashXPath *path);

  static SplashPipeResultColorCtrl pipeResultColorNoAlphaBlend[];
  static SplashPipeResultColorCtrl pipeResultColorAlphaNoBlend[];
  static SplashPipeResultColorCtrl pipeResultColorAlphaBlend[];
  static int pipeNonIsoGroupCorrection[];

  SplashBitmap *bitmap;
  SplashState *state;
  SplashBitmap *aaBuf;
  int aaBufY;
  SplashBitmap *alpha0Bitmap;	// for non-isolated groups, this is the
				//   bitmap containing the alpha0 values
  int alpha0X, alpha0Y;		// offset within alpha0Bitmap
  SplashCoord aaGamma[splashAASize * splashAASize + 1];
  int modXMin, modYMin, modXMax, modYMax;
  SplashClipResult opClipRes;
  GBool vectorAntialias;
  GBool debugMode;
};

#endif
