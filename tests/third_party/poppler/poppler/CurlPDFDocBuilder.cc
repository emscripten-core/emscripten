//========================================================================
//
// CurlPDFDocBuilder.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include <config.h>

#include "CurlPDFDocBuilder.h"

#include "CachedFile.h"
#include "CurlCachedFile.h"

//------------------------------------------------------------------------
// CurlPDFDocBuilder
//------------------------------------------------------------------------

PDFDoc *
CurlPDFDocBuilder::buildPDFDoc(const GooString &uri,
        GooString *ownerPassword, GooString *userPassword, void *guiDataA)
{
    Object obj;

    CachedFile *cachedFile = new CachedFile(
        new CurlCachedFileLoader(), uri.copy());

    obj.initNull();
    BaseStream *str = new CachedFileStream(
         cachedFile, 0, gFalse, cachedFile->getLength(), &obj);

    return new PDFDoc(str, ownerPassword, userPassword, guiDataA);
}

GBool CurlPDFDocBuilder::supports(const GooString &uri)
{
  if (uri.cmpN("http://", 7) == 0 || uri.cmpN("https://", 8) == 0) {
    return gTrue;
  } else {
    return gFalse;
  }
}

