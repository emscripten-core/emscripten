//========================================================================
//
// SplashFontEngine.h
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHFONTENGINE_H
#define SPLASHFONTENGINE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"

class SplashT1FontEngine;
class SplashFTFontEngine;
class SplashDTFontEngine;
class SplashDT4FontEngine;
class SplashFontFile;
class SplashFontFileID;
class SplashFont;
class SplashFontSrc;

//------------------------------------------------------------------------

#define splashFontCacheSize 16

//------------------------------------------------------------------------
// SplashFontEngine
//------------------------------------------------------------------------

class SplashFontEngine {
public:

  // Create a font engine.
  SplashFontEngine(
#if HAVE_T1LIB_H
		   GBool enableT1lib,
#endif
#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
		   GBool enableFreeType,
		   GBool enableFreeTypeHinting,
#endif
		   GBool aa);

  ~SplashFontEngine();

  // Get a font file from the cache.  Returns NULL if there is no
  // matching entry in the cache.
  SplashFontFile *getFontFile(SplashFontFileID *id);

  // Load fonts - these create new SplashFontFile objects.
  SplashFontFile *loadType1Font(SplashFontFileID *idA, SplashFontSrc *src, char **enc);
  SplashFontFile *loadType1CFont(SplashFontFileID *idA, SplashFontSrc *src, char **enc);
  SplashFontFile *loadOpenTypeT1CFont(SplashFontFileID *idA, SplashFontSrc *src, char **enc);
  SplashFontFile *loadCIDFont(SplashFontFileID *idA, SplashFontSrc *src);
  SplashFontFile *loadOpenTypeCFFFont(SplashFontFileID *idA, SplashFontSrc *src);
  SplashFontFile *loadTrueTypeFont(SplashFontFileID *idA, SplashFontSrc *src,
				   Gushort *codeToGID, int codeToGIDLen, int faceIndex = 0);

  // Get a font - this does a cache lookup first, and if not found,
  // creates a new SplashFont object and adds it to the cache.  The
  // matrix, mat = textMat * ctm:
  //    [ mat[0] mat[1] ]
  //    [ mat[2] mat[3] ]
  // specifies the font transform in PostScript style:
  //    [x' y'] = [x y] * mat
  // Note that the Splash y axis points downward.
  SplashFont *getFont(SplashFontFile *fontFile,
		      SplashCoord *textMat, SplashCoord *ctm);

private:

  SplashFont *fontCache[splashFontCacheSize];

#if HAVE_T1LIB_H
  SplashT1FontEngine *t1Engine;
#endif
#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
  SplashFTFontEngine *ftEngine;
#endif
};

#endif
