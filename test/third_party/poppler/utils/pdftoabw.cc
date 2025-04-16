/*
 * Copyright (C) 2007 Jauco Noordzij <jauco@jauco.nl>
 * Copyright (C) 2007 Dominic Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2007 Kouhei Sutou <kou@cozmixng.org>
 * Copyright (C) 2009 Jakub Wilk <ubanus@users.sf.net>
 * Copyright (C) 2009, 2010 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <time.h>
#include "parseargs.h"
#include "goo/GooString.h"
#include "goo/gmem.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "ABWOutputDev.h"
#include "PSOutputDev.h"
#include "GlobalParams.h"
#include "Error.h"
#include "goo/gfile.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

static int firstPage = 1;
static int lastPage = 0;
static GBool quiet = gFalse;
static GBool printHelp = gFalse;
static GBool printVersion = gFalse;
static GBool stout = gFalse;
static char ownerPassword[33] = "";
static char userPassword[33] = "";

// static char textEncName[128] = "";

static const ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to convert"},
  {"--stdout"  ,argFlag,    &stout,         0,
   "use standard output"},
  {"--opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"--upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {"-q",       argFlag,     &quiet,         0,
   "don't print any messages or errors"},
  {"-v",       argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",       argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",    argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",       argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};

int main(int argc, char *argv[]) {
  GBool ok;
  PDFDoc *doc = NULL;
  GooString *fileName = NULL;
//  GooString *abwFileName = NULL;
  ABWOutputDev *abwOut = NULL;
//  GBool ok;
  GooString *ownerPW, *userPW;
  Object info;

  int result = 1;
  
  char * outpName;
  xmlDocPtr XMLdoc = NULL;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if (!ok || argc < 2 || argc > 3 || printVersion || printHelp) {
    fprintf(stderr, "pdftoabw version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdftoabw", "<PDF-file> [abw-file]", argDesc);
    }
    goto err0;
  }
  globalParams = new GlobalParams();
  if (quiet) {
    globalParams->setErrQuiet(quiet);
  }

  fileName = new GooString(argv[1]);
  if (stout || (argc < 3)){
    outpName = "-";
  }
  else {
    outpName = argv[2];
  }

  if (ownerPassword[0]) {
    ownerPW = new GooString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0]) {
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

  if (!doc || !doc->isOk())
    {
      fprintf (stderr, "Error opening PDF %s\n", fileName->getCString());
      goto error;
    }

  // check for copy permission
  if (!doc->okToCopy()) {
    fprintf(stderr, "Copying of text from this document is not allowed.\n");
    goto error;
  }

  XMLdoc = xmlNewDoc(BAD_CAST "1.0");
  abwOut = new ABWOutputDev(XMLdoc);
  abwOut->setPDFDoc(doc);

  if (lastPage == 0 || lastPage > doc->getNumPages ()) lastPage = doc->getNumPages();
  if (firstPage < 1) firstPage = 1;

  if (abwOut->isOk())
  {
    doc->displayPages(abwOut, firstPage, lastPage, 72, 72, 0, gTrue, gFalse, gFalse);
    abwOut->createABW();
  }

  if (xmlSaveFormatFileEnc(outpName, XMLdoc, "UTF-8", 1) == -1)
    {
      fprintf (stderr, "Error saving to %s\n", outpName);
      goto error;
    }

  result = 0;

 error:
  // clean up
  if(globalParams) delete globalParams;
  if(doc) delete doc;
  delete fileName;
  if(XMLdoc) xmlFreeDoc(XMLdoc);
  if(abwOut) delete abwOut;
 err0:
  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return result;
}
