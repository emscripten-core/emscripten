//========================================================================
//
// SplashFTFontEngine.cc
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
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "fofi/FoFiTrueType.h"
#include "fofi/FoFiType1C.h"
#include "SplashFTFontFile.h"
#include "SplashFTFontEngine.h"

#ifdef VMS
#if (__VMS_VER < 70000000)
extern "C" int unlink(char *filename);
#endif
#endif

//------------------------------------------------------------------------

#if 0
static void fileWrite(void *stream, char *data, int len) {
  fwrite(data, 1, len, (FILE *)stream);
}
#endif

//------------------------------------------------------------------------
// SplashFTFontEngine
//------------------------------------------------------------------------

SplashFTFontEngine::SplashFTFontEngine(GBool aaA, GBool enableFreeTypeHintingA, FT_Library libA) {
  FT_Int major, minor, patch;

  aa = aaA;
  enableFreeTypeHinting = enableFreeTypeHintingA;
  lib = libA;

  // as of FT 2.1.8, CID fonts are indexed by CID instead of GID
  FT_Library_Version(lib, &major, &minor, &patch);
  useCIDs = major > 2 ||
            (major == 2 && (minor > 1 || (minor == 1 && patch > 7)));
}

SplashFTFontEngine *SplashFTFontEngine::init(GBool aaA, GBool enableFreeTypeHintingA) {
  FT_Library libA;

  if (FT_Init_FreeType(&libA)) {
    return NULL;
  }
  return new SplashFTFontEngine(aaA, enableFreeTypeHintingA, libA);
}

SplashFTFontEngine::~SplashFTFontEngine() {
  FT_Done_FreeType(lib);
}

SplashFontFile *SplashFTFontEngine::loadType1Font(SplashFontFileID *idA,
						  SplashFontSrc *src,
						  char **enc) {
  return SplashFTFontFile::loadType1Font(this, idA, src, enc);
}

SplashFontFile *SplashFTFontEngine::loadType1CFont(SplashFontFileID *idA,
						   SplashFontSrc *src,
						   char **enc) {
  return SplashFTFontFile::loadType1Font(this, idA, src, enc);
}

SplashFontFile *SplashFTFontEngine::loadOpenTypeT1CFont(SplashFontFileID *idA,
							SplashFontSrc *src,
							char **enc) {
  return SplashFTFontFile::loadType1Font(this, idA, src, enc);
}

SplashFontFile *SplashFTFontEngine::loadCIDFont(SplashFontFileID *idA,
						SplashFontSrc *src) {
  FoFiType1C *ff;
  Gushort *cidToGIDMap;
  int nCIDs;
  SplashFontFile *ret;

  // check for a CFF font
  if (useCIDs) {
    cidToGIDMap = NULL;
    nCIDs = 0;
  } else {
    if (src->isFile) {
      ff = FoFiType1C::load(src->fileName->getCString());
    } else {
      ff = FoFiType1C::make(src->buf, src->bufLen);
    }
    if (ff) {
      cidToGIDMap = ff->getCIDToGIDMap(&nCIDs);
      delete ff;
    } else {
      cidToGIDMap = NULL;
      nCIDs = 0;
    }
  }
  ret = SplashFTFontFile::loadCIDFont(this, idA, src, cidToGIDMap, nCIDs);
  if (!ret) {
    gfree(cidToGIDMap);
  }
  return ret;
}

SplashFontFile *SplashFTFontEngine::loadOpenTypeCFFFont(SplashFontFileID *idA,
							SplashFontSrc *src) {
  FoFiTrueType *ff;
  GBool isCID;
  Gushort *cidToGIDMap;
  int nCIDs;
  SplashFontFile *ret;

  cidToGIDMap = NULL;
  nCIDs = 0;
  isCID = gFalse;
  if (!useCIDs) {
    if (src->isFile) {
      ff = FoFiTrueType::load(src->fileName->getCString());
    } else {
      ff = FoFiTrueType::make(src->buf, src->bufLen);
    }
    if (ff) {
      if (ff->isOpenTypeCFF()) {
	cidToGIDMap = ff->getCIDToGIDMap(&nCIDs);
      }
      delete ff;
    }
  }
  ret = SplashFTFontFile::loadCIDFont(this, idA, src,
				      cidToGIDMap, nCIDs);
  if (!ret) {
    gfree(cidToGIDMap);
  }
  return ret;
}

SplashFontFile *SplashFTFontEngine::loadTrueTypeFont(SplashFontFileID *idA,
						     SplashFontSrc *src,
						     Gushort *codeToGID,
						     int codeToGIDLen,
						     int faceIndex) {
#if 0
  FoFiTrueType *ff;
  GooString *tmpFileName;
  FILE *tmpFile;
  SplashFontFile *ret;

  if (!(ff = FoFiTrueType::load(fileName))) {
    return NULL;
  }
  tmpFileName = NULL;
  if (!openTempFile(&tmpFileName, &tmpFile, "wb")) {
    delete ff;
    return NULL;
  }
  ff->writeTTF(&fileWrite, tmpFile);
  delete ff;
  fclose(tmpFile);
  ret = SplashFTFontFile::loadTrueTypeFont(this, idA,
					   tmpFileName->getCString(),
					   gTrue, codeToGID, codeToGIDLen,
					   faceIndex);
  if (ret) {
    if (deleteFile) {
      unlink(fileName);
    }
  } else {
    unlink(tmpFileName->getCString());
  }
  delete tmpFileName;
  return ret;
#else
  SplashFontFile *ret;
  ret = SplashFTFontFile::loadTrueTypeFont(this, idA, src,
					   codeToGID, codeToGIDLen,
					   faceIndex);
  return ret;
#endif
}

#endif // HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
