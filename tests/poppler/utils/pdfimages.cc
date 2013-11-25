//========================================================================
//
// pdfimages.cc
//
// Copyright 1998-2003 Glyph & Cog, LLC
//
// Modified for Debian by Hamish Moffatt, 22 May 2002.
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007-2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Jakob Voss <jakob.voss@gbv.de>
// Copyright (C) 2012, 2013 Adrian Johnson <ajohnson@redneon.com>
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
#include "parseargs.h"
#include "goo/GooString.h"
#include "goo/gmem.h"
#include "GlobalParams.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "ImageOutputDev.h"
#include "Error.h"

static int firstPage = 1;
static int lastPage = 0;
static GBool listImages = gFalse;
static GBool enablePNG = gFalse;
static GBool enableTiff = gFalse;
static GBool dumpJPEG = gFalse;
static GBool dumpJP2 = gFalse;
static GBool dumpJBIG2 = gFalse;
static GBool dumpCCITT = gFalse;
static GBool allFormats = gFalse;
static GBool pageNames = gFalse;
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static GBool quiet = gFalse;
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to convert"},
#if ENABLE_LIBPNG
  {"-png",      argFlag,     &enablePNG,      0,
   "change the default output format to PNG"},
#endif
#if ENABLE_LIBTIFF
  {"-tiff",      argFlag,     &enableTiff,      0,
   "change the default output format to TIFF"},
#endif
  {"-j",      argFlag,     &dumpJPEG,      0,
   "write JPEG images as JPEG files"},
  {"-jp2",      argFlag,     &dumpJP2,      0,
   "write JPEG2000 images as JP2 files"},
  {"-jbig2",      argFlag,     &dumpJBIG2,      0,
   "write JBIG2 images as JBIG2 files"},
  {"-ccitt",      argFlag,     &dumpCCITT,      0,
   "write CCITT images as CCITT files"},
  {"-all",      argFlag,     &allFormats,    0,
   "equivalent to -png -tiff -j -jp2 -jbig2 -ccitt"},
  {"-list",   argFlag,     &listImages,      0,
   "print list of images instead of saving"},
  {"-opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"-upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-p",      argFlag,     &pageNames,     0,
   "include page numbers in output file names"},
  {"-q",      argFlag,     &quiet,         0,
   "don't print any messages or errors"},
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
  char *imgRoot = NULL;
  GooString *ownerPW, *userPW;
  ImageOutputDev *imgOut;
  GBool ok;
  int exitCode;

  exitCode = 99;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || (listImages && argc != 2) || (!listImages && argc != 3) || printVersion || printHelp) {
    fprintf(stderr, "pdfimages version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdfimages", "<PDF-file> <image-root>", argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    goto err0;
  }
  fileName = new GooString(argv[1]);
  if (!listImages)
    imgRoot = argv[2];

  // read config file
  globalParams = new GlobalParams();
  if (quiet) {
    globalParams->setErrQuiet(quiet);
  }

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

  // check for copy permission
#ifdef ENFORCE_PERMISSIONS
  if (!doc->okToCopy()) {
    error(errNotAllowed, -1, "Copying of images from this document is not allowed.");
    exitCode = 3;
    goto err1;
  }
#endif

  // get page range
  if (firstPage < 1)
    firstPage = 1;
  if (lastPage < 1 || lastPage > doc->getNumPages())
    lastPage = doc->getNumPages();

  // write image files
  imgOut = new ImageOutputDev(imgRoot, pageNames, listImages);
  if (imgOut->isOk()) {
    if (allFormats) {
      imgOut->enablePNG(gTrue);
      imgOut->enableTiff(gTrue);
      imgOut->enableJpeg(gTrue);
      imgOut->enableJpeg2000(gTrue);
      imgOut->enableJBig2(gTrue);
      imgOut->enableCCITT(gTrue);
    } else {
      imgOut->enablePNG(enablePNG);
      imgOut->enableTiff(enableTiff);
      imgOut->enableJpeg(dumpJPEG);
      imgOut->enableJpeg2000(dumpJP2);
      imgOut->enableJBig2(dumpJBIG2);
      imgOut->enableCCITT(dumpCCITT);
    }
    doc->displayPages(imgOut, firstPage, lastPage, 72, 72, 0,
                      gTrue, gFalse, gFalse);
  }
  delete imgOut;

  exitCode = 0;

  // clean up
 err1:
  delete doc;
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}
