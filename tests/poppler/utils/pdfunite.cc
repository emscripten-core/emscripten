//========================================================================
//
// pdfunite.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2011, 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Arseny Solokha <asolokha@gmx.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Hib Eris <hib@hiberis.nl>
//
//========================================================================
#include <PDFDoc.h>
#include <GlobalParams.h>
#include "parseargs.h"
#include "config.h"
#include <poppler-config.h>
#include <vector>

static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-v", argFlag, &printVersion, 0,
   "print copyright and version info"},
  {"-h", argFlag, &printHelp, 0,
   "print usage information"},
  {"-help", argFlag, &printHelp, 0,
   "print usage information"},
  {"--help", argFlag, &printHelp, 0,
   "print usage information"},
  {"-?", argFlag, &printHelp, 0,
   "print usage information"},
  {NULL}
};

///////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
///////////////////////////////////////////////////////////////////////////
// Merge PDF files given by arguments 1 to argc-2 and write the result
// to the file specified by argument argc-1.
///////////////////////////////////////////////////////////////////////////
{
  int objectsCount = 0;
  Guint numOffset = 0;
  std::vector<Object> pages;
  std::vector<Guint> offsets;
  XRef *yRef, *countRef;
  FILE *f;
  OutStream *outStr;
  int i;
  int j, rootNum;
  std::vector<PDFDoc *>docs;
  int majorVersion = 0;
  int minorVersion = 0;
  char *fileName = argv[argc - 1];
  int exitCode;

  exitCode = 99;
  const GBool ok = parseArgs (argDesc, &argc, argv);
  if (!ok || argc <= 3 || printVersion || printHelp) {
    fprintf(stderr, "pdfunite version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdfunite", "<PDF-sourcefile-1>..<PDF-sourcefile-n> <PDF-destfile>",
	argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    return exitCode;
  }
  exitCode = 0;
  globalParams = new GlobalParams();

  for (i = 1; i < argc - 1; i++) {
    GooString *gfileName = new GooString(argv[i]);
    PDFDoc *doc = new PDFDoc(gfileName, NULL, NULL, NULL);
    if (doc->isOk() && !doc->isEncrypted()) {
      docs.push_back(doc);
      if (doc->getPDFMajorVersion() > majorVersion) {
        majorVersion = doc->getPDFMajorVersion();
        minorVersion = doc->getPDFMinorVersion();
      } else if (doc->getPDFMajorVersion() == majorVersion) {
        if (doc->getPDFMinorVersion() > minorVersion) {
          minorVersion = doc->getPDFMinorVersion();
        }
      }
    } else if (doc->isOk()) {
      error(errUnimplemented, -1, "Could not merge encrypted files ('{0:s}')", argv[i]);
      return -1;
    } else {
      error(errSyntaxError, -1, "Could not merge damaged documents ('{0:s}')", argv[i]);
      return -1;
    }
  }

  if (!(f = fopen(fileName, "wb"))) {
    error(errIO, -1, "Could not open file '{0:s}'", fileName);
    return -1;
  }
  outStr = new FileOutStream(f, 0);

  yRef = new XRef();
  countRef = new XRef();
  yRef->add(0, 65535, 0, gFalse);
  PDFDoc::writeHeader(outStr, majorVersion, minorVersion);

  for (i = 0; i < (int) docs.size(); i++) {
    for (j = 1; j <= docs[i]->getNumPages(); j++) {
      PDFRectangle *cropBox = NULL;
      if (docs[i]->getCatalog()->getPage(j)->isCropped())
        cropBox = docs[i]->getCatalog()->getPage(j)->getCropBox();
      docs[i]->replacePageDict(j,
	    docs[i]->getCatalog()->getPage(j)->getRotate(),
	    docs[i]->getCatalog()->getPage(j)->getMediaBox(), cropBox, NULL);
      Ref *refPage = docs[i]->getCatalog()->getPageRef(j);
      Object page;
      docs[i]->getXRef()->fetch(refPage->num, refPage->gen, &page);
      pages.push_back(page);
      offsets.push_back(numOffset);
      Dict *pageDict = page.getDict();
      docs[i]->markPageObjects(pageDict, yRef, countRef, numOffset);
    }
    objectsCount += docs[i]->writePageObjects(outStr, yRef, numOffset, gTrue);
    numOffset = yRef->getNumObjects() + 1;
  }

  rootNum = yRef->getNumObjects() + 1;
  yRef->add(rootNum, 0, outStr->getPos(), gTrue);
  outStr->printf("%d 0 obj\n", rootNum);
  outStr->printf("<< /Type /Catalog /Pages %d 0 R", rootNum + 1);
  outStr->printf(">>\nendobj\n");
  objectsCount++;

  yRef->add(rootNum + 1, 0, outStr->getPos(), gTrue);
  outStr->printf("%d 0 obj\n", rootNum + 1);
  outStr->printf("<< /Type /Pages /Kids [");
  for (j = 0; j < (int) pages.size(); j++)
    outStr->printf(" %d 0 R", rootNum + j + 2);
  outStr->printf(" ] /Count %zd >>\nendobj\n", pages.size());
  objectsCount++;

  for (i = 0; i < (int) pages.size(); i++) {
    yRef->add(rootNum + i + 2, 0, outStr->getPos(), gTrue);
    outStr->printf("%d 0 obj\n", rootNum + i + 2);
    outStr->printf("<< ");
    Dict *pageDict = pages[i].getDict();
    for (j = 0; j < pageDict->getLength(); j++) {
      if (j > 0)
	outStr->printf(" ");
      const char *key = pageDict->getKey(j);
      Object value;
      pageDict->getValNF(j, &value);
      if (strcmp(key, "Parent") == 0) {
        outStr->printf("/Parent %d 0 R", rootNum + 1);
      } else {
        outStr->printf("/%s ", key);
        PDFDoc::writeObject(&value, outStr, yRef, offsets[i], NULL, cryptRC4, 0, 0, 0);
      }
      value.free();
    }
    outStr->printf(" >>\nendobj\n");
    objectsCount++;
  }
  Goffset uxrefOffset = outStr->getPos();
  Ref ref;
  ref.num = rootNum;
  ref.gen = 0;
  Dict *trailerDict = PDFDoc::createTrailerDict(objectsCount, gFalse, 0, &ref, yRef,
                                                fileName, outStr->getPos());
  PDFDoc::writeXRefTableTrailer(trailerDict, yRef, gFalse /* do not write unnecessary entries */,
                                uxrefOffset, outStr, yRef);
  delete trailerDict;

  outStr->close();
  fclose(f);
  delete yRef;
  delete countRef;
  for (j = 0; j < (int) pages.size (); j++) pages[j].free();
  for (i = 0; i < (int) docs.size (); i++) delete docs[i];
  delete globalParams;
  return exitCode;
}
