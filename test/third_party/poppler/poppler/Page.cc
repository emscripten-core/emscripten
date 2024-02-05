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
// Copyright (C) 2005-2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006-2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2006 Nickolay V. Shmyrev <nshmyrev@yandex.ru>
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2006-2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2008 Brad Hards <bradh@kde.org>
// Copyright (C) 2008 Ilya Gorenbein <igorenbein@finjan.com>
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
#include "XRef.h"
#include "Link.h"
#include "OutputDev.h"
#ifndef PDF_PARSER_ONLY
#include "Gfx.h"
#include "GfxState.h"
#include "Annot.h"
#include "TextOutputDev.h"
#include "Form.h"
#endif
#include "Error.h"
#include "Page.h"
#include "Catalog.h"
#include "Form.h"

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

  if (isPage) {
    // clip all other boxes to the media box
    cropBox.clipTo(&mediaBox);
    bleedBox.clipTo(&mediaBox);
    trimBox.clipTo(&mediaBox);
    artBox.clipTo(&mediaBox);
  }

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

GBool PageAttrs::readBox(Dict *dict, char *key, PDFRectangle *box) {
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

Page::Page(XRef *xrefA, int numA, Dict *pageDict, Ref pageRefA, PageAttrs *attrsA, Form *form) {
  Object tmp;
	
  ok = gTrue;
  xref = xrefA;
  num = numA;
  duration = -1;
  pageWidgets = NULL;

  pageObj.initDict(pageDict);
  pageRef = pageRefA;

  // get attributes
  attrs = attrsA;

  // transtion
  pageDict->lookupNF("Trans", &trans);
  if (!(trans.isRef() || trans.isDict() || trans.isNull())) {
    error(-1, "Page transition object (page %d) is wrong type (%s)",
	  num, trans.getTypeName());
    trans.free();
  }

  // duration
  pageDict->lookupNF("Dur", &tmp);
  if (!(tmp.isNum() || tmp.isNull())) {
    error(-1, "Page duration object (page %d) is wrong type (%s)",
	  num, tmp.getTypeName());
  } else if (tmp.isNum()) {
    duration = tmp.getNum();
  }
  tmp.free();

  // annotations
  pageDict->lookupNF("Annots", &annots);
  if (!(annots.isRef() || annots.isArray() || annots.isNull())) {
    error(-1, "Page annotations object (page %d) is wrong type (%s)",
	  num, annots.getTypeName());
    annots.free();
    goto err2;
  }

  // forms
  pageWidgets = new FormPageWidgets(xrefA, this->getAnnots(&tmp),num,form);
  tmp.free();

  // contents
  pageDict->lookupNF("Contents", &contents);
  if (!(contents.isRef() || contents.isArray() ||
	contents.isNull())) {
    error(-1, "Page contents object (page %d) is wrong type (%s)",
	  num, contents.getTypeName());
    contents.free();
    goto err1;
  }

  // thumb
  pageDict->lookupNF("Thumb", &thumb);
  if (!(thumb.isStream() || thumb.isNull() || thumb.isRef())) {
      error(-1, "Page thumb object (page %d) is wrong type (%s)",
            num, thumb.getTypeName());
      thumb.initNull(); 
  }

  // actions
  pageDict->lookupNF("AA", &actions);
  if (!(actions.isDict() || actions.isNull())) {
      error(-1, "Page additional action object (page %d) is wrong type (%s)",
            num, actions.getTypeName());
      actions.initNull();
  }
  
  return;

  trans.initNull();
 err2:
  annots.initNull();
 err1:
  contents.initNull();
  ok = gFalse;
}

Page::~Page() {
  delete pageWidgets;
  delete attrs;
  pageObj.free();
  annots.free();
  contents.free();
  trans.free();
  thumb.free();
  actions.free();
}

Annots *Page::getAnnots(Catalog *catalog) {
  Annots *annots;
  Object obj;

  annots = new Annots(xref, catalog, getAnnots(&obj));
  obj.free();
  return annots;
}

void Page::addAnnot(Annot *annot) {
  Object obj1;
  Object tmp;
  Ref annotRef = annot->getRef ();

  if (annots.isNull()) {
    Ref annotsRef;
    // page doesn't have annots array,
    // we have to create it

    obj1.initArray(xref);
    obj1.arrayAdd(tmp.initRef (annotRef.num, annotRef.gen));
    tmp.free();

    annotsRef = xref->addIndirectObject (&obj1);
    annots.initRef(annotsRef.num, annotsRef.gen);
    pageObj.dictSet ("Annots", &annots);
    xref->setModifiedObject (&pageObj, pageRef);
  } else {
    getAnnots(&obj1);
    if (obj1.isArray()) {
      obj1.arrayAdd (tmp.initRef (annotRef.num, annotRef.gen));
      if (annots.isRef())
        xref->setModifiedObject (&obj1, annots.getRef());
      else
        xref->setModifiedObject (&pageObj, pageRef);
    }
    obj1.free();
  }

  annot->setPage(&pageRef, num);
}

Links *Page::getLinks(Catalog *catalog) {
  Links *links;
  Object obj;

  links = new Links(getAnnots(&obj), catalog->getBaseURI());
  obj.free();
  return links;
}

void Page::display(OutputDev *out, double hDPI, double vDPI,
		   int rotate, GBool useMediaBox, GBool crop,
		   GBool printing, Catalog *catalog,
		   GBool (*abortCheckCbk)(void *data),
		   void *abortCheckCbkData,
                   GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                   void *annotDisplayDecideCbkData) {
  displaySlice(out, hDPI, vDPI, rotate, useMediaBox, crop, -1, -1, -1, -1, printing, catalog,
	       abortCheckCbk, abortCheckCbkData,
               annotDisplayDecideCbk, annotDisplayDecideCbkData);
}

Gfx *Page::createGfx(OutputDev *out, double hDPI, double vDPI,
		     int rotate, GBool useMediaBox, GBool crop,
		     int sliceX, int sliceY, int sliceW, int sliceH,
		     GBool printing, Catalog *catalog,
		     GBool (*abortCheckCbk)(void *data),
		     void *abortCheckCbkData,
		     GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
		     void *annotDisplayDecideCbkData) {
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

  gfx = new Gfx(xref, out, num, attrs->getResourceDict(), catalog,
		hDPI, vDPI, &box, crop ? cropBox : (PDFRectangle *)NULL,
		rotate, abortCheckCbk, abortCheckCbkData);

  return gfx;
}

void Page::displaySlice(OutputDev *out, double hDPI, double vDPI,
			int rotate, GBool useMediaBox, GBool crop,
			int sliceX, int sliceY, int sliceW, int sliceH,
			GBool printing, Catalog *catalog,
			GBool (*abortCheckCbk)(void *data),
			void *abortCheckCbkData,
                        GBool (*annotDisplayDecideCbk)(Annot *annot, void *user_data),
                        void *annotDisplayDecideCbkData) {
  Gfx *gfx;
  Object obj;
  Annots *annotList;
  int i;
  
  if (!out->checkPageSlice(this, hDPI, vDPI, rotate, useMediaBox, crop,
			   sliceX, sliceY, sliceW, sliceH,
			   printing, catalog,
			   abortCheckCbk, abortCheckCbkData)) {
    return;
  }

  gfx = createGfx(out, hDPI, vDPI, rotate, useMediaBox, crop,
		  sliceX, sliceY, sliceW, sliceH,
		  printing, catalog,
		  abortCheckCbk, abortCheckCbkData,
		  annotDisplayDecideCbk, annotDisplayDecideCbkData);

  contents.fetch(xref, &obj);
  if (!obj.isNull()) {
    gfx->saveState();
    gfx->display(&obj);
    gfx->restoreState();
  }
  obj.free();

  // draw annotations
  annotList = new Annots(xref, catalog, getAnnots(&obj));
  obj.free();
  
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
  delete annotList;

  delete gfx;
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
  colorSpace = GfxColorSpace::parse(&obj1, NULL);
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

void Page::processLinks(OutputDev *out, Catalog *catalog) {
  Links *links;
  int i;

  links = getLinks(catalog);
  for (i = 0; i < links->getNumLinks(); ++i) {
    out->processLink(links->getLink(i), catalog);
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
