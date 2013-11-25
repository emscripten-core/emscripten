//========================================================================
//
// PDFDoc.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2005, 2007-2009, 2011-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008, 2010 Pino Toscano <pino@kde.org>
// Copyright (C) 2008, 2010, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Eric Toombs <ewtoombs@uwaterloo.ca>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2009, 2011 Axel Struebing <axel.struebing@freenet.de>
// Copyright (C) 2010-2012 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Jakub Wilk <ubanus@users.sf.net>
// Copyright (C) 2010 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2010 Srinivas Adicherla <srinivas.adicherla@geodesic.com>
// Copyright (C) 2010 Philip Lorenz <lorenzph+freedesktop@gmail.com>
// Copyright (C) 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Adam Reichold <adamreichold@myopera.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#  include <windows.h>
#endif
#include <sys/stat.h>
#include "goo/gstrtod.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "poppler-config.h"
#include "GlobalParams.h"
#include "Page.h"
#include "Catalog.h"
#include "Stream.h"
#include "XRef.h"
#include "Linearization.h"
#include "Link.h"
#include "OutputDev.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "Lexer.h"
#include "Parser.h"
#include "SecurityHandler.h"
#include "Decrypt.h"
#ifndef DISABLE_OUTLINE
#include "Outline.h"
#endif
#include "PDFDoc.h"
#include "Hints.h"

#if MULTITHREADED
#  define pdfdocLocker()   MutexLocker locker(&mutex)
#else
#  define pdfdocLocker()
#endif

//------------------------------------------------------------------------

#define headerSearchSize 1024	// read this many bytes at beginning of
				//   file to look for '%PDF'
#define pdfIdLength 32   // PDF Document IDs (PermanentId, UpdateId) length

#define linearizationSearchSize 1024	// read this many bytes at beginning of
					// file to look for linearization
					// dictionary

#define xrefSearchSize 1024	// read this many bytes at end of file
				//   to look for 'startxref'

//------------------------------------------------------------------------
// PDFDoc
//------------------------------------------------------------------------

void PDFDoc::init()
{
#if MULTITHREADED
  gInitMutex(&mutex);
#endif
  ok = gFalse;
  errCode = errNone;
  fileName = NULL;
  file = NULL;
  str = NULL;
  xref = NULL;
  linearization = NULL;
  catalog = NULL;
  hints = NULL;
#ifndef DISABLE_OUTLINE
  outline = NULL;
#endif
  startXRefPos = -1;
  secHdlr = NULL;
  pageCache = NULL;
}

PDFDoc::PDFDoc()
{
  init();
}

PDFDoc::PDFDoc(GooString *fileNameA, GooString *ownerPassword,
	       GooString *userPassword, void *guiDataA) {
  Object obj;
#ifdef _WIN32
  int n, i;
#endif

  init();

  fileName = fileNameA;
  guiData = guiDataA;
#ifdef _WIN32
  n = fileName->getLength();
  fileNameU = (wchar_t *)gmallocn(n + 1, sizeof(wchar_t));
  for (i = 0; i < n; ++i) {
    fileNameU[i] = (wchar_t)(fileName->getChar(i) & 0xff);
  }
  fileNameU[n] = L'\0';
#endif

  // try to open file
  file = GooFile::open(fileName);
  if (file == NULL) {
    // fopen() has failed.
    // Keep a copy of the errno returned by fopen so that it can be 
    // referred to later.
    fopenErrno = errno;
    error(errIO, -1, "Couldn't open file '{0:t}': {1:s}.", fileName, strerror(errno));
    errCode = errOpenFile;
    return;
  }

  // create stream
  obj.initNull();
  str = new FileStream(file, 0, gFalse, file->size(), &obj);

  ok = setup(ownerPassword, userPassword);
}

#ifdef _WIN32
PDFDoc::PDFDoc(wchar_t *fileNameA, int fileNameLen, GooString *ownerPassword,
	       GooString *userPassword, void *guiDataA) {
  OSVERSIONINFO version;
  Object obj;
  int i;

  init();

  guiData = guiDataA;

  // save both Unicode and 8-bit copies of the file name
  fileName = new GooString();
  fileNameU = (wchar_t *)gmallocn(fileNameLen + 1, sizeof(wchar_t));
  for (i = 0; i < fileNameLen; ++i) {
    fileName->append((char)fileNameA[i]);
    fileNameU[i] = fileNameA[i];
  }
  fileNameU[fileNameLen] = L'\0';
  
  // try to open file
  // NB: _wfopen is only available in NT
  version.dwOSVersionInfoSize = sizeof(version);
  GetVersionEx(&version);
  if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
    file = GooFile::open(fileNameU);
  } else {
    file = GooFile::open(fileName);
  }
  if (!file) {
    error(errIO, -1, "Couldn't open file '{0:t}'", fileName);
    errCode = errOpenFile;
    return;
  }

  // create stream
  obj.initNull();
  str = new FileStream(file, 0, gFalse, file->size(), &obj);

  ok = setup(ownerPassword, userPassword);
}
#endif

PDFDoc::PDFDoc(BaseStream *strA, GooString *ownerPassword,
	       GooString *userPassword, void *guiDataA) {
#ifdef _WIN32
  int n, i;
#endif

  init();
  guiData = guiDataA;
  if (strA->getFileName()) {
    fileName = strA->getFileName()->copy();
#ifdef _WIN32
    n = fileName->getLength();
    fileNameU = (wchar_t *)gmallocn(n + 1, sizeof(wchar_t));
    for (i = 0; i < n; ++i) {
      fileNameU[i] = (wchar_t)(fileName->getChar(i) & 0xff);
    }
    fileNameU[n] = L'\0';
#endif
  } else {
    fileName = NULL;
#ifdef _WIN32
    fileNameU = NULL;
#endif
  }
  str = strA;
  ok = setup(ownerPassword, userPassword);
}

GBool PDFDoc::setup(GooString *ownerPassword, GooString *userPassword) {
  pdfdocLocker();
  str->setPos(0, -1);
  if (str->getPos() < 0)
  {
    error(errSyntaxError, -1, "Document base stream is not seekable");
    return gFalse;
  }

  str->reset();

  // check footer
  // Adobe does not seem to enforce %%EOF, so we do the same
//  if (!checkFooter()) return gFalse;
  
  // check header
  checkHeader();

  GBool wasReconstructed = false;

  // read xref table
  xref = new XRef(str, getStartXRef(), getMainXRefEntriesOffset(), &wasReconstructed);
  if (!xref->isOk()) {
    error(errSyntaxError, -1, "Couldn't read xref table");
    errCode = xref->getErrorCode();
    return gFalse;
  }

  // check for encryption
  if (!checkEncryption(ownerPassword, userPassword)) {
    errCode = errEncrypted;
    return gFalse;
  }

  // read catalog
  catalog = new Catalog(this);
  if (catalog && !catalog->isOk()) {
    if (!wasReconstructed)
    {
      // try one more time to contruct the Catalog, maybe the problem is damaged XRef 
      delete catalog;
      delete xref;
      xref = new XRef(str, 0, 0, NULL, true);
      catalog = new Catalog(this);
    }

    if (catalog && !catalog->isOk()) {
      error(errSyntaxError, -1, "Couldn't read page catalog");
      errCode = errBadCatalog;
      return gFalse;
    }
  }

  // done
  return gTrue;
}

PDFDoc::~PDFDoc() {
  if (pageCache) {
    for (int i = 0; i < getNumPages(); i++) {
      if (pageCache[i]) {
        delete pageCache[i];
      }
    }
    gfree(pageCache);
  }
  delete secHdlr;
#ifndef DISABLE_OUTLINE
  if (outline) {
    delete outline;
  }
#endif
  if (catalog) {
    delete catalog;
  }
  if (xref) {
    delete xref;
  }
  if (hints) {
    delete hints;
  }
  if (linearization) {
    delete linearization;
  }
  if (str) {
    delete str;
  }
  if (file) {
    delete file;
  }
  if (fileName) {
    delete fileName;
  }
#ifdef _WIN32
  if (fileNameU) {
    gfree(fileNameU);
  }
#endif
#if MULTITHREADED
  gDestroyMutex(&mutex);
#endif
}


// Check for a %%EOF at the end of this stream
GBool PDFDoc::checkFooter() {
  // we look in the last 1024 chars because Adobe does the same
  char *eof = new char[1025];
  Goffset pos = str->getPos();
  str->setPos(1024, -1);
  int i, ch;
  for (i = 0; i < 1024; i++)
  {
    ch = str->getChar();
    if (ch == EOF)
      break;
    eof[i] = ch;
  }
  eof[i] = '\0';

  bool found = false;
  for (i = i - 5; i >= 0; i--) {
    if (strncmp (&eof[i], "%%EOF", 5) == 0) {
      found = true;
      break;
    }
  }
  if (!found)
  {
    error(errSyntaxError, -1, "Document has not the mandatory ending %%EOF");
    errCode = errDamaged;
    delete[] eof;
    return gFalse;
  }
  delete[] eof;
  str->setPos(pos);
  return gTrue;
}
  
// Check for a PDF header on this stream.  Skip past some garbage
// if necessary.
void PDFDoc::checkHeader() {
  char hdrBuf[headerSearchSize+1];
  char *p;
  char *tokptr;
  int i;

  pdfMajorVersion = 0;
  pdfMinorVersion = 0;
  for (i = 0; i < headerSearchSize; ++i) {
    hdrBuf[i] = str->getChar();
  }
  hdrBuf[headerSearchSize] = '\0';
  for (i = 0; i < headerSearchSize - 5; ++i) {
    if (!strncmp(&hdrBuf[i], "%PDF-", 5)) {
      break;
    }
  }
  if (i >= headerSearchSize - 5) {
    error(errSyntaxWarning, -1, "May not be a PDF file (continuing anyway)");
    return;
  }
  str->moveStart(i);
  if (!(p = strtok_r(&hdrBuf[i+5], " \t\n\r", &tokptr))) {
    error(errSyntaxWarning, -1, "May not be a PDF file (continuing anyway)");
    return;
  }
  sscanf(p, "%d.%d", &pdfMajorVersion, &pdfMinorVersion);
  // We don't do the version check. Don't add it back in.
}

GBool PDFDoc::checkEncryption(GooString *ownerPassword, GooString *userPassword) {
  Object encrypt;
  GBool encrypted;
  GBool ret;

  xref->getTrailerDict()->dictLookup("Encrypt", &encrypt);
  if ((encrypted = encrypt.isDict())) {
    if ((secHdlr = SecurityHandler::make(this, &encrypt))) {
      if (secHdlr->isUnencrypted()) {
	// no encryption
	ret = gTrue;
      } else if (secHdlr->checkEncryption(ownerPassword, userPassword)) {
	// authorization succeeded
       	xref->setEncryption(secHdlr->getPermissionFlags(),
			    secHdlr->getOwnerPasswordOk(),
			    secHdlr->getFileKey(),
			    secHdlr->getFileKeyLength(),
			    secHdlr->getEncVersion(),
			    secHdlr->getEncRevision(),
			    secHdlr->getEncAlgorithm());
	ret = gTrue;
      } else {
	// authorization failed
	ret = gFalse;
      }
    } else {
      // couldn't find the matching security handler
      ret = gFalse;
    }
  } else {
    // document is not encrypted
    ret = gTrue;
  }
  encrypt.free();
  return ret;
}

void PDFDoc::displayPage(OutputDev *out, int page,
			 double hDPI, double vDPI, int rotate,
			 GBool useMediaBox, GBool crop, GBool printing,
			 GBool (*abortCheckCbk)(void *data),
			 void *abortCheckCbkData,
                         GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                         void *annotDisplayDecideCbkData, GBool copyXRef) {
  if (globalParams->getPrintCommands()) {
    printf("***** page %d *****\n", page);
  }

  if (getPage(page))
    getPage(page)->display(out, hDPI, vDPI,
				    rotate, useMediaBox, crop, printing,
				    abortCheckCbk, abortCheckCbkData,
				    annotDisplayDecideCbk, annotDisplayDecideCbkData, copyXRef);

}

void PDFDoc::displayPages(OutputDev *out, int firstPage, int lastPage,
			  double hDPI, double vDPI, int rotate,
			  GBool useMediaBox, GBool crop, GBool printing,
			  GBool (*abortCheckCbk)(void *data),
			  void *abortCheckCbkData,
                          GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                          void *annotDisplayDecideCbkData) {
  int page;

  for (page = firstPage; page <= lastPage; ++page) {
    displayPage(out, page, hDPI, vDPI, rotate, useMediaBox, crop, printing,
		abortCheckCbk, abortCheckCbkData,
                annotDisplayDecideCbk, annotDisplayDecideCbkData);
  }
}

void PDFDoc::displayPageSlice(OutputDev *out, int page,
			      double hDPI, double vDPI, int rotate,
			      GBool useMediaBox, GBool crop, GBool printing,
			      int sliceX, int sliceY, int sliceW, int sliceH,
			      GBool (*abortCheckCbk)(void *data),
			      void *abortCheckCbkData,
                              GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                              void *annotDisplayDecideCbkData, GBool copyXRef) {
  if (getPage(page))
    getPage(page)->displaySlice(out, hDPI, vDPI,
					 rotate, useMediaBox, crop,
					 sliceX, sliceY, sliceW, sliceH,
					 printing,
					 abortCheckCbk, abortCheckCbkData,
					 annotDisplayDecideCbk, annotDisplayDecideCbkData, copyXRef);
}

Links *PDFDoc::getLinks(int page) {
  Page *p = getPage(page);
  if (!p) {
    return new Links (NULL);
  }
  return p->getLinks();
}

void PDFDoc::processLinks(OutputDev *out, int page) {
  if (getPage(page))
    getPage(page)->processLinks(out);
}

Linearization *PDFDoc::getLinearization()
{
  if (!linearization) {
    linearization = new Linearization(str);
  }
  return linearization;
}

GBool PDFDoc::isLinearized() {
  if ((str->getLength()) &&
      (getLinearization()->getLength() == str->getLength()))
    return gTrue;
  else
    return gFalse;
}

static GBool
get_id (GooString *encodedidstring, GooString *id) {
  const char *encodedid = encodedidstring->getCString();
  char pdfid[pdfIdLength + 1];
  int n;

  if (encodedidstring->getLength() != pdfIdLength / 2)
    return gFalse;

  n = sprintf(pdfid, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	      encodedid[0] & 0xff, encodedid[1] & 0xff, encodedid[2] & 0xff, encodedid[3] & 0xff,
	      encodedid[4] & 0xff, encodedid[5] & 0xff, encodedid[6] & 0xff, encodedid[7] & 0xff,
	      encodedid[8] & 0xff, encodedid[9] & 0xff, encodedid[10] & 0xff, encodedid[11] & 0xff,
	      encodedid[12] & 0xff, encodedid[13] & 0xff, encodedid[14] & 0xff, encodedid[15] & 0xff);
  if (n != pdfIdLength)
    return gFalse;

  id->Set(pdfid, pdfIdLength);
  return gTrue;
}

GBool PDFDoc::getID(GooString *permanent_id, GooString *update_id) {
  Object obj;
  xref->getTrailerDict()->dictLookup ("ID", &obj);

  if (obj.isArray() && obj.arrayGetLength() == 2) {
    Object obj2;

    if (permanent_id) {
      if (obj.arrayGet(0, &obj2)->isString()) {
        if (!get_id (obj2.getString(), permanent_id)) {
	  obj2.free();
	  return gFalse;
	}
      } else {
        error(errSyntaxError, -1, "Invalid permanent ID");
	obj2.free();
	return gFalse;
      }
      obj2.free();
    }

    if (update_id) {
      if (obj.arrayGet(1, &obj2)->isString()) {
        if (!get_id (obj2.getString(), update_id)) {
	  obj2.free();
	  return gFalse;
	}
      } else {
        error(errSyntaxError, -1, "Invalid update ID");
	obj2.free();
	return gFalse;
      }
      obj2.free();
    }

    obj.free();

    return gTrue;
  }
  obj.free();

  return gFalse;
}

Hints *PDFDoc::getHints()
{
  if (!hints && isLinearized()) {
    hints = new Hints(str, getLinearization(), getXRef(), secHdlr);
  }

  return hints;
}

int PDFDoc::savePageAs(GooString *name, int pageNo) 
{
  FILE *f;
  OutStream *outStr;
  XRef *yRef, *countRef;
  int rootNum = getXRef()->getNumObjects() + 1;

  // Make sure that special flags are set, because we are going to read
  // all objects, including Unencrypted ones.
  xref->scanSpecialFlags();

  Guchar *fileKey;
  CryptAlgorithm encAlgorithm;
  int keyLength;
  xref->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

  if (pageNo < 1 || pageNo > getNumPages()) {
    error(errInternal, -1, "Illegal pageNo: {0:d}({1:d})", pageNo, getNumPages() );
    return errOpenFile;
  }
  PDFRectangle *cropBox = NULL;
  if (getCatalog()->getPage(pageNo)->isCropped()) {
    cropBox = getCatalog()->getPage(pageNo)->getCropBox();
  }
  replacePageDict(pageNo, 
    getCatalog()->getPage(pageNo)->getRotate(),
    getCatalog()->getPage(pageNo)->getMediaBox(),
    cropBox, NULL);
  Ref *refPage = getCatalog()->getPageRef(pageNo);
  Object page;
  getXRef()->fetch(refPage->num, refPage->gen, &page);

  if (!(f = fopen(name->getCString(), "wb"))) {
    error(errIO, -1, "Couldn't open file '{0:t}'", name);
    return errOpenFile;
  }
  outStr = new FileOutStream(f,0);

  yRef = new XRef(getXRef()->getTrailerDict());
  Object encrypt;
  getXRef()->getTrailerDict()->dictLookup("Encrypt", &encrypt);

  if (secHdlr != NULL && !secHdlr->isUnencrypted()) {
    yRef->setEncryption(secHdlr->getPermissionFlags(), 
      secHdlr->getOwnerPasswordOk(), fileKey, keyLength, secHdlr->getEncVersion(), secHdlr->getEncRevision(), encAlgorithm);
  }
  countRef = new XRef();
  Object *trailerObj = getXRef()->getTrailerDict();
  if (trailerObj->isDict()) {
    markPageObjects(trailerObj->getDict(), yRef, countRef, 0);
  }
  yRef->add(0, 65535, 0, gFalse);
  writeHeader(outStr, getPDFMajorVersion(), getPDFMinorVersion());

  // get and mark info dict
  Object infoObj;
  getXRef()->getDocInfo(&infoObj);
  if (infoObj.isDict()) {
    Dict *infoDict = infoObj.getDict();
    markPageObjects(infoDict, yRef, countRef, 0);
    if (trailerObj->isDict()) {
      Dict *trailerDict = trailerObj->getDict();
      Object ref;
      trailerDict->lookupNF("Info", &ref);
      if (ref.isRef()) {
        yRef->add(ref.getRef().num, ref.getRef().gen, 0, gTrue);
        if (getXRef()->getEntry(ref.getRef().num)->type == xrefEntryCompressed) {
          yRef->getEntry(ref.getRef().num)->type = xrefEntryCompressed;
        }
      }
      ref.free();
    }
  }
  infoObj.free();
  
  // get and mark output intents etc.
  Object catObj, pagesObj, resourcesObj;
  getXRef()->getCatalog(&catObj);
  Dict *catDict = catObj.getDict();
  catDict->lookup("Pages", &pagesObj);
  Dict *pagesDict = pagesObj.getDict();
  pagesDict->lookup("Resources", &resourcesObj);
  if (resourcesObj.isDict())
    markPageObjects(resourcesObj.getDict(), yRef, countRef, 0);
  markPageObjects(catDict, yRef, countRef, 0);

  Dict *pageDict = page.getDict();
  markPageObjects(pageDict, yRef, countRef, 0);
  yRef->markUnencrypted();
  Guint objectsCount = writePageObjects(outStr, yRef, 0);

  yRef->add(rootNum,0,outStr->getPos(),gTrue);
  outStr->printf("%d 0 obj\n", rootNum);
  outStr->printf("<< /Type /Catalog /Pages %d 0 R", rootNum + 1); 
  for (int j = 0; j < catDict->getLength(); j++) {
    const char *key = catDict->getKey(j);
    if (strcmp(key, "Type") != 0 &&
      strcmp(key, "Catalog") != 0 &&
      strcmp(key, "Pages") != 0) 
    {
      if (j > 0) outStr->printf(" ");
      Object value; catDict->getValNF(j, &value);
      outStr->printf("/%s ", key);
      writeObject(&value, outStr, getXRef(), 0, NULL, cryptRC4, 0, 0, 0);
      value.free();
    }
  }
  catObj.free();
  pagesObj.free();
  outStr->printf(">>\nendobj\n");
  objectsCount++;

  yRef->add(rootNum + 1,0,outStr->getPos(),gTrue);
  outStr->printf("%d 0 obj\n", rootNum + 1);
  outStr->printf("<< /Type /Pages /Kids [ %d 0 R ] /Count 1 ", rootNum + 2);
  if (resourcesObj.isDict()) {
    outStr->printf("/Resources ");
    writeObject(&resourcesObj, outStr, getXRef(), 0, NULL, cryptRC4, 0, 0, 0);
    resourcesObj.free();
  }
  outStr->printf(">>\n");
  outStr->printf("endobj\n");
  objectsCount++;

  yRef->add(rootNum + 2,0,outStr->getPos(),gTrue);
  outStr->printf("%d 0 obj\n", rootNum + 2);
  outStr->printf("<< ");
  for (int n = 0; n < pageDict->getLength(); n++) {
    if (n > 0) outStr->printf(" ");
    const char *key = pageDict->getKey(n);
    Object value; pageDict->getValNF(n, &value);
    if (strcmp(key, "Parent") == 0) {
      outStr->printf("/Parent %d 0 R", rootNum + 1);
    } else {
      outStr->printf("/%s ", key);
      writeObject(&value, outStr, getXRef(), 0, NULL, cryptRC4, 0, 0, 0);
    }
    value.free();
  }
  outStr->printf(" >>\nendobj\n");
  objectsCount++;
  page.free();

  Goffset uxrefOffset = outStr->getPos();
  Ref ref;
  ref.num = rootNum;
  ref.gen = 0;
  Dict *trailerDict = createTrailerDict(objectsCount, gFalse, 0, &ref, getXRef(),
                                        name->getCString(), uxrefOffset);
  writeXRefTableTrailer(trailerDict, yRef, gFalse /* do not write unnecessary entries */,
                        uxrefOffset, outStr, getXRef());
  delete trailerDict;

  outStr->close();
  fclose(f);
  delete yRef;
  delete countRef;

  return errNone;
}

int PDFDoc::saveAs(GooString *name, PDFWriteMode mode) {
  FILE *f;
  OutStream *outStr;
  int res;

  if (!(f = fopen(name->getCString(), "wb"))) {
    error(errIO, -1, "Couldn't open file '{0:t}'", name);
    return errOpenFile;
  }
  outStr = new FileOutStream(f,0);
  res = saveAs(outStr, mode);
  delete outStr;
  fclose(f);
  return res;
}

int PDFDoc::saveAs(OutStream *outStr, PDFWriteMode mode) {

  // find if we have updated objects
  GBool updated = gFalse;
  for(int i=0; i<xref->getNumObjects(); i++) {
    if (xref->getEntry(i)->getFlag(XRefEntry::Updated)) {
      updated = gTrue;
      break;
    }
  }

  if (!updated && mode == writeStandard) {
    // simply copy the original file
    saveWithoutChangesAs (outStr);
  } else if (mode == writeForceRewrite) {
    saveCompleteRewrite(outStr);
  } else {
    saveIncrementalUpdate(outStr);
  }

  return errNone;
}

int PDFDoc::saveWithoutChangesAs(GooString *name) {
  FILE *f;
  OutStream *outStr;
  int res;

  if (!(f = fopen(name->getCString(), "wb"))) {
    error(errIO, -1, "Couldn't open file '{0:t}'", name);
    return errOpenFile;
  }
  
  outStr = new FileOutStream(f,0);
  res = saveWithoutChangesAs(outStr);
  delete outStr;

  fclose(f);

  return res;
}

int PDFDoc::saveWithoutChangesAs(OutStream *outStr) {
  int c;
  
  BaseStream *copyStr = str->copy();
  copyStr->reset();
  while ((c = copyStr->getChar()) != EOF) {
    outStr->put(c);
  }
  copyStr->close();
  delete copyStr;

  return errNone;
}

void PDFDoc::saveIncrementalUpdate (OutStream* outStr)
{
  XRef *uxref;
  int c;
  //copy the original file
  BaseStream *copyStr = str->copy();
  copyStr->reset();
  while ((c = copyStr->getChar()) != EOF) {
    outStr->put(c);
  }
  copyStr->close();
  delete copyStr;

  Guchar *fileKey;
  CryptAlgorithm encAlgorithm;
  int keyLength;
  xref->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

  uxref = new XRef();
  uxref->add(0, 65535, 0, gFalse);
  xref->lock();
  for(int i=0; i<xref->getNumObjects(); i++) {
    if ((xref->getEntry(i)->type == xrefEntryFree) && 
        (xref->getEntry(i)->gen == 0)) //we skip the irrelevant free objects
      continue;

    if (xref->getEntry(i)->getFlag(XRefEntry::Updated)) { //we have an updated object
      Ref ref;
      ref.num = i;
      ref.gen = xref->getEntry(i)->type == xrefEntryCompressed ? 0 : xref->getEntry(i)->gen;
      if (xref->getEntry(i)->type != xrefEntryFree) {
        Object obj1;
        xref->fetch(ref.num, ref.gen, &obj1, 1);
        Goffset offset = writeObjectHeader(&ref, outStr);
        writeObject(&obj1, outStr, fileKey, encAlgorithm, keyLength, ref.num, ref.gen);
        writeObjectFooter(outStr);
        uxref->add(ref.num, ref.gen, offset, gTrue);
        obj1.free();
      } else {
        uxref->add(ref.num, ref.gen, 0, gFalse);
      }
    }
  }
  xref->unlock();
  if (uxref->getNumObjects() == 0) { //we have nothing to update
    delete uxref;
    return;
  }

  Goffset uxrefOffset = outStr->getPos();
  int numobjects = xref->getNumObjects();
  const char *fileNameA = fileName ? fileName->getCString() : NULL;
  Ref rootRef, uxrefStreamRef;
  rootRef.num = getXRef()->getRootNum();
  rootRef.gen = getXRef()->getRootGen();

  // Output a xref stream if there is a xref stream already
  GBool xRefStream = xref->isXRefStream();

  if (xRefStream) {
    // Append an entry for the xref stream itself
    uxrefStreamRef.num = numobjects++;
    uxrefStreamRef.gen = 0;
    uxref->add(uxrefStreamRef.num, uxrefStreamRef.gen, uxrefOffset, gTrue);
  }

  Dict *trailerDict = createTrailerDict(numobjects, gTrue, getStartXRef(), &rootRef, getXRef(), fileNameA, uxrefOffset);
  if (xRefStream) {
    writeXRefStreamTrailer(trailerDict, uxref, &uxrefStreamRef, uxrefOffset, outStr, getXRef());
  } else {
    writeXRefTableTrailer(trailerDict, uxref, gFalse, uxrefOffset, outStr, getXRef());
  }

  delete trailerDict;
  delete uxref;
}

void PDFDoc::saveCompleteRewrite (OutStream* outStr)
{
  // Make sure that special flags are set, because we are going to read
  // all objects, including Unencrypted ones.
  xref->scanSpecialFlags();

  Guchar *fileKey;
  CryptAlgorithm encAlgorithm;
  int keyLength;
  xref->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

  outStr->printf("%%PDF-%d.%d\r\n",pdfMajorVersion,pdfMinorVersion);
  XRef *uxref = new XRef();
  uxref->add(0, 65535, 0, gFalse);
  xref->lock();
  for(int i=0; i<xref->getNumObjects(); i++) {
    Object obj1;
    Ref ref;
    XRefEntryType type = xref->getEntry(i)->type;
    if (type == xrefEntryFree) {
      ref.num = i;
      ref.gen = xref->getEntry(i)->gen;
      /* the XRef class adds a lot of irrelevant free entries, we only want the significant one
          and we don't want the one with num=0 because it has already been added (gen = 65535)*/
      if (ref.gen > 0 && ref.num > 0)
        uxref->add(ref.num, ref.gen, 0, gFalse);
    } else if (xref->getEntry(i)->getFlag(XRefEntry::DontRewrite)) {
      // This entry must not be written, put a free entry instead (with incremented gen)
      ref.num = i;
      ref.gen = xref->getEntry(i)->gen + 1;
      uxref->add(ref.num, ref.gen, 0, gFalse);
    } else if (type == xrefEntryUncompressed){ 
      ref.num = i;
      ref.gen = xref->getEntry(i)->gen;
      xref->fetch(ref.num, ref.gen, &obj1, 1);
      Goffset offset = writeObjectHeader(&ref, outStr);
      // Write unencrypted objects in unencrypted form
      if (xref->getEntry(i)->getFlag(XRefEntry::Unencrypted)) {
        writeObject(&obj1, outStr, NULL, cryptRC4, 0, 0, 0);
      } else {
        writeObject(&obj1, outStr, fileKey, encAlgorithm, keyLength, ref.num, ref.gen);
      }
      writeObjectFooter(outStr);
      uxref->add(ref.num, ref.gen, offset, gTrue);
      obj1.free();
    } else if (type == xrefEntryCompressed) {
      ref.num = i;
      ref.gen = 0; //compressed entries have gen == 0
      xref->fetch(ref.num, ref.gen, &obj1, 1);
      Goffset offset = writeObjectHeader(&ref, outStr);
      writeObject(&obj1, outStr, fileKey, encAlgorithm, keyLength, ref.num, ref.gen);
      writeObjectFooter(outStr);
      uxref->add(ref.num, ref.gen, offset, gTrue);
      obj1.free();
    }
  }
  xref->unlock();
  Goffset uxrefOffset = outStr->getPos();
  writeXRefTableTrailer(uxrefOffset, uxref, gTrue /* write all entries */,
                        uxref->getNumObjects(), outStr, gFalse /* complete rewrite */);
  delete uxref;
}

void PDFDoc::writeDictionnary (Dict* dict, OutStream* outStr, XRef *xRef, Guint numOffset, Guchar *fileKey,
                               CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen)
{
  Object obj1;
  outStr->printf("<<");
  for (int i=0; i<dict->getLength(); i++) {
    GooString keyName(dict->getKey(i));
    GooString *keyNameToPrint = keyName.sanitizedName(gFalse /* non ps mode */);
    outStr->printf("/%s ", keyNameToPrint->getCString());
    delete keyNameToPrint;
    writeObject(dict->getValNF(i, &obj1), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, objNum, objGen);
    obj1.free();
  }
  outStr->printf(">> ");
}

void PDFDoc::writeStream (Stream* str, OutStream* outStr)
{
  outStr->printf("stream\r\n");
  str->reset();
  for (int c=str->getChar(); c!= EOF; c=str->getChar()) {
    outStr->printf("%c", c);  
  }
  outStr->printf("\r\nendstream\r\n");
}

void PDFDoc::writeRawStream (Stream* str, OutStream* outStr)
{
  Object obj1;
  str->getDict()->lookup("Length", &obj1);
  if (!obj1.isInt() && !obj1.isInt64()) {
    error (errSyntaxError, -1, "PDFDoc::writeRawStream, no Length in stream dict");
    return;
  }

  Goffset length;
  if (obj1.isInt())
    length = obj1.getInt();
  else
    length = obj1.getInt64();
  obj1.free();

  outStr->printf("stream\r\n");
  str->unfilteredReset();
  for (Goffset i = 0; i < length; i++) {
    int c = str->getUnfilteredChar();
    outStr->printf("%c", c);  
  }
  str->reset();
  outStr->printf("\r\nendstream\r\n");
}

void PDFDoc::writeString (GooString* s, OutStream* outStr, Guchar *fileKey,
                          CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen)
{
  // Encrypt string if encryption is enabled
  GooString *sEnc = NULL;
  if (fileKey) {
    Object obj;
    EncryptStream *enc = new EncryptStream(new MemStream(s->getCString(), 0, s->getLength(), obj.initNull()),
                                           fileKey, encAlgorithm, keyLength, objNum, objGen);
    sEnc = new GooString();
    int c;
    enc->reset();
    while ((c = enc->getChar()) != EOF) {
      sEnc->append((char)c);
    }

    delete enc;
    s = sEnc;
  }

  // Write data
  if (s->hasUnicodeMarker()) {
    //unicode string don't necessary end with \0
    const char* c = s->getCString();
    outStr->printf("(");
    for(int i=0; i<s->getLength(); i++) {
      char unescaped = *(c+i)&0x000000ff;
      //escape if needed
      if (unescaped == '(' || unescaped == ')' || unescaped == '\\')
        outStr->printf("%c", '\\');
      outStr->printf("%c", unescaped);
    }
    outStr->printf(") ");
  } else {
    const char* c = s->getCString();
    outStr->printf("(");
    for(int i=0; i<s->getLength(); i++) {
      char unescaped = *(c+i)&0x000000ff;
      //escape if needed
      if (unescaped == '\r')
        outStr->printf("\\r");
      else if (unescaped == '\n')
        outStr->printf("\\n");
      else {
        if (unescaped == '(' || unescaped == ')' || unescaped == '\\') {
          outStr->printf("%c", '\\');
        }
        outStr->printf("%c", unescaped);
      }
    }
    outStr->printf(") ");
  }

  delete sEnc;
}

Goffset PDFDoc::writeObjectHeader (Ref *ref, OutStream* outStr)
{
  Goffset offset = outStr->getPos();
  outStr->printf("%i %i obj ", ref->num, ref->gen);
  return offset;
}

void PDFDoc::writeObject (Object* obj, OutStream* outStr, XRef *xRef, Guint numOffset, Guchar *fileKey,
                          CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen)
{
  Array *array;
  Object obj1;
  Goffset tmp;

  switch (obj->getType()) {
    case objBool:
      outStr->printf("%s ", obj->getBool()?"true":"false");
      break;
    case objInt:
      outStr->printf("%i ", obj->getInt());
      break;
    case objInt64:
      outStr->printf("%lli ", obj->getInt64());
      break;
    case objReal:
    {
      GooString s;
      s.appendf("{0:.10g}", obj->getReal());
      outStr->printf("%s ", s.getCString());
      break;
    }
    case objString:
      writeString(obj->getString(), outStr, fileKey, encAlgorithm, keyLength, objNum, objGen);
      break;
    case objName:
    {
      GooString name(obj->getName());
      GooString *nameToPrint = name.sanitizedName(gFalse /* non ps mode */);
      outStr->printf("/%s ", nameToPrint->getCString());
      delete nameToPrint;
      break;
    }
    case objNull:
      outStr->printf( "null ");
      break;
    case objArray:
      array = obj->getArray();
      outStr->printf("[");
      for (int i=0; i<array->getLength(); i++) {
        writeObject(array->getNF(i, &obj1), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, objNum, objGen);
        obj1.free();
      }
      outStr->printf("] ");
      break;
    case objDict:
      writeDictionnary (obj->getDict(), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, objNum, objGen);
      break;
    case objStream: 
      {
        //We can't modify stream with the current implementation (no write functions in Stream API)
        // => the only type of streams which that have been modified are internal streams (=strWeird)
        Stream *stream = obj->getStream();
        if (stream->getKind() == strWeird || stream->getKind() == strCrypt) {
          //we write the stream unencoded => TODO: write stream encoder

          // Encrypt stream
          EncryptStream *encStream = NULL;
          GBool removeFilter = gTrue;
          if (stream->getKind() == strWeird && fileKey) {
            Object filter;
            stream->getDict()->lookup("Filter", &filter);
            if (!filter.isName("Crypt")) {
              if (filter.isArray()) {
                for (int i = 0; i < filter.arrayGetLength(); i++) {
                  Object filterEle;
                  filter.arrayGet(i, &filterEle);
                  if (filterEle.isName("Crypt")) {
                    filterEle.free();
                    removeFilter = gFalse;
                    break;
                  }
                  filterEle.free();
                }
                if (removeFilter) {
                  encStream = new EncryptStream(stream, fileKey, encAlgorithm, keyLength, objNum, objGen);
                  encStream->setAutoDelete(gFalse);
                  stream = encStream;
                }
              } else {
                encStream = new EncryptStream(stream, fileKey, encAlgorithm, keyLength, objNum, objGen);
                encStream->setAutoDelete(gFalse);
                stream = encStream;
              }
            } else {
              removeFilter = gFalse;
            }
            filter.free();
          } else if (fileKey != NULL) { // Encrypt stream
            encStream = new EncryptStream(stream, fileKey, encAlgorithm, keyLength, objNum, objGen);
            encStream->setAutoDelete(gFalse);
            stream = encStream;
          }

          stream->reset();
          //recalculate stream length
          tmp = 0;
          for (int c=stream->getChar(); c!=EOF; c=stream->getChar()) {
            tmp++;
          }
          obj1.initInt64(tmp);
          stream->getDict()->set("Length", &obj1);

          //Remove Stream encoding
          if (removeFilter) {
            stream->getDict()->remove("Filter");
          }
          stream->getDict()->remove("DecodeParms");

          writeDictionnary (stream->getDict(),outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, objNum, objGen);
          writeStream (stream,outStr);
          delete encStream;
          obj1.free();
        } else {
          //raw stream copy
          FilterStream *fs = dynamic_cast<FilterStream*>(stream);
          if (fs) {
            BaseStream *bs = fs->getBaseStream();
            if (bs) {
              Goffset streamEnd;
                if (xRef->getStreamEnd(bs->getStart(), &streamEnd)) {
                  Object val;
                  val.initInt64(streamEnd - bs->getStart());
                  stream->getDict()->set("Length", &val);
                }
              }
          }
          writeDictionnary (stream->getDict(), outStr, xRef, numOffset, fileKey, encAlgorithm, keyLength, objNum, objGen);
          writeRawStream (stream, outStr);
        }
        break;
      }
    case objRef:
      outStr->printf("%i %i R ", obj->getRef().num + numOffset, obj->getRef().gen);
      break;
    case objCmd:
      outStr->printf("%s\n", obj->getCmd());
      break;
    case objError:
      outStr->printf("error\r\n");
      break;
    case objEOF:
      outStr->printf("eof\r\n");
      break;
    case objNone:
      outStr->printf("none\r\n");
      break;
    default:
      error(errUnimplemented, -1,"Unhandled objType : {0:d}, please report a bug with a testcase\r\n", obj->getType());
      break;
  }
}

void PDFDoc::writeObjectFooter (OutStream* outStr)
{
  outStr->printf("endobj\r\n");
}

Dict *PDFDoc::createTrailerDict(int uxrefSize, GBool incrUpdate, Goffset startxRef,
                                Ref *root, XRef *xRef, const char *fileName, Goffset fileSize)
{
  Dict *trailerDict = new Dict(xRef);
  Object obj1;
  obj1.initInt(uxrefSize);
  trailerDict->set("Size", &obj1);
  obj1.free();

  //build a new ID, as recommended in the reference, uses:
  // - current time
  // - file name
  // - file size
  // - values of entry in information dictionnary
  GooString message;
  char buffer[256];
  sprintf(buffer, "%i", (int)time(NULL));
  message.append(buffer);

  if (fileName)
    message.append(fileName);

  sprintf(buffer, "%lli", (long long)fileSize);
  message.append(buffer);

  //info dict -- only use text string
  if (!xRef->getTrailerDict()->isNone() && xRef->getDocInfo(&obj1)->isDict()) {
    for(int i=0; i<obj1.getDict()->getLength(); i++) {
      Object obj2;
      obj1.getDict()->getVal(i, &obj2);  
      if (obj2.isString()) {
        message.append(obj2.getString());
      }
      obj2.free();
    }
  }
  obj1.free();

  GBool hasEncrypt = gFalse;
  if (!xRef->getTrailerDict()->isNone()) {
    Object obj2;
    xRef->getTrailerDict()->dictLookupNF("Encrypt", &obj2);
    if (!obj2.isNull()) {
      trailerDict->set("Encrypt", &obj2);
      hasEncrypt = gTrue;
      obj2.free();
    }
  }

  //calculate md5 digest
  Guchar digest[16];
  md5((Guchar*)message.getCString(), message.getLength(), digest);
  obj1.initString(new GooString((const char*)digest, 16));

  //create ID array
  Object obj2,obj3,obj5;
  obj2.initArray(xRef);

  // In case of encrypted files, the ID must not be changed because it's used to calculate the key
  if (incrUpdate || hasEncrypt) {
    Object obj4;
    //only update the second part of the array
    xRef->getTrailerDict()->getDict()->lookup("ID", &obj4);
    if (!obj4.isArray()) {
      error(errSyntaxWarning, -1, "PDFDoc::createTrailerDict original file's ID entry isn't an array. Trying to continue");
    } else {
      //Get the first part of the ID
      obj4.arrayGet(0,&obj3); 

      obj2.arrayAdd(&obj3); 
      obj2.arrayAdd(&obj1);
      trailerDict->set("ID", &obj2);
    }
    obj4.free();
  } else {
    //new file => same values for the two identifiers
    obj2.arrayAdd(&obj1);
    obj1.initString(new GooString((const char*)digest, 16));
    obj2.arrayAdd(&obj1);
    trailerDict->set("ID", &obj2);
  }

  obj1.initRef(root->num, root->gen);
  trailerDict->set("Root", &obj1);

  if (incrUpdate) { 
    obj1.initInt64(startxRef);
    trailerDict->set("Prev", &obj1);
  }
  
  if (!xRef->getTrailerDict()->isNone()) {
    xRef->getDocInfoNF(&obj5);
    if (!obj5.isNull()) {
      trailerDict->set("Info", &obj5);
    }
  }

  return trailerDict;
}

void PDFDoc::writeXRefTableTrailer(Dict *trailerDict, XRef *uxref, GBool writeAllEntries, Goffset uxrefOffset, OutStream* outStr, XRef *xRef)
{
  uxref->writeTableToFile( outStr, writeAllEntries );
  outStr->printf( "trailer\r\n");
  writeDictionnary(trailerDict, outStr, xRef, 0, NULL, cryptRC4, 0, 0, 0);
  outStr->printf( "\r\nstartxref\r\n");
  outStr->printf( "%lli\r\n", uxrefOffset);
  outStr->printf( "%%%%EOF\r\n");
}

void PDFDoc::writeXRefStreamTrailer (Dict *trailerDict, XRef *uxref, Ref *uxrefStreamRef, Goffset uxrefOffset, OutStream* outStr, XRef *xRef)
{
  GooString stmData;

  // Fill stmData and some trailerDict fields
  uxref->writeStreamToBuffer(&stmData, trailerDict, xRef);

  // Create XRef stream object and write it
  Object obj1;
  MemStream *mStream = new MemStream( stmData.getCString(), 0,
                                      stmData.getLength(), obj1.initDict(trailerDict) );
  writeObjectHeader(uxrefStreamRef, outStr);
  writeObject(obj1.initStream(mStream), outStr, xRef, 0, NULL, cryptRC4, 0, 0, 0);
  writeObjectFooter(outStr);
  obj1.free();

  outStr->printf( "startxref\r\n");
  outStr->printf( "%lli\r\n", uxrefOffset);
  outStr->printf( "%%%%EOF\r\n");
}

void PDFDoc::writeXRefTableTrailer(Goffset uxrefOffset, XRef *uxref, GBool writeAllEntries,
                                   int uxrefSize, OutStream* outStr, GBool incrUpdate)
{
  const char *fileNameA = fileName ? fileName->getCString() : NULL;
  // file size (doesn't include the trailer)
  unsigned int fileSize = 0;
  int c;
  str->reset();
  while ((c = str->getChar()) != EOF) {
    fileSize++;
  }
  str->close();
  Ref ref;
  ref.num = getXRef()->getRootNum();
  ref.gen = getXRef()->getRootGen();
  Dict * trailerDict = createTrailerDict(uxrefSize, incrUpdate, getStartXRef(), &ref,
                                         getXRef(), fileNameA, fileSize);
  writeXRefTableTrailer(trailerDict, uxref, writeAllEntries, uxrefOffset, outStr, getXRef());
  delete trailerDict;
}

void PDFDoc::writeHeader(OutStream *outStr, int major, int minor)
{
   outStr->printf("%%PDF-%d.%d\n", major, minor);
   outStr->printf("%%\xE2\xE3\xCF\xD3\n");
}

void PDFDoc::markDictionnary (Dict* dict, XRef * xRef, XRef *countRef, Guint numOffset)
{
  Object obj1;
  for (int i=0; i<dict->getLength(); i++) {
    markObject(dict->getValNF(i, &obj1), xRef, countRef, numOffset);
    obj1.free();
  }
}

void PDFDoc::markObject (Object* obj, XRef *xRef, XRef *countRef, Guint numOffset)
{
  Array *array;
  Object obj1;

  switch (obj->getType()) {
    case objArray:
      array = obj->getArray();
      for (int i=0; i<array->getLength(); i++) {
        markObject(array->getNF(i, &obj1), xRef, countRef, numOffset);
        obj1.free();
      }
      break;
    case objDict:
      markDictionnary (obj->getDict(), xRef, countRef, numOffset);
      break;
    case objStream: 
      {
        Stream *stream = obj->getStream();
        markDictionnary (stream->getDict(), xRef, countRef, numOffset);
      }
      break;
    case objRef:
      {
        if (obj->getRef().num + (int) numOffset >= xRef->getNumObjects() || xRef->getEntry(obj->getRef().num + numOffset)->type == xrefEntryFree) {
          if (getXRef()->getEntry(obj->getRef().num)->type == xrefEntryFree) {
            return;  // already marked as free => should be replaced
          }
          xRef->add(obj->getRef().num + numOffset, obj->getRef().gen, 0, gTrue);
          if (getXRef()->getEntry(obj->getRef().num)->type == xrefEntryCompressed) {
            xRef->getEntry(obj->getRef().num + numOffset)->type = xrefEntryCompressed;
          }
        }
        if (obj->getRef().num + (int) numOffset >= countRef->getNumObjects() || 
            countRef->getEntry(obj->getRef().num + numOffset)->type == xrefEntryFree)
        {
          countRef->add(obj->getRef().num + numOffset, 1, 0, gTrue);
        } else {
          XRefEntry *entry = countRef->getEntry(obj->getRef().num + numOffset);
          entry->gen++;
          if (entry->gen > 9)
            break;
        } 
        Object obj1;
        getXRef()->fetch(obj->getRef().num, obj->getRef().gen, &obj1);
        markObject(&obj1, xRef, countRef, numOffset);
        obj1.free();
      }
      break;
    default:
      break;
  }
}

void PDFDoc::replacePageDict(int pageNo, int rotate,
                             PDFRectangle *mediaBox, 
                             PDFRectangle *cropBox, Object *pageCTM)
{
  Ref *refPage = getCatalog()->getPageRef(pageNo);
  Object page;
  getXRef()->fetch(refPage->num, refPage->gen, &page);
  Dict *pageDict = page.getDict();
  pageDict->remove("MediaBox");
  pageDict->remove("CropBox");
  pageDict->remove("ArtBox");
  pageDict->remove("BleedBox");
  pageDict->remove("TrimBox");
  pageDict->remove("Rotate");
  Object *mediaBoxObj = new Object();
  mediaBoxObj->initArray(getXRef());
  Object *murx = new Object();
  murx->initReal(mediaBox->x1);
  Object *mury = new Object();
  mury->initReal(mediaBox->y1);
  Object *mllx = new Object();
  mllx->initReal(mediaBox->x2);
  Object *mlly = new Object();
  mlly->initReal(mediaBox->y2);
  mediaBoxObj->arrayAdd(murx);
  mediaBoxObj->arrayAdd(mury);
  mediaBoxObj->arrayAdd(mllx);
  mediaBoxObj->arrayAdd(mlly);
  pageDict->add(copyString("MediaBox"), mediaBoxObj);
  if (cropBox != NULL) {
    Object *cropBoxObj = new Object();
    cropBoxObj->initArray(getXRef());
    Object *curx = new Object();
    curx->initReal(cropBox->x1);
    Object *cury = new Object();
    cury->initReal(cropBox->y1);
    Object *cllx = new Object();
    cllx->initReal(cropBox->x2);
    Object *clly = new Object();
    clly->initReal(cropBox->y2);
    cropBoxObj->arrayAdd(curx);
    cropBoxObj->arrayAdd(cury);
    cropBoxObj->arrayAdd(cllx);
    cropBoxObj->arrayAdd(clly);
    pageDict->add(copyString("CropBox"), cropBoxObj);
    pageDict->add(copyString("TrimBox"), cropBoxObj);
  } else {
    pageDict->add(copyString("TrimBox"), mediaBoxObj);
  }
  Object *rotateObj = new Object();
  rotateObj->initInt(rotate);
  pageDict->add(copyString("Rotate"), rotateObj);
  if (pageCTM != NULL) {
    Object *contents = new Object();
    Ref cmRef = getXRef()->addIndirectObject(pageCTM);
    Object *ref = new Object();
    ref->initRef(cmRef.num, cmRef.gen);
    pageDict->lookupNF("Contents", contents);
    Object *newContents = new Object();
    newContents->initArray(getXRef());
    if (contents->getType() == objRef) {
      newContents->arrayAdd(ref);
      newContents->arrayAdd(contents);
    } else {
      newContents->arrayAdd(ref);
      for (int i = 0; i < contents->arrayGetLength(); i++) {
        Object *contentEle = new Object();
        contents->arrayGetNF(i, contentEle);
        newContents->arrayAdd(contentEle);
      }
    }
    pageDict->remove("Contents");
    pageDict->add(copyString("Contents"), newContents);
  }
  getXRef()->setModifiedObject(&page, *refPage);
  page.free();
}

void PDFDoc::markPageObjects(Dict *pageDict, XRef *xRef, XRef *countRef, Guint numOffset) 
{
  pageDict->remove("Names");
  pageDict->remove("OpenAction");
  pageDict->remove("Outlines");
  pageDict->remove("StructTreeRoot");

  for (int n = 0; n < pageDict->getLength(); n++) {
    const char *key = pageDict->getKey(n);
    Object value; pageDict->getValNF(n, &value);
    if (strcmp(key, "Parent") != 0 &&
	      strcmp(key, "Pages") != 0 &&
        strcmp(key, "Root") != 0) {
      markObject(&value, xRef, countRef, numOffset);
    }
    value.free();
  }
}

Guint PDFDoc::writePageObjects(OutStream *outStr, XRef *xRef, Guint numOffset, GBool combine) 
{
  Guint objectsCount = 0; //count the number of objects in the XRef(s)
  Guchar *fileKey;
  CryptAlgorithm encAlgorithm;
  int keyLength;
  xRef->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

  for (int n = numOffset; n < xRef->getNumObjects(); n++) {
    if (xRef->getEntry(n)->type != xrefEntryFree) {
      Object obj;
      Ref ref;
      ref.num = n;
      ref.gen = xRef->getEntry(n)->gen;
      objectsCount++;
      getXRef()->fetch(ref.num - numOffset, ref.gen, &obj);
      Goffset offset = writeObjectHeader(&ref, outStr);
      if (combine) {
        writeObject(&obj, outStr, getXRef(), numOffset, NULL, cryptRC4, 0, 0, 0);
      } else if (xRef->getEntry(n)->getFlag(XRefEntry::Unencrypted)) {
        writeObject(&obj, outStr, NULL, cryptRC4, 0, 0, 0);
      } else {
        writeObject(&obj, outStr, fileKey, encAlgorithm, keyLength, ref.num, ref.gen);
      }
      writeObjectFooter(outStr);
      xRef->add(ref.num, ref.gen, offset, gTrue);
      obj.free();
    }
  }
  return objectsCount;
}

#ifndef DISABLE_OUTLINE
Outline *PDFDoc::getOutline()
{
  if (!outline) {
    pdfdocLocker();
    // read outline
    outline = new Outline(catalog->getOutline(), xref);
  }

  return outline;
}
#endif

PDFDoc *PDFDoc::ErrorPDFDoc(int errorCode, GooString *fileNameA)
{
  PDFDoc *doc = new PDFDoc();
  doc->errCode = errorCode;
  doc->fileName = fileNameA;

  return doc;
}

long long PDFDoc::strToLongLong(char *s) {
  long long x, d;
  char *p;

  x = 0;
  for (p = s; *p && isdigit(*p & 0xff); ++p) {
    d = *p - '0';
    if (x > (LLONG_MAX - d) / 10) {
      break;
    }
    x = 10 * x + d;
  }
  return x;
}

// Read the 'startxref' position.
Goffset PDFDoc::getStartXRef()
{
  if (startXRefPos == -1) {

    if (isLinearized()) {
      char buf[linearizationSearchSize+1];
      int c, n, i;

      str->setPos(0);
      for (n = 0; n < linearizationSearchSize; ++n) {
        if ((c = str->getChar()) == EOF) {
          break;
        }
        buf[n] = c;
      }
      buf[n] = '\0';

      // find end of first obj (linearization dictionary)
      startXRefPos = 0;
      for (i = 0; i < n; i++) {
        if (!strncmp("endobj", &buf[i], 6)) {
	  i += 6;
	  //skip whitespace 
	  while (buf[i] && Lexer::isSpace(buf[i])) ++i;
	  startXRefPos = i;
	  break;
        }
      }
    } else {
      char buf[xrefSearchSize+1];
      char *p;
      int c, n, i;

      // read last xrefSearchSize bytes
      str->setPos(xrefSearchSize, -1);
      for (n = 0; n < xrefSearchSize; ++n) {
        if ((c = str->getChar()) == EOF) {
          break;
        }
        buf[n] = c;
      }
      buf[n] = '\0';

      // find startxref
      for (i = n - 9; i >= 0; --i) {
        if (!strncmp(&buf[i], "startxref", 9)) {
          break;
        }
      }
      if (i < 0) {
        startXRefPos = 0;
      } else {
        for (p = &buf[i+9]; isspace(*p); ++p) ;
        startXRefPos =  strToLongLong(p);
      }
    }

  }

  return startXRefPos;
}

Goffset PDFDoc::getMainXRefEntriesOffset()
{
  Guint mainXRefEntriesOffset = 0;

  if (isLinearized()) {
    mainXRefEntriesOffset = getLinearization()->getMainXRefEntriesOffset();
  }

  return mainXRefEntriesOffset;
}

int PDFDoc::getNumPages()
{
  if (isLinearized()) {
    int n;
    if ((n = getLinearization()->getNumPages())) {
      return n;
    }
  }

  return catalog->getNumPages();
}

Page *PDFDoc::parsePage(int page)
{
  Page *p = NULL;
  Object obj;
  Ref pageRef;
  Dict *pageDict;

  pageRef.num = getHints()->getPageObjectNum(page);
  if (!pageRef.num) {
    error(errSyntaxWarning, -1, "Failed to get object num from hint tables for page {0:d}", page);
    return NULL;
  }

  // check for bogus ref - this can happen in corrupted PDF files
  if (pageRef.num < 0 || pageRef.num >= xref->getNumObjects()) {
    error(errSyntaxWarning, -1, "Invalid object num ({0:d}) for page {1:d}", pageRef.num, page);
    return NULL;
  }

  pageRef.gen = xref->getEntry(pageRef.num)->gen;
  xref->fetch(pageRef.num, pageRef.gen, &obj);
  if (!obj.isDict("Page")) {
    obj.free();
    error(errSyntaxWarning, -1, "Object ({0:d} {1:d}) is not a pageDict", pageRef.num, pageRef.gen);
    return NULL;
  }
  pageDict = obj.getDict();

  p = new Page(this, page, pageDict, pageRef,
               new PageAttrs(NULL, pageDict), catalog->getForm());
  obj.free();

  return p;
}

Page *PDFDoc::getPage(int page)
{
  if ((page < 1) || page > getNumPages()) return NULL;

  if (isLinearized()) {
    pdfdocLocker();
    if (!pageCache) {
      pageCache = (Page **) gmallocn(getNumPages(), sizeof(Page *));
      for (int i = 0; i < getNumPages(); i++) {
        pageCache[i] = NULL;
      }
    }
    if (!pageCache[page-1]) {
      pageCache[page-1] = parsePage(page);
    }
    if (pageCache[page-1]) {
       return pageCache[page-1];
    } else {
       error(errSyntaxWarning, -1, "Failed parsing page {0:d} using hint tables", page);
    }
  }

  return catalog->getPage(page);
}
