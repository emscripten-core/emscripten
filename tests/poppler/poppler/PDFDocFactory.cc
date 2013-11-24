//========================================================================
//
// PDFDocFactory.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include <config.h>

#include "PDFDocFactory.h"

#include "goo/GooList.h"
#include "goo/GooString.h"
#include "PDFDoc.h"
#include "LocalPDFDocBuilder.h"
#include "StdinPDFDocBuilder.h"
#if ENABLE_LIBCURL
#include "CurlPDFDocBuilder.h"
#endif
#include "ErrorCodes.h"

//------------------------------------------------------------------------
// PDFDocFactory
//------------------------------------------------------------------------

PDFDocFactory::PDFDocFactory(GooList *pdfDocBuilders)
{
  if (pdfDocBuilders) {
    builders = pdfDocBuilders;
  } else {
    builders = new GooList();
  }
#if ENABLE_LIBCURL
  builders->insert(0, new CurlPDFDocBuilder());
#endif
  builders->insert(0, new StdinPDFDocBuilder());
  builders->insert(0, new LocalPDFDocBuilder());
}

PDFDocFactory::~PDFDocFactory()
{
  if (builders) {
    deleteGooList(builders, PDFDocBuilder);
  }
}

PDFDoc *
PDFDocFactory::createPDFDoc(const GooString &uri, GooString *ownerPassword,
                                    GooString *userPassword, void *guiDataA)
{
  for (int i = builders->getLength() - 1; i >= 0 ; i--) {
    PDFDocBuilder *builder = (PDFDocBuilder *) builders->get(i);
    if (builder->supports(uri)) {
      return builder->buildPDFDoc(uri, ownerPassword, userPassword, guiDataA);
    }
  }

  error(errInternal, -1, "Cannot handle URI '{0:t}'.", &uri);
  GooString *fileName = uri.copy();
  return PDFDoc::ErrorPDFDoc(errOpenFile, fileName);
}

void PDFDocFactory::registerPDFDocBuilder(PDFDocBuilder *pdfDocBuilder)
{
  builders->append(pdfDocBuilder);
}


