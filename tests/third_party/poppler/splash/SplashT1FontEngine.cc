//========================================================================
//
// SplashT1FontEngine.cc
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
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#if HAVE_T1LIB_H

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <t1lib.h>
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "fofi/FoFiType1C.h"
#include "SplashT1FontFile.h"
#include "SplashT1FontEngine.h"

#ifdef VMS
#if (__VMS_VER < 70000000)
extern "C" int unlink(char *filename);
#endif
#endif

//------------------------------------------------------------------------

int SplashT1FontEngine::t1libInitCount = 0;

//------------------------------------------------------------------------

static void fileWrite(void *stream, char *data, int len) {
  fwrite(data, 1, len, (FILE *)stream);
}

//------------------------------------------------------------------------
// SplashT1FontEngine
//------------------------------------------------------------------------

SplashT1FontEngine::SplashT1FontEngine(GBool aaA) {
  aa = aaA;
}

SplashT1FontEngine *SplashT1FontEngine::init(GBool aaA) {
  // grayVals[i] = round(i * 255 / 16)
  static unsigned long grayVals[17] = {
    0, 16, 32, 48, 64, 80, 96, 112, 128, 143, 159, 175, 191, 207, 223, 239, 255
  };

  //~ for multithreading: need a mutex here
  if (t1libInitCount == 0) {
    T1_SetBitmapPad(8);
    if (!T1_InitLib(NO_LOGFILE | IGNORE_CONFIGFILE | IGNORE_FONTDATABASE |
		    T1_NO_AFM)) {
      return NULL;
    }
    if (aaA) {
      T1_AASetBitsPerPixel(8);
      T1_AASetLevel(T1_AA_HIGH);
      T1_AAHSetGrayValues(grayVals);
    } else {
      T1_AANSetGrayValues(0, 1);
    }
  }
  ++t1libInitCount;

  return new SplashT1FontEngine(aaA);
}

SplashT1FontEngine::~SplashT1FontEngine() {
  //~ for multithreading: need a mutex here
  if (--t1libInitCount == 0) {
    T1_CloseLib();
  }
}

SplashFontFile *SplashT1FontEngine::loadType1Font(SplashFontFileID *idA,
						  SplashFontSrc *src,
						  char **enc) {
  return SplashT1FontFile::loadType1Font(this, idA, src, enc);
}

SplashFontFile *SplashT1FontEngine::loadType1CFont(SplashFontFileID *idA,
						   SplashFontSrc *src,
						   char **enc) {
  FoFiType1C *ff;
  GooString *tmpFileName;
  FILE *tmpFile;
  SplashFontFile *ret;

  SplashFontSrc *newsrc;
  
  if (src->isFile)
    ff = FoFiType1C::load(src->fileName->getCString());
  else
    ff = FoFiType1C::make(src->buf, src->bufLen);
  if (! ff)
    return NULL;

  tmpFileName = NULL;
  if (!openTempFile(&tmpFileName, &tmpFile, "wb")) {
    delete ff;
    return NULL;
  }
  ff->convertToType1(NULL, NULL, gTrue, &fileWrite, tmpFile);
  delete ff;
  fclose(tmpFile);
  newsrc = new SplashFontSrc;
  newsrc->setFile(tmpFileName, gTrue);
  delete tmpFileName;
  ret = SplashT1FontFile::loadType1Font(this, idA, newsrc, enc);
  newsrc->unref();
  return ret;
}

#endif // HAVE_T1LIB_H
