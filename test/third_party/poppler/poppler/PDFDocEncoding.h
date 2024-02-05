//========================================================================
//
// PDFDocEncoding.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PDFDOCENCODING_H
#define PDFDOCENCODING_H

#include "CharTypes.h"

class GooString;

extern Unicode pdfDocEncoding[256];

char* pdfDocEncodingToUTF16 (GooString* orig, int* length);

#endif
