//========================================================================
//
// PDFDocFactory.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef PDFDOCFACTORY_H
#define PDFDOCFACTORY_H

#include "PDFDoc.h"

class GooList;
class GooString;
class PDFDocBuilder;

//------------------------------------------------------------------------
// PDFDocFactory
//
// PDFDocFactory allows the construction of PDFDocs from different URIs.
//
// By default, it supports local files, 'file://' and 'fd:0' (stdin). When
// compiled with libcurl, it also supports 'http://' and 'https://'.
//
// You can extend the supported URIs by giving a list of PDFDocBuilders to
// the constructor, or by registering a new PDFDocBuilder afterwards.
//------------------------------------------------------------------------

class PDFDocFactory {

public:

  PDFDocFactory(GooList *pdfDocBuilders = NULL);
  ~PDFDocFactory();

  // Create a PDFDoc. Returns a PDFDoc. You should check this PDFDoc
  // with PDFDoc::isOk() for failures.
  // The caller is responsible for deleting ownerPassword, userPassWord and guiData.
  PDFDoc *createPDFDoc(const GooString &uri, GooString *ownerPassword = NULL,
      GooString *userPassword = NULL, void *guiDataA = NULL);

  // Extend supported URIs with the ones from the PDFDocBuilder.
  void registerPDFDocBuilder(PDFDocBuilder *pdfDocBuilder);

private:

  GooList *builders;

};

#endif /* PDFDOCFACTORY_H */
