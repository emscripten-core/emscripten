//========================================================================
//
// SplashT1Font.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHT1FONT_H
#define SPLASHT1FONT_H

#if HAVE_T1LIB_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashFont.h"

class SplashT1FontFile;

//------------------------------------------------------------------------
// SplashT1Font
//------------------------------------------------------------------------

class SplashT1Font: public SplashFont {
public:

  SplashT1Font(SplashT1FontFile *fontFileA, SplashCoord *matA,
	       SplashCoord *textMatA);

  virtual ~SplashT1Font();

  // Munge xFrac and yFrac before calling SplashFont::getGlyph.
  virtual GBool getGlyph(int c, int xFrac, int yFrac,
			 SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes);

  // Rasterize a glyph.  The <xFrac> and <yFrac> values are the same
  // as described for getGlyph.
  virtual GBool makeGlyph(int c, int xFrac, int yFrac,
			  SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes);

  // Return the path for a glyph.
  virtual SplashPath *getGlyphPath(int c);

private:

  int t1libID;			// t1lib font ID
  int outlineID;		// t1lib font ID for glyph outlines
  float size;
  float outlineSize;		// size for glyph outlines
  float outlineMul;
};

#endif // HAVE_T1LIB_H

#endif
