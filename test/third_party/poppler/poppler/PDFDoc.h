//========================================================================
//
// PDFDoc.h
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
// Copyright (C) 2005, 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Eric Toombs <ewtoombs@uwaterloo.ca>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Srinivas Adicherla <srinivas.adicherla@geodesic.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PDFDOC_H
#define PDFDOC_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "Annot.h"
#include "OptionalContent.h"

class GooString;
class BaseStream;
class OutputDev;
class Links;
class LinkAction;
class LinkDest;
class Outline;
class Linearization;
class SecurityHandler;
class Hints;

enum PDFWriteMode {
  writeStandard,
  writeForceRewrite,
  writeForceIncremental
};

//------------------------------------------------------------------------
// PDFDoc
//------------------------------------------------------------------------

class PDFDoc {
public:

  PDFDoc(GooString *fileNameA, GooString *ownerPassword = NULL,
	 GooString *userPassword = NULL, void *guiDataA = NULL);

#ifdef _WIN32
  PDFDoc(wchar_t *fileNameA, int fileNameLen, GooString *ownerPassword = NULL,
	 GooString *userPassword = NULL, void *guiDataA = NULL);
#endif

  PDFDoc(BaseStream *strA, GooString *ownerPassword = NULL,
	 GooString *userPassword = NULL, void *guiDataA = NULL);
  ~PDFDoc();

  static PDFDoc *ErrorPDFDoc(int errorCode, GooString *fileNameA = NULL);

  // Was PDF document successfully opened?
  GBool isOk() { return ok; }

  // Get the error code (if isOk() returns false).
  int getErrorCode() { return errCode; }

  // Get the error code returned by fopen() (if getErrorCode() == 
  // errOpenFile).
  int getFopenErrno() { return fopenErrno; }

  // Get file name.
  GooString *getFileName() { return fileName; }

  // Get the linearization table.
  Linearization *getLinearization();

  // Get the xref table.
  XRef *getXRef() { return xref; }

  // Get catalog.
  Catalog *getCatalog() { return catalog; }

  // Get optional content configuration
  OCGs *getOptContentConfig() { return catalog->getOptContentConfig(); }

  // Get base stream.
  BaseStream *getBaseStream() { return str; }

  // Get page parameters.
  double getPageMediaWidth(int page)
    { return getPage(page) ? getPage(page)->getMediaWidth() : 0.0 ; }
  double getPageMediaHeight(int page)
    { return getPage(page) ? getPage(page)->getMediaHeight() : 0.0 ; }
  double getPageCropWidth(int page)
    { return getPage(page) ? getPage(page)->getCropWidth() : 0.0 ; }
  double getPageCropHeight(int page)
    { return getPage(page) ? getPage(page)->getCropHeight() : 0.0 ; }
  int getPageRotate(int page)
    { return getPage(page) ? getPage(page)->getRotate() : 0 ; }

  // Get number of pages.
  int getNumPages();

  // Return the contents of the metadata stream, or NULL if there is
  // no metadata.
  GooString *readMetadata() { return catalog->readMetadata(); }

  // Return the structure tree root object.
  Object *getStructTreeRoot() { return catalog->getStructTreeRoot(); }

  // Get page.
  Page *getPage(int page);

  // Display a page.
  void displayPage(OutputDev *out, int page,
		   double hDPI, double vDPI, int rotate,
		   GBool useMediaBox, GBool crop, GBool printing,
		   GBool (*abortCheckCbk)(void *data) = NULL,
		   void *abortCheckCbkData = NULL,
                   GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                   void *annotDisplayDecideCbkData = NULL);

  // Display a range of pages.
  void displayPages(OutputDev *out, int firstPage, int lastPage,
		    double hDPI, double vDPI, int rotate,
		    GBool useMediaBox, GBool crop, GBool printing,
		    GBool (*abortCheckCbk)(void *data) = NULL,
		    void *abortCheckCbkData = NULL,
                    GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                    void *annotDisplayDecideCbkData = NULL);

  // Display part of a page.
  void displayPageSlice(OutputDev *out, int page,
			double hDPI, double vDPI, int rotate, 
			GBool useMediaBox, GBool crop, GBool printing,
			int sliceX, int sliceY, int sliceW, int sliceH,
			GBool (*abortCheckCbk)(void *data) = NULL,
			void *abortCheckCbkData = NULL,
                        GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                        void *annotDisplayDecideCbkData = NULL);

  // Find a page, given its object ID.  Returns page number, or 0 if
  // not found.
  int findPage(int num, int gen) { return catalog->findPage(num, gen); }

  // Returns the links for the current page, transferring ownership to
  // the caller.
  Links *getLinks(int page);

  // Find a named destination.  Returns the link destination, or
  // NULL if <name> is not a destination.
  LinkDest *findDest(GooString *name)
    { return catalog->findDest(name); }

  // Process the links for a page.
  void processLinks(OutputDev *out, int page);


#ifndef DISABLE_OUTLINE
  // Return the outline object.
  Outline *getOutline();
#endif

  // Is the file encrypted?
  GBool isEncrypted() { return xref->isEncrypted(); }

  // Check various permissions.
  GBool okToPrint(GBool ignoreOwnerPW = gFalse)
    { return xref->okToPrint(ignoreOwnerPW); }
  GBool okToPrintHighRes(GBool ignoreOwnerPW = gFalse)
    { return xref->okToPrintHighRes(ignoreOwnerPW); }
  GBool okToChange(GBool ignoreOwnerPW = gFalse)
    { return xref->okToChange(ignoreOwnerPW); }
  GBool okToCopy(GBool ignoreOwnerPW = gFalse)
    { return xref->okToCopy(ignoreOwnerPW); }
  GBool okToAddNotes(GBool ignoreOwnerPW = gFalse)
    { return xref->okToAddNotes(ignoreOwnerPW); }
  GBool okToFillForm(GBool ignoreOwnerPW = gFalse)
    { return xref->okToFillForm(ignoreOwnerPW); }
  GBool okToAccessibility(GBool ignoreOwnerPW = gFalse)
    { return xref->okToAccessibility(ignoreOwnerPW); }
  GBool okToAssemble(GBool ignoreOwnerPW = gFalse)
    { return xref->okToAssemble(ignoreOwnerPW); }


  // Is this document linearized?
  GBool isLinearized();

  // Return the document's Info dictionary (if any).
  Object *getDocInfo(Object *obj) { return xref->getDocInfo(obj); }
  Object *getDocInfoNF(Object *obj) { return xref->getDocInfoNF(obj); }

  // Return the PDF version specified by the file.
  int getPDFMajorVersion() { return pdfMajorVersion; }
  int getPDFMinorVersion() { return pdfMinorVersion; }

  //Return the PDF ID in the trailer dictionary (if any).
  GBool getID(GooString *permanent_id, GooString *update_id);

  // Save this file with another name.
  int saveAs(GooString *name, PDFWriteMode mode=writeStandard);
  // Save this file in the given output stream.
  int saveAs(OutStream *outStr, PDFWriteMode mode=writeStandard);
  // Save this file with another name without saving changes
  int saveWithoutChangesAs(GooString *name);
  // Save this file in the given output stream without saving changes
  int saveWithoutChangesAs(OutStream *outStr);

  // Return a pointer to the GUI (XPDFCore or WinPDFCore object).
  void *getGUIData() { return guiData; }

private:
  // Add object to current file stream and return the offset of the beginning of the object
  Guint writeObject (Object *obj, Ref *ref, OutStream* outStr);
  void writeDictionnary (Dict* dict, OutStream* outStr);
  void writeStream (Stream* str, OutStream* outStr);
  void writeRawStream (Stream* str, OutStream* outStr);
  void writeTrailer (Guint uxrefOffset, int uxrefSize, OutStream* outStr, GBool incrUpdate);
  void writeString (GooString* s, OutStream* outStr);
  void saveIncrementalUpdate (OutStream* outStr);
  void saveCompleteRewrite (OutStream* outStr);

  Page *parsePage(int page);

  // Get hints.
  Hints *getHints();

  PDFDoc();
  void init();
  GBool setup(GooString *ownerPassword, GooString *userPassword);
  GBool checkFooter();
  void checkHeader();
  GBool checkEncryption(GooString *ownerPassword, GooString *userPassword);
  // Get the offset of the start xref table.
  Guint getStartXRef();
  // Get the offset of the entries in the main XRef table of a
  // linearized document (0 for non linearized documents).
  Guint getMainXRefEntriesOffset();
  Guint strToUnsigned(char *s);

  GooString *fileName;
  FILE *file;
  BaseStream *str;
  void *guiData;
  int pdfMajorVersion;
  int pdfMinorVersion;
  Linearization *linearization;
  XRef *xref;
  SecurityHandler *secHdlr;
  Catalog *catalog;
  Hints *hints;
#ifndef DISABLE_OUTLINE
  Outline *outline;
#endif
  Page **pageCache;

  GBool ok;
  int errCode;
  //If there is an error opening the PDF file with fopen() in the constructor, 
  //then the POSIX errno will be here.
  int fopenErrno;

  Guint startXRefPos;		// offset of last xref table
};

#endif
