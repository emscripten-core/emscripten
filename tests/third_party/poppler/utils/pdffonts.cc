//========================================================================
//
// pdffonts.cc
//
// Copyright 2001-2007 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Dominic Lachowicz <cinamod@hotmail.com>
// Copyright (C) 2007-2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "parseargs.h"
#include "goo/GooString.h"
#include "goo/gmem.h"
#include "GlobalParams.h"
#include "Object.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "FontInfo.h"

static char *fontTypeNames[] = {
  "unknown",
  "Type 1",
  "Type 1C",
  "Type 1C (OT)",
  "Type 3",
  "TrueType",
  "TrueType (OT)",
  "CID Type 0",
  "CID Type 0C",
  "CID Type 0C (OT)",
  "CID TrueType",
  "CID TrueType (OT)"
};

static int firstPage = 1;
static int lastPage = 0;
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to examine"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to examine"},
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-v",      argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",  argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

int main(int argc, char *argv[]) {
  PDFDoc *doc;
  GooString *fileName;
  GooString *ownerPW, *userPW;
  GBool ok;
  int exitCode;

  exitCode = 99;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc != 2 || printVersion || printHelp) {
    fprintf(stderr, "pdffonts version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdffonts", "<PDF-file>", argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    goto err0;
  }
  fileName = new GooString(argv[1]);

  // read config file
  globalParams = new GlobalParams();

  // open PDF file
  if (ownerPassword[0] != '\001') {
    ownerPW = new GooString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0] != '\001') {
    userPW = new GooString(userPassword);
  } else {
    userPW = NULL;
  }
  if (fileName->cmp("-") == 0) {
      delete fileName;
      fileName = new GooString("fd://0");
  }

  doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);
  delete fileName;

  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }
  if (!doc->isOk()) {
    exitCode = 1;
    goto err1;
  }

  // get page range
  if (firstPage < 1) {
    firstPage = 1;
  }
  if (lastPage < 1 || lastPage > doc->getNumPages()) {
    lastPage = doc->getNumPages();
  }

  // get the fonts
  {
    FontInfoScanner scanner(doc, firstPage - 1);
    GooList *fonts = scanner.scan(lastPage - firstPage + 1);

    // print the font info
    printf("name                                 type              emb sub uni object ID\n");
    printf("------------------------------------ ----------------- --- --- --- ---------\n");
    if (fonts) {
      for (int i = 0; i < fonts->getLength(); ++i) {
        FontInfo *font = (FontInfo *)fonts->get(i);
        printf("%-36s %-17s %-3s %-3s %-3s",
              font->getName() ? font->getName()->getCString() : "[none]",
              fontTypeNames[font->getType()],
              font->getEmbedded() ? "yes" : "no",
              font->getSubset() ? "yes" : "no",
              font->getToUnicode() ? "yes" : "no");
        const Ref fontRef = font->getRef();
        if (fontRef.gen >= 100000) {
          printf(" [none]\n");
        } else {
          printf(" %6d %2d\n", fontRef.num, fontRef.gen);
        }
        delete font;
      }
      delete fonts;
    }
  }

  exitCode = 0;

 err1:
  delete doc;
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}


