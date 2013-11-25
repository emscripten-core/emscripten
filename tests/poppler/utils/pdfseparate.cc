//========================================================================
//
// pdfseparate.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2011, 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012, 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Pino Toscano <pino@kde.org>
// Copyright (C) 2013 Daniel Kahn Gillmor <dkg@fifthhorseman.net>
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
#include "PDFDoc.h"
#include "ErrorCodes.h"
#include "GlobalParams.h"
#include <ctype.h>

static int firstPage = 0;
static int lastPage = 0;
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static const ArgDesc argDesc[] = {
  {"-f", argInt, &firstPage, 0,
   "first page to extract"},
  {"-l", argInt, &lastPage, 0,
   "last page to extract"},
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

bool extractPages (const char *srcFileName, const char *destFileName) {
  char pathName[4096];
  GooString *gfileName = new GooString (srcFileName);
  PDFDoc *doc = new PDFDoc (gfileName, NULL, NULL, NULL);

  if (!doc->isOk()) {
    error(errSyntaxError, -1, "Could not extract page(s) from damaged file ('{0:s}')", srcFileName);
    return false;
  }

  // destFileName can have multiple %% and one %d
  // We use auxDestFileName to replace all the valid % appearances
  // by 'A' (random char that is not %), if at the end of replacing
  // any of the valid appearances there is still any % around, the
  // pattern is wrong
  char *auxDestFileName = strdup(destFileName);
  if (firstPage == 0 && lastPage == 0) {
    firstPage = 1;
    lastPage = doc->getNumPages();
  }
  if (lastPage == 0)
    lastPage = doc->getNumPages();
  if (firstPage == 0)
    firstPage = 1;
  bool foundmatch = false;
  char *p = strstr(auxDestFileName, "%d");
  if (p != NULL) {
    foundmatch = true;
    *p = 'A';
  } else {
    char pattern[5];
    for (int i = 2; i < 10; i++) {
      sprintf(pattern, "%%0%dd", i);
      p = strstr(auxDestFileName, pattern);
      if (p != NULL) {
       foundmatch = true;
       *p = 'A';
       break;
      }
    }
  }
  if (!foundmatch && firstPage != lastPage) {
    error(errSyntaxError, -1, "'{0:s}' must contain '%%d' if more than one page should be extracted", destFileName);
    free(auxDestFileName);
    return false;
  }

  // at this point auxDestFileName can only contain %%
  p = strstr(auxDestFileName, "%%");
  while (p != NULL) {
    *p = 'A';
    *(p + 1) = 'A';
    p = strstr(p, "%%"); 
  }

  // at this point any other % is wrong
  p = strstr(auxDestFileName, "%");
  if (p != NULL) {
    error(errSyntaxError, -1, "'{0:s}' can only contain one '%d' pattern", destFileName);
    free(auxDestFileName);
    return false;
  }
  free(auxDestFileName);
  
  for (int pageNo = firstPage; pageNo <= lastPage; pageNo++) {
    snprintf (pathName, sizeof (pathName) - 1, destFileName, pageNo);
    GooString *gpageName = new GooString (pathName);
    int errCode = doc->savePageAs(gpageName, pageNo);
    if ( errCode != errNone) {
      delete gpageName;
      delete gfileName;
      return false;
    }
    delete gpageName;
  }
  delete gfileName;
  return true;
}

int
main (int argc, char *argv[])
{
  Object info;
  GBool ok;
  int exitCode;

  exitCode = 99;

  // parse args
  ok = parseArgs (argDesc, &argc, argv);
  if (!ok || argc != 3 || printVersion || printHelp)
    {
      fprintf (stderr, "pdfseparate version %s\n", PACKAGE_VERSION);
      fprintf (stderr, "%s\n", popplerCopyright);
      fprintf (stderr, "%s\n", xpdfCopyright);
      if (!printVersion)
	{
	  printUsage ("pdfseparate", "<PDF-sourcefile> <PDF-pattern-destfile>",
		      argDesc);
	}
      if (printVersion || printHelp)
	exitCode = 0;
      goto err0;
    }
  globalParams = new GlobalParams();
  ok = extractPages (argv[1], argv[2]);
  if (ok) {
    exitCode = 0;
  }
  delete globalParams;

err0:

  return exitCode;
}
