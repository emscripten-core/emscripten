//========================================================================
//
// LocalPDFDocBuilder.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef LOCALPDFDOCBUILDER_H
#define LOCALPDFDOCBUILDER_H

#include "PDFDocBuilder.h"

//------------------------------------------------------------------------
// LocalPDFDocBuilder
//
// The LocalPDFDocBuilder implements a PDFDocBuilder for local files.
//------------------------------------------------------------------------

class LocalPDFDocBuilder : public PDFDocBuilder {

public:

  PDFDoc *buildPDFDoc(const GooString &uri, GooString *ownerPassword = NULL,
    GooString *userPassword = NULL, void *guiDataA = NULL);
  GBool supports(const GooString &uri);

};

#endif /* LOCALPDFDOCBUILDER_H */
