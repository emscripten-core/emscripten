//========================================================================
//
// SplashT1FontEngine.h
//
//========================================================================

#ifndef SPLASHT1FONTENGINE_H
#define SPLASHT1FONTENGINE_H

#if HAVE_T1LIB_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"

class SplashFontFile;
class SplashFontFileID;
class SplashFontSrc;

//------------------------------------------------------------------------
// SplashT1FontEngine
//------------------------------------------------------------------------

class SplashT1FontEngine {
public:

  static SplashT1FontEngine *init(GBool aaA);

  ~SplashT1FontEngine();

  // Load fonts.
  SplashFontFile *loadType1Font(SplashFontFileID *idA, SplashFontSrc *src, char **enc);
  SplashFontFile *loadType1CFont(SplashFontFileID *idA, SplashFontSrc *src, char **enc);

private:

  SplashT1FontEngine(GBool aaA);

  static int t1libInitCount;
  GBool aa;

  friend class SplashT1FontFile;
  friend class SplashT1Font;
};

#endif // HAVE_T1LIB_H

#endif
