//========================================================================
//
// SplashT1FontFile.h
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
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHT1FONTFILE_H
#define SPLASHT1FONTFILE_H

#if HAVE_T1LIB_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashFontFile.h"

class SplashT1FontEngine;

//------------------------------------------------------------------------
// SplashT1FontFile
//------------------------------------------------------------------------

class SplashT1FontFile: public SplashFontFile {
public:

  static SplashFontFile *loadType1Font(SplashT1FontEngine *engineA,
				       SplashFontFileID *idA,
				       SplashFontSrc *src,
				       const char **encA);

  virtual ~SplashT1FontFile();

  // Create a new SplashT1Font, i.e., a scaled instance of this font
  // file.
  virtual SplashFont *makeFont(SplashCoord *mat,
			       SplashCoord *textMat);

private:

  SplashT1FontFile(SplashT1FontEngine *engineA,
		   SplashFontFileID *idA,
		   SplashFontSrc *src,
		   int t1libIDA, const char **encA, char *encStrA);

  SplashT1FontEngine *engine;
  int t1libID;			// t1lib font ID
  const char **enc;
  char *encStr;

  friend class SplashT1Font;
};

#endif // HAVE_T1LIB_H

#endif
