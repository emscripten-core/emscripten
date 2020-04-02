//========================================================================
//
// SplashFontFile.cc
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
// Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "SplashFontFile.h"
#include "SplashFontFileID.h"

#ifdef VMS
#if (__VMS_VER < 70000000)
extern "C" int unlink(char *filename);
#endif
#endif

//------------------------------------------------------------------------
// SplashFontFile
//------------------------------------------------------------------------

SplashFontFile::SplashFontFile(SplashFontFileID *idA, SplashFontSrc *srcA) {
  id = idA;
  src = srcA;
  src->ref();
  refCnt = 0;
  doAdjustMatrix = gFalse;
}

SplashFontFile::~SplashFontFile() {
  src->unref();
  delete id;
}

void SplashFontFile::incRefCnt() {
  ++refCnt;
}

void SplashFontFile::decRefCnt() {
  if (!--refCnt) {
    delete this;
  }
}

//

SplashFontSrc::SplashFontSrc() {
  isFile = gFalse;
  deleteSrc = gFalse;
  fileName = NULL;
  buf = NULL;
  refcnt = 1;
}

SplashFontSrc::~SplashFontSrc() {
  if (deleteSrc) {
    if (isFile) {
      if (fileName)
	unlink(fileName->getCString());
    } else {
      if (buf)
	gfree(buf);
    }
  }

  if (isFile && fileName)
    delete fileName;
}

void SplashFontSrc::ref() {
  refcnt++;
}

void SplashFontSrc::unref() {
  if (! --refcnt)
    delete this;
}

void SplashFontSrc::setFile(GooString *file, GBool del)
{
  isFile = gTrue;
  fileName = file->copy();
  deleteSrc = del;
}

void SplashFontSrc::setFile(const char *file, GBool del)
{
  isFile = gTrue;
  fileName = new GooString(file);
  deleteSrc = del;
}

void SplashFontSrc::setBuf(char *bufA, int bufLenA, GBool del)
{
  isFile = gFalse;
  buf = bufA;
  bufLen = bufLenA;
  deleteSrc = del;
}
