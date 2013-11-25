//========================================================================
//
// pdfinfo.cc
//
// Copyright 1998-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Dom Lachowicz <cinamod@hotmail.com>
// Copyright (C) 2007-2010, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2011 Vittal Aithal <vittal.aithal@cognidox.com>
// Copyright (C) 2012, 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Adrian Perez de Castro <aperez@igalia.com>
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
#include <time.h>
#include <math.h>
#include "parseargs.h"
#include "printencodings.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
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
#include "CharTypes.h"
#include "UnicodeMap.h"
#include "UTF.h"
#include "Error.h"
#include "DateInfo.h"
#include "JSInfo.h"

static void printInfoString(Dict *infoDict, const char *key, const char *text,
			    UnicodeMap *uMap);
static void printInfoDate(Dict *infoDict, const char *key, const char *text);
static void printBox(const char *text, PDFRectangle *box);

static int firstPage = 1;
static int lastPage = 0;
static GBool printBoxes = gFalse;
static GBool printMetadata = gFalse;
static GBool printJS = gFalse;
static GBool rawDates = gFalse;
static char textEncName[128] = "";
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;
static GBool printEnc = gFalse;

static const ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,        0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,         0,
   "last page to convert"},
  {"-box",    argFlag,     &printBoxes,       0,
   "print the page bounding boxes"},
  {"-meta",   argFlag,     &printMetadata,    0,
   "print the document metadata (XML)"},
  {"-js",     argFlag,     &printJS,          0,
   "print all JavaScript in the PDF"},
  {"-rawdates", argFlag,   &rawDates,         0,
   "print the undecoded date strings directly from the PDF file"},
  {"-enc",    argString,   textEncName,    sizeof(textEncName),
   "output text encoding name"},
  {"-listenc",argFlag,     &printEnc,      0,
   "list available encodings"},
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
  UnicodeMap *uMap;
  Page *page;
  Object info;
  char buf[256];
  double w, h, wISO, hISO;
  FILE *f;
  GooString *metadata;
  GBool ok;
  int exitCode;
  int pg, i;
  GBool multiPage;
  int r;

  exitCode = 99;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || (argc != 2 && !printEnc) || printVersion || printHelp) {
    fprintf(stderr, "pdfinfo version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdfinfo", "<PDF-file>", argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    goto err0;
  }

  // read config file
  globalParams = new GlobalParams();

  if (printEnc) {
    printEncodings();
    delete globalParams;
    exitCode = 0;
    goto err0;
  }

  fileName = new GooString(argv[1]);

  if (textEncName[0]) {
    globalParams->setTextEncoding(textEncName);
  }

  // get mapping to output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    error(errCommandLine, -1, "Couldn't get text encoding");
    delete fileName;
    goto err1;
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

  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }
  if (!doc->isOk()) {
    exitCode = 1;
    goto err2;
  }

  // get page range
  if (firstPage < 1) {
    firstPage = 1;
  }
  if (lastPage == 0) {
    multiPage = gFalse;
    lastPage = 1;
  } else {
    multiPage = gTrue;
  }
  if (lastPage < 1 || lastPage > doc->getNumPages()) {
    lastPage = doc->getNumPages();
  }

  // print doc info
  doc->getDocInfo(&info);
  if (info.isDict()) {
    printInfoString(info.getDict(), "Title",        "Title:          ", uMap);
    printInfoString(info.getDict(), "Subject",      "Subject:        ", uMap);
    printInfoString(info.getDict(), "Keywords",     "Keywords:       ", uMap);
    printInfoString(info.getDict(), "Author",       "Author:         ", uMap);
    printInfoString(info.getDict(), "Creator",      "Creator:        ", uMap);
    printInfoString(info.getDict(), "Producer",     "Producer:       ", uMap);
    if (rawDates) {
      printInfoString(info.getDict(), "CreationDate", "CreationDate:   ",
		      uMap);
      printInfoString(info.getDict(), "ModDate",      "ModDate:        ",
		      uMap);
    } else {
      printInfoDate(info.getDict(),   "CreationDate", "CreationDate:   ");
      printInfoDate(info.getDict(),   "ModDate",      "ModDate:        ");
    }
  }
  info.free();

  // print tagging info
   printf("Tagged:         %s\n",
	  (doc->getCatalog()->getMarkInfo() & Catalog::markInfoMarked) ? "yes" : "no");
   printf("UserProperties: %s\n",
	  (doc->getCatalog()->getMarkInfo() & Catalog::markInfoUserProperties) ? "yes" : "no");
   printf("Suspects:       %s\n",
	  (doc->getCatalog()->getMarkInfo() & Catalog::markInfoSuspects) ? "yes" : "no");

  // print form info
  switch (doc->getCatalog()->getFormType())
  {
    case Catalog::NoForm:
      printf("Form:           none\n");
      break;
    case Catalog::AcroForm:
      printf("Form:           AcroForm\n");
      break;
    case Catalog::XfaForm:
      printf("Form:           XFA\n");
      break;
  }

  // print javascript info
  {
    JSInfo jsInfo(doc, firstPage - 1);
    jsInfo.scanJS(lastPage - firstPage + 1);
    printf("JavaScript:     %s\n", jsInfo.containsJS() ? "yes" : "no");
  }

  // print page count
  printf("Pages:          %d\n", doc->getNumPages());

  // print encryption info
  printf("Encrypted:      ");
  if (doc->isEncrypted()) {
    Guchar *fileKey;
    CryptAlgorithm encAlgorithm;
    int keyLength;
    doc->getXRef()->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

    const char *encAlgorithmName = "unknown";
    switch (encAlgorithm)
    {
      case cryptRC4:
	encAlgorithmName = "RC4";
	break;
      case cryptAES:
	encAlgorithmName = "AES";
	break;
      case cryptAES256:
	encAlgorithmName = "AES-256";
	break;
    }

    printf("yes (print:%s copy:%s change:%s addNotes:%s algorithm:%s)\n",
	   doc->okToPrint(gTrue) ? "yes" : "no",
	   doc->okToCopy(gTrue) ? "yes" : "no",
	   doc->okToChange(gTrue) ? "yes" : "no",
	   doc->okToAddNotes(gTrue) ? "yes" : "no",
	   encAlgorithmName);
  } else {
    printf("no\n");
  }

  // print page size
  for (pg = firstPage; pg <= lastPage; ++pg) {
    w = doc->getPageCropWidth(pg);
    h = doc->getPageCropHeight(pg);
    if (multiPage) {
      printf("Page %4d size: %g x %g pts", pg, w, h);
    } else {
      printf("Page size:      %g x %g pts", w, h);
    }
    if ((fabs(w - 612) < 0.1 && fabs(h - 792) < 0.1) ||
	(fabs(w - 792) < 0.1 && fabs(h - 612) < 0.1)) {
      printf(" (letter)");
    } else {
      hISO = sqrt(sqrt(2.0)) * 7200 / 2.54;
      wISO = hISO / sqrt(2.0);
      for (i = 0; i <= 6; ++i) {
	if ((fabs(w - wISO) < 1 && fabs(h - hISO) < 1) ||
	    (fabs(w - hISO) < 1 && fabs(h - wISO) < 1)) {
	  printf(" (A%d)", i);
	  break;
	}
	hISO = wISO;
	wISO /= sqrt(2.0);
      }
    }
    printf("\n");
    r = doc->getPageRotate(pg);
    if (multiPage) {
      printf("Page %4d rot:  %d\n", pg, r);
    } else {
      printf("Page rot:       %d\n", r);
    }
  } 

  // print the boxes
  if (printBoxes) {
    if (multiPage) {
      for (pg = firstPage; pg <= lastPage; ++pg) {
	page = doc->getPage(pg);
	if (!page) {
          error(errSyntaxError, -1, "Failed to print boxes for page {0:d}", pg);
	  continue;
	}
	sprintf(buf, "Page %4d MediaBox: ", pg);
	printBox(buf, page->getMediaBox());
	sprintf(buf, "Page %4d CropBox:  ", pg);
	printBox(buf, page->getCropBox());
	sprintf(buf, "Page %4d BleedBox: ", pg);
	printBox(buf, page->getBleedBox());
	sprintf(buf, "Page %4d TrimBox:  ", pg);
	printBox(buf, page->getTrimBox());
	sprintf(buf, "Page %4d ArtBox:   ", pg);
	printBox(buf, page->getArtBox());
      }
    } else {
      page = doc->getPage(firstPage);
      if (!page) {
        error(errSyntaxError, -1, "Failed to print boxes for page {0:d}", firstPage);
      } else {
        printBox("MediaBox:       ", page->getMediaBox());
        printBox("CropBox:        ", page->getCropBox());
        printBox("BleedBox:       ", page->getBleedBox());
        printBox("TrimBox:        ", page->getTrimBox());
        printBox("ArtBox:         ", page->getArtBox());
      }
    }
  }

  // print file size
#ifdef VMS
  f = fopen(fileName->getCString(), "rb", "ctx=stm");
#else
  f = fopen(fileName->getCString(), "rb");
#endif
  if (f) {
    Gfseek(f, 0, SEEK_END);
    printf("File size:      %lld bytes\n", (long long)Gftell(f));
    fclose(f);
  }

  // print linearization info
  printf("Optimized:      %s\n", doc->isLinearized() ? "yes" : "no");

  // print PDF version
  printf("PDF version:    %d.%d\n", doc->getPDFMajorVersion(), doc->getPDFMinorVersion());

  // print the metadata
  if (printMetadata && (metadata = doc->readMetadata())) {
    fputs("Metadata:\n", stdout);
    fputs(metadata->getCString(), stdout);
    fputc('\n', stdout);
    delete metadata;
  }

  // print javascript
  if (printJS) {
    JSInfo jsInfo(doc, firstPage - 1);
    fputs("\n", stdout);
    jsInfo.scanJS(lastPage - firstPage + 1, stdout, uMap);
  }

  exitCode = 0;

  // clean up
 err2:
  uMap->decRefCnt();
  delete doc;
  delete fileName;
 err1:
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}

static void printInfoString(Dict *infoDict, const char *key, const char *text,
			    UnicodeMap *uMap) {
  Object obj;
  GooString *s1;
  Unicode *u;
  char buf[8];
  int i, n, len;

  if (infoDict->lookup(key, &obj)->isString()) {
    fputs(text, stdout);
    s1 = obj.getString();
    len = TextStringToUCS4(s1, &u);
    for (i = 0; i < len; i++) {
      n = uMap->mapUnicode(u[i], buf, sizeof(buf));
      fwrite(buf, 1, n, stdout);
    }
    fputc('\n', stdout);
  }
  obj.free();
}

static void printInfoDate(Dict *infoDict, const char *key, const char *text) {
  Object obj;
  char *s;
  int year, mon, day, hour, min, sec, tz_hour, tz_minute;
  char tz;
  struct tm tmStruct;
  char buf[256];

  if (infoDict->lookup(key, &obj)->isString()) {
    fputs(text, stdout);
    s = obj.getString()->getCString();
    // TODO do something with the timezone info
    if ( parseDateString( s, &year, &mon, &day, &hour, &min, &sec, &tz, &tz_hour, &tz_minute ) ) {
      tmStruct.tm_year = year - 1900;
      tmStruct.tm_mon = mon - 1;
      tmStruct.tm_mday = day;
      tmStruct.tm_hour = hour;
      tmStruct.tm_min = min;
      tmStruct.tm_sec = sec;
      tmStruct.tm_wday = -1;
      tmStruct.tm_yday = -1;
      tmStruct.tm_isdst = -1;
      // compute the tm_wday and tm_yday fields
      if (mktime(&tmStruct) != (time_t)-1 &&
	  strftime(buf, sizeof(buf), "%c", &tmStruct)) {
	fputs(buf, stdout);
      } else {
	fputs(s, stdout);
      }
    } else {
      fputs(s, stdout);
    }
    fputc('\n', stdout);
  }
  obj.free();
}

static void printBox(const char *text, PDFRectangle *box) {
  printf("%s%8.2f %8.2f %8.2f %8.2f\n",
	 text, box->x1, box->y1, box->x2, box->y2);
}
