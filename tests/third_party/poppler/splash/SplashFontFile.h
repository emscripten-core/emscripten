//========================================================================
//
// SplashFontFile.h
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
// Copyright (C) 2008, 2010 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef SPLASHFONTFILE_H
#define SPLASHFONTFILE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "SplashTypes.h"

class GooString;
class SplashFontEngine;
class SplashFont;
class SplashFontFileID;

//------------------------------------------------------------------------
// SplashFontFile
//------------------------------------------------------------------------

class SplashFontSrc {
public:
  SplashFontSrc();

  void setFile(GooString *file, GBool del);
  void setFile(const char *file, GBool del);
  void setBuf(char *bufA, int buflenA, GBool del);

  void ref();
  void unref();

  GBool isFile;
  GooString *fileName;
  char *buf;
  int bufLen;

private:
  ~SplashFontSrc();
  int refcnt;
  GBool deleteSrc;
};

class SplashFontFile {
public:

  virtual ~SplashFontFile();

  // Create a new SplashFont, i.e., a scaled instance of this font
  // file.
  virtual SplashFont *makeFont(SplashCoord *mat, SplashCoord *textMat) = 0;

  // Get the font file ID.
  SplashFontFileID *getID() { return id; }

  // Increment the reference count.
  void incRefCnt();

  // Decrement the reference count.  If the new value is zero, delete
  // the SplashFontFile object.
  void decRefCnt();

  GBool doAdjustMatrix;

protected:

  SplashFontFile(SplashFontFileID *idA, SplashFontSrc *srcA);

  SplashFontFileID *id;
  SplashFontSrc *src;
  int refCnt;

  friend class SplashFontEngine;
};

#endif
