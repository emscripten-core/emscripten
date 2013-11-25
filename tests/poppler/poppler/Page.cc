//========================================================================
//
// Page.cc
//
// Copyright 1996-2007 Glyph & Cog, LLC
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
// Copyright (C) 2005-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006-2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2006 Nickolay V. Shmyrev <nshmyrev@yandex.ru>
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2006-2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2008 Brad Hards <bradh@kde.org>
// Copyright (C) 2008 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <limits.h>
#include "GlobalParams.h"
#include "Object.h"
#include "Array.h"
#include "Dict.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "Link.h"
#include "OutputDev.h"
#include "Gfx.h"
#include "GfxState.h"
#include "Annot.h"
#include "TextOutputDev.h"
#include "Form.h"
#include "Error.h"
#include "Page.h"
#include "Catalog.h"
#include "Form.h"

#if MULTITHREADED
#  define pageLocker()   MutexLocker locker(&mutex)
#else
#  define pageLocker()
#endif
//------------------------------------------------------------------------
// PDFRectangle
//------------------------------------------------------------------------

void PDFRectangle::clipTo(PDFRectangle *rect) {
  if (x1 < rect->x1) {
    x1 = rect->x1;
  } else if (x1 > rect->x2) {
    x1 = rect->x2;
  }
  if (x2 < rect->x1) {
    x2 = rect->x1;
  } else if (x2 > rect->x2) {
    x2 = rect->x2;
  }
  if (y1 < rect->y1) {
    y1 = rect->y1;
  } else if (y1 > rect->y2) {
    y1 = rect->y2;
  }
  if (y2 < rect->y1) {
    y2 = rect->y1;
  } else if (y2 > rect->y2) {
    y2 = rect->y2;
  }
}

//------------------------------------------------------------------------
// PageAttrs
//------------------------------------------------------------------------

PageAttrs::PageAttrs(PageAttrs *attrs, Dict *dict) {
  Object obj1;
  PDFRectangle mBox;
  const GBool isPage = dict->is("Page");

  // get old/default values
  if (attrs) {
    mediaBox = attrs->mediaBox;
    cropBox = attrs->cropBox;
    haveCropBox = attrs->haveCropBox;
    rotate = attrs->rotate;
    attrs->resources.copy(&resources);
  } else {
    // set default MediaBox to 8.5" x 11" -- this shouldn't be necessary
    // but some (non-compliant) PDF files don't specify a MediaBox
    mediaBox.x1 = 0;
    mediaBox.y1 = 0;
    mediaBox.x2 = 612;
    mediaBox.y2 = 792;
    cropBox.x1 = cropBox.y1 = cropBox.x2 = cropBox.y2 = 0;
    haveCropBox = gFalse;
    rotate = 0;
    resources.initNull();
  }

  // media box
  if (readBox(dict, "MediaBox", &mBox)) {
    mediaBox = mBox;
  }

  // crop box
  if (readBox(dict, "CropBox", &cropBox)) {
    haveCropBox = gTrue;
  }
  if (!haveCropBox) {
    cropBox = mediaBox;
  }

  if (isPage) {
    // cropBox can not be bigger than mediaBox
    if (cropBox.x2 - cropBox.x1 > mediaBox.x2 - mediaBox.x1)
    {
      cropBox.x1 = mediaBox.x1;
      cropBox.x2 = mediaBox.x2;
    }
    if (cropBox.y2 - cropBox.y1 > mediaBox.y2 - mediaBox.y1)
    {
      cropBox.y1 = mediaBox.y1;
      cropBox.y2 = mediaBox.y2;
    }
  }

  // other boxes
  bleedBox = cropBox;
  readBox(dict, "BleedBox", &bleedBox);
  trimBox = cropBox;
  readBox(dict, "TrimBox", &trimBox);
  artBox = cropBox;
  readBox(dict, "ArtBox", &artBox);

  // rotate
  dict->lookup("Rotate", &obj1);
  if (obj1.isInt()) {
    rotate = obj1.getInt();
  }
  obj1.free();
  while (rotate < 0) {
    rotate += 360;
  }
  while (rotate >= 360) {
    rotate -= 360;
  }

  // misc attributes
  dict->lookup("LastModified", &lastModified);
  dict->lookup("BoxColorInfo", &boxColorInfo);
  dict->lookup("Group", &group);
  dict->lookup("Metadata", &metadata);
  dict->lookup("PieceInfo", &pieceInfo);
  dict->lookup("SeparationInfo", &separationInfo);

  // resource dictionary
  dict->lookup("Resources", &obj1);
  if (obj1.isDict()) {
    resources.free();
    obj1.copy(&resources);
  }
  obj1.free();
}

PageAttrs::~PageAttrs() {
  lastModified.free();
  boxColorInfo.free();
  group.free();
  metadata.free();
  pieceInfo.free();
  separationInfo.free();
  resources.free();
}

void PageAttrs::clipBoxes() {
  cropBox.clipTo(&mediaBox);
  bleedBox.clipTo(&mediaBox);
  trimBox.clipTo(&mediaBox);
  artBox.clipTo(&mediaBox);
}

GBool PageAttrs::readBox(Dict *dict, const char *key, PDFRectangle *box) {
  PDFRectangle tmp;
  double t;
  Object obj1, obj2;
  GBool ok;

  dict->lookup(key, &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 4) {
    ok = gTrue;
    obj1.arrayGet(0, &obj2);
    if (obj2.isNum()) {
      tmp.x1 = obj2.getNum();
    } else {
      ok = gFalse;
    }
    obj2.free();
    obj1.arrayGet(1, &obj2);
    if (obj2.isNum()) {
      tmp.y1 = obj2.getNum();
    } else {
      ok = gFalse;
    }
    obj2.free();
    obj1.arrayGet(2, &obj2);
    if (obj2.isNum()) {
      tmp.x2 = obj2.getNum();
    } else {
      ok = gFalse;
    }
    obj2.free();
    obj1.arrayGet(3, &obj2);
    if (obj2.isNum()) {
      tmp.y2 = obj2.getNum();
    } else {
      ok = gFalse;
    }
    obj2.free();
    if (tmp.x1 == 0 && tmp.x2 == 0 && tmp.y1 == 0 && tmp.y2 == 0)
      ok = gFalse;
    if (ok) {
      if (tmp.x1 > tmp.x2) {
	t = tmp.x1; tmp.x1 = tmp.x2; tmp.x2 = t;
      }
      if (tmp.y1 > tmp.y2) {
	t = tmp.y1; tmp.y1 = tmp.y2; tmp.y2 = t;
      }
      *box = tmp;
    }
  } else {
    ok = gFalse;
  }
  obj1.free();
  return ok;
}

//------------------------------------------------------------------------
// Page
//------------------------------------------------------------------------

Page::Page(PDFDoc *docA, int numA, Dict *pageDict, Ref pageRefA, PageAttrs *attrsA, Form *form) {
  Object tmp;
	
#if MULTITHREADED
  gInitMutex(&mutex);
#endif
  ok = gTrue;
  doc = docA;
  xref = doc->getXRef();
  num = numA;
  duration = -1;
  annots = NULL;

  pageObj.initDict(pageDict);
  pageRef = pageRefA;

  // get attributes
  attrs = attrsA;
  attrs->clipBoxes();

  // transtion
  pageDict->lookupNF("Trans", &trans);
  if (!(trans.isRef() || trans.isDict() || trans.isNull())) {
    error(errSyntaxError, -1, "Page transition object (page {0:d}) is wrong type ({1:s})",
	  num, trans.getTypeName());
    trans.free();
  }

  // duration
  pageDict->lookupNF("Dur", &tmp);
  if (!(tmp.isNum() || tmp.isNull())) {
    error(errSyntaxError, -1, "Page duration object (page {0:d}) is wrong type ({1:s})",
	  num, tmp.getTypeName());
  } else if (tmp.isNum()) {
    duration = tmp.getNum();
  }
  tmp.free();

  // annotations
  pageDict->lookupNF("Annots", &annotsObj);
  if (!(annotsObj.isRef() || annotsObj.isArray() || annotsObj.isNull())) {
    error(errSyntaxError, -1, "Page annotations object (page {0:d}) is wrong type ({1:s})",
	  num, annotsObj.getTypeName());
    annotsObj.free();
    goto err2;
  }

  // contents
  pageDict->lookupNF("Contents", &contents);
  if (!(contents.isRef() || contents.isArray() ||
	contents.isNull())) {
    error(errSyntaxError, -1, "Page contents object (page {0:d}) is wrong type ({1:s})",
	  num, contents.getTypeName());
    contents.free();
    goto err1;
  }

  // thumb
  pageDict->lookupNF("Thumb", &thumb);
  if (!(thumb.isStream() || thumb.isNull() || thumb.isRef())) {
      error(errSyntaxError, -1, "Page thumb object (page {0:d}) is wrong type ({1:s})",
            num, thumb.getTypeName());
      thumb.initNull(); 
  }

  // actions
  pageDict->lookupNF("AA", &actions);
  if (!(actions.isDict() || actions.isNull())) {
      error(errSyntaxError, -1, "Page additional action object (page {0:d}) is wrong type ({1:s})",
            num, actions.getTypeName());
      actions.initNull();
  }
  
  return;

  trans.initNull();
 err2:
  annotsObj.initNull();
 err1:
  contents.initNull();
  ok = gFalse;
}

Page::~Page() {
  delete attrs;
  delete annots;
  pageObj.free();
  annotsObj.free();
  contents.free();
  trans.free();
  thumb.free();
  actions.free();
#if MULTITHREADED
  gDestroyMutex(&mutex);
#endif
}

Dict *Page::getResourceDict() { 
  return attrs->getResourceDict();
}

Dict *Page::getResourceDictCopy(XRef *xrefA) { 
  pageLocker();
  Dict *dict = attrs->getResourceDict();
  return dict ? dict->copy(xrefA) : NULL;
}

void Page::replaceXRef(XRef *xrefA) {
  Object obj1;
  Dict *pageDict = pageObj.getDict()->copy(xrefA);
  xref = xrefA;
  trans.free();
  pageDict->lookupNF("Trans", &trans);
  annotsObj.free();
  pageDict->lookupNF("Annots", &annotsObj);
  contents.free();
  pageDict->lookupNF("Contents", &contents);
  if (contents.isArray()) {
    contents.free();
    pageDict->lookupNF("Contents", &obj1)->getArray()->copy(xrefA, &contents);
    obj1.free();
  }
  thumb.free();
  pageDict->lookupNF("Thumb", &thumb);
  actions.free();
  pageDict->lookupNF("AA", &actions);
  pageDict->lookup("Resources", &obj1);
  if (obj1.isDict()) {
    attrs->replaceResource(obj1);
  }
  obj1.free();
  delete pageDict;
}

Annots *Page::getAnnots(XRef *xrefA) {
  if (!annots) {
    Object obj;
    annots = new Annots(doc, num, getAnnots(&obj, (xrefA == NULL) ? xref : xrefA));
    obj.free();
  }

  return annots;
}

void Page::addAnnot(Annot *annot) {
  Object obj1;
  Object tmp;
  Ref annotRef = annot->getRef ();

  // Make sure we have annots before adding the new one
  // even if it's an empty list so that we can safely
  // call annots->appendAnnot(annot)
  pageLocker();
  getAnnots();

  if (annotsObj.isNull()) {
    Ref annotsRef;
    // page doesn't have annots array,
    // we have to create it

    obj1.initArray(xref);
    obj1.arrayAdd(tmp.initRef (annotRef.num, annotRef.gen));
    tmp.free();

    annotsRef = xref->addIndirectObject (&obj1);
    annotsObj.initRef(annotsRef.num, annotsRef.gen);
    pageObj.dictSet ("Annots", &annotsObj);
    xref->setModifiedObject (&pageObj, pageRef);
  } else {
    getAnnots(&obj1);
    if (obj1.isArray()) {
      obj1.arrayAdd (tmp.initRef (annotRef.num, annotRef.gen));
      if (annotsObj.isRef())
        xref->setModifiedObject (&obj1, annotsObj.getRef());
      else
        xref->setModifiedObject (&pageObj, pageRef);
    }
    obj1.free();
  }

  annots->appendAnnot(annot);
  annot->setPage(num, gTrue);
}

void Page::removeAnnot(Annot *annot) {
  Ref annotRef = annot->getRef();
  Object annArray;

  pageLocker();
  getAnnots(&annArray);
  if (annArray.isArray()) {
    int idx = -1;
    // Get annotation position
    for (int i = 0; idx == -1 && i < annArray.arrayGetLength(); ++i) {
      Object tmp;
      if (annArray.arrayGetNF(i, &tmp)->isRef()) {
        Ref currAnnot = tmp.getRef();
        if (currAnnot.num == annotRef.num && currAnnot.gen == annotRef.gen) {
          idx = i;
        }
      }
      tmp.free();
    }

    if (idx == -1) {
      error(errInternal, -1, "Annotation doesn't belong to this page");
      annArray.free();
      return;
    }
    annots->removeAnnot(annot); // Gracefully fails on popup windows
    annArray.arrayRemove(idx);
    xref->removeIndirectObject(annotRef);

    if (annotsObj.isRef()) {
      xref->setModifiedObject (&annArray, annotsObj.getRef());
    } else {
      xref->setModifiedObject (&pageObj, pageRef);
    }
  }
  annArray.free();
  annot->removeReferencedObjects(); // Note: Might recurse in removeAnnot again
  annot->setPage(0, gFalse);
}

Links *Page::getLinks() {
  return new Links(getAnnots());
}

FormPageWidgets *Page::getFormWidgets() {
  return new FormPageWidgets(getAnnots(), num, doc->getCatalog()->getForm());
}

void Page::display(OutputDev *out, double hDPI, double vDPI,
		   int rotate, GBool useMediaBox, GBool crop,
		   GBool printing,
		   GBool (*abortCheckCbk)(void *data),
		   void *abortCheckCbkData,
                   GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                   void *annotDisplayDecideCbkData,
                   GBool copyXRef) {
  displaySlice(out, hDPI, vDPI, rotate, useMediaBox, crop, -1, -1, -1, -1, printing,
	       abortCheckCbk, abortCheckCbkData,
               annotDisplayDecideCbk, annotDisplayDecideCbkData, copyXRef);
}

Gfx *Page::createGfx(OutputDev *out, double hDPI, double vDPI,
		     int rotate, GBool useMediaBox, GBool crop,
		     int sliceX, int sliceY, int sliceW, int sliceH,
		     GBool printing,
		     GBool (*abortCheckCbk)(void *data),
		     void *abortCheckCbkData, XRef *xrefA) {
  PDFRectangle *mediaBox, *cropBox;
  PDFRectangle box;
  Gfx *gfx;

  rotate += getRotate();
  if (rotate >= 360) {
    rotate -= 360;
  } else if (rotate < 0) {
    rotate += 360;
  }

  makeBox(hDPI, vDPI, rotate, useMediaBox, out->upsideDown(),
	  sliceX, sliceY, sliceW, sliceH, &box, &crop);
  cropBox = getCropBox();
  mediaBox = getMediaBox();

  if (globalParams->getPrintCommands()) {
    printf("***** MediaBox = ll:%g,%g ur:%g,%g\n",
	    mediaBox->x1, mediaBox->y1, mediaBox->x2, mediaBox->y2);
      printf("***** CropBox = ll:%g,%g ur:%g,%g\n",
	     cropBox->x1, cropBox->y1, cropBox->x2, cropBox->y2);
    printf("***** Rotate = %d\n", attrs->getRotate());
  }

  if (!crop) {
    crop = (box == *cropBox) && out->needClipToCropBox();
  }
  gfx = new Gfx(doc, out, num, attrs->getResourceDict(),
		hDPI, vDPI, &box, crop ? cropBox : (PDFRectangle *)NULL,
		rotate, abortCheckCbk, abortCheckCbkData, xrefA);

  return gfx;
}

void Page::displaySlice(OutputDev *out, double hDPI, double vDPI,
			int rotate, GBool useMediaBox, GBool crop,
			int sliceX, int sliceY, int sliceW, int sliceH,
			GBool printing,
			GBool (*abortCheckCbk)(void *data),
			void *abortCheckCbkData,
                        GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                        void *annotDisplayDecideCbkData,
                        GBool copyXRef) {
  Gfx *gfx;
  Object obj;
  Annots *annotList;
  int i;
  
  if (!out->checkPageSlice(this, hDPI, vDPI, rotate, useMediaBox, crop,
			   sliceX, sliceY, sliceW, sliceH,
			   printing,
			   abortCheckCbk, abortCheckCbkData,
			   annotDisplayDecideCbk, annotDisplayDecideCbkData)) {
    return;
  }
  pageLocker();
  XRef *localXRef = (copyXRef) ? xref->copy() : xref;
  if (copyXRef) {
    replaceXRef(localXRef);
  }

  gfx = createGfx(out, hDPI, vDPI, rotate, useMediaBox, crop,
		  sliceX, sliceY, sliceW, sliceH,
		  printing,
		  abortCheckCbk, abortCheckCbkData, localXRef);

  contents.fetch(localXRef, &obj);
  if (!obj.isNull()) {
    gfx->saveState();
    gfx->display(&obj);
    gfx->restoreState();
  } else {
    // empty pages need to call dump to do any setup required by the
    // OutputDev
    out->dump();
  }
  obj.free();

  // draw annotations
  annotList = getAnnots();

  if (annotList->getNumAnnots() > 0) {
    if (globalParams->getPrintCommands()) {
      printf("***** Annotations\n");
    }
    for (i = 0; i < annotList->getNumAnnots(); ++i) {
        Annot *annot = annotList->getAnnot(i);
        if ((annotDisplayDecideCbk &&
             (*annotDisplayDecideCbk)(annot, annotDisplayDecideCbkData)) || 
            !annotDisplayDecideCbk) {
             annotList->getAnnot(i)->draw(gfx, printing);
	}
    }
    out->dump();
  }

  delete gfx;
  if (copyXRef) {
    replaceXRef(doc->getXRef());
    delete localXRef;
  }
}

void Page::display(Gfx *gfx) {
  Object obj;

  contents.fetch(xref, &obj);
  if (!obj.isNull()) {
    gfx->saveState();
    gfx->display(&obj);
    gfx->restoreState();
  }
  obj.free();
}

GBool Page::loadThumb(unsigned char **data_out,
		      int *width_out, int *height_out,
		      int *rowstride_out)
{
  unsigned int pixbufdatasize;
  int width, height, bits;
  Object obj1, fetched_thumb;
  Dict *dict;
  GfxColorSpace *colorSpace;
  GBool success = gFalse;
  Stream *str;
  GfxImageColorMap *colorMap;

  /* Get stream dict */
  pageLocker();
  thumb.fetch(xref, &fetched_thumb);
  if (!fetched_thumb.isStream()) {
    fetched_thumb.free();
    return gFalse;
  }

  dict = fetched_thumb.streamGetDict();
  str = fetched_thumb.getStream(); 
		
  if (!dict->lookupInt("Width", "W", &width))
    goto fail1;
  if (!dict->lookupInt("Height", "H", &height))
    goto fail1;
  if (!dict->lookupInt("BitsPerComponent", "BPC", &bits))
    goto fail1;
		
  /* Check for invalid dimensions and integer overflow. */
  if (width <= 0 || height <= 0)
    goto fail1;
  if (width > INT_MAX / 3 / height)
    goto fail1;
  pixbufdatasize = width * height * 3;

  /* Get color space */
  dict->lookup ("ColorSpace", &obj1);
  if (obj1.isNull ()) {
    obj1.free ();
    dict->lookup ("CS", &obj1);
  }
  colorSpace = GfxColorSpace::parse(&obj1, NULL, NULL);
  obj1.free();
  if (!colorSpace) {
    fprintf (stderr, "Error: Cannot parse color space\n");
    goto fail1;
  }

  dict->lookup("Decode", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("D", &obj1);
  }
  colorMap = new GfxImageColorMap(bits, &obj1, colorSpace);
  obj1.free();
  if (!colorMap->isOk()) {
    fprintf (stderr, "Error: invalid colormap\n");
    delete colorMap;
    goto fail1;
  }

  if (data_out) {
    unsigned char *pixbufdata = (unsigned char *) gmalloc(pixbufdatasize);
    unsigned char *p = pixbufdata;
    ImageStream *imgstr = new ImageStream(str, width,
			   colorMap->getNumPixelComps(),
			   colorMap->getBits());
    imgstr->reset();
    for (int row = 0; row < height; ++row) {
      for (int col = 0; col < width; ++col) {
        Guchar pix[gfxColorMaxComps];
        GfxRGB rgb;

        imgstr->getPixel(pix);
        colorMap->getRGB(pix, &rgb);

        *p++ = colToByte(rgb.r);
        *p++ = colToByte(rgb.g);
        *p++ = colToByte(rgb.b);
      }
    }
    *data_out = pixbufdata;
    imgstr->close();
    delete imgstr;
  }

  success = gTrue;

  if (width_out)
    *width_out = width;
  if (height_out)
    *height_out = height;
  if (rowstride_out)
    *rowstride_out = width * 3;

  delete colorMap;
 fail1:
  fetched_thumb.free();

  return success;
}

void Page::makeBox(double hDPI, double vDPI, int rotate,
		   GBool useMediaBox, GBool upsideDown,
		   double sliceX, double sliceY, double sliceW, double sliceH,
		   PDFRectangle *box, GBool *crop) {
  PDFRectangle *mediaBox, *cropBox, *baseBox;
  double kx, ky;

  mediaBox = getMediaBox();
  cropBox = getCropBox();
  if (sliceW >= 0 && sliceH >= 0) {
    baseBox = useMediaBox ? mediaBox : cropBox;
    kx = 72.0 / hDPI;
    ky = 72.0 / vDPI;
    if (rotate == 90) {
      if (upsideDown) {
	box->x1 = baseBox->x1 + ky * sliceY;
	box->x2 = baseBox->x1 + ky * (sliceY + sliceH);
      } else {
	box->x1 = baseBox->x2 - ky * (sliceY + sliceH);
	box->x2 = baseBox->x2 - ky * sliceY;
      }
      box->y1 = baseBox->y1 + kx * sliceX;
      box->y2 = baseBox->y1 + kx * (sliceX + sliceW);
    } else if (rotate == 180) {
      box->x1 = baseBox->x2 - kx * (sliceX + sliceW);
      box->x2 = baseBox->x2 - kx * sliceX;
      if (upsideDown) {
	box->y1 = baseBox->y1 + ky * sliceY;
	box->y2 = baseBox->y1 + ky * (sliceY + sliceH);
      } else {
	box->y1 = baseBox->y2 - ky * (sliceY + sliceH);
	box->y2 = baseBox->y2 - ky * sliceY;
      }
    } else if (rotate == 270) {
      if (upsideDown) {
	box->x1 = baseBox->x2 - ky * (sliceY + sliceH);
	box->x2 = baseBox->x2 - ky * sliceY;
      } else {
	box->x1 = baseBox->x1 + ky * sliceY;
	box->x2 = baseBox->x1 + ky * (sliceY + sliceH);
      }
      box->y1 = baseBox->y2 - kx * (sliceX + sliceW);
      box->y2 = baseBox->y2 - kx * sliceX;
    } else {
      box->x1 = baseBox->x1 + kx * sliceX;
      box->x2 = baseBox->x1 + kx * (sliceX + sliceW);
      if (upsideDown) {
	box->y1 = baseBox->y2 - ky * (sliceY + sliceH);
	box->y2 = baseBox->y2 - ky * sliceY;
      } else {
	box->y1 = baseBox->y1 + ky * sliceY;
	box->y2 = baseBox->y1 + ky * (sliceY + sliceH);
      }
    }
  } else if (useMediaBox) {
    *box = *mediaBox;
  } else {
    *box = *cropBox;
    *crop = gFalse;
  }
}

void Page::processLinks(OutputDev *out) {
  Links *links;
  int i;

  links = getLinks();
  for (i = 0; i < links->getNumLinks(); ++i) {
    out->processLink(links->getLink(i));
  }
  delete links;
}

void Page::getDefaultCTM(double *ctm, double hDPI, double vDPI,
			 int rotate, GBool useMediaBox, GBool upsideDown) {
  GfxState *state;
  int i;
  rotate += getRotate();
  if (rotate >= 360) {
    rotate -= 360;
  } else if (rotate < 0) {
    rotate += 360;
  }
  state = new GfxState(hDPI, vDPI,
		       useMediaBox ? getMediaBox() : getCropBox(),
		       rotate, upsideDown);
  for (i = 0; i < 6; ++i) {
    ctm[i] = state->getCTM()[i];
  }
 delete state;
}

LinkAction* Page::getAdditionalAction(PageAdditionalActionsType type) {
  Object additionalActionsObject;
  LinkAction *linkAction = NULL;

  if (actions.fetch(doc->getXRef(), &additionalActionsObject)->isDict()) {
    const char *key = (type == actionOpenPage ?  "O" :
                       type == actionClosePage ? "C" : NULL);

    Object actionObject;

    if (additionalActionsObject.dictLookup(key, &actionObject)->isDict())
      linkAction = LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
    actionObject.free();
  }

  additionalActionsObject.free();

  return linkAction;
}
