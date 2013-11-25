//========================================================================
//
// FontInfo.h
//
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005-2008, 2010, 2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2009 Pino Toscano <pino@kde.org>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

//========================================================================
//
// Based on code from pdffonts.cc
//
// Copyright 2001-2007 Glyph & Cog, LLC
//
//========================================================================

#ifndef FONT_INFO_H
#define FONT_INFO_H

#include "Object.h"
#include "goo/gtypes.h"
#include "goo/GooList.h"

class GfxFont;
class PDFDoc;

class FontInfo {
public:
  enum Type {
    unknown,
    Type1,
    Type1C,
    Type1COT,
    Type3,
    TrueType,
    TrueTypeOT,
    CIDType0,
    CIDType0C,
    CIDType0COT,
    CIDTrueType,
    CIDTrueTypeOT
  };
    
  // Constructor.
  FontInfo(GfxFont *fontA, XRef *xrefA);
  // Copy constructor
  FontInfo(FontInfo& f);
  // Destructor.
  ~FontInfo();

  GooString *getName()      { return name; };
  GooString *getSubstituteName() { return substituteName; };
  GooString *getFile()      { return file; };
  GooString *getEncoding()      { return encoding; };
  Type       getType()      { return type; };
  GBool      getEmbedded()  { return emb; };
  GBool      getSubset()    { return subset; };
  GBool      getToUnicode() { return hasToUnicode; };
  Ref        getRef()       { return fontRef; };
  Ref        getEmbRef()    { return embRef; };

private:
  GooString *name;
  GooString *substituteName;
  GooString *file;
  GooString *encoding;
  Type type;
  GBool emb;
  GBool subset;
  GBool hasToUnicode;
  Ref fontRef;
  Ref embRef;
};

class FontInfoScanner {
public:

  // Constructor.
  FontInfoScanner(PDFDoc *doc, int firstPage = 0);
  // Destructor.
  ~FontInfoScanner();

  GooList *scan(int nPages);

private:

  PDFDoc *doc;
  int currentPage;
  std::set<int> fonts;
  std::set<int> visitedObjects;

  void scanFonts(XRef *xrefA, Dict *resDict, GooList *fontsList);
};

#endif
