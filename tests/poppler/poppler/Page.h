//========================================================================
//
// Page.h
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
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2006 Pino Toscano <pino@kde.org>
// Copyright (C) 2006, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PAGE_H
#define PAGE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include "Object.h"
#include "goo/GooMutex.h"

class Dict;
class PDFDoc;
class XRef;
class OutputDev;
class Links;
class LinkAction;
class Annots;
class Annot;
class Gfx;
class FormPageWidgets;
class Form;

//------------------------------------------------------------------------

class PDFRectangle {
public:
  double x1, y1, x2, y2;

  PDFRectangle() { x1 = y1 = x2 = y2 = 0; }
  PDFRectangle(double x1A, double y1A, double x2A, double y2A)
    { x1 = x1A; y1 = y1A; x2 = x2A; y2 = y2A; }
  GBool isValid() { return x1 != 0 || y1 != 0 || x2 != 0 || y2 != 0; }
  GBool contains(double x, double y) { return x1 <= x && x <= x2 && y1 <= y && y <= y2; }
  void clipTo(PDFRectangle *rect);
  
  bool operator==(const PDFRectangle &rect) const { return x1 == rect.x1 && y1 == rect.y1 && x2 == rect.x2 && y2 == rect.y2; }
};

//------------------------------------------------------------------------
// PageAttrs
//------------------------------------------------------------------------

class PageAttrs {
public:

  // Construct a new PageAttrs object by merging a dictionary
  // (of type Pages or Page) into another PageAttrs object.  If
  // <attrs> is NULL, uses defaults.
  PageAttrs(PageAttrs *attrs, Dict *dict);

  // Destructor.
  ~PageAttrs();

  // Accessors.
  PDFRectangle *getMediaBox() { return &mediaBox; }
  PDFRectangle *getCropBox() { return &cropBox; }
  GBool isCropped() { return haveCropBox; }
  PDFRectangle *getBleedBox() { return &bleedBox; }
  PDFRectangle *getTrimBox() { return &trimBox; }
  PDFRectangle *getArtBox() { return &artBox; }
  int getRotate() { return rotate; }
  GooString *getLastModified()
    { return lastModified.isString()
	? lastModified.getString() : (GooString *)NULL; }
  Dict *getBoxColorInfo()
    { return boxColorInfo.isDict() ? boxColorInfo.getDict() : (Dict *)NULL; }
  Dict *getGroup()
    { return group.isDict() ? group.getDict() : (Dict *)NULL; }
  Stream *getMetadata()
    { return metadata.isStream() ? metadata.getStream() : (Stream *)NULL; }
  Dict *getPieceInfo()
    { return pieceInfo.isDict() ? pieceInfo.getDict() : (Dict *)NULL; }
  Dict *getSeparationInfo()
    { return separationInfo.isDict()
	? separationInfo.getDict() : (Dict *)NULL; }
  Dict *getResourceDict()
    { return resources.isDict() ? resources.getDict() : (Dict *)NULL; }
  void replaceResource(Object obj1) 
  {  resources.free(); obj1.copy(&resources); }

  // Clip all other boxes to the MediaBox.
  void clipBoxes();

private:

  GBool readBox(Dict *dict, const char *key, PDFRectangle *box);

  PDFRectangle mediaBox;
  PDFRectangle cropBox;
  GBool haveCropBox;
  PDFRectangle bleedBox;
  PDFRectangle trimBox;
  PDFRectangle artBox;
  int rotate;
  Object lastModified;
  Object boxColorInfo;
  Object group;
  Object metadata;
  Object pieceInfo;
  Object separationInfo;
  Object resources;
};

//------------------------------------------------------------------------
// Page
//------------------------------------------------------------------------

class Page {
public:

  // Constructor.
  Page(PDFDoc *docA, int numA, Dict *pageDict, Ref pageRefA, PageAttrs *attrsA, Form *form);

  // Destructor.
  ~Page();

  // Is page valid?
  GBool isOk() { return ok; }

  // Get page parameters.
  int getNum() { return num; }
  PDFRectangle *getMediaBox() { return attrs->getMediaBox(); }
  PDFRectangle *getCropBox() { return attrs->getCropBox(); }
  GBool isCropped() { return attrs->isCropped(); }
  double getMediaWidth() 
    { return attrs->getMediaBox()->x2 - attrs->getMediaBox()->x1; }
  double getMediaHeight()
    { return attrs->getMediaBox()->y2 - attrs->getMediaBox()->y1; }
  double getCropWidth() 
    { return attrs->getCropBox()->x2 - attrs->getCropBox()->x1; }
  double getCropHeight()
    { return attrs->getCropBox()->y2 - attrs->getCropBox()->y1; }
  PDFRectangle *getBleedBox() { return attrs->getBleedBox(); }
  PDFRectangle *getTrimBox() { return attrs->getTrimBox(); }
  PDFRectangle *getArtBox() { return attrs->getArtBox(); }
  int getRotate() { return attrs->getRotate(); }
  GooString *getLastModified() { return attrs->getLastModified(); }
  Dict *getBoxColorInfo() { return attrs->getBoxColorInfo(); }
  Dict *getGroup() { return attrs->getGroup(); }
  Stream *getMetadata() { return attrs->getMetadata(); }
  Dict *getPieceInfo() { return attrs->getPieceInfo(); }
  Dict *getSeparationInfo() { return attrs->getSeparationInfo(); }
  PDFDoc *getDoc() { return doc; }
  Ref getRef() { return pageRef; }

  // Get resource dictionary.
  Dict *getResourceDict();
  Dict *getResourceDictCopy(XRef *xrefA);

  // Get annotations array.
  Object *getAnnots(Object *obj, XRef *xrefA = NULL) { return annotsObj.fetch((xrefA == NULL) ? xref : xrefA, obj); }
  // Add a new annotation to the page
  void addAnnot(Annot *annot);
  // Remove an existing annotation from the page
  void removeAnnot(Annot *annot);

  // Return a list of links.
  Links *getLinks();

  // Return a list of annots. It will be valid until the page is destroyed
  Annots *getAnnots(XRef *xrefA = NULL);

  // Get contents.
  Object *getContents(Object *obj) { return contents.fetch(xref, obj); }

  // Get thumb.
  Object *getThumb(Object *obj) { return thumb.fetch(xref, obj); }
  GBool loadThumb(unsigned char **data, int *width, int *height, int *rowstride);

  // Get transition.
  Object *getTrans(Object *obj) { return trans.fetch(xref, obj); }

  // Get form.
  FormPageWidgets *getFormWidgets();

  // Get duration, the maximum length of time, in seconds,
  // that the page is displayed before the presentation automatically
  // advances to the next page
  double getDuration() { return duration; }

  // Get actions
  Object *getActions(Object *obj) { return actions.fetch(xref, obj); }

  enum PageAdditionalActionsType {
    actionOpenPage,     ///< Performed when opening the page
    actionClosePage,    ///< Performed when closing the page
  };

  LinkAction *getAdditionalAction(PageAdditionalActionsType type);

  Gfx *createGfx(OutputDev *out, double hDPI, double vDPI,
		 int rotate, GBool useMediaBox, GBool crop,
		 int sliceX, int sliceY, int sliceW, int sliceH,
		 GBool printing,
		 GBool (*abortCheckCbk)(void *data),
		 void *abortCheckCbkData, XRef *xrefA = NULL);

  // Display a page.
  void display(OutputDev *out, double hDPI, double vDPI,
	       int rotate, GBool useMediaBox, GBool crop,
	       GBool printing,
	       GBool (*abortCheckCbk)(void *data) = NULL,
	       void *abortCheckCbkData = NULL,
               GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
               void *annotDisplayDecideCbkData = NULL,
               GBool copyXRef = gFalse);

  // Display part of a page.
  void displaySlice(OutputDev *out, double hDPI, double vDPI,
		    int rotate, GBool useMediaBox, GBool crop,
		    int sliceX, int sliceY, int sliceW, int sliceH,
		    GBool printing,
		    GBool (*abortCheckCbk)(void *data) = NULL,
		    void *abortCheckCbkData = NULL,
                    GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data) = NULL,
                    void *annotDisplayDecideCbkData = NULL,
                    GBool copyXRef = gFalse);

  void display(Gfx *gfx);

  void makeBox(double hDPI, double vDPI, int rotate,
	       GBool useMediaBox, GBool upsideDown,
	       double sliceX, double sliceY, double sliceW, double sliceH,
	       PDFRectangle *box, GBool *crop);

  void processLinks(OutputDev *out);

  // Get the page's default CTM.
  void getDefaultCTM(double *ctm, double hDPI, double vDPI,
		     int rotate, GBool useMediaBox, GBool upsideDown);

private:
  // replace xref
  void replaceXRef(XRef *xrefA);

  PDFDoc *doc;
  XRef *xref;			// the xref table for this PDF file
  Object pageObj;               // page dictionary
  Ref pageRef;                  // page reference
  int num;			// page number
  PageAttrs *attrs;		// page attributes
  Annots *annots;               // annotations
  Object annotsObj;		// annotations array
  Object contents;		// page contents
  Object thumb;			// page thumbnail
  Object trans;			// page transition
  Object actions;		// page additional actions
  double duration;              // page duration
  GBool ok;			// true if page is valid
#if MULTITHREADED
  GooMutex mutex;
#endif
};

#endif
