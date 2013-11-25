//========================================================================
//
// pdfdetach.cc
//
// Copyright 2010 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2013 Yury G. Kudryashov <urkud.urkud@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include "goo/gtypes.h"
#include "goo/gmem.h"
#include "goo/GooList.h"
#include "parseargs.h"
#include "Annot.h"
#include "GlobalParams.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "FileSpec.h"
#include "CharTypes.h"
#include "Catalog.h"
#include "UnicodeMap.h"
#include "PDFDocEncoding.h"
#include "Error.h"

static GBool doList = gFalse;
static int saveNum = 0;
static GBool saveAll = gFalse;
static char savePath[1024] = "";
static char textEncName[128] = "";
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static GBool printVersion = gFalse;
static GBool printHelp = gFalse;

static ArgDesc argDesc[] = {
  {"-list",   argFlag,     &doList,        0,
   "list all embedded files"},
  {"-save",   argInt,      &saveNum,       0,
   "save the specified embedded file"},
  {"-saveall", argFlag,    &saveAll,       0,
   "save all embedded files"},
  {"-o",      argString,   savePath,       sizeof(savePath),
   "file name for the saved embedded file"},
  {"-enc",    argString,   textEncName,    sizeof(textEncName),
   "output text encoding name"},
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
  GooString *fileName;
  UnicodeMap *uMap;
  GooString *ownerPW, *userPW;
  PDFDoc *doc;
  char uBuf[8];
  char path[1024];
  char *p;
  GBool ok;
  int exitCode;
  GooList *embeddedFiles = NULL;
  int nFiles, nPages, n, i, j;
  FileSpec *fileSpec;
  Page *page;
  Annots *annots;
  Annot *annot;
  GooString *s1;
  Unicode u;
  GBool isUnicode;

  exitCode = 99;

  // parse args
  ok = parseArgs(argDesc, &argc, argv);
  if ((doList ? 1 : 0) +
      ((saveNum != 0) ? 1 : 0) +
      (saveAll ? 1 : 0) != 1) {
    ok = gFalse;
  }
  if (!ok || argc != 2 || printVersion || printHelp) {
    fprintf(stderr, "pdfdetach version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdfdetach", "<PDF-file>", argDesc);
    }
    goto err0;
  }
  fileName = new GooString(argv[1]);

  // read config file
  globalParams = new GlobalParams();
  if (textEncName[0]) {
    globalParams->setTextEncoding(textEncName);
  }

  // get mapping to output encoding
  if (!(uMap = globalParams->getTextEncoding())) {
    error(errConfig, -1, "Couldn't get text encoding");
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

  embeddedFiles = new GooList();
  for (i = 0; i < doc->getCatalog()->numEmbeddedFiles(); ++i)
    embeddedFiles->append(doc->getCatalog()->embeddedFile(i));

  nPages = doc->getCatalog()->getNumPages();
  for (i = 0; i < nPages; ++i) {
    page = doc->getCatalog()->getPage(i + 1);
    annots = page->getAnnots();
    if (!annots)
      break;

    for (j = 0; j < annots->getNumAnnots(); ++j) {
      annot = annots->getAnnot(j);
      if (annot->getType() != Annot::typeFileAttachment)
        continue;
      embeddedFiles->append(new FileSpec(static_cast<AnnotFileAttachment *>(annot)->getFile()));
    }
  }

  nFiles = embeddedFiles->getLength();

  // list embedded files
  if (doList) {
    printf("%d embedded files\n", nFiles);
    for (i = 0; i < nFiles; ++i) {
      fileSpec = static_cast<FileSpec *>(embeddedFiles->get(i));
      printf("%d: ", i+1);
      s1 = fileSpec->getFileName();
      if ((s1->getChar(0) & 0xff) == 0xfe && (s1->getChar(1) & 0xff) == 0xff) {
        isUnicode = gTrue;
        j = 2;
      } else {
        isUnicode = gFalse;
        j = 0;
      }
      while (j < fileSpec->getFileName()->getLength()) {
        if (isUnicode) {
          u = ((s1->getChar(j) & 0xff) << 8) | (s1->getChar(j+1) & 0xff);
          j += 2;
        } else {
          u = pdfDocEncoding[s1->getChar(j) & 0xff];
          ++j;
        }
        n = uMap->mapUnicode(u, uBuf, sizeof(uBuf));
        fwrite(uBuf, 1, n, stdout);
      }
      fputc('\n', stdout);
    }

  // save all embedded files
  } else if (saveAll) {
    for (i = 0; i < nFiles; ++i) {
      fileSpec = static_cast<FileSpec *>(embeddedFiles->get(i));
      if (savePath[0]) {
	n = strlen(savePath);
	if (n > (int)sizeof(path) - 2) {
	  n = sizeof(path) - 2;
	}
	memcpy(path, savePath, n);
	path[n] = '/';
	p = path + n + 1;
      } else {
	p = path;
      }
      s1 = fileSpec->getFileName();
      if ((s1->getChar(0) & 0xff) == 0xfe && (s1->getChar(1) & 0xff) == 0xff) {
        isUnicode = gTrue;
        j = 2;
      } else {
        isUnicode = gFalse;
        j = 0;
      }
      while (j < fileSpec->getFileName()->getLength()) {
        if (isUnicode) {
          u = ((s1->getChar(j) & 0xff) << 8) | (s1->getChar(j+1) & 0xff);
          j += 2;
        } else {
          u = pdfDocEncoding[s1->getChar(j) & 0xff];
          ++j;
        }
        n = uMap->mapUnicode(u, uBuf, sizeof(uBuf));
        if (p + n >= path + sizeof(path))
          break;
        memcpy(p, uBuf, n);
        p += n;
      }
      *p = '\0';

      if (!fileSpec->getEmbeddedFile()->save(path)) {
	error(errIO, -1, "Error saving embedded file as '{0:s}'", p);
	exitCode = 2;
	goto err2;
      }
    }

  // save an embedded file
  } else {
    if (saveNum < 1 || saveNum > nFiles) {
      error(errCommandLine, -1, "Invalid file number");
      goto err2;
    }

    fileSpec = static_cast<FileSpec *>(embeddedFiles->get(saveNum - 1));
    if (savePath[0]) {
      p = savePath;
    } else {
      p = path;
      s1 = fileSpec->getFileName();
      if ((s1->getChar(0) & 0xff) == 0xfe && (s1->getChar(1) & 0xff) == 0xff) {
        isUnicode = gTrue;
        j = 2;
      } else {
        isUnicode = gFalse;
        j = 0;
      }
      while (j < fileSpec->getFileName()->getLength()) {
        if (isUnicode) {
          u = ((s1->getChar(j) & 0xff) << 8) | (s1->getChar(j+1) & 0xff);
          j += 2;
        } else {
          u = pdfDocEncoding[s1->getChar(j) & 0xff];
          ++j;
        }
        n = uMap->mapUnicode(u, uBuf, sizeof(uBuf));
        if (p + n >= path + sizeof(path))
          break;
        memcpy(p, uBuf, n);
        p += n;
      }
      *p = '\0';
      p = path;
    }

    if (!fileSpec->getEmbeddedFile()->save(p)) {
      error(errIO, -1, "Error saving embedded file as '{0:s}'", p);
      exitCode = 2;
      goto err2;
    }
  }

  exitCode = 0;

  // clean up
 err2:
  if (embeddedFiles)
    deleteGooList(embeddedFiles, FileSpec);
  uMap->decRefCnt();
  delete doc;
 err1:
  delete globalParams;
 err0:

  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return exitCode;
}
