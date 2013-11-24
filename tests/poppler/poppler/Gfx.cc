//========================================================================
//
// Gfx.cc
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
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2005-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Thorkild Stray <thorkild@ifi.uio.no>
// Copyright (C) 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006-2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2006, 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2007, 2008 Brad Hards <bradh@kde.org>
// Copyright (C) 2007, 2011 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2007, 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2007 Koji Otani <sho@bbr.jp>
// Copyright (C) 2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009 M Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
// Copyright (C) 2009-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 William Bader <williambader@hotmail.com>
// Copyright (C) 2009, 2010 David Benjamin <davidben@mit.edu>
// Copyright (C) 2010 Nils Höglund <nils.hoglund@gmail.com>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011 Axel Strübing <axel.struebing@freenet.de>
// Copyright (C) 2012 Even Rouault <even.rouault@mines-paris.org>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012 Lu Wang <coolwanglu@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "goo/gmem.h"
#include "goo/GooTimer.h"
#include "goo/GooHash.h"
#include "GlobalParams.h"
#include "CharTypes.h"
#include "Object.h"
#include "PDFDoc.h"
#include "Array.h"
#include "Annot.h"
#include "Dict.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"
#include "GfxFont.h"
#include "GfxState.h"
#include "OutputDev.h"
#include "Page.h"
#include "Annot.h"
#include "Error.h"
#include "Gfx.h"
#include "ProfileData.h"
#include "Catalog.h"
#include "OptionalContent.h"

// the MSVC math.h doesn't define this
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
// constants
//------------------------------------------------------------------------

// Max recursive depth for a function shading fill.
#define functionMaxDepth 6

// Max delta allowed in any color component for a function shading fill.
#define functionColorDelta (dblToCol(1 / 256.0))

// Max number of splits along the t axis for an axial shading fill.
#define axialMaxSplits 256

// Max delta allowed in any color component for an axial shading fill.
#define axialColorDelta (dblToCol(1 / 256.0))

// Max number of splits along the t axis for a radial shading fill.
#define radialMaxSplits 256

// Max delta allowed in any color component for a radial shading fill.
#define radialColorDelta (dblToCol(1 / 256.0))

// Max recursive depth for a Gouraud triangle shading fill.
//
// Triangles will be split at most gouraudMaxDepth times (each time into 4
// smaller ones). That makes pow(4,gouraudMaxDepth) many triangles for
// every triangle.
#define gouraudMaxDepth 6

// Max delta allowed in any color component for a Gouraud triangle
// shading fill.
#define gouraudColorDelta (dblToCol(3. / 256.0))

// Gouraud triangle: if the three color parameters differ by at more than this percend of 
// the total color parameter range, the triangle will be refined
#define gouraudParameterizedColorDelta 5e-3

// Max recursive depth for a patch mesh shading fill.
#define patchMaxDepth 6

// Max delta allowed in any color component for a patch mesh shading
// fill.
#define patchColorDelta (dblToCol((3. / 256.0)))

//------------------------------------------------------------------------
// Operator table
//------------------------------------------------------------------------

#ifdef _MSC_VER // this works around a bug in the VC7 compiler
#  pragma optimize("",off)
#endif

Operator Gfx::opTab[] = {
  {"\"",  3, {tchkNum,    tchkNum,    tchkString},
          &Gfx::opMoveSetShowText},
  {"'",   1, {tchkString},
          &Gfx::opMoveShowText},
  {"B",   0, {tchkNone},
          &Gfx::opFillStroke},
  {"B*",  0, {tchkNone},
          &Gfx::opEOFillStroke},
  {"BDC", 2, {tchkName,   tchkProps},
          &Gfx::opBeginMarkedContent},
  {"BI",  0, {tchkNone},
          &Gfx::opBeginImage},
  {"BMC", 1, {tchkName},
          &Gfx::opBeginMarkedContent},
  {"BT",  0, {tchkNone},
          &Gfx::opBeginText},
  {"BX",  0, {tchkNone},
          &Gfx::opBeginIgnoreUndef},
  {"CS",  1, {tchkName},
          &Gfx::opSetStrokeColorSpace},
  {"DP",  2, {tchkName,   tchkProps},
          &Gfx::opMarkPoint},
  {"Do",  1, {tchkName},
          &Gfx::opXObject},
  {"EI",  0, {tchkNone},
          &Gfx::opEndImage},
  {"EMC", 0, {tchkNone},
          &Gfx::opEndMarkedContent},
  {"ET",  0, {tchkNone},
          &Gfx::opEndText},
  {"EX",  0, {tchkNone},
          &Gfx::opEndIgnoreUndef},
  {"F",   0, {tchkNone},
          &Gfx::opFill},
  {"G",   1, {tchkNum},
          &Gfx::opSetStrokeGray},
  {"ID",  0, {tchkNone},
          &Gfx::opImageData},
  {"J",   1, {tchkInt},
          &Gfx::opSetLineCap},
  {"K",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeCMYKColor},
  {"M",   1, {tchkNum},
          &Gfx::opSetMiterLimit},
  {"MP",  1, {tchkName},
          &Gfx::opMarkPoint},
  {"Q",   0, {tchkNone},
          &Gfx::opRestore},
  {"RG",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeRGBColor},
  {"S",   0, {tchkNone},
          &Gfx::opStroke},
  {"SC",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeColor},
  {"SCN", -33, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN},
          &Gfx::opSetStrokeColorN},
  {"T*",  0, {tchkNone},
          &Gfx::opTextNextLine},
  {"TD",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMoveSet},
  {"TJ",  1, {tchkArray},
          &Gfx::opShowSpaceText},
  {"TL",  1, {tchkNum},
          &Gfx::opSetTextLeading},
  {"Tc",  1, {tchkNum},
          &Gfx::opSetCharSpacing},
  {"Td",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMove},
  {"Tf",  2, {tchkName,   tchkNum},
          &Gfx::opSetFont},
  {"Tj",  1, {tchkString},
          &Gfx::opShowText},
  {"Tm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetTextMatrix},
  {"Tr",  1, {tchkInt},
          &Gfx::opSetTextRender},
  {"Ts",  1, {tchkNum},
          &Gfx::opSetTextRise},
  {"Tw",  1, {tchkNum},
          &Gfx::opSetWordSpacing},
  {"Tz",  1, {tchkNum},
          &Gfx::opSetHorizScaling},
  {"W",   0, {tchkNone},
          &Gfx::opClip},
  {"W*",  0, {tchkNone},
          &Gfx::opEOClip},
  {"b",   0, {tchkNone},
          &Gfx::opCloseFillStroke},
  {"b*",  0, {tchkNone},
          &Gfx::opCloseEOFillStroke},
  {"c",   6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opCurveTo},
  {"cm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opConcat},
  {"cs",  1, {tchkName},
          &Gfx::opSetFillColorSpace},
  {"d",   2, {tchkArray,  tchkNum},
          &Gfx::opSetDash},
  {"d0",  2, {tchkNum,    tchkNum},
          &Gfx::opSetCharWidth},
  {"d1",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetCacheDevice},
  {"f",   0, {tchkNone},
          &Gfx::opFill},
  {"f*",  0, {tchkNone},
          &Gfx::opEOFill},
  {"g",   1, {tchkNum},
          &Gfx::opSetFillGray},
  {"gs",  1, {tchkName},
          &Gfx::opSetExtGState},
  {"h",   0, {tchkNone},
          &Gfx::opClosePath},
  {"i",   1, {tchkNum},
          &Gfx::opSetFlat},
  {"j",   1, {tchkInt},
          &Gfx::opSetLineJoin},
  {"k",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillCMYKColor},
  {"l",   2, {tchkNum,    tchkNum},
          &Gfx::opLineTo},
  {"m",   2, {tchkNum,    tchkNum},
          &Gfx::opMoveTo},
  {"n",   0, {tchkNone},
          &Gfx::opEndPath},
  {"q",   0, {tchkNone},
          &Gfx::opSave},
  {"re",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opRectangle},
  {"rg",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillRGBColor},
  {"ri",  1, {tchkName},
          &Gfx::opSetRenderingIntent},
  {"s",   0, {tchkNone},
          &Gfx::opCloseStroke},
  {"sc",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillColor},
  {"scn", -33, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	        tchkSCN},
          &Gfx::opSetFillColorN},
  {"sh",  1, {tchkName},
          &Gfx::opShFill},
  {"v",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo1},
  {"w",   1, {tchkNum},
          &Gfx::opSetLineWidth},
  {"y",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo2},
};

#ifdef _MSC_VER // this works around a bug in the VC7 compiler
#  pragma optimize("",on)
#endif

#define numOps (sizeof(opTab) / sizeof(Operator))

static inline GBool isSameGfxColor(const GfxColor &colorA, const GfxColor &colorB, Guint nComps, double delta) {
  for (Guint k = 0; k < nComps; ++k) {
    if (abs(colorA.c[k] - colorB.c[k]) > delta) {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------------
// GfxResources
//------------------------------------------------------------------------

GfxResources::GfxResources(XRef *xref, Dict *resDictA, GfxResources *nextA) :
    gStateCache(2, xref) {
  Object obj1, obj2;
  Ref r;

  if (resDictA) {

    // build font dictionary
    Dict *resDict = resDictA->copy(xref);
    fonts = NULL;
    resDict->lookupNF("Font", &obj1);
    if (obj1.isRef()) {
      obj1.fetch(xref, &obj2);
      if (obj2.isDict()) {
	r = obj1.getRef();
	fonts = new GfxFontDict(xref, &r, obj2.getDict());
      }
      obj2.free();
    } else if (obj1.isDict()) {
      fonts = new GfxFontDict(xref, NULL, obj1.getDict());
    }
    obj1.free();

    // get XObject dictionary
    resDict->lookup("XObject", &xObjDict);

    // get color space dictionary
    resDict->lookup("ColorSpace", &colorSpaceDict);

    // get pattern dictionary
    resDict->lookup("Pattern", &patternDict);

    // get shading dictionary
    resDict->lookup("Shading", &shadingDict);

    // get graphics state parameter dictionary
    resDict->lookup("ExtGState", &gStateDict);

    // get properties dictionary
    resDict->lookup("Properties", &propertiesDict);

    delete resDict;
  } else {
    fonts = NULL;
    xObjDict.initNull();
    colorSpaceDict.initNull();
    patternDict.initNull();
    shadingDict.initNull();
    gStateDict.initNull();
    propertiesDict.initNull();
  }

  next = nextA;
}

GfxResources::~GfxResources() {
  if (fonts) {
    delete fonts;
  }
  xObjDict.free();
  colorSpaceDict.free();
  patternDict.free();
  shadingDict.free();
  gStateDict.free();
  propertiesDict.free();
}

GfxFont *GfxResources::lookupFont(char *name) {
  GfxFont *font;
  GfxResources *resPtr;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->fonts) {
      if ((font = resPtr->fonts->lookup(name)))
	return font;
    }
  }
  error(errSyntaxError, -1, "Unknown font tag '{0:s}'", name);
  return NULL;
}

GBool GfxResources::lookupXObject(char *name, Object *obj) {
  GfxResources *resPtr;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->xObjDict.isDict()) {
      if (!resPtr->xObjDict.dictLookup(name, obj)->isNull())
	return gTrue;
      obj->free();
    }
  }
  error(errSyntaxError, -1, "XObject '{0:s}' is unknown", name);
  return gFalse;
}

GBool GfxResources::lookupXObjectNF(char *name, Object *obj) {
  GfxResources *resPtr;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->xObjDict.isDict()) {
      if (!resPtr->xObjDict.dictLookupNF(name, obj)->isNull())
	return gTrue;
      obj->free();
    }
  }
  error(errSyntaxError, -1, "XObject '{0:s}' is unknown", name);
  return gFalse;
}

GBool GfxResources::lookupMarkedContentNF(char *name, Object *obj) {
  GfxResources *resPtr;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->propertiesDict.isDict()) {
      if (!resPtr->propertiesDict.dictLookupNF(name, obj)->isNull())
	return gTrue;
      obj->free();
    }
  }
  error(errSyntaxError, -1, "Marked Content '{0:s}' is unknown", name);
  return gFalse;
}

void GfxResources::lookupColorSpace(const char *name, Object *obj) {
  GfxResources *resPtr;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->colorSpaceDict.isDict()) {
      if (!resPtr->colorSpaceDict.dictLookup(name, obj)->isNull()) {
	return;
      }
      obj->free();
    }
  }
  obj->initNull();
}

GfxPattern *GfxResources::lookupPattern(char *name, OutputDev *out, GfxState *state) {
  GfxResources *resPtr;
  GfxPattern *pattern;
  Object obj;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->patternDict.isDict()) {
      if (!resPtr->patternDict.dictLookup(name, &obj)->isNull()) {
	pattern = GfxPattern::parse(&obj, out, state);
	obj.free();
	return pattern;
      }
      obj.free();
    }
  }
  error(errSyntaxError, -1, "Unknown pattern '{0:s}'", name);
  return NULL;
}

GfxShading *GfxResources::lookupShading(char *name, OutputDev *out, GfxState *state) {
  GfxResources *resPtr;
  GfxShading *shading;
  Object obj;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->shadingDict.isDict()) {
      if (!resPtr->shadingDict.dictLookup(name, &obj)->isNull()) {
	shading = GfxShading::parse(&obj, out, state);
	obj.free();
	return shading;
      }
      obj.free();
    }
  }
  error(errSyntaxError, -1, "ExtGState '{0:s}' is unknown", name);
  return NULL;
}

GBool GfxResources::lookupGState(char *name, Object *obj) {
  if (!lookupGStateNF(name, obj))
    return gFalse;

  if (!obj->isRef())
    return gTrue;
  
  const Ref ref = obj->getRef();
  if (!gStateCache.lookup(ref, obj)->isNull())
    return gTrue;
  obj->free();

  gStateCache.put(ref)->copy(obj);
  return gTrue;
}

GBool GfxResources::lookupGStateNF(char *name, Object *obj) {
  GfxResources *resPtr;

  for (resPtr = this; resPtr; resPtr = resPtr->next) {
    if (resPtr->gStateDict.isDict()) {
      if (!resPtr->gStateDict.dictLookupNF(name, obj)->isNull()) {
	return gTrue;
      }
      obj->free();
    }
  }
  error(errSyntaxError, -1, "ExtGState '{0:s}' is unknown", name);
  return gFalse;
}

//------------------------------------------------------------------------
// Gfx
//------------------------------------------------------------------------

Gfx::Gfx(PDFDoc *docA, OutputDev *outA, int pageNum, Dict *resDict,
	 double hDPI, double vDPI, PDFRectangle *box,
	 PDFRectangle *cropBox, int rotate,
	 GBool (*abortCheckCbkA)(void *data),
	 void *abortCheckCbkDataA, XRef *xrefA)
{
  int i;

  doc = docA;
  xref = (xrefA == NULL) ? doc->getXRef() : xrefA;
  catalog = doc->getCatalog();
  subPage = gFalse;
  printCommands = globalParams->getPrintCommands();
  profileCommands = globalParams->getProfileCommands();
  mcStack = NULL;
  parser = NULL;

  // start the resource stack
  res = new GfxResources(xref, resDict, NULL);

  // initialize
  out = outA;
  state = new GfxState(hDPI, vDPI, box, rotate, out->upsideDown());
  stackHeight = 1;
  pushStateGuard();
  fontChanged = gFalse;
  clip = clipNone;
  ignoreUndef = 0;
  out->startPage(pageNum, state, xref);
  out->setDefaultCTM(state->getCTM());
  out->updateAll(state);
  for (i = 0; i < 6; ++i) {
    baseMatrix[i] = state->getCTM()[i];
  }
  formDepth = 0;
  ocState = gTrue;
  parser = NULL;
  abortCheckCbk = abortCheckCbkA;
  abortCheckCbkData = abortCheckCbkDataA;

  // set crop box
  if (cropBox) {
    state->moveTo(cropBox->x1, cropBox->y1);
    state->lineTo(cropBox->x2, cropBox->y1);
    state->lineTo(cropBox->x2, cropBox->y2);
    state->lineTo(cropBox->x1, cropBox->y2);
    state->closePath();
    state->clip();
    out->clip(state);
    state->clearPath();
  }
#ifdef USE_CMS
  initDisplayProfile();
#endif
}

Gfx::Gfx(PDFDoc *docA, OutputDev *outA, Dict *resDict,
	 PDFRectangle *box, PDFRectangle *cropBox,
	 GBool (*abortCheckCbkA)(void *data),
	 void *abortCheckCbkDataA, XRef *xrefA)
{
  int i;

  doc = docA;
  xref = (xrefA == NULL) ? doc->getXRef() : xrefA;
  catalog = doc->getCatalog();
  subPage = gTrue;
  printCommands = globalParams->getPrintCommands();
  profileCommands = globalParams->getProfileCommands();
  mcStack = NULL;
  parser = NULL;

  // start the resource stack
  res = new GfxResources(xref, resDict, NULL);

  // initialize
  out = outA;
  state = new GfxState(72, 72, box, 0, gFalse);
  stackHeight = 1;
  pushStateGuard();
  fontChanged = gFalse;
  clip = clipNone;
  ignoreUndef = 0;
  for (i = 0; i < 6; ++i) {
    baseMatrix[i] = state->getCTM()[i];
  }
  formDepth = 0;
  ocState = gTrue;
  parser = NULL;
  abortCheckCbk = abortCheckCbkA;
  abortCheckCbkData = abortCheckCbkDataA;

  // set crop box
  if (cropBox) {
    state->moveTo(cropBox->x1, cropBox->y1);
    state->lineTo(cropBox->x2, cropBox->y1);
    state->lineTo(cropBox->x2, cropBox->y2);
    state->lineTo(cropBox->x1, cropBox->y2);
    state->closePath();
    state->clip();
    out->clip(state);
    state->clearPath();
  }
#ifdef USE_CMS
  initDisplayProfile();
#endif
}

#ifdef USE_CMS

#ifdef USE_LCMS1
#include <lcms.h>
#else
#include <lcms2.h>
#define LCMS_FLAGS cmsFLAGS_NOOPTIMIZE
#endif

void Gfx::initDisplayProfile() {
   Object catDict;
   xref->getCatalog(&catDict);
   if (catDict.isDict()) {
     Object outputIntents;
     catDict.dictLookup("OutputIntents", &outputIntents);
     if (outputIntents.isArray() && outputIntents.arrayGetLength() == 1) {
          Object firstElement;
          outputIntents.arrayGet(0, &firstElement);
          if (firstElement.isDict()) {
              Object profile;
              firstElement.dictLookup("DestOutputProfile", &profile);
              if (profile.isStream()) {
                Stream *iccStream = profile.getStream();
                int length = 0;
                Guchar *profBuf = iccStream->toUnsignedChars(&length, 65536, 65536);
                cmsHPROFILE hp = cmsOpenProfileFromMem(profBuf,length);
                if (hp == 0) {
                  error(errSyntaxWarning, -1, "read ICCBased color space profile error");
                } else {
                  state->setDisplayProfile(hp);
                }
                gfree(profBuf);
              }
              profile.free();
          }
          firstElement.free();
     }
     outputIntents.free();
   }
   catDict.free();
}

#endif

Gfx::~Gfx() {
  while (stateGuards.size()) {
    popStateGuard();
  }
  if (!subPage) {
    out->endPage();
  }
  // There shouldn't be more saves, but pop them if there were any
  while (state->hasSaves()) {
    error(errSyntaxError, -1, "Found state under last state guard. Popping.");
    restoreState();
  }
  delete state;
  while (res) {
    popResources();
  }
  while (mcStack) {
    popMarkedContent();
  }
}

void Gfx::display(Object *obj, GBool topLevel) {
  Object obj2;
  int i;

  if (obj->isArray()) {
    for (i = 0; i < obj->arrayGetLength(); ++i) {
      obj->arrayGet(i, &obj2);
      if (!obj2.isStream()) {
	error(errSyntaxError, -1, "Weird page contents");
	obj2.free();
	return;
      }
      obj2.free();
    }
  } else if (!obj->isStream()) {
    error(errSyntaxError, -1, "Weird page contents");
    return;
  }
  parser = new Parser(xref, new Lexer(xref, obj), gFalse);
  go(topLevel);
  delete parser;
  parser = NULL;
}

void Gfx::go(GBool topLevel) {
  Object obj;
  Object args[maxArgs];
  int numArgs, i;
  int lastAbortCheck;

  // scan a sequence of objects
  pushStateGuard();
  updateLevel = 1; // make sure even empty pages trigger a call to dump()
  lastAbortCheck = 0;
  numArgs = 0;
  parser->getObj(&obj);
  while (!obj.isEOF()) {
    commandAborted = gFalse;

    // got a command - execute it
    if (obj.isCmd()) {
      if (printCommands) {
	obj.print(stdout);
	for (i = 0; i < numArgs; ++i) {
	  printf(" ");
	  args[i].print(stdout);
	}
	printf("\n");
	fflush(stdout);
      }
      GooTimer timer;

      // Run the operation
      execOp(&obj, args, numArgs);

      // Update the profile information
      if (profileCommands) {
	GooHash *hash;

	hash = out->getProfileHash ();
	if (hash) {
	  GooString *cmd_g;
	  ProfileData *data_p;

	  cmd_g = new GooString (obj.getCmd());
	  data_p = (ProfileData *)hash->lookup (cmd_g);
	  if (data_p == NULL) {
	    data_p = new ProfileData();
	    hash->add (cmd_g, data_p);
	  }
	  
	  data_p->addElement(timer.getElapsed ());
	}
      }
      obj.free();
      for (i = 0; i < numArgs; ++i)
	args[i].free();
      numArgs = 0;

      // periodically update display
      if (++updateLevel >= 20000) {
	out->dump();
	updateLevel = 0;
      }

      // did the command throw an exception
      if (commandAborted) {
	// don't propogate; recursive drawing comes from Form XObjects which
	// should probably be drawn in a separate context anyway for caching
	commandAborted = gFalse;
	break;
      }

      // check for an abort
      if (abortCheckCbk) {
	if (updateLevel - lastAbortCheck > 10) {
	  if ((*abortCheckCbk)(abortCheckCbkData)) {
	    break;
	  }
	  lastAbortCheck = updateLevel;
	}
      }

    // got an argument - save it
    } else if (numArgs < maxArgs) {
      args[numArgs++] = obj;

    // too many arguments - something is wrong
    } else {
      error(errSyntaxError, getPos(), "Too many args in content stream");
      if (printCommands) {
	printf("throwing away arg: ");
	obj.print(stdout);
	printf("\n");
	fflush(stdout);
      }
      obj.free();
    }

    // grab the next object
    parser->getObj(&obj);
  }
  obj.free();

  // args at end with no command
  if (numArgs > 0) {
    error(errSyntaxError, getPos(), "Leftover args in content stream");
    if (printCommands) {
      printf("%d leftovers:", numArgs);
      for (i = 0; i < numArgs; ++i) {
	printf(" ");
	args[i].print(stdout);
      }
      printf("\n");
      fflush(stdout);
    }
    for (i = 0; i < numArgs; ++i)
      args[i].free();
  }

  popStateGuard();

  // update display
  if (topLevel && updateLevel > 0) {
    out->dump();
  }
}

void Gfx::execOp(Object *cmd, Object args[], int numArgs) {
  Operator *op;
  char *name;
  Object *argPtr;
  int i;

  // find operator
  name = cmd->getCmd();
  if (!(op = findOp(name))) {
    if (ignoreUndef == 0)
      error(errSyntaxError, getPos(), "Unknown operator '{0:s}'", name);
    return;
  }

  // type check args
  argPtr = args;
  if (op->numArgs >= 0) {
    if (numArgs < op->numArgs) {
      error(errSyntaxError, getPos(), "Too few ({0:d}) args to '{1:s}' operator", numArgs, name);
      commandAborted = gTrue;
      return;
    }
    if (numArgs > op->numArgs) {
#if 0
      error(errSyntaxWarning, getPos(),
	    "Too many ({0:d}) args to '{1:s}' operator", numArgs, name);
#endif
      argPtr += numArgs - op->numArgs;
      numArgs = op->numArgs;
    }
  } else {
    if (numArgs > -op->numArgs) {
      error(errSyntaxError, getPos(), "Too many ({0:d}) args to '{1:s}' operator",
	    numArgs, name);
      return;
    }
  }
  for (i = 0; i < numArgs; ++i) {
    if (!checkArg(&argPtr[i], op->tchk[i])) {
      error(errSyntaxError, getPos(), "Arg #{0:d} to '{1:s}' operator is wrong type ({2:s})",
	    i, name, argPtr[i].getTypeName());
      return;
    }
  }

  // do it
  (this->*op->func)(argPtr, numArgs);
}

Operator *Gfx::findOp(char *name) {
  int a, b, m, cmp;

  a = -1;
  b = numOps;
  cmp = 0; // make gcc happy
  // invariant: opTab[a] < name < opTab[b]
  while (b - a > 1) {
    m = (a + b) / 2;
    cmp = strcmp(opTab[m].name, name);
    if (cmp < 0)
      a = m;
    else if (cmp > 0)
      b = m;
    else
      a = b = m;
  }
  if (cmp != 0)
    return NULL;
  return &opTab[a];
}

GBool Gfx::checkArg(Object *arg, TchkType type) {
  switch (type) {
  case tchkBool:   return arg->isBool();
  case tchkInt:    return arg->isInt();
  case tchkNum:    return arg->isNum();
  case tchkString: return arg->isString();
  case tchkName:   return arg->isName();
  case tchkArray:  return arg->isArray();
  case tchkProps:  return arg->isDict() || arg->isName();
  case tchkSCN:    return arg->isNum() || arg->isName();
  case tchkNone:   return gFalse;
  }
  return gFalse;
}

Goffset Gfx::getPos() {
  return parser ? parser->getPos() : -1;
}

//------------------------------------------------------------------------
// graphics state operators
//------------------------------------------------------------------------

void Gfx::opSave(Object args[], int numArgs) {
  saveState();
}

void Gfx::opRestore(Object args[], int numArgs) {
  restoreState();
}

void Gfx::opConcat(Object args[], int numArgs) {
  state->concatCTM(args[0].getNum(), args[1].getNum(),
		   args[2].getNum(), args[3].getNum(),
		   args[4].getNum(), args[5].getNum());
  out->updateCTM(state, args[0].getNum(), args[1].getNum(),
		 args[2].getNum(), args[3].getNum(),
		 args[4].getNum(), args[5].getNum());
  fontChanged = gTrue;
}

void Gfx::opSetDash(Object args[], int numArgs) {
  Array *a;
  int length;
  Object obj;
  double *dash;
  int i;

  a = args[0].getArray();
  length = a->getLength();
  if (length == 0) {
    dash = NULL;
  } else {
    dash = (double *)gmallocn(length, sizeof(double));
    for (i = 0; i < length; ++i) {
      dash[i] = a->get(i, &obj)->getNum();
      obj.free();
    }
  }
  state->setLineDash(dash, length, args[1].getNum());
  out->updateLineDash(state);
}

void Gfx::opSetFlat(Object args[], int numArgs) {
  state->setFlatness((int)args[0].getNum());
  out->updateFlatness(state);
}

void Gfx::opSetLineJoin(Object args[], int numArgs) {
  state->setLineJoin(args[0].getInt());
  out->updateLineJoin(state);
}

void Gfx::opSetLineCap(Object args[], int numArgs) {
  state->setLineCap(args[0].getInt());
  out->updateLineCap(state);
}

void Gfx::opSetMiterLimit(Object args[], int numArgs) {
  state->setMiterLimit(args[0].getNum());
  out->updateMiterLimit(state);
}

void Gfx::opSetLineWidth(Object args[], int numArgs) {
  state->setLineWidth(args[0].getNum());
  out->updateLineWidth(state);
}

void Gfx::opSetExtGState(Object args[], int numArgs) {
  Object obj1, obj2, obj3, obj4, obj5;
  Object args2[2];
  GfxBlendMode mode;
  GBool haveFillOP;
  Function *funcs[4];
  GfxColor backdropColor;
  GBool haveBackdropColor;
  GfxColorSpace *blendingColorSpace;
  GBool alpha, isolated, knockout;
  double opac;
  int i;

  if (!res->lookupGState(args[0].getName(), &obj1)) {
    return;
  }
  if (!obj1.isDict()) {
    error(errSyntaxError, getPos(), "ExtGState '{0:s}' is wrong type", args[0].getName());
    obj1.free();
    return;
  }
  if (printCommands) {
    printf("  gfx state dict: ");
    obj1.print();
    printf("\n");
  }

  // parameters that are also set by individual PDF operators
  if (obj1.dictLookup("LW", &obj2)->isNum()) {
    opSetLineWidth(&obj2, 1);
  }
  obj2.free();
  if (obj1.dictLookup("LC", &obj2)->isInt()) {
    opSetLineCap(&obj2, 1);
  }
  obj2.free();
  if (obj1.dictLookup("LJ", &obj2)->isInt()) {
    opSetLineJoin(&obj2, 1);
  }
  obj2.free();
  if (obj1.dictLookup("ML", &obj2)->isNum()) {
    opSetMiterLimit(&obj2, 1);
  }
  obj2.free();
  if (obj1.dictLookup("D", &obj2)->isArray() &&
      obj2.arrayGetLength() == 2) {
    obj2.arrayGet(0, &args2[0]);
    obj2.arrayGet(1, &args2[1]);
    if (args2[0].isArray() && args2[1].isNum()) {
      opSetDash(args2, 2);
    }
    args2[0].free();
    args2[1].free();
  }
  obj2.free();
#if 0 //~ need to add a new version of GfxResources::lookupFont() that
      //~ takes an indirect ref instead of a name
  if (obj1.dictLookup("Font", &obj2)->isArray() &&
      obj2.arrayGetLength() == 2) {
    obj2.arrayGet(0, &args2[0]);
    obj2.arrayGet(1, &args2[1]);
    if (args2[0].isDict() && args2[1].isNum()) {
      opSetFont(args2, 2);
    }
    args2[0].free();
    args2[1].free();
  }
  obj2.free();
#endif
  if (obj1.dictLookup("FL", &obj2)->isNum()) {
    opSetFlat(&obj2, 1);
  }
  obj2.free();

  // transparency support: blend mode, fill/stroke opacity
  if (!obj1.dictLookup("BM", &obj2)->isNull()) {
    if (state->parseBlendMode(&obj2, &mode)) {
      state->setBlendMode(mode);
      out->updateBlendMode(state);
    } else {
      error(errSyntaxError, getPos(), "Invalid blend mode in ExtGState");
    }
  }
  obj2.free();
  if (obj1.dictLookup("ca", &obj2)->isNum()) {
    opac = obj2.getNum();
    state->setFillOpacity(opac < 0 ? 0 : opac > 1 ? 1 : opac);
    out->updateFillOpacity(state);
  }
  obj2.free();
  if (obj1.dictLookup("CA", &obj2)->isNum()) {
    opac = obj2.getNum();
    state->setStrokeOpacity(opac < 0 ? 0 : opac > 1 ? 1 : opac);
    out->updateStrokeOpacity(state);
  }
  obj2.free();

  // fill/stroke overprint, overprint mode
  if ((haveFillOP = (obj1.dictLookup("op", &obj2)->isBool()))) {
    state->setFillOverprint(obj2.getBool());
    out->updateFillOverprint(state);
  }
  obj2.free();
  if (obj1.dictLookup("OP", &obj2)->isBool()) {
    state->setStrokeOverprint(obj2.getBool());
    out->updateStrokeOverprint(state);
    if (!haveFillOP) {
      state->setFillOverprint(obj2.getBool());
      out->updateFillOverprint(state);
    }
  }
  obj2.free();
  if (obj1.dictLookup("OPM", &obj2)->isInt()) {
    state->setOverprintMode(obj2.getInt());
    out->updateOverprintMode(state);
  }
  obj2.free();

  // stroke adjust
  if (obj1.dictLookup("SA", &obj2)->isBool()) {
    state->setStrokeAdjust(obj2.getBool());
    out->updateStrokeAdjust(state);
  }
  obj2.free();

  // transfer function
  if (obj1.dictLookup("TR2", &obj2)->isNull()) {
    obj2.free();
    obj1.dictLookup("TR", &obj2);
  }
  if (obj2.isName("Default") ||
      obj2.isName("Identity")) {
    funcs[0] = funcs[1] = funcs[2] = funcs[3] = NULL;
    state->setTransfer(funcs);
    out->updateTransfer(state);
  } else if (obj2.isArray() && obj2.arrayGetLength() == 4) {
    for (i = 0; i < 4; ++i) {
      obj2.arrayGet(i, &obj3);
      funcs[i] = Function::parse(&obj3);
      obj3.free();
      if (!funcs[i]) {
	break;
      }
    }
    if (i == 4) {
      state->setTransfer(funcs);
      out->updateTransfer(state);
    }
  } else if (obj2.isName() || obj2.isDict() || obj2.isStream()) {
    if ((funcs[0] = Function::parse(&obj2))) {
      funcs[1] = funcs[2] = funcs[3] = NULL;
      state->setTransfer(funcs);
      out->updateTransfer(state);
    }
  } else if (!obj2.isNull()) {
    error(errSyntaxError, getPos(), "Invalid transfer function in ExtGState");
  }
  obj2.free();

  // alpha is shape
  if (obj1.dictLookup("AIS", &obj2)->isBool()) {
    state->setAlphaIsShape(obj2.getBool());
    out->updateAlphaIsShape(state);
  }
  obj2.free();

  // text knockout
  if (obj1.dictLookup("TK", &obj2)->isBool()) {
    state->setTextKnockout(obj2.getBool());
    out->updateTextKnockout(state);
  }
  obj2.free();

  // soft mask
  if (!obj1.dictLookup("SMask", &obj2)->isNull()) {
    if (obj2.isName("None")) {
      out->clearSoftMask(state);
    } else if (obj2.isDict()) {
      if (obj2.dictLookup("S", &obj3)->isName("Alpha")) {
	alpha = gTrue;
      } else { // "Luminosity"
	alpha = gFalse;
      }
      obj3.free();
      funcs[0] = NULL;
      if (!obj2.dictLookup("TR", &obj3)->isNull()) {
	if (obj3.isName("Default") ||
	    obj3.isName("Identity")) {
	  funcs[0] = NULL;
	} else {
	  funcs[0] = Function::parse(&obj3);
      if (funcs[0] == NULL ||
          funcs[0]->getInputSize() != 1 ||
          funcs[0]->getOutputSize() != 1) {
	    error(errSyntaxError, getPos(),
		  "Invalid transfer function in soft mask in ExtGState");
	    delete funcs[0];
	    funcs[0] = NULL;
	  }
	}
      }
      obj3.free();
      if ((haveBackdropColor = obj2.dictLookup("BC", &obj3)->isArray())) {
	for (i = 0; i < gfxColorMaxComps; ++i) {
	  backdropColor.c[i] = 0;
	}
	for (i = 0; i < obj3.arrayGetLength() && i < gfxColorMaxComps; ++i) {
	  obj3.arrayGet(i, &obj4);
	  if (obj4.isNum()) {
	    backdropColor.c[i] = dblToCol(obj4.getNum());
	  }
	  obj4.free();
	}
      }
      obj3.free();
      if (obj2.dictLookup("G", &obj3)->isStream()) {
	if (obj3.streamGetDict()->lookup("Group", &obj4)->isDict()) {
	  blendingColorSpace = NULL;
	  isolated = knockout = gFalse;
	  if (!obj4.dictLookup("CS", &obj5)->isNull()) {
	    blendingColorSpace = GfxColorSpace::parse(&obj5, out, state);
	  }
	  obj5.free();
	  if (obj4.dictLookup("I", &obj5)->isBool()) {
	    isolated = obj5.getBool();
	  }
	  obj5.free();
	  if (obj4.dictLookup("K", &obj5)->isBool()) {
	    knockout = obj5.getBool();
	  }
	  obj5.free();
	  if (!haveBackdropColor) {
	    if (blendingColorSpace) {
	      blendingColorSpace->getDefaultColor(&backdropColor);
	    } else {
	      //~ need to get the parent or default color space (?)
	      for (i = 0; i < gfxColorMaxComps; ++i) {
		backdropColor.c[i] = 0;
	      }
	    }
	  }
	  doSoftMask(&obj3, alpha, blendingColorSpace,
		     isolated, knockout, funcs[0], &backdropColor);
	  if (funcs[0]) {
	    delete funcs[0];
	  }
	} else {
	  error(errSyntaxError, getPos(), "Invalid soft mask in ExtGState - missing group");
	}
	obj4.free();
      } else {
	error(errSyntaxError, getPos(), "Invalid soft mask in ExtGState - missing group");
      }
      obj3.free();
    } else if (!obj2.isNull()) {
      error(errSyntaxError, getPos(), "Invalid soft mask in ExtGState");
    }
  }
  obj2.free();
  if (obj1.dictLookup("Font", &obj2)->isArray()) {
    GfxFont *font;
    if (obj2.arrayGetLength() == 2) {
      Object fargs0, fargs1;

      obj2.arrayGetNF(0,&fargs0);
      obj2.arrayGet(1,&fargs1);
      if (fargs0.isRef() && fargs1.isNum()) {
	Object fobj;
	Ref r;

	fargs0.fetch(xref, &fobj);
	if (fobj.isDict()) {
	  r = fargs0.getRef();
	  font = GfxFont::makeFont(xref,args[0].getName(),r,fobj.getDict());
	  state->setFont(font,fargs1.getNum());
	  fontChanged = gTrue;
	}
	fobj.free();
      }
      fargs0.free();
      fargs1.free();
    } else {
      error(errSyntaxError, getPos(), "Number of args mismatch for /Font in ExtGState");
    }
  }
  obj2.free();
  if (obj1.dictLookup("LW", &obj2)->isNum()) {
    opSetLineWidth(&obj2,1);
  }
  obj2.free();
  if (obj1.dictLookup("LC", &obj2)->isInt()) {
    opSetLineCap(&obj2,1);
  }
  obj2.free();
  if (obj1.dictLookup("LJ", &obj2)->isInt()) {
    opSetLineJoin(&obj2,1);
  }
  obj2.free();
  if (obj1.dictLookup("ML", &obj2)->isNum()) {
    opSetMiterLimit(&obj2,1);
  }
  obj2.free();
  if (obj1.dictLookup("D", &obj2)->isArray()) {
    if (obj2.arrayGetLength() == 2) {
      Object dargs[2];

      obj2.arrayGetNF(0,&dargs[0]);
      obj2.arrayGet(1,&dargs[1]);
      if (dargs[0].isArray() && dargs[1].isInt()) {
	opSetDash(dargs,2);
      }
      dargs[0].free();
      dargs[1].free();
    } else {
      error(errSyntaxError, getPos(), "Number of args mismatch for /D in ExtGState");
    }
  }
  obj2.free();
  if (obj1.dictLookup("RI", &obj2)->isName()) {
    opSetRenderingIntent(&obj2,1);
  }
  obj2.free();
  if (obj1.dictLookup("FL", &obj2)->isNum()) {
    opSetFlat(&obj2,1);
  }
  obj2.free();

  obj1.free();
}

void Gfx::doSoftMask(Object *str, GBool alpha,
		     GfxColorSpace *blendingColorSpace,
		     GBool isolated, GBool knockout,
		     Function *transferFunc, GfxColor *backdropColor) {
  Dict *dict, *resDict;
  double m[6], bbox[4];
  Object obj1, obj2;
  int i;

  // check for excessive recursion
  if (formDepth > 20) {
    return;
  }

  // get stream dict
  dict = str->streamGetDict();

  // check form type
  dict->lookup("FormType", &obj1);
  if (!(obj1.isNull() || (obj1.isInt() && obj1.getInt() == 1))) {
    error(errSyntaxError, getPos(), "Unknown form type");
  }
  obj1.free();

  // get bounding box
  dict->lookup("BBox", &obj1);
  if (!obj1.isArray()) {
    obj1.free();
    error(errSyntaxError, getPos(), "Bad form bounding box");
    return;
  }
  for (i = 0; i < 4; ++i) {
    obj1.arrayGet(i, &obj2);
    if (likely(obj2.isNum())) bbox[i] = obj2.getNum();
    else {
      obj2.free();
      obj1.free();
      error(errSyntaxError, getPos(), "Bad form bounding box (non number)");
      return;
    }
    obj2.free();
  }
  obj1.free();

  // get matrix
  dict->lookup("Matrix", &obj1);
  if (obj1.isArray()) {
    for (i = 0; i < 6; ++i) {
      obj1.arrayGet(i, &obj2);
      if (likely(obj2.isNum())) m[i] = obj2.getNum();
      else m[i] = 0;
      obj2.free();
    }
  } else {
    m[0] = 1; m[1] = 0;
    m[2] = 0; m[3] = 1;
    m[4] = 0; m[5] = 0;
  }
  obj1.free();

  // get resources
  dict->lookup("Resources", &obj1);
  resDict = obj1.isDict() ? obj1.getDict() : (Dict *)NULL;

  // draw it
  ++formDepth;
  drawForm(str, resDict, m, bbox, gTrue, gTrue,
	  blendingColorSpace, isolated, knockout,
	  alpha, transferFunc, backdropColor);
  --formDepth;

  if (blendingColorSpace) {
    delete blendingColorSpace;
  }
  obj1.free();
}

void Gfx::opSetRenderingIntent(Object args[], int numArgs) {
  state->setRenderingIntent(args[0].getName());
}

//------------------------------------------------------------------------
// color operators
//------------------------------------------------------------------------

void Gfx::opSetFillGray(Object args[], int numArgs) {
  GfxColor color;
  GfxColorSpace *colorSpace = NULL;
  Object obj;

  state->setFillPattern(NULL);
  res->lookupColorSpace("DefaultGray", &obj);
  if (!obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  if (colorSpace == NULL) {
    colorSpace = new GfxDeviceGrayColorSpace();
  }
  obj.free();
  state->setFillColorSpace(colorSpace);
  out->updateFillColorSpace(state);
  color.c[0] = dblToCol(args[0].getNum());
  state->setFillColor(&color);
  out->updateFillColor(state);
}

void Gfx::opSetStrokeGray(Object args[], int numArgs) {
  GfxColor color;
  GfxColorSpace *colorSpace = NULL;
  Object obj;

  state->setStrokePattern(NULL);
  res->lookupColorSpace("DefaultGray", &obj);
  if (!obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  if (colorSpace == NULL) {
    colorSpace = new GfxDeviceGrayColorSpace();
  }
  obj.free();
  state->setStrokeColorSpace(colorSpace);
  out->updateStrokeColorSpace(state);
  color.c[0] = dblToCol(args[0].getNum());
  state->setStrokeColor(&color);
  out->updateStrokeColor(state);
}

void Gfx::opSetFillCMYKColor(Object args[], int numArgs) {
  GfxColor color;
  GfxColorSpace *colorSpace = NULL;
  Object obj;
  int i;

  res->lookupColorSpace("DefaultCMYK", &obj);
  if (!obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  if (colorSpace == NULL) {
    colorSpace = new GfxDeviceCMYKColorSpace();
  }
  obj.free();
  state->setFillPattern(NULL);
  state->setFillColorSpace(colorSpace);
  out->updateFillColorSpace(state);
  for (i = 0; i < 4; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setFillColor(&color);
  out->updateFillColor(state);
}

void Gfx::opSetStrokeCMYKColor(Object args[], int numArgs) {
  GfxColor color;
  GfxColorSpace *colorSpace = NULL;
  Object obj;
  int i;

  state->setStrokePattern(NULL);
  res->lookupColorSpace("DefaultCMYK", &obj);
  if (!obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  if (colorSpace == NULL) {
    colorSpace = new GfxDeviceCMYKColorSpace();
  }
  obj.free();
  state->setStrokeColorSpace(colorSpace);
  out->updateStrokeColorSpace(state);
  for (i = 0; i < 4; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setStrokeColor(&color);
  out->updateStrokeColor(state);
}

void Gfx::opSetFillRGBColor(Object args[], int numArgs) {
  Object obj;
  GfxColorSpace *colorSpace = NULL;
  GfxColor color;
  int i;

  state->setFillPattern(NULL);
  res->lookupColorSpace("DefaultRGB", &obj);
  if (!obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  if (colorSpace == NULL) {
    colorSpace = new GfxDeviceRGBColorSpace();
  }
  obj.free();
  state->setFillColorSpace(colorSpace);
  out->updateFillColorSpace(state);
  for (i = 0; i < 3; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setFillColor(&color);
  out->updateFillColor(state);
}

void Gfx::opSetStrokeRGBColor(Object args[], int numArgs) {
  Object obj;
  GfxColorSpace *colorSpace = NULL;
  GfxColor color;
  int i;

  state->setStrokePattern(NULL);
  res->lookupColorSpace("DefaultRGB", &obj);
  if (!obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  if (colorSpace == NULL) {
    colorSpace = new GfxDeviceRGBColorSpace();
  }
  obj.free();
  state->setStrokeColorSpace(colorSpace);
  out->updateStrokeColorSpace(state);
  for (i = 0; i < 3; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setStrokeColor(&color);
  out->updateStrokeColor(state);
}

void Gfx::opSetFillColorSpace(Object args[], int numArgs) {
  Object obj;
  GfxColorSpace *colorSpace;
  GfxColor color;

  res->lookupColorSpace(args[0].getName(), &obj);
  if (obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&args[0], out, state);
  } else {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  obj.free();
  if (colorSpace) {
    state->setFillPattern(NULL);
    state->setFillColorSpace(colorSpace);
    out->updateFillColorSpace(state);
    colorSpace->getDefaultColor(&color);
    state->setFillColor(&color);
    out->updateFillColor(state);
  } else {
    error(errSyntaxError, getPos(), "Bad color space (fill)");
  }
}

void Gfx::opSetStrokeColorSpace(Object args[], int numArgs) {
  Object obj;
  GfxColorSpace *colorSpace;
  GfxColor color;

  state->setStrokePattern(NULL);
  res->lookupColorSpace(args[0].getName(), &obj);
  if (obj.isNull()) {
    colorSpace = GfxColorSpace::parse(&args[0], out, state);
  } else {
    colorSpace = GfxColorSpace::parse(&obj, out, state);
  }
  obj.free();
  if (colorSpace) {
    state->setStrokeColorSpace(colorSpace);
    out->updateStrokeColorSpace(state);
    colorSpace->getDefaultColor(&color);
    state->setStrokeColor(&color);
    out->updateStrokeColor(state);
  } else {
    error(errSyntaxError, getPos(), "Bad color space (stroke)");
  }
}

void Gfx::opSetFillColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  if (numArgs != state->getFillColorSpace()->getNComps()) {
    error(errSyntaxError, getPos(), "Incorrect number of arguments in 'sc' command");
    return;
  }
  state->setFillPattern(NULL);
  for (i = 0; i < numArgs; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setFillColor(&color);
  out->updateFillColor(state);
}

void Gfx::opSetStrokeColor(Object args[], int numArgs) {
  GfxColor color;
  int i;

  if (numArgs != state->getStrokeColorSpace()->getNComps()) {
    error(errSyntaxError, getPos(), "Incorrect number of arguments in 'SC' command");
    return;
  }
  state->setStrokePattern(NULL);
  for (i = 0; i < numArgs; ++i) {
    color.c[i] = dblToCol(args[i].getNum());
  }
  state->setStrokeColor(&color);
  out->updateStrokeColor(state);
}

void Gfx::opSetFillColorN(Object args[], int numArgs) {
  GfxColor color;
  GfxPattern *pattern;
  int i;

  if (state->getFillColorSpace()->getMode() == csPattern) {
    if (numArgs > 1) {
      if (!((GfxPatternColorSpace *)state->getFillColorSpace())->getUnder() ||
	  numArgs - 1 != ((GfxPatternColorSpace *)state->getFillColorSpace())
	                     ->getUnder()->getNComps()) {
	error(errSyntaxError, getPos(), "Incorrect number of arguments in 'scn' command");
	return;
      }
      for (i = 0; i < numArgs - 1 && i < gfxColorMaxComps; ++i) {
	if (args[i].isNum()) {
	  color.c[i] = dblToCol(args[i].getNum());
	} else {
	  color.c[i] = 0; // TODO Investigate if this is what Adobe does
	}
      }
      state->setFillColor(&color);
      out->updateFillColor(state);
    }
    if (numArgs > 0) {
      if (args[numArgs-1].isName() &&
	  (pattern = res->lookupPattern(args[numArgs-1].getName(), out, state))) {
        state->setFillPattern(pattern);
      }
    }

  } else {
    if (numArgs != state->getFillColorSpace()->getNComps()) {
      error(errSyntaxError, getPos(), "Incorrect number of arguments in 'scn' command");
      return;
    }
    state->setFillPattern(NULL);
    for (i = 0; i < numArgs && i < gfxColorMaxComps; ++i) {
      if (args[i].isNum()) {
	color.c[i] = dblToCol(args[i].getNum());
      } else {
        color.c[i] = 0; // TODO Investigate if this is what Adobe does
      }
    }
    state->setFillColor(&color);
    out->updateFillColor(state);
  }
}

void Gfx::opSetStrokeColorN(Object args[], int numArgs) {
  GfxColor color;
  GfxPattern *pattern;
  int i;

  if (state->getStrokeColorSpace()->getMode() == csPattern) {
    if (numArgs > 1) {
      if (!((GfxPatternColorSpace *)state->getStrokeColorSpace())
	       ->getUnder() ||
	  numArgs - 1 != ((GfxPatternColorSpace *)state->getStrokeColorSpace())
	                     ->getUnder()->getNComps()) {
	error(errSyntaxError, getPos(), "Incorrect number of arguments in 'SCN' command");
	return;
      }
      for (i = 0; i < numArgs - 1 && i < gfxColorMaxComps; ++i) {
	if (args[i].isNum()) {
	  color.c[i] = dblToCol(args[i].getNum());
	} else {
	  color.c[i] = 0; // TODO Investigate if this is what Adobe does
	}
      }
      state->setStrokeColor(&color);
      out->updateStrokeColor(state);
    }
    if (unlikely(numArgs <= 0)) {
      error(errSyntaxError, getPos(), "Incorrect number of arguments in 'SCN' command");
      return;
    }
    if (args[numArgs-1].isName() &&
	(pattern = res->lookupPattern(args[numArgs-1].getName(), out, state))) {
      state->setStrokePattern(pattern);
    }

  } else {
    if (numArgs != state->getStrokeColorSpace()->getNComps()) {
      error(errSyntaxError, getPos(), "Incorrect number of arguments in 'SCN' command");
      return;
    }
    state->setStrokePattern(NULL);
    for (i = 0; i < numArgs && i < gfxColorMaxComps; ++i) {
      if (args[i].isNum()) {
	color.c[i] = dblToCol(args[i].getNum());
      } else {
        color.c[i] = 0; // TODO Investigate if this is what Adobe does
      }
    }
    state->setStrokeColor(&color);
    out->updateStrokeColor(state);
  }
}

//------------------------------------------------------------------------
// path segment operators
//------------------------------------------------------------------------

void Gfx::opMoveTo(Object args[], int numArgs) {
  state->moveTo(args[0].getNum(), args[1].getNum());
}

void Gfx::opLineTo(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    error(errSyntaxError, getPos(), "No current point in lineto");
    return;
  }
  state->lineTo(args[0].getNum(), args[1].getNum());
}

void Gfx::opCurveTo(Object args[], int numArgs) {
  double x1, y1, x2, y2, x3, y3;

  if (!state->isCurPt()) {
    error(errSyntaxError, getPos(), "No current point in curveto");
    return;
  }
  x1 = args[0].getNum();
  y1 = args[1].getNum();
  x2 = args[2].getNum();
  y2 = args[3].getNum();
  x3 = args[4].getNum();
  y3 = args[5].getNum();
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void Gfx::opCurveTo1(Object args[], int numArgs) {
  double x1, y1, x2, y2, x3, y3;

  if (!state->isCurPt()) {
    error(errSyntaxError, getPos(), "No current point in curveto1");
    return;
  }
  x1 = state->getCurX();
  y1 = state->getCurY();
  x2 = args[0].getNum();
  y2 = args[1].getNum();
  x3 = args[2].getNum();
  y3 = args[3].getNum();
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void Gfx::opCurveTo2(Object args[], int numArgs) {
  double x1, y1, x2, y2, x3, y3;

  if (!state->isCurPt()) {
    error(errSyntaxError, getPos(), "No current point in curveto2");
    return;
  }
  x1 = args[0].getNum();
  y1 = args[1].getNum();
  x2 = args[2].getNum();
  y2 = args[3].getNum();
  x3 = x2;
  y3 = y2;
  state->curveTo(x1, y1, x2, y2, x3, y3);
}

void Gfx::opRectangle(Object args[], int numArgs) {
  double x, y, w, h;

  x = args[0].getNum();
  y = args[1].getNum();
  w = args[2].getNum();
  h = args[3].getNum();
  state->moveTo(x, y);
  state->lineTo(x + w, y);
  state->lineTo(x + w, y + h);
  state->lineTo(x, y + h);
  state->closePath();
}

void Gfx::opClosePath(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    error(errSyntaxError, getPos(), "No current point in closepath");
    return;
  }
  state->closePath();
}

//------------------------------------------------------------------------
// path painting operators
//------------------------------------------------------------------------

void Gfx::opEndPath(Object args[], int numArgs) {
  doEndPath();
}

void Gfx::opStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in stroke");
    return;
  }
  if (state->isPath()) {
    if (ocState) {
      if (state->getStrokeColorSpace()->getMode() == csPattern) {
	doPatternStroke();
      } else {
	out->stroke(state);
      }
    }
  }
  doEndPath();
}

void Gfx::opCloseStroke(Object * /*args[]*/, int /*numArgs*/) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in closepath/stroke");
    return;
  }
  if (state->isPath()) {
    state->closePath();
    if (ocState) {
      if (state->getStrokeColorSpace()->getMode() == csPattern) {
	doPatternStroke();
      } else {
	out->stroke(state);
      }
    }
  }
  doEndPath();
}

void Gfx::opFill(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in fill");
    return;
  }
  if (state->isPath()) {
    if (ocState) {
      if (state->getFillColorSpace()->getMode() == csPattern) {
	doPatternFill(gFalse);
      } else {
	out->fill(state);
      }
    }
  }
  doEndPath();
}

void Gfx::opEOFill(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in eofill");
    return;
  }
  if (state->isPath()) {
    if (ocState) {
      if (state->getFillColorSpace()->getMode() == csPattern) {
	doPatternFill(gTrue);
      } else {
	out->eoFill(state);
      }
    }
  }
  doEndPath();
}

void Gfx::opFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in fill/stroke");
    return;
  }
  if (state->isPath()) {
    if (ocState) {
      if (state->getFillColorSpace()->getMode() == csPattern) {
	doPatternFill(gFalse);
      } else {
	out->fill(state);
      }
      if (state->getStrokeColorSpace()->getMode() == csPattern) {
	doPatternStroke();
      } else {
	out->stroke(state);
      }
    }
  }
  doEndPath();
}

void Gfx::opCloseFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in closepath/fill/stroke");
    return;
  }
  if (state->isPath()) {
    state->closePath();
    if (ocState) {
      if (state->getFillColorSpace()->getMode() == csPattern) {
	doPatternFill(gFalse);
      } else {
	out->fill(state);
      }
      if (state->getStrokeColorSpace()->getMode() == csPattern) {
	doPatternStroke();
      } else {
	out->stroke(state);
      }
    }
  }
  doEndPath();
}

void Gfx::opEOFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in eofill/stroke");
    return;
  }
  if (state->isPath()) {
    if (ocState) {
      if (state->getFillColorSpace()->getMode() == csPattern) {
	doPatternFill(gTrue);
      } else {
	out->eoFill(state);
      }
      if (state->getStrokeColorSpace()->getMode() == csPattern) {
	doPatternStroke();
      } else {
	out->stroke(state);
      }
    }
  }
  doEndPath();
}

void Gfx::opCloseEOFillStroke(Object args[], int numArgs) {
  if (!state->isCurPt()) {
    //error(errSyntaxError, getPos(), "No path in closepath/eofill/stroke");
    return;
  }
  if (state->isPath()) {
    state->closePath();
    if (ocState) {
      if (state->getFillColorSpace()->getMode() == csPattern) {
	doPatternFill(gTrue);
      } else {
	out->eoFill(state);
      }
      if (state->getStrokeColorSpace()->getMode() == csPattern) {
	doPatternStroke();
      } else {
	out->stroke(state);
      }
    }
  }
  doEndPath();
}

void Gfx::doPatternFill(GBool eoFill) {
  GfxPattern *pattern;

  // this is a bit of a kludge -- patterns can be really slow, so we
  // skip them if we're only doing text extraction, since they almost
  // certainly don't contain any text
  if (!out->needNonText()) {
    return;
  }

  if (!(pattern = state->getFillPattern())) {
    return;
  }
  switch (pattern->getType()) {
  case 1:
    doTilingPatternFill((GfxTilingPattern *)pattern, gFalse, eoFill, gFalse);
    break;
  case 2:
    doShadingPatternFill((GfxShadingPattern *)pattern, gFalse, eoFill, gFalse);
    break;
  default:
    error(errSyntaxError, getPos(), "Unknown pattern type ({0:d}) in fill",
	  pattern->getType());
    break;
  }
}

void Gfx::doPatternStroke() {
  GfxPattern *pattern;

  // this is a bit of a kludge -- patterns can be really slow, so we
  // skip them if we're only doing text extraction, since they almost
  // certainly don't contain any text
  if (!out->needNonText()) {
    return;
  }

  if (!(pattern = state->getStrokePattern())) {
    return;
  }
  switch (pattern->getType()) {
  case 1:
    doTilingPatternFill((GfxTilingPattern *)pattern, gTrue, gFalse, gFalse);
    break;
  case 2:
    doShadingPatternFill((GfxShadingPattern *)pattern, gTrue, gFalse, gFalse);
    break;
  default:
    error(errSyntaxError, getPos(), "Unknown pattern type ({0:d}) in stroke",
	  pattern->getType());
    break;
  }
}

void Gfx::doPatternText() {
  GfxPattern *pattern;

  // this is a bit of a kludge -- patterns can be really slow, so we
  // skip them if we're only doing text extraction, since they almost
  // certainly don't contain any text
  if (!out->needNonText()) {
    return;
  }

  if (!(pattern = state->getFillPattern())) {
    return;
  }
  switch (pattern->getType()) {
  case 1:
    doTilingPatternFill((GfxTilingPattern *)pattern, gFalse, gFalse, gTrue);
    break;
  case 2:
    doShadingPatternFill((GfxShadingPattern *)pattern, gFalse, gFalse, gTrue);
    break;
  default:
    error(errSyntaxError, getPos(), "Unknown pattern type ({0:d}) in fill",
	  pattern->getType());
    break;
  }
}

void Gfx::doPatternImageMask(Object *ref, Stream *str, int width, int height,
			     GBool invert, GBool inlineImg) {
  saveState();

  out->setSoftMaskFromImageMask(state, ref, str,
				width, height, invert, inlineImg, baseMatrix);

  state->clearPath();
  state->moveTo(0, 0);
  state->lineTo(1, 0);
  state->lineTo(1, 1);
  state->lineTo(0, 1);
  state->closePath();
  doPatternText();

  out->unsetSoftMaskFromImageMask(state, baseMatrix);
  restoreState();
}

void Gfx::doTilingPatternFill(GfxTilingPattern *tPat,
			      GBool stroke, GBool eoFill, GBool text) {
  GfxPatternColorSpace *patCS;
  GfxColorSpace *cs;
  GfxColor color;
  GfxState *savedState;
  double xMin, yMin, xMax, yMax, x, y, x1, y1;
  double cxMin, cyMin, cxMax, cyMax;
  int xi0, yi0, xi1, yi1, xi, yi;
  double *ctm, *btm, *ptm;
  double m[6], ictm[6], m1[6], imb[6];
  double det;
  double xstep, ystep;
  int i;

  // get color space
  patCS = (GfxPatternColorSpace *)(stroke ? state->getStrokeColorSpace()
				          : state->getFillColorSpace());

  // construct a (pattern space) -> (current space) transform matrix
  ctm = state->getCTM();
  btm = baseMatrix;
  ptm = tPat->getMatrix();
  // iCTM = invert CTM
  det = 1 / (ctm[0] * ctm[3] - ctm[1] * ctm[2]);
  ictm[0] = ctm[3] * det;
  ictm[1] = -ctm[1] * det;
  ictm[2] = -ctm[2] * det;
  ictm[3] = ctm[0] * det;
  ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
  ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;
  // m1 = PTM * BTM = PTM * base transform matrix
  m1[0] = ptm[0] * btm[0] + ptm[1] * btm[2];
  m1[1] = ptm[0] * btm[1] + ptm[1] * btm[3];
  m1[2] = ptm[2] * btm[0] + ptm[3] * btm[2];
  m1[3] = ptm[2] * btm[1] + ptm[3] * btm[3];
  m1[4] = ptm[4] * btm[0] + ptm[5] * btm[2] + btm[4];
  m1[5] = ptm[4] * btm[1] + ptm[5] * btm[3] + btm[5];
  // m = m1 * iCTM = (PTM * BTM) * (iCTM)
  m[0] = m1[0] * ictm[0] + m1[1] * ictm[2];
  m[1] = m1[0] * ictm[1] + m1[1] * ictm[3];
  m[2] = m1[2] * ictm[0] + m1[3] * ictm[2];
  m[3] = m1[2] * ictm[1] + m1[3] * ictm[3];
  m[4] = m1[4] * ictm[0] + m1[5] * ictm[2] + ictm[4];
  m[5] = m1[4] * ictm[1] + m1[5] * ictm[3] + ictm[5];

  // construct a (device space) -> (pattern space) transform matrix
  det = 1 / (m1[0] * m1[3] - m1[1] * m1[2]);
  imb[0] = m1[3] * det;
  imb[1] = -m1[1] * det;
  imb[2] = -m1[2] * det;
  imb[3] = m1[0] * det;
  imb[4] = (m1[2] * m1[5] - m1[3] * m1[4]) * det;
  imb[5] = (m1[1] * m1[4] - m1[0] * m1[5]) * det;

  // save current graphics state
  savedState = saveStateStack();

  // set underlying color space (for uncolored tiling patterns); set
  // various other parameters (stroke color, line width) to match
  // Adobe's behavior
  state->setFillPattern(NULL);
  state->setStrokePattern(NULL);
  if (tPat->getPaintType() == 2 && (cs = patCS->getUnder())) {
    state->setFillColorSpace(cs->copy());
    out->updateFillColorSpace(state);
    state->setStrokeColorSpace(cs->copy());
    out->updateStrokeColorSpace(state);
    if (stroke) {
	state->setFillColor(state->getStrokeColor());
    } else {
	state->setStrokeColor(state->getFillColor());
    }
    out->updateFillColor(state);
    out->updateStrokeColor(state);
  } else {
    cs = new GfxDeviceGrayColorSpace();
    state->setFillColorSpace(cs);
    cs->getDefaultColor(&color);
    state->setFillColor(&color);
    out->updateFillColorSpace(state);
    state->setStrokeColorSpace(new GfxDeviceGrayColorSpace());
    state->setStrokeColor(&color);
    out->updateStrokeColorSpace(state);
  }
  if (!stroke) {
    state->setLineWidth(0);
    out->updateLineWidth(state);
  }

  // clip to current path
  if (stroke) {
    state->clipToStrokePath();
    out->clipToStrokePath(state);
  } else if (!text) {
    state->clip();
    if (eoFill) {
      out->eoClip(state);
    } else {
      out->clip(state);
    }
  }
  state->clearPath();

  // get the clip region, check for empty
  state->getClipBBox(&cxMin, &cyMin, &cxMax, &cyMax);
  if (cxMin > cxMax || cyMin > cyMax) {
    goto restore;
  }

  // transform clip region bbox to pattern space
  xMin = xMax = cxMin * imb[0] + cyMin * imb[2] + imb[4];
  yMin = yMax = cxMin * imb[1] + cyMin * imb[3] + imb[5];
  x1 = cxMin * imb[0] + cyMax * imb[2] + imb[4];
  y1 = cxMin * imb[1] + cyMax * imb[3] + imb[5];
  if (x1 < xMin) {
    xMin = x1;
  } else if (x1 > xMax) {
    xMax = x1;
  }
  if (y1 < yMin) {
    yMin = y1;
  } else if (y1 > yMax) {
    yMax = y1;
  }
  x1 = cxMax * imb[0] + cyMin * imb[2] + imb[4];
  y1 = cxMax * imb[1] + cyMin * imb[3] + imb[5];
  if (x1 < xMin) {
    xMin = x1;
  } else if (x1 > xMax) {
    xMax = x1;
  }
  if (y1 < yMin) {
    yMin = y1;
  } else if (y1 > yMax) {
    yMax = y1;
  }
  x1 = cxMax * imb[0] + cyMax * imb[2] + imb[4];
  y1 = cxMax * imb[1] + cyMax * imb[3] + imb[5];
  if (x1 < xMin) {
    xMin = x1;
  } else if (x1 > xMax) {
    xMax = x1;
  }
  if (y1 < yMin) {
    yMin = y1;
  } else if (y1 > yMax) {
    yMax = y1;
  }

  // draw the pattern
  //~ this should treat negative steps differently -- start at right/top
  //~ edge instead of left/bottom (?)
  xstep = fabs(tPat->getXStep());
  ystep = fabs(tPat->getYStep());
  if (tPat->getBBox()[0] < tPat->getBBox()[2]) {
    xi0 = (int)ceil((xMin - tPat->getBBox()[2]) / xstep);
    xi1 = (int)floor((xMax - tPat->getBBox()[0]) / xstep) + 1;
  } else {
    xi0 = (int)ceil((xMin - tPat->getBBox()[0]) / xstep);
    xi1 = (int)floor((xMax - tPat->getBBox()[2]) / xstep) + 1;
  }
  if (tPat->getBBox()[1] < tPat->getBBox()[3]) {
    yi0 = (int)ceil((yMin - tPat->getBBox()[3]) / ystep);
    yi1 = (int)floor((yMax - tPat->getBBox()[1]) / ystep) + 1;
  } else {
    yi0 = (int)ceil((yMin - tPat->getBBox()[1]) / ystep);
    yi1 = (int)floor((yMax - tPat->getBBox()[3]) / ystep) + 1;
  }
  for (i = 0; i < 4; ++i) {
    m1[i] = m[i];
  }
  m1[4] = m[4];
  m1[5] = m[5];
  if (out->useTilingPatternFill() &&
	out->tilingPatternFill(state, this, catalog, tPat->getContentStream(),
		       tPat->getMatrix(), tPat->getPaintType(), tPat->getTilingType(),
		       tPat->getResDict(), m1, tPat->getBBox(),
		       xi0, yi0, xi1, yi1, xstep, ystep)) {
    goto restore;
  } else {
    for (yi = yi0; yi < yi1; ++yi) {
      for (xi = xi0; xi < xi1; ++xi) {
        x = xi * xstep;
        y = yi * ystep;
        m1[4] = x * m[0] + y * m[2] + m[4];
        m1[5] = x * m[1] + y * m[3] + m[5];
        drawForm(tPat->getContentStream(), tPat->getResDict(),
        	  m1, tPat->getBBox());
      }
    }
  }

  // restore graphics state
 restore:
  restoreStateStack(savedState);
}

void Gfx::doShadingPatternFill(GfxShadingPattern *sPat,
			       GBool stroke, GBool eoFill, GBool text) {
  GfxShading *shading;
  GfxState *savedState;
  double *ctm, *btm, *ptm;
  double m[6], ictm[6], m1[6];
  double xMin, yMin, xMax, yMax;
  double det;

  shading = sPat->getShading();

  // save current graphics state
  savedState = saveStateStack();

  // clip to current path
  if (stroke) {
    state->clipToStrokePath();
    out->clipToStrokePath(state);
  } else if (!text) {
    state->clip();
    if (eoFill) {
      out->eoClip(state);
    } else {
      out->clip(state);
    }
  }
  state->clearPath();

  // construct a (pattern space) -> (current space) transform matrix
  ctm = state->getCTM();
  btm = baseMatrix;
  ptm = sPat->getMatrix();
  // iCTM = invert CTM
  det = 1 / (ctm[0] * ctm[3] - ctm[1] * ctm[2]);
  ictm[0] = ctm[3] * det;
  ictm[1] = -ctm[1] * det;
  ictm[2] = -ctm[2] * det;
  ictm[3] = ctm[0] * det;
  ictm[4] = (ctm[2] * ctm[5] - ctm[3] * ctm[4]) * det;
  ictm[5] = (ctm[1] * ctm[4] - ctm[0] * ctm[5]) * det;
  // m1 = PTM * BTM = PTM * base transform matrix
  m1[0] = ptm[0] * btm[0] + ptm[1] * btm[2];
  m1[1] = ptm[0] * btm[1] + ptm[1] * btm[3];
  m1[2] = ptm[2] * btm[0] + ptm[3] * btm[2];
  m1[3] = ptm[2] * btm[1] + ptm[3] * btm[3];
  m1[4] = ptm[4] * btm[0] + ptm[5] * btm[2] + btm[4];
  m1[5] = ptm[4] * btm[1] + ptm[5] * btm[3] + btm[5];
  // m = m1 * iCTM = (PTM * BTM) * (iCTM)
  m[0] = m1[0] * ictm[0] + m1[1] * ictm[2];
  m[1] = m1[0] * ictm[1] + m1[1] * ictm[3];
  m[2] = m1[2] * ictm[0] + m1[3] * ictm[2];
  m[3] = m1[2] * ictm[1] + m1[3] * ictm[3];
  m[4] = m1[4] * ictm[0] + m1[5] * ictm[2] + ictm[4];
  m[5] = m1[4] * ictm[1] + m1[5] * ictm[3] + ictm[5];

  // set the new matrix
  state->concatCTM(m[0], m[1], m[2], m[3], m[4], m[5]);
  out->updateCTM(state, m[0], m[1], m[2], m[3], m[4], m[5]);

  // clip to bbox
  if (shading->getHasBBox()) {
    shading->getBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMax, yMin);
    state->lineTo(xMax, yMax);
    state->lineTo(xMin, yMax);
    state->closePath();
    state->clip();
    out->clip(state);
    state->clearPath();
  }

  // set the color space
  state->setFillColorSpace(shading->getColorSpace()->copy());
  out->updateFillColorSpace(state);

  // background color fill
  if (shading->getHasBackground()) {
    state->setFillColor(shading->getBackground());
    out->updateFillColor(state);
    state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMax, yMin);
    state->lineTo(xMax, yMax);
    state->lineTo(xMin, yMax);
    state->closePath();
    out->fill(state);
    state->clearPath();
  }

#if 1 //~tmp: turn off anti-aliasing temporarily
  GBool vaa = out->getVectorAntialias();
  if (vaa) {
    out->setVectorAntialias(gFalse);
  }
#endif

  // do shading type-specific operations
  switch (shading->getType()) {
  case 1:
    doFunctionShFill((GfxFunctionShading *)shading);
    break;
  case 2:
    doAxialShFill((GfxAxialShading *)shading);
    break;
  case 3:
    doRadialShFill((GfxRadialShading *)shading);
    break;
  case 4:
  case 5:
    doGouraudTriangleShFill((GfxGouraudTriangleShading *)shading);
    break;
  case 6:
  case 7:
    doPatchMeshShFill((GfxPatchMeshShading *)shading);
    break;
  }

#if 1 //~tmp: turn off anti-aliasing temporarily
  if (vaa) {
    out->setVectorAntialias(gTrue);
  }
#endif

  // restore graphics state
  restoreStateStack(savedState);
}

void Gfx::opShFill(Object args[], int numArgs) {
  GfxShading *shading;
  GfxState *savedState;
  double xMin, yMin, xMax, yMax;

  if (!ocState) {
    return;
  }

  if (!(shading = res->lookupShading(args[0].getName(), out, state))) {
    return;
  }

  // save current graphics state
  savedState = saveStateStack();

  // clip to bbox
  if (shading->getHasBBox()) {
    shading->getBBox(&xMin, &yMin, &xMax, &yMax);
    state->moveTo(xMin, yMin);
    state->lineTo(xMax, yMin);
    state->lineTo(xMax, yMax);
    state->lineTo(xMin, yMax);
    state->closePath();
    state->clip();
    out->clip(state);
    state->clearPath();
  }

  // set the color space
  state->setFillColorSpace(shading->getColorSpace()->copy());
  out->updateFillColorSpace(state);

#if 1 //~tmp: turn off anti-aliasing temporarily
  GBool vaa = out->getVectorAntialias();
  if (vaa) {
    out->setVectorAntialias(gFalse);
  }
#endif

  // do shading type-specific operations
  switch (shading->getType()) {
  case 1:
    doFunctionShFill((GfxFunctionShading *)shading);
    break;
  case 2:
    doAxialShFill((GfxAxialShading *)shading);
    break;
  case 3:
    doRadialShFill((GfxRadialShading *)shading);
    break;
  case 4:
  case 5:
    doGouraudTriangleShFill((GfxGouraudTriangleShading *)shading);
    break;
  case 6:
  case 7:
    doPatchMeshShFill((GfxPatchMeshShading *)shading);
    break;
  }

#if 1 //~tmp: turn off anti-aliasing temporarily
  if (vaa) {
    out->setVectorAntialias(gTrue);
  }
#endif

  // restore graphics state
  restoreStateStack(savedState);

  delete shading;
}

void Gfx::doFunctionShFill(GfxFunctionShading *shading) {
  double x0, y0, x1, y1;
  GfxColor colors[4];

  if (out->useShadedFills( shading->getType() ) &&
      out->functionShadedFill(state, shading)) {
    return;
  }

  shading->getDomain(&x0, &y0, &x1, &y1);
  shading->getColor(x0, y0, &colors[0]);
  shading->getColor(x0, y1, &colors[1]);
  shading->getColor(x1, y0, &colors[2]);
  shading->getColor(x1, y1, &colors[3]);
  doFunctionShFill1(shading, x0, y0, x1, y1, colors, 0);
}

void Gfx::doFunctionShFill1(GfxFunctionShading *shading,
			    double x0, double y0,
			    double x1, double y1,
			    GfxColor *colors, int depth) {
  GfxColor fillColor;
  GfxColor color0M, color1M, colorM0, colorM1, colorMM;
  GfxColor colors2[4];
  double *matrix;
  double xM, yM;
  int nComps, i, j;

  nComps = shading->getColorSpace()->getNComps();
  matrix = shading->getMatrix();

  // compare the four corner colors
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < nComps; ++j) {
      if (abs(colors[i].c[j] - colors[(i+1)&3].c[j]) > functionColorDelta) {
	break;
      }
    }
    if (j < nComps) {
      break;
    }
  }

  // center of the rectangle
  xM = 0.5 * (x0 + x1);
  yM = 0.5 * (y0 + y1);

  // the four corner colors are close (or we hit the recursive limit)
  // -- fill the rectangle; but require at least one subdivision
  // (depth==0) to avoid problems when the four outer corners of the
  // shaded region are the same color
  if ((i == 4 && depth > 0) || depth == functionMaxDepth) {

    // use the center color
    shading->getColor(xM, yM, &fillColor);
    state->setFillColor(&fillColor);
    out->updateFillColor(state);

    // fill the rectangle
    state->moveTo(x0 * matrix[0] + y0 * matrix[2] + matrix[4],
		  x0 * matrix[1] + y0 * matrix[3] + matrix[5]);
    state->lineTo(x1 * matrix[0] + y0 * matrix[2] + matrix[4],
		  x1 * matrix[1] + y0 * matrix[3] + matrix[5]);
    state->lineTo(x1 * matrix[0] + y1 * matrix[2] + matrix[4],
		  x1 * matrix[1] + y1 * matrix[3] + matrix[5]);
    state->lineTo(x0 * matrix[0] + y1 * matrix[2] + matrix[4],
		  x0 * matrix[1] + y1 * matrix[3] + matrix[5]);
    state->closePath();
    out->fill(state);
    state->clearPath();

  // the four corner colors are not close enough -- subdivide the
  // rectangle
  } else {

    // colors[0]       colorM0       colors[2]
    //   (x0,y0)       (xM,y0)       (x1,y0)
    //         +----------+----------+
    //         |          |          |
    //         |    UL    |    UR    |
    // color0M |       colorMM       | color1M
    // (x0,yM) +----------+----------+ (x1,yM)
    //         |       (xM,yM)       |
    //         |    LL    |    LR    |
    //         |          |          |
    //         +----------+----------+
    // colors[1]       colorM1       colors[3]
    //   (x0,y1)       (xM,y1)       (x1,y1)

    shading->getColor(x0, yM, &color0M);
    shading->getColor(x1, yM, &color1M);
    shading->getColor(xM, y0, &colorM0);
    shading->getColor(xM, y1, &colorM1);
    shading->getColor(xM, yM, &colorMM);

    // upper-left sub-rectangle
    colors2[0] = colors[0];
    colors2[1] = color0M;
    colors2[2] = colorM0;
    colors2[3] = colorMM;
    doFunctionShFill1(shading, x0, y0, xM, yM, colors2, depth + 1);
    
    // lower-left sub-rectangle
    colors2[0] = color0M;
    colors2[1] = colors[1];
    colors2[2] = colorMM;
    colors2[3] = colorM1;
    doFunctionShFill1(shading, x0, yM, xM, y1, colors2, depth + 1);
    
    // upper-right sub-rectangle
    colors2[0] = colorM0;
    colors2[1] = colorMM;
    colors2[2] = colors[2];
    colors2[3] = color1M;
    doFunctionShFill1(shading, xM, y0, x1, yM, colors2, depth + 1);

    // lower-right sub-rectangle
    colors2[0] = colorMM;
    colors2[1] = colorM1;
    colors2[2] = color1M;
    colors2[3] = colors[3];
    doFunctionShFill1(shading, xM, yM, x1, y1, colors2, depth + 1);
  }
}

static void bubbleSort(double array[])
{
  for (int j = 0; j < 3; ++j) {
    int kk = j;
    for (int k = j + 1; k < 4; ++k) {
      if (array[k] < array[kk]) {
        kk = k;
      }
    }
    double tmp = array[j];
    array[j] = array[kk];
    array[kk] = tmp;
  }
}

void Gfx::doAxialShFill(GfxAxialShading *shading) {
  double xMin, yMin, xMax, yMax;
  double x0, y0, x1, y1;
  double dx, dy, mul;
  GBool dxZero, dyZero;
  double bboxIntersections[4];
  double tMin, tMax, tx, ty;
  double s[4], sMin, sMax, tmp;
  double ux0, uy0, ux1, uy1, vx0, vy0, vx1, vy1;
  double t0, t1, tt;
  double ta[axialMaxSplits + 1];
  int next[axialMaxSplits + 1];
  GfxColor color0, color1;
  int nComps;
  int i, j, k;
  GBool needExtend = gTrue;

  // get the clip region bbox
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);

  // compute min and max t values, based on the four corners of the
  // clip region bbox
  shading->getCoords(&x0, &y0, &x1, &y1);
  dx = x1 - x0;
  dy = y1 - y0;
  dxZero = fabs(dx) < 0.01;
  dyZero = fabs(dy) < 0.01;
  if (dxZero && dyZero) {
    tMin = tMax = 0;
  } else {
    mul = 1 / (dx * dx + dy * dy);
    bboxIntersections[0] = ((xMin - x0) * dx + (yMin - y0) * dy) * mul;
    bboxIntersections[1] = ((xMin - x0) * dx + (yMax - y0) * dy) * mul;
    bboxIntersections[2] = ((xMax - x0) * dx + (yMin - y0) * dy) * mul;
    bboxIntersections[3] = ((xMax - x0) * dx + (yMax - y0) * dy) * mul;
    bubbleSort(bboxIntersections);
    tMin = bboxIntersections[0];
    tMax = bboxIntersections[3];
    if (tMin < 0 && !shading->getExtend0()) {
      tMin = 0;
    }
    if (tMax > 1 && !shading->getExtend1()) {
      tMax = 1;
    }
  }

  if (out->useShadedFills( shading->getType() ) &&
      out->axialShadedFill(state, shading, tMin, tMax)) {
	  return;
  }

  // get the function domain
  t0 = shading->getDomain0();
  t1 = shading->getDomain1();

  // Traverse the t axis and do the shading.
  //
  // For each point (tx, ty) on the t axis, consider a line through
  // that point perpendicular to the t axis:
  //
  //     x(s) = tx + s * -dy   -->   s = (x - tx) / -dy
  //     y(s) = ty + s * dx    -->   s = (y - ty) / dx
  //
  // Then look at the intersection of this line with the bounding box
  // (xMin, yMin, xMax, yMax).  In the general case, there are four
  // intersection points:
  //
  //     s0 = (xMin - tx) / -dy
  //     s1 = (xMax - tx) / -dy
  //     s2 = (yMin - ty) / dx
  //     s3 = (yMax - ty) / dx
  //
  // and we want the middle two s values.
  //
  // In the case where dx = 0, take s0 and s1; in the case where dy =
  // 0, take s2 and s3.
  //
  // Each filled polygon is bounded by two of these line segments
  // perpdendicular to the t axis.
  //
  // The t axis is bisected into smaller regions until the color
  // difference across a region is small enough, and then the region
  // is painted with a single color.

  // set up: require at least one split to avoid problems when the two
  // ends of the t axis have the same color
  nComps = shading->getColorSpace()->getNComps();
  ta[0] = tMin;
  next[0] = axialMaxSplits / 2;
  ta[axialMaxSplits / 2] = 0.5 * (tMin + tMax);
  next[axialMaxSplits / 2] = axialMaxSplits;
  ta[axialMaxSplits] = tMax;

  // compute the color at t = tMin
  if (tMin < 0) {
    tt = t0;
  } else if (tMin > 1) {
    tt = t1;
  } else {
    tt = t0 + (t1 - t0) * tMin;
  }
  shading->getColor(tt, &color0);

  if (out->useFillColorStop()) {
    // make sure we add stop color when t = tMin
    state->setFillColor(&color0);
    out->updateFillColorStop(state, 0);
  }

  // compute the coordinates of the point on the t axis at t = tMin;
  // then compute the intersection of the perpendicular line with the
  // bounding box
  tx = x0 + tMin * dx;
  ty = y0 + tMin * dy;
  if (dxZero && dyZero) {
    sMin = sMax = 0;
  } else if (dxZero) {
    sMin = (xMin - tx) / -dy;
    sMax = (xMax - tx) / -dy;
    if (sMin > sMax) { tmp = sMin; sMin = sMax; sMax = tmp; }
  } else if (dyZero) {
    sMin = (yMin - ty) / dx;
    sMax = (yMax - ty) / dx;
    if (sMin > sMax) { tmp = sMin; sMin = sMax; sMax = tmp; }
  } else {
    s[0] = (yMin - ty) / dx;
    s[1] = (yMax - ty) / dx;
    s[2] = (xMin - tx) / -dy;
    s[3] = (xMax - tx) / -dy;
    bubbleSort(s);
    sMin = s[1];
    sMax = s[2];
  }
  ux0 = tx - sMin * dy;
  uy0 = ty + sMin * dx;
  vx0 = tx - sMax * dy;
  vy0 = ty + sMax * dx;

  i = 0;
  bool doneBBox1, doneBBox2;
  if (dxZero && dyZero) {
    doneBBox1 = doneBBox2 = true;
  } else {
    doneBBox1 = bboxIntersections[1] < tMin;
    doneBBox2 = bboxIntersections[2] > tMax;
  }

  // If output device doesn't support the extended mode required
  // we have to do it here
  needExtend = !out->axialShadedSupportExtend(state, shading);

  while (i < axialMaxSplits) {

    // bisect until color difference is small enough or we hit the
    // bisection limit
    j = next[i];
    while (j > i + 1) {
      if (ta[j] < 0) {
	tt = t0;
      } else if (ta[j] > 1) {
	tt = t1;
      } else {
	tt = t0 + (t1 - t0) * ta[j];
      }
      shading->getColor(tt, &color1);
      if (isSameGfxColor(color1, color0, nComps, axialColorDelta)) {
         // in these two if what we guarantee is that if we are skipping lots of 
         // positions because the colors are the same, we still create a region
         // with vertexs passing by bboxIntersections[1] and bboxIntersections[2]
         // otherwise we can have empty regions that should really be painted 
         // like happened in bug 19896
         // What we do to ensure that we pass a line through this points
         // is making sure use the exact bboxIntersections[] value as one of the used ta[] values
         if (!doneBBox1 && ta[i] < bboxIntersections[1] && ta[j] > bboxIntersections[1]) {
           int teoricalj = (int) ((bboxIntersections[1] - tMin) * axialMaxSplits / (tMax - tMin));
           if (teoricalj <= i) teoricalj = i + 1;
           if (teoricalj < j) {
             next[i] = teoricalj;
             next[teoricalj] = j;
           }
           else {
             teoricalj = j;
           }
           ta[teoricalj] = bboxIntersections[1];
           j = teoricalj;
           doneBBox1 = true;
         }
         if (!doneBBox2 && ta[i] < bboxIntersections[2] && ta[j] > bboxIntersections[2]) {
           int teoricalj = (int) ((bboxIntersections[2] - tMin) * axialMaxSplits / (tMax - tMin));
           if (teoricalj <= i) teoricalj = i + 1;
           if (teoricalj < j) {
             next[i] = teoricalj;
             next[teoricalj] = j;
           }
           else {
             teoricalj = j;
           }
           ta[teoricalj] = bboxIntersections[2];
           j = teoricalj;
           doneBBox2 = true;
         }
         break;
      }
      k = (i + j) / 2;
      ta[k] = 0.5 * (ta[i] + ta[j]);
      next[i] = k;
      next[k] = j;
      j = k;
    }

    // use the average of the colors of the two sides of the region
    for (k = 0; k < nComps; ++k) {
      color0.c[k] = (color0.c[k] + color1.c[k]) / 2;
    }

    // compute the coordinates of the point on the t axis; then
    // compute the intersection of the perpendicular line with the
    // bounding box
    tx = x0 + ta[j] * dx;
    ty = y0 + ta[j] * dy;
    if (dxZero && dyZero) {
      sMin = sMax = 0;
    } else if (dxZero) {
      sMin = (xMin - tx) / -dy;
      sMax = (xMax - tx) / -dy;
      if (sMin > sMax) { tmp = sMin; sMin = sMax; sMax = tmp; }
    } else if (dyZero) {
      sMin = (yMin - ty) / dx;
      sMax = (yMax - ty) / dx;
      if (sMin > sMax) { tmp = sMin; sMin = sMax; sMax = tmp; }
    } else {
      s[0] = (yMin - ty) / dx;
      s[1] = (yMax - ty) / dx;
      s[2] = (xMin - tx) / -dy;
      s[3] = (xMax - tx) / -dy;
      bubbleSort(s);
      sMin = s[1];
      sMax = s[2];
    }
    ux1 = tx - sMin * dy;
    uy1 = ty + sMin * dx;
    vx1 = tx - sMax * dy;
    vy1 = ty + sMax * dx;

    // set the color
    state->setFillColor(&color0);
    if (out->useFillColorStop())
      out->updateFillColorStop(state, (ta[j] - tMin)/(tMax - tMin));
    else
      out->updateFillColor(state);

    if (needExtend) {
      // fill the region
      state->moveTo(ux0, uy0);
      state->lineTo(vx0, vy0);
      state->lineTo(vx1, vy1);
      state->lineTo(ux1, uy1);
      state->closePath();
    }

    if (!out->useFillColorStop()) {
      out->fill(state);
      state->clearPath();
    }

    // set up for next region
    ux0 = ux1;
    uy0 = uy1;
    vx0 = vx1;
    vy0 = vy1;
    color0 = color1;
    i = next[i];
  }

  if (out->useFillColorStop()) {
    if (!needExtend) {
      state->moveTo(xMin, yMin);
      state->lineTo(xMin, yMax);
      state->lineTo(xMax, yMax);
      state->lineTo(xMax, yMin);
      state->closePath();
    }
    out->fill(state);
    state->clearPath();
  }
}

static inline void getShadingColorRadialHelper(double t0, double t1, double t, GfxRadialShading *shading, GfxColor *color)
{
  if (t0 < t1) {
    if (t < t0) {
      shading->getColor(t0, color);
    } else if (t > t1) {
      shading->getColor(t1, color);
    } else {
      shading->getColor(t, color);
    }
  } else {
    if (t > t0) {
      shading->getColor(t0, color);
    } else if (t < t1) {
      shading->getColor(t1, color);
    } else {
      shading->getColor(t, color);
    }
  }
}

void Gfx::doRadialShFill(GfxRadialShading *shading) {
  double xMin, yMin, xMax, yMax;
  double x0, y0, r0, x1, y1, r1, t0, t1;
  int nComps;
  GfxColor colorA, colorB;
  double xa, ya, xb, yb, ra, rb;
  double ta, tb, sa, sb;
  double sz, xz, yz, sMin, sMax;
  GBool enclosed;
  int ia, ib, k, n;
  double *ctm;
  double theta, alpha, angle, t;
  GBool needExtend = gTrue;

  // get the shading info
  shading->getCoords(&x0, &y0, &r0, &x1, &y1, &r1);
  t0 = shading->getDomain0();
  t1 = shading->getDomain1();
  nComps = shading->getColorSpace()->getNComps();

  // Compute the point at which r(s) = 0; check for the enclosed
  // circles case; and compute the angles for the tangent lines.
  if (x0 == x1 && y0 == y1) {
    enclosed = gTrue;
    theta = 0; // make gcc happy
    sz = 0; // make gcc happy
  } else if (r0 == r1) {
    enclosed = gFalse;
    theta = 0;
    sz = 0; // make gcc happy
  } else {
    sz = (r1 > r0) ? -r0 / (r1 - r0) : -r1 / (r0 - r1);
    xz = x0 + sz * (x1 - x0);
    yz = y0 + sz * (y1 - y0);
    enclosed = (xz - x0) * (xz - x0) + (yz - y0) * (yz - y0) <= r0 * r0;
    theta = asin(r0 / sqrt((x0 - xz) * (x0 - xz) + (y0 - yz) * (y0 - yz)));
    if (r0 > r1) {
      theta = -theta;
    }
  }
  if (enclosed) {
    alpha = 0;
  } else {
    alpha = atan2(y1 - y0, x1 - x0);
  }

  // compute the (possibly extended) s range
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
  if (enclosed) {
    sMin = 0;
    sMax = 1;
  } else {
    sMin = 1;
    sMax = 0;
    // solve for x(s) + r(s) = xMin
    if ((x1 + r1) - (x0 + r0) != 0) {
      sa = (xMin - (x0 + r0)) / ((x1 + r1) - (x0 + r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // solve for x(s) - r(s) = xMax
    if ((x1 - r1) - (x0 - r0) != 0) {
      sa = (xMax - (x0 - r0)) / ((x1 - r1) - (x0 - r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // solve for y(s) + r(s) = yMin
    if ((y1 + r1) - (y0 + r0) != 0) {
      sa = (yMin - (y0 + r0)) / ((y1 + r1) - (y0 + r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // solve for y(s) - r(s) = yMax
    if ((y1 - r1) - (y0 - r0) != 0) {
      sa = (yMax - (y0 - r0)) / ((y1 - r1) - (y0 - r0));
      if (sa < sMin) {
	sMin = sa;
      } else if (sa > sMax) {
	sMax = sa;
      }
    }
    // check against sz
    if (r0 < r1) {
      if (sMin < sz) {
	sMin = sz;
      }
    } else if (r0 > r1) {
      if (sMax > sz) {
	sMax = sz;
      }
    }
    // check the 'extend' flags
    if (!shading->getExtend0() && sMin < 0) {
      sMin = 0;
    }
    if (!shading->getExtend1() && sMax > 1) {
      sMax = 1;
    }
  }

  if (out->useShadedFills( shading->getType() ) &&
      out->radialShadedFill(state, shading, sMin, sMax)) {
    return;
  }

  // compute the number of steps into which circles must be divided to
  // achieve a curve flatness of 0.1 pixel in device space for the
  // largest circle (note that "device space" is 72 dpi when generating
  // PostScript, hence the relatively small 0.1 pixel accuracy)
  ctm = state->getCTM();
  t = fabs(ctm[0]);
  if (fabs(ctm[1]) > t) {
    t = fabs(ctm[1]);
  }
  if (fabs(ctm[2]) > t) {
    t = fabs(ctm[2]);
  }
  if (fabs(ctm[3]) > t) {
    t = fabs(ctm[3]);
  }
  if (r0 > r1) {
    t *= r0;
  } else {
    t *= r1;
  }
  if (t < 1) {
    n = 3;
  } else {
    n = (int)(M_PI / acos(1 - 0.1 / t));
    if (n < 3) {
      n = 3;
    } else if (n > 200) {
      n = 200;
    }
  }

  // setup for the start circle
  ia = 0;
  sa = sMin;
  ta = t0 + sa * (t1 - t0);
  xa = x0 + sa * (x1 - x0);
  ya = y0 + sa * (y1 - y0);
  ra = r0 + sa * (r1 - r0);
  getShadingColorRadialHelper(t0, t1, ta, shading, &colorA);

  needExtend = !out->radialShadedSupportExtend(state, shading);

  // fill the circles
  while (ia < radialMaxSplits) {

    // go as far along the t axis (toward t1) as we can, such that the
    // color difference is within the tolerance (radialColorDelta) --
    // this uses bisection (between the current value, t, and t1),
    // limited to radialMaxSplits points along the t axis; require at
    // least one split to avoid problems when the innermost and
    // outermost colors are the same
    ib = radialMaxSplits;
    sb = sMax;
    tb = t0 + sb * (t1 - t0);
    getShadingColorRadialHelper(t0, t1, tb, shading, &colorB);
    while (ib - ia > 1) {
      if (isSameGfxColor(colorB, colorA, nComps, radialColorDelta)) {
        // The shading is not necessarily lineal so having two points with the
        // same color does not mean all the areas in between have the same color too
        int ic = ia + 1;
        for (; ic <= ib; ic++) {
          GfxColor colorC;
          const double sc = sMin + ((double)ic / (double)radialMaxSplits) * (sMax - sMin);
          const double tc = t0 + sc * (t1 - t0);
          getShadingColorRadialHelper(t0, t1, tc, shading, &colorC);
          if (!isSameGfxColor(colorC, colorA, nComps, radialColorDelta)) {
            break;
          }
        }
        ib = (ic > ia + 1) ? ic - 1 : ia + 1;
        sb = sMin + ((double)ib / (double)radialMaxSplits) * (sMax - sMin);
        tb = t0 + sb * (t1 - t0);
        getShadingColorRadialHelper(t0, t1, tb, shading, &colorB);
        break;
      }
      ib = (ia + ib) / 2;
      sb = sMin + ((double)ib / (double)radialMaxSplits) * (sMax - sMin);
      tb = t0 + sb * (t1 - t0);
      getShadingColorRadialHelper(t0, t1, tb, shading, &colorB);
    }

    // compute center and radius of the circle
    xb = x0 + sb * (x1 - x0);
    yb = y0 + sb * (y1 - y0);
    rb = r0 + sb * (r1 - r0);

    // use the average of the colors at the two circles
    for (k = 0; k < nComps; ++k) {
      colorA.c[k] = (colorA.c[k] + colorB.c[k]) / 2;
    }
    state->setFillColor(&colorA);
    if (out->useFillColorStop())
      out->updateFillColorStop(state, (sa - sMin)/(sMax - sMin));
    else
      out->updateFillColor(state);

    if (needExtend) {
      if (enclosed) {
        // construct path for first circle (counterclockwise)
        state->moveTo(xa + ra, ya);
        for (k = 1; k < n; ++k) {
	  angle = ((double)k / (double)n) * 2 * M_PI;
	  state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
        }
        state->closePath();

        // construct and append path for second circle (clockwise)
        state->moveTo(xb + rb, yb);
        for (k = 1; k < n; ++k) {
	  angle = -((double)k / (double)n) * 2 * M_PI;
	  state->lineTo(xb + rb * cos(angle), yb + rb * sin(angle));
        }
        state->closePath();
      } else {
        // construct the first subpath (clockwise)
        state->moveTo(xa + ra * cos(alpha + theta + 0.5 * M_PI),
		      ya + ra * sin(alpha + theta + 0.5 * M_PI));
        for (k = 0; k < n; ++k) {
	  angle = alpha + theta + 0.5 * M_PI
		  - ((double)k / (double)n) * (2 * theta + M_PI);
	  state->lineTo(xb + rb * cos(angle), yb + rb * sin(angle));
        }
	for (k = 0; k < n; ++k) {
	  angle = alpha - theta - 0.5 * M_PI
		  + ((double)k / (double)n) * (2 * theta - M_PI);
	  state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
	}
	state->closePath();

        // construct the second subpath (counterclockwise)
	state->moveTo(xa + ra * cos(alpha + theta + 0.5 * M_PI),
		      ya + ra * sin(alpha + theta + 0.5 * M_PI));
	for (k = 0; k < n; ++k) {
	  angle = alpha + theta + 0.5 * M_PI
		  + ((double)k / (double)n) * (-2 * theta + M_PI);
	  state->lineTo(xb + rb * cos(angle), yb + rb * sin(angle));
	}
	for (k = 0; k < n; ++k) {
	  angle = alpha - theta - 0.5 * M_PI
		  + ((double)k / (double)n) * (2 * theta + M_PI);
	  state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
	}
	state->closePath();
      }
    }

    if (!out->useFillColorStop()) {
      // fill the path
      out->fill(state);
      state->clearPath();
    }

    // step to the next value of t
    ia = ib;
    sa = sb;
    ta = tb;
    xa = xb;
    ya = yb;
    ra = rb;
    colorA = colorB;
  }

  if (out->useFillColorStop()) {
    // make sure we add stop color when sb = sMax
    state->setFillColor(&colorA);
    out->updateFillColorStop(state, (sb - sMin)/(sMax - sMin));

    // fill the path
    state->moveTo(xMin, yMin);
    state->lineTo(xMin, yMax);
    state->lineTo(xMax, yMax);
    state->lineTo(xMax, yMin);
    state->closePath();

    out->fill(state);
    state->clearPath();
  }

  if (!needExtend)
    return;

  if (enclosed) {
    // extend the smaller circle
    if ((shading->getExtend0() && r0 <= r1) ||
	(shading->getExtend1() && r1 < r0)) {
      if (r0 <= r1) {
	ta = t0;
	ra = r0;
	xa = x0;
	ya = y0;
      } else {
	ta = t1;
	ra = r1;
	xa = x1;
	ya = y1;
      }
      shading->getColor(ta, &colorA);
      state->setFillColor(&colorA);
      out->updateFillColor(state);
      state->moveTo(xa + ra, ya);
      for (k = 1; k < n; ++k) {
	angle = ((double)k / (double)n) * 2 * M_PI;
	state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
      }
      state->closePath();
      out->fill(state);
      state->clearPath();
    }

    // extend the larger circle
    if ((shading->getExtend0() && r0 > r1) ||
	(shading->getExtend1() && r1 >= r0)) {
      if (r0 > r1) {
	ta = t0;
	ra = r0;
	xa = x0;
	ya = y0;
      } else {
	ta = t1;
	ra = r1;
	xa = x1;
	ya = y1;
      }
      shading->getColor(ta, &colorA);
      state->setFillColor(&colorA);
      out->updateFillColor(state);
      state->moveTo(xMin, yMin);
      state->lineTo(xMin, yMax);
      state->lineTo(xMax, yMax);
      state->lineTo(xMax, yMin);
      state->closePath();
      state->moveTo(xa + ra, ya);
      for (k = 1; k < n; ++k) {
	angle = ((double)k / (double)n) * 2 * M_PI;
	state->lineTo(xa + ra * cos(angle), ya + ra * sin(angle));
      }
      state->closePath();
      out->fill(state);
      state->clearPath();
    }
  }
}

void Gfx::doGouraudTriangleShFill(GfxGouraudTriangleShading *shading) {
  double x0, y0, x1, y1, x2, y2;
  int i;

  if (out->useShadedFills( shading->getType())) {
    if (out->gouraudTriangleShadedFill( state, shading))
      return;
  }

  // preallocate a path (speed improvements)
  state->moveTo(0., 0.);
  state->lineTo(1., 0.);
  state->lineTo(0., 1.);
  state->closePath();

  GfxState::ReusablePathIterator *reusablePath = state->getReusablePath();

  if (shading->isParameterized()) {
    // work with parameterized values:
    double color0, color1, color2;
    // a relative threshold:
    const double refineColorThreshold = gouraudParameterizedColorDelta *
                                        (shading->getParameterDomainMax() - shading->getParameterDomainMin());
    for (i = 0; i < shading->getNTriangles(); ++i) {
      shading->getTriangle(i, &x0, &y0, &color0,
                              &x1, &y1, &color1,
                              &x2, &y2, &color2);
      gouraudFillTriangle(x0, y0, color0, x1, y1, color1, x2, y2, color2, refineColorThreshold, 0, shading, reusablePath);
    }

  } else {
    // this always produces output -- even for parameterized ranges.
    // But it ignores the parameterized color map (the function). 
    //
    // Note that using this code in for parameterized shadings might be
    // correct in circumstances (namely if the function is linear in the actual
    // triangle), but in general, it will simply be wrong.
    GfxColor color0, color1, color2;
    for (i = 0; i < shading->getNTriangles(); ++i) {
      shading->getTriangle(i, &x0, &y0, &color0,
                              &x1, &y1, &color1,
                              &x2, &y2, &color2);
      gouraudFillTriangle(x0, y0, &color0, x1, y1, &color1, x2, y2, &color2, shading->getColorSpace()->getNComps(), 0, reusablePath);
    }
  }

  delete reusablePath;
}

void Gfx::gouraudFillTriangle(double x0, double y0, GfxColor *color0,
			      double x1, double y1, GfxColor *color1,
			      double x2, double y2, GfxColor *color2,
			      int nComps, int depth, GfxState::ReusablePathIterator *path) {
  double x01, y01, x12, y12, x20, y20;
  GfxColor color01, color12, color20;
  int i;

  for (i = 0; i < nComps; ++i) {
    if (abs(color0->c[i] - color1->c[i]) > gouraudColorDelta ||
	abs(color1->c[i] - color2->c[i]) > gouraudColorDelta) {
      break;
    }
  }
  if (i == nComps || depth == gouraudMaxDepth) {
    state->setFillColor(color0);
    out->updateFillColor(state);

    path->reset();                         assert(!path->isEnd());
    path->setCoord(x0,y0);  path->next();  assert(!path->isEnd());
    path->setCoord(x1,y1);  path->next();  assert(!path->isEnd());
    path->setCoord(x2,y2);  path->next();  assert(!path->isEnd());
    path->setCoord(x0,y0);  path->next();  assert( path->isEnd());
    out->fill(state);

  } else {
    x01 = 0.5 * (x0 + x1);
    y01 = 0.5 * (y0 + y1);
    x12 = 0.5 * (x1 + x2);
    y12 = 0.5 * (y1 + y2);
    x20 = 0.5 * (x2 + x0);
    y20 = 0.5 * (y2 + y0);
    for (i = 0; i < nComps; ++i) {
      color01.c[i] = (color0->c[i] + color1->c[i]) / 2;
      color12.c[i] = (color1->c[i] + color2->c[i]) / 2;
      color20.c[i] = (color2->c[i] + color0->c[i]) / 2;
    }
    gouraudFillTriangle(x0, y0, color0, x01, y01, &color01,
			x20, y20, &color20, nComps, depth + 1, path);
    gouraudFillTriangle(x01, y01, &color01, x1, y1, color1,
			x12, y12, &color12, nComps, depth + 1, path);
    gouraudFillTriangle(x01, y01, &color01, x12, y12, &color12,
			x20, y20, &color20, nComps, depth + 1, path);
    gouraudFillTriangle(x20, y20, &color20, x12, y12, &color12,
			x2, y2, color2, nComps, depth + 1, path);
  }
}
void Gfx::gouraudFillTriangle(double x0, double y0, double color0,
                              double x1, double y1, double color1,
                              double x2, double y2, double color2,
                              double refineColorThreshold, int depth, GfxGouraudTriangleShading *shading, GfxState::ReusablePathIterator *path) {
  const double meanColor = (color0 + color1 + color2) / 3;

  const bool isFineEnough = 
       fabs(color0 - meanColor) < refineColorThreshold &&
       fabs(color1 - meanColor) < refineColorThreshold &&
       fabs(color2 - meanColor) < refineColorThreshold;

  if (isFineEnough || depth == gouraudMaxDepth) {
    GfxColor color;

    shading->getParameterizedColor(meanColor, &color);
    state->setFillColor(&color);
    out->updateFillColor(state);

    path->reset();                         assert(!path->isEnd());
    path->setCoord(x0,y0);  path->next();  assert(!path->isEnd());
    path->setCoord(x1,y1);  path->next();  assert(!path->isEnd());
    path->setCoord(x2,y2);  path->next();  assert(!path->isEnd());
    path->setCoord(x0,y0);  path->next();  assert( path->isEnd());
    out->fill(state);

  } else {
    const double x01 = 0.5 * (x0 + x1);
    const double y01 = 0.5 * (y0 + y1);
    const double x12 = 0.5 * (x1 + x2);
    const double y12 = 0.5 * (y1 + y2);
    const double x20 = 0.5 * (x2 + x0);
    const double y20 = 0.5 * (y2 + y0);
    const double color01 = (color0 + color1) / 2.;
    const double color12 = (color1 + color2) / 2.; 
    const double color20 = (color2 + color0) / 2.;
    ++depth;
    gouraudFillTriangle(x0, y0, color0, 
                        x01, y01, color01,
                        x20, y20, color20, 
                        refineColorThreshold, depth, shading, path);
    gouraudFillTriangle(x01, y01, color01, 
                        x1, y1, color1,
                        x12, y12, color12, 
                        refineColorThreshold, depth, shading, path);
    gouraudFillTriangle(x01, y01, color01, 
                        x12, y12, color12,
                        x20, y20, color20, 
                        refineColorThreshold, depth, shading, path);
    gouraudFillTriangle(x20, y20, color20, 
                        x12, y12, color12,
                        x2, y2, color2, 
                        refineColorThreshold, depth, shading, path);
  }
}

void Gfx::doPatchMeshShFill(GfxPatchMeshShading *shading) {
  int start, i;

  if (out->useShadedFills( shading->getType())) {
    if (out->patchMeshShadedFill( state, shading))
      return;
  }

  if (shading->getNPatches() > 128) {
    start = 3;
  } else if (shading->getNPatches() > 64) {
    start = 2;
  } else if (shading->getNPatches() > 16) {
    start = 1;
  } else {
    start = 0;
  }
  /*
   * Parameterized shadings take one parameter [t_0,t_e]
   * and map it into the color space.
   *
   * Consequently, all color values are stored as doubles.
   *
   * These color values are interpreted as parameters for parameterized
   * shadings and as colorspace entities otherwise.
   *
   * The only difference is that color space entities are stored into
   * DOUBLE arrays, not into arrays of type GfxColorComp.
   */
  const int colorComps = shading->getColorSpace()->getNComps();
  double refineColorThreshold;
  if( shading->isParameterized() ) {
	  refineColorThreshold = gouraudParameterizedColorDelta *
		  (shading->getParameterDomainMax() - shading->getParameterDomainMin());

  } else {
	  refineColorThreshold = patchColorDelta;
  }

  for (i = 0; i < shading->getNPatches(); ++i) {
    fillPatch(shading->getPatch(i),
             colorComps, 
             shading->isParameterized() ? 1 : colorComps,
             refineColorThreshold,
             start,
             shading);
  }
}


void Gfx::fillPatch(GfxPatch *patch, int colorComps, int patchColorComps, double refineColorThreshold, int depth, GfxPatchMeshShading *shading) {
  GfxPatch patch00, patch01, patch10, patch11;
  double xx[4][8], yy[4][8];
  double xxm, yym;
  int i;

  for (i = 0; i < patchColorComps; ++i) {
    // these comparisons are done in double arithmetics.
    //
    // For non-parameterized shadings, they are done in color space
    // components.
    if (fabs(patch->color[0][0].c[i] - patch->color[0][1].c[i]) > refineColorThreshold ||
        fabs(patch->color[0][1].c[i] - patch->color[1][1].c[i]) > refineColorThreshold ||
        fabs(patch->color[1][1].c[i] - patch->color[1][0].c[i]) > refineColorThreshold ||
        fabs(patch->color[1][0].c[i] - patch->color[0][0].c[i]) > refineColorThreshold) {
      break;
    }
  }
  if (i == patchColorComps || depth == patchMaxDepth) {
    GfxColor flatColor;
    if( shading->isParameterized() ) {
      shading->getParameterizedColor( patch->color[0][0].c[0], &flatColor );
    } else {
      for( i = 0; i<colorComps; ++i ) {
        // simply cast to the desired type; that's all what is needed.
        flatColor.c[i] = GfxColorComp(patch->color[0][0].c[i]);
      }
    }
    state->setFillColor(&flatColor);
    out->updateFillColor(state);
    state->moveTo(patch->x[0][0], patch->y[0][0]);
    state->curveTo(patch->x[0][1], patch->y[0][1],
		   patch->x[0][2], patch->y[0][2],
		   patch->x[0][3], patch->y[0][3]);
    state->curveTo(patch->x[1][3], patch->y[1][3],
		   patch->x[2][3], patch->y[2][3],
		   patch->x[3][3], patch->y[3][3]);
    state->curveTo(patch->x[3][2], patch->y[3][2],
		   patch->x[3][1], patch->y[3][1],
		   patch->x[3][0], patch->y[3][0]);
    state->curveTo(patch->x[2][0], patch->y[2][0],
		   patch->x[1][0], patch->y[1][0],
		   patch->x[0][0], patch->y[0][0]);
    state->closePath();
    out->fill(state);
    state->clearPath();
  } else {
    for (i = 0; i < 4; ++i) {
      xx[i][0] = patch->x[i][0];
      yy[i][0] = patch->y[i][0];
      xx[i][1] = 0.5 * (patch->x[i][0] + patch->x[i][1]);
      yy[i][1] = 0.5 * (patch->y[i][0] + patch->y[i][1]);
      xxm = 0.5 * (patch->x[i][1] + patch->x[i][2]);
      yym = 0.5 * (patch->y[i][1] + patch->y[i][2]);
      xx[i][6] = 0.5 * (patch->x[i][2] + patch->x[i][3]);
      yy[i][6] = 0.5 * (patch->y[i][2] + patch->y[i][3]);
      xx[i][2] = 0.5 * (xx[i][1] + xxm);
      yy[i][2] = 0.5 * (yy[i][1] + yym);
      xx[i][5] = 0.5 * (xxm + xx[i][6]);
      yy[i][5] = 0.5 * (yym + yy[i][6]);
      xx[i][3] = xx[i][4] = 0.5 * (xx[i][2] + xx[i][5]);
      yy[i][3] = yy[i][4] = 0.5 * (yy[i][2] + yy[i][5]);
      xx[i][7] = patch->x[i][3];
      yy[i][7] = patch->y[i][3];
    }
    for (i = 0; i < 4; ++i) {
      patch00.x[0][i] = xx[0][i];
      patch00.y[0][i] = yy[0][i];
      patch00.x[1][i] = 0.5 * (xx[0][i] + xx[1][i]);
      patch00.y[1][i] = 0.5 * (yy[0][i] + yy[1][i]);
      xxm = 0.5 * (xx[1][i] + xx[2][i]);
      yym = 0.5 * (yy[1][i] + yy[2][i]);
      patch10.x[2][i] = 0.5 * (xx[2][i] + xx[3][i]);
      patch10.y[2][i] = 0.5 * (yy[2][i] + yy[3][i]);
      patch00.x[2][i] = 0.5 * (patch00.x[1][i] + xxm);
      patch00.y[2][i] = 0.5 * (patch00.y[1][i] + yym);
      patch10.x[1][i] = 0.5 * (xxm + patch10.x[2][i]);
      patch10.y[1][i] = 0.5 * (yym + patch10.y[2][i]);
      patch00.x[3][i] = 0.5 * (patch00.x[2][i] + patch10.x[1][i]);
      patch00.y[3][i] = 0.5 * (patch00.y[2][i] + patch10.y[1][i]);
      patch10.x[0][i] = patch00.x[3][i];
      patch10.y[0][i] = patch00.y[3][i];
      patch10.x[3][i] = xx[3][i];
      patch10.y[3][i] = yy[3][i];
    }
    for (i = 4; i < 8; ++i) {
      patch01.x[0][i-4] = xx[0][i];
      patch01.y[0][i-4] = yy[0][i];
      patch01.x[1][i-4] = 0.5 * (xx[0][i] + xx[1][i]);
      patch01.y[1][i-4] = 0.5 * (yy[0][i] + yy[1][i]);
      xxm = 0.5 * (xx[1][i] + xx[2][i]);
      yym = 0.5 * (yy[1][i] + yy[2][i]);
      patch11.x[2][i-4] = 0.5 * (xx[2][i] + xx[3][i]);
      patch11.y[2][i-4] = 0.5 * (yy[2][i] + yy[3][i]);
      patch01.x[2][i-4] = 0.5 * (patch01.x[1][i-4] + xxm);
      patch01.y[2][i-4] = 0.5 * (patch01.y[1][i-4] + yym);
      patch11.x[1][i-4] = 0.5 * (xxm + patch11.x[2][i-4]);
      patch11.y[1][i-4] = 0.5 * (yym + patch11.y[2][i-4]);
      patch01.x[3][i-4] = 0.5 * (patch01.x[2][i-4] + patch11.x[1][i-4]);
      patch01.y[3][i-4] = 0.5 * (patch01.y[2][i-4] + patch11.y[1][i-4]);
      patch11.x[0][i-4] = patch01.x[3][i-4];
      patch11.y[0][i-4] = patch01.y[3][i-4];
      patch11.x[3][i-4] = xx[3][i];
      patch11.y[3][i-4] = yy[3][i];
    }
    for (i = 0; i < patchColorComps; ++i) {
      patch00.color[0][0].c[i] = patch->color[0][0].c[i];
      patch00.color[0][1].c[i] = (patch->color[0][0].c[i] +
				  patch->color[0][1].c[i]) / 2;
      patch01.color[0][0].c[i] = patch00.color[0][1].c[i];
      patch01.color[0][1].c[i] = patch->color[0][1].c[i];
      patch01.color[1][1].c[i] = (patch->color[0][1].c[i] +
				  patch->color[1][1].c[i]) / 2;
      patch11.color[0][1].c[i] = patch01.color[1][1].c[i];
      patch11.color[1][1].c[i] = patch->color[1][1].c[i];
      patch11.color[1][0].c[i] = (patch->color[1][1].c[i] +
				  patch->color[1][0].c[i]) / 2;
      patch10.color[1][1].c[i] = patch11.color[1][0].c[i];
      patch10.color[1][0].c[i] = patch->color[1][0].c[i];
      patch10.color[0][0].c[i] = (patch->color[1][0].c[i] +
				  patch->color[0][0].c[i]) / 2;
      patch00.color[1][0].c[i] = patch10.color[0][0].c[i];
      patch00.color[1][1].c[i] = (patch00.color[1][0].c[i] +
				  patch01.color[1][1].c[i]) / 2;
      patch01.color[1][0].c[i] = patch00.color[1][1].c[i];
      patch11.color[0][0].c[i] = patch00.color[1][1].c[i];
      patch10.color[0][1].c[i] = patch00.color[1][1].c[i];
    }
    fillPatch(&patch00, colorComps, patchColorComps, refineColorThreshold, depth + 1, shading);
    fillPatch(&patch10, colorComps, patchColorComps, refineColorThreshold, depth + 1, shading);
    fillPatch(&patch01, colorComps, patchColorComps, refineColorThreshold, depth + 1, shading);
    fillPatch(&patch11, colorComps, patchColorComps, refineColorThreshold, depth + 1, shading);
  }
}

void Gfx::doEndPath() {
  if (state->isCurPt() && clip != clipNone) {
    state->clip();
    if (clip == clipNormal) {
      out->clip(state);
    } else {
      out->eoClip(state);
    }
  }
  clip = clipNone;
  state->clearPath();
}

//------------------------------------------------------------------------
// path clipping operators
//------------------------------------------------------------------------

void Gfx::opClip(Object args[], int numArgs) {
  clip = clipNormal;
}

void Gfx::opEOClip(Object args[], int numArgs) {
  clip = clipEO;
}

//------------------------------------------------------------------------
// text object operators
//------------------------------------------------------------------------

void Gfx::opBeginText(Object args[], int numArgs) {
  out->beginTextObject(state);
  state->setTextMat(1, 0, 0, 1, 0, 0);
  state->textMoveTo(0, 0);
  out->updateTextMat(state);
  out->updateTextPos(state);
  fontChanged = gTrue;
}

void Gfx::opEndText(Object args[], int numArgs) {
  out->endTextObject(state);
}

//------------------------------------------------------------------------
// text state operators
//------------------------------------------------------------------------

void Gfx::opSetCharSpacing(Object args[], int numArgs) {
  state->setCharSpace(args[0].getNum());
  out->updateCharSpace(state);
}

void Gfx::opSetFont(Object args[], int numArgs) {
  GfxFont *font;

  if (!(font = res->lookupFont(args[0].getName()))) {
    // unsetting the font (drawing no text) is better than using the
    // previous one and drawing random glyphs from it
    state->setFont(NULL, args[1].getNum());
    fontChanged = gTrue;
    return;
  }
  if (printCommands) {
    printf("  font: tag=%s name='%s' %g\n",
	   font->getTag()->getCString(),
	   font->getName() ? font->getName()->getCString() : "???",
	   args[1].getNum());
    fflush(stdout);
  }

  font->incRefCnt();
  state->setFont(font, args[1].getNum());
  fontChanged = gTrue;
}

void Gfx::opSetTextLeading(Object args[], int numArgs) {
  state->setLeading(args[0].getNum());
}

void Gfx::opSetTextRender(Object args[], int numArgs) {
  state->setRender(args[0].getInt());
  out->updateRender(state);
}

void Gfx::opSetTextRise(Object args[], int numArgs) {
  state->setRise(args[0].getNum());
  out->updateRise(state);
}

void Gfx::opSetWordSpacing(Object args[], int numArgs) {
  state->setWordSpace(args[0].getNum());
  out->updateWordSpace(state);
}

void Gfx::opSetHorizScaling(Object args[], int numArgs) {
  state->setHorizScaling(args[0].getNum());
  out->updateHorizScaling(state);
  fontChanged = gTrue;
}

//------------------------------------------------------------------------
// text positioning operators
//------------------------------------------------------------------------

void Gfx::opTextMove(Object args[], int numArgs) {
  double tx, ty;

  tx = state->getLineX() + args[0].getNum();
  ty = state->getLineY() + args[1].getNum();
  state->textMoveTo(tx, ty);
  out->updateTextPos(state);
}

void Gfx::opTextMoveSet(Object args[], int numArgs) {
  double tx, ty;

  tx = state->getLineX() + args[0].getNum();
  ty = args[1].getNum();
  state->setLeading(-ty);
  ty += state->getLineY();
  state->textMoveTo(tx, ty);
  out->updateTextPos(state);
}

void Gfx::opSetTextMatrix(Object args[], int numArgs) {
  state->setTextMat(args[0].getNum(), args[1].getNum(),
		    args[2].getNum(), args[3].getNum(),
		    args[4].getNum(), args[5].getNum());
  state->textMoveTo(0, 0);
  out->updateTextMat(state);
  out->updateTextPos(state);
  fontChanged = gTrue;
}

void Gfx::opTextNextLine(Object args[], int numArgs) {
  double tx, ty;

  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  out->updateTextPos(state);
}

//------------------------------------------------------------------------
// text string operators
//------------------------------------------------------------------------

void Gfx::opShowText(Object args[], int numArgs) {
  if (!state->getFont()) {
    error(errSyntaxError, getPos(), "No font in show");
    return;
  }
  if (fontChanged) {
    out->updateFont(state);
    fontChanged = gFalse;
  }
  out->beginStringOp(state);
  doShowText(args[0].getString());
  out->endStringOp(state);
  if (!ocState) {
    doIncCharCount(args[0].getString());
  }
}

void Gfx::opMoveShowText(Object args[], int numArgs) {
  double tx, ty;

  if (!state->getFont()) {
    error(errSyntaxError, getPos(), "No font in move/show");
    return;
  }
  if (fontChanged) {
    out->updateFont(state);
    fontChanged = gFalse;
  }
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  out->updateTextPos(state);
  out->beginStringOp(state);
  doShowText(args[0].getString());
  out->endStringOp(state);
  if (!ocState) {
    doIncCharCount(args[0].getString());
  }
}

void Gfx::opMoveSetShowText(Object args[], int numArgs) {
  double tx, ty;

  if (!state->getFont()) {
    error(errSyntaxError, getPos(), "No font in move/set/show");
    return;
  }
  if (fontChanged) {
    out->updateFont(state);
    fontChanged = gFalse;
  }
  state->setWordSpace(args[0].getNum());
  state->setCharSpace(args[1].getNum());
  tx = state->getLineX();
  ty = state->getLineY() - state->getLeading();
  state->textMoveTo(tx, ty);
  out->updateWordSpace(state);
  out->updateCharSpace(state);
  out->updateTextPos(state);
  out->beginStringOp(state);
  doShowText(args[2].getString());
  out->endStringOp(state);
  if (ocState) {
    doIncCharCount(args[2].getString());
  }
}

void Gfx::opShowSpaceText(Object args[], int numArgs) {
  Array *a;
  Object obj;
  int wMode;
  int i;

  if (!state->getFont()) {
    error(errSyntaxError, getPos(), "No font in show/space");
    return;
  }
  if (fontChanged) {
    out->updateFont(state);
    fontChanged = gFalse;
  }
  out->beginStringOp(state);
  wMode = state->getFont()->getWMode();
  a = args[0].getArray();
  for (i = 0; i < a->getLength(); ++i) {
    a->get(i, &obj);
    if (obj.isNum()) {
      // this uses the absolute value of the font size to match
      // Acrobat's behavior
      if (wMode) {
        state->textShift(0, -obj.getNum() * 0.001 *
          state->getFontSize());
      } else {
        state->textShift(-obj.getNum() * 0.001 *
          state->getFontSize() *
          state->getHorizScaling(), 0);
      }
      out->updateTextShift(state, obj.getNum());
    } else if (obj.isString()) {
      doShowText(obj.getString());
    } else {
      error(errSyntaxError, getPos(),
        "Element of show/space array must be number or string");
    }
    obj.free();
  }
  out->endStringOp(state);
  if (!ocState) {
    a = args[0].getArray();
    for (i = 0; i < a->getLength(); ++i) {
      a->get(i, &obj);
      if (obj.isString()) {
	doIncCharCount(obj.getString());
      }
      obj.free();
    }
  }
}

void Gfx::doShowText(GooString *s) {
  GfxFont *font;
  int wMode;
  double riseX, riseY;
  CharCode code;
  Unicode *u = NULL;
  double x, y, dx, dy, dx2, dy2, curX, curY, tdx, tdy, ddx, ddy;
  double originX, originY, tOriginX, tOriginY;
  double x0, y0, x1, y1;
  double oldCTM[6], newCTM[6];
  double *mat;
  Object charProc;
  Dict *resDict;
  Parser *oldParser;
  GfxState *savedState;
  char *p;
  int render;
  GBool patternFill;
  int len, n, uLen, nChars, nSpaces, i;

  font = state->getFont();
  wMode = font->getWMode();

  if (out->useDrawChar()) {
    out->beginString(state, s);
  }

  // if we're doing a pattern fill, set up clipping
  render = state->getRender();
  if (!(render & 1) &&
      state->getFillColorSpace()->getMode() == csPattern) {
    patternFill = gTrue;
    saveState();
    // disable fill, enable clipping, leave stroke unchanged
    if ((render ^ (render >> 1)) & 1) {
      render = 5;
    } else {
      render = 7;
    }
    state->setRender(render);
    out->updateRender(state);
  } else {
    patternFill = gFalse;
  }

  state->textTransformDelta(0, state->getRise(), &riseX, &riseY);
  x0 = state->getCurX() + riseX;
  y0 = state->getCurY() + riseY;

  // handle a Type 3 char
  if (font->getType() == fontType3 && out->interpretType3Chars()) {
    mat = state->getCTM();
    for (i = 0; i < 6; ++i) {
      oldCTM[i] = mat[i];
    }
    mat = state->getTextMat();
    newCTM[0] = mat[0] * oldCTM[0] + mat[1] * oldCTM[2];
    newCTM[1] = mat[0] * oldCTM[1] + mat[1] * oldCTM[3];
    newCTM[2] = mat[2] * oldCTM[0] + mat[3] * oldCTM[2];
    newCTM[3] = mat[2] * oldCTM[1] + mat[3] * oldCTM[3];
    mat = font->getFontMatrix();
    newCTM[0] = mat[0] * newCTM[0] + mat[1] * newCTM[2];
    newCTM[1] = mat[0] * newCTM[1] + mat[1] * newCTM[3];
    newCTM[2] = mat[2] * newCTM[0] + mat[3] * newCTM[2];
    newCTM[3] = mat[2] * newCTM[1] + mat[3] * newCTM[3];
    newCTM[0] *= state->getFontSize();
    newCTM[1] *= state->getFontSize();
    newCTM[2] *= state->getFontSize();
    newCTM[3] *= state->getFontSize();
    newCTM[0] *= state->getHorizScaling();
    newCTM[2] *= state->getHorizScaling();
    curX = state->getCurX();
    curY = state->getCurY();
    oldParser = parser;
    p = s->getCString();
    len = s->getLength();
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
			    &u, &uLen,
			    &dx, &dy, &originX, &originY);
      dx = dx * state->getFontSize() + state->getCharSpace();
      if (n == 1 && *p == ' ') {
	dx += state->getWordSpace();
      }
      dx *= state->getHorizScaling();
      dy *= state->getFontSize();
      state->textTransformDelta(dx, dy, &tdx, &tdy);
      state->transform(curX + riseX, curY + riseY, &x, &y);
      savedState = saveStateStack();
      state->setCTM(newCTM[0], newCTM[1], newCTM[2], newCTM[3], x, y);
      //~ the CTM concat values here are wrong (but never used)
      out->updateCTM(state, 1, 0, 0, 1, 0, 0);
      state->transformDelta(dx, dy, &ddx, &ddy);
      if (!out->beginType3Char(state, curX + riseX, curY + riseY, ddx, ddy,
			       code, u, uLen)) {
	((Gfx8BitFont *)font)->getCharProc(code, &charProc);
	if ((resDict = ((Gfx8BitFont *)font)->getResources())) {
	  pushResources(resDict);
	}
	if (charProc.isStream()) {
	  display(&charProc, gFalse);
	} else {
	  error(errSyntaxError, getPos(), "Missing or bad Type3 CharProc entry");
	}
	out->endType3Char(state);
	if (resDict) {
	  popResources();
	}
	charProc.free();
      }
      restoreStateStack(savedState);
      // GfxState::restore() does *not* restore the current position,
      // so we deal with it here using (curX, curY) and (lineX, lineY)
      curX += tdx;
      curY += tdy;
      state->moveTo(curX, curY);
      p += n;
      len -= n;
    }
    parser = oldParser;

  } else if (out->useDrawChar()) {
    p = s->getCString();
    len = s->getLength();
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
			    &u, &uLen,
			    &dx, &dy, &originX, &originY);
      if (wMode) {
	dx *= state->getFontSize();
	dy = dy * state->getFontSize() + state->getCharSpace();
	if (n == 1 && *p == ' ') {
	  dy += state->getWordSpace();
	}
      } else {
	dx = dx * state->getFontSize() + state->getCharSpace();
	if (n == 1 && *p == ' ') {
	  dx += state->getWordSpace();
	}
	dx *= state->getHorizScaling();
	dy *= state->getFontSize();
      }
      state->textTransformDelta(dx, dy, &tdx, &tdy);
      originX *= state->getFontSize();
      originY *= state->getFontSize();
      state->textTransformDelta(originX, originY, &tOriginX, &tOriginY);
      if (ocState)
        out->drawChar(state, state->getCurX() + riseX, state->getCurY() + riseY,
		      tdx, tdy, tOriginX, tOriginY, code, n, u, uLen);
      state->shift(tdx, tdy);
      p += n;
      len -= n;
    }
  } else {
    dx = dy = 0;
    p = s->getCString();
    len = s->getLength();
    nChars = nSpaces = 0;
    while (len > 0) {
      n = font->getNextChar(p, len, &code,
			    &u, &uLen,
			    &dx2, &dy2, &originX, &originY);
      dx += dx2;
      dy += dy2;
      if (n == 1 && *p == ' ') {
	++nSpaces;
      }
      ++nChars;
      p += n;
      len -= n;
    }
    if (wMode) {
      dx *= state->getFontSize();
      dy = dy * state->getFontSize()
	   + nChars * state->getCharSpace()
	   + nSpaces * state->getWordSpace();
    } else {
      dx = dx * state->getFontSize()
	   + nChars * state->getCharSpace()
	   + nSpaces * state->getWordSpace();
      dx *= state->getHorizScaling();
      dy *= state->getFontSize();
    }
    state->textTransformDelta(dx, dy, &tdx, &tdy);
    if (ocState)
      out->drawString(state, s);
    state->shift(tdx, tdy);
  }

  if (out->useDrawChar()) {
    out->endString(state);
  }

  if (patternFill && ocState) {
    out->saveTextPos(state);
    // tell the OutputDev to do the clipping
    out->endTextObject(state);
    // set up a clipping bbox so doPatternText will work -- assume
    // that the text bounding box does not extend past the baseline in
    // any direction by more than twice the font size
    x1 = state->getCurX() + riseX;
    y1 = state->getCurY() + riseY;
    if (x0 > x1) {
      x = x0; x0 = x1; x1 = x;
    }
    if (y0 > y1) {
      y = y0; y0 = y1; y1 = y;
    }
    state->textTransformDelta(0, state->getFontSize(), &dx, &dy);
    state->textTransformDelta(state->getFontSize(), 0, &dx2, &dy2);
    dx = fabs(dx);
    dx2 = fabs(dx2);
    if (dx2 > dx) {
      dx = dx2;
    }
    dy = fabs(dy);
    dy2 = fabs(dy2);
    if (dy2 > dy) {
      dy = dy2;
    }
    state->clipToRect(x0 - 2 * dx, y0 - 2 * dy, x1 + 2 * dx, y1 + 2 * dy);
    // set render mode to fill-only
    state->setRender(0);
    out->updateRender(state);
    doPatternText();
    restoreState();
    out->restoreTextPos(state);
  }

  updateLevel += 10 * s->getLength();
}

// NB: this is only called when ocState is false.
void Gfx::doIncCharCount(GooString *s) {
  if (out->needCharCount()) {
    out->incCharCount(s->getLength());
  }
}

//------------------------------------------------------------------------
// XObject operators
//------------------------------------------------------------------------

void Gfx::opXObject(Object args[], int numArgs) {
  char *name;
  Object obj1, obj2, obj3, refObj;
#if OPI_SUPPORT
  Object opiDict;
#endif

  if (!ocState && !out->needCharCount()) {
    return;
  }
  name = args[0].getName();
  if (!res->lookupXObject(name, &obj1)) {
    return;
  }
  if (!obj1.isStream()) {
      error(errSyntaxError, getPos(), "XObject '{0:s}' is wrong type", name);
    obj1.free();
    return;
  }

#if OPI_SUPPORT
  obj1.streamGetDict()->lookup("OPI", &opiDict);
  if (opiDict.isDict()) {
    out->opiBegin(state, opiDict.getDict());
  }
#endif
  obj1.streamGetDict()->lookup("Subtype", &obj2);
  if (obj2.isName("Image")) {
    if (out->needNonText()) {
      res->lookupXObjectNF(name, &refObj);
      doImage(&refObj, obj1.getStream(), gFalse);
      refObj.free();
    }
  } else if (obj2.isName("Form")) {
    res->lookupXObjectNF(name, &refObj);
    GBool shouldDoForm = gTrue;
    std::set<int>::iterator drawingFormIt;
    if (refObj.isRef()) {
      const int num = refObj.getRef().num;
      if (formsDrawing.find(num) == formsDrawing.end()) {
	drawingFormIt = formsDrawing.insert(num).first;
      } else {
	shouldDoForm = gFalse;	
      }
    }
    if (shouldDoForm) {
      if (out->useDrawForm() && refObj.isRef()) {
	out->drawForm(refObj.getRef());
      } else {
	doForm(&obj1);
      }
    }
    if (refObj.isRef() && shouldDoForm) {
      formsDrawing.erase(drawingFormIt);
    }
    refObj.free();
  } else if (obj2.isName("PS")) {
    obj1.streamGetDict()->lookup("Level1", &obj3);
    out->psXObject(obj1.getStream(),
		   obj3.isStream() ? obj3.getStream() : (Stream *)NULL);
  } else if (obj2.isName()) {
    error(errSyntaxError, getPos(), "Unknown XObject subtype '{0:s}'", obj2.getName());
  } else {
    error(errSyntaxError, getPos(), "XObject subtype is missing or wrong type");
  }
  obj2.free();
#if OPI_SUPPORT
  if (opiDict.isDict()) {
    out->opiEnd(state, opiDict.getDict());
  }
  opiDict.free();
#endif
  obj1.free();
}

void Gfx::doImage(Object *ref, Stream *str, GBool inlineImg) {
  Dict *dict, *maskDict;
  int width, height;
  int bits, maskBits;
  GBool interpolate;
  StreamColorSpaceMode csMode;
  GBool mask;
  GBool invert;
  GfxColorSpace *colorSpace, *maskColorSpace;
  GfxImageColorMap *colorMap, *maskColorMap;
  Object maskObj, smaskObj;
  GBool haveColorKeyMask, haveExplicitMask, haveSoftMask;
  int maskColors[2*gfxColorMaxComps];
  int maskWidth, maskHeight;
  GBool maskInvert;
  GBool maskInterpolate;
  Stream *maskStr;
  Object obj1, obj2;
  int i, n;

  // get info from the stream
  bits = 0;
  csMode = streamCSNone;
  str->getImageParams(&bits, &csMode);

  // get stream dict
  dict = str->getDict();

  // check for optional content key
  if (ref) {
    dict->lookupNF("OC", &obj1);
    if (catalog->getOptContentConfig() && !catalog->getOptContentConfig()->optContentIsVisible(&obj1)) {
      obj1.free();
      return;
    }
    obj1.free();
  }

  // get size
  dict->lookup("Width", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("W", &obj1);
  }
  if (obj1.isInt())
    width = obj1.getInt();
  else if (obj1.isReal())
    width = (int)obj1.getReal();
  else
    goto err2;
  obj1.free();
  dict->lookup("Height", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("H", &obj1);
  }
  if (obj1.isInt())
    height = obj1.getInt();
  else if (obj1.isReal())
    height = (int)obj1.getReal();
  else
    goto err2;
  obj1.free();

  if (width < 1 || height < 1)
    goto err1;

  // image interpolation
  dict->lookup("Interpolate", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("I", &obj1);
  }
  if (obj1.isBool())
    interpolate = obj1.getBool();
  else
    interpolate = gFalse;
  obj1.free();
  maskInterpolate = gFalse;

  // image or mask?
  dict->lookup("ImageMask", &obj1);
  if (obj1.isNull()) {
    obj1.free();
    dict->lookup("IM", &obj1);
  }
  mask = gFalse;
  if (obj1.isBool())
    mask = obj1.getBool();
  else if (!obj1.isNull())
    goto err2;
  obj1.free();

  // bit depth
  if (bits == 0) {
    dict->lookup("BitsPerComponent", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("BPC", &obj1);
    }
    if (obj1.isInt()) {
      bits = obj1.getInt();
    } else if (mask) {
      bits = 1;
    } else {
      goto err2;
    }
    obj1.free();
  }

  // display a mask
  if (mask) {

    // check for inverted mask
    if (bits != 1)
      goto err1;
    invert = gFalse;
    dict->lookup("Decode", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("D", &obj1);
    }
    if (obj1.isArray()) {
      obj1.arrayGet(0, &obj2);
      // Table 4.39 says /Decode must be [1 0] or [0 1]. Adobe
      // accepts [1.0 0.0] as well.
      if (obj2.isNum() && obj2.getNum() >= 0.9)
	invert = gTrue;
      obj2.free();
    } else if (!obj1.isNull()) {
      goto err2;
    }
    obj1.free();

    // if drawing is disabled, skip over inline image data
    if (!ocState || !out->needNonText()) {
      str->reset();
      n = height * ((width + 7) / 8);
      for (i = 0; i < n; ++i) {
	str->getChar();
      }
      str->close();

    // draw it
    } else {
      if (state->getFillColorSpace()->getMode() == csPattern) {
	doPatternImageMask(ref, str, width, height, invert, inlineImg);
      } else {
	out->drawImageMask(state, ref, str, width, height, invert, interpolate, inlineImg);
      }
    }
  } else {

    // get color space and color map
    dict->lookup("ColorSpace", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("CS", &obj1);
    }
    if (obj1.isName() && inlineImg) {
      res->lookupColorSpace(obj1.getName(), &obj2);
      if (!obj2.isNull()) {
	obj1.free();
	obj1 = obj2;
      } else {
	obj2.free();
      }
    }
    if (!obj1.isNull()) {
      Object objIntent;
      char *tempIntent = NULL;
      dict->lookup("Intent", &objIntent);
      if (objIntent.isName()) {
        tempIntent = state->getRenderingIntent();
        if (tempIntent != NULL) {
          tempIntent = strdup(tempIntent);
        }
        state->setRenderingIntent(objIntent.getName());
      }
      colorSpace = GfxColorSpace::parse(&obj1, out, state);
      if (objIntent.isName()) {
        state->setRenderingIntent(tempIntent);
        free(tempIntent);
      }
      objIntent.free();
    } else if (csMode == streamCSDeviceGray) {
      Object objCS;
      res->lookupColorSpace("DefaultGray", &objCS);
      if (objCS.isNull()) {
        colorSpace = new GfxDeviceGrayColorSpace();
      } else {
        colorSpace = GfxColorSpace::parse(&objCS, out, state);
      }
      objCS.free();
    } else if (csMode == streamCSDeviceRGB) {
      Object objCS;
      res->lookupColorSpace("DefaultRGB", &objCS);
      if (objCS.isNull()) {
        colorSpace = new GfxDeviceRGBColorSpace();
      } else {
        colorSpace = GfxColorSpace::parse(&objCS, out, state);
      }
      objCS.free();
    } else if (csMode == streamCSDeviceCMYK) {
      Object objCS;
      res->lookupColorSpace("DefaultCMYK", &objCS);
      if (objCS.isNull()) {
        colorSpace = new GfxDeviceCMYKColorSpace();
      } else {
        colorSpace = GfxColorSpace::parse(&objCS, out, state);
      }
      objCS.free();
    } else {
      colorSpace = NULL;
    }
    obj1.free();
    if (!colorSpace) {
      goto err1;
    }
    dict->lookup("Decode", &obj1);
    if (obj1.isNull()) {
      obj1.free();
      dict->lookup("D", &obj1);
    }
    if (bits == 0) {
      delete colorSpace;
      goto err2;
    }
    colorMap = new GfxImageColorMap(bits, &obj1, colorSpace);
    obj1.free();
    if (!colorMap->isOk()) {
      delete colorMap;
      goto err1;
    }

    // get the mask
    haveColorKeyMask = haveExplicitMask = haveSoftMask = gFalse;
    maskStr = NULL; // make gcc happy
    maskWidth = maskHeight = 0; // make gcc happy
    maskInvert = gFalse; // make gcc happy
    maskColorMap = NULL; // make gcc happy
    dict->lookup("Mask", &maskObj);
    dict->lookup("SMask", &smaskObj);
    if (smaskObj.isStream()) {
      // soft mask
      if (inlineImg) {
	goto err1;
      }
      maskStr = smaskObj.getStream();
      maskDict = smaskObj.streamGetDict();
      maskDict->lookup("Width", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("W", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskWidth = obj1.getInt();
      obj1.free();
      maskDict->lookup("Height", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("H", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskHeight = obj1.getInt();
      obj1.free();
      maskDict->lookup("Interpolate", &obj1);
      if (obj1.isNull()) {
        obj1.free();
        maskDict->lookup("I", &obj1);
      }
      if (obj1.isBool())
        maskInterpolate = obj1.getBool();
      else
        maskInterpolate = gFalse;
      obj1.free();
      maskDict->lookup("BitsPerComponent", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("BPC", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskBits = obj1.getInt();
      obj1.free();
      maskDict->lookup("ColorSpace", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("CS", &obj1);
      }
      if (obj1.isName()) {
	res->lookupColorSpace(obj1.getName(), &obj2);
	if (!obj2.isNull()) {
	  obj1.free();
	  obj1 = obj2;
	} else {
	  obj2.free();
	}
      }
      maskColorSpace = GfxColorSpace::parse(&obj1, out, state);
      obj1.free();
      if (!maskColorSpace || maskColorSpace->getMode() != csDeviceGray) {
	goto err1;
      }
      maskDict->lookup("Decode", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("D", &obj1);
      }
      maskColorMap = new GfxImageColorMap(maskBits, &obj1, maskColorSpace);
      obj1.free();
      if (!maskColorMap->isOk()) {
	delete maskColorMap;
	goto err1;
      }
      //~ handle the Matte entry
      haveSoftMask = gTrue;
    } else if (maskObj.isArray()) {
      // color key mask
      for (i = 0;
	   i < maskObj.arrayGetLength() && i < 2*gfxColorMaxComps;
	   ++i) {
	maskObj.arrayGet(i, &obj1);
	if (obj1.isInt()) {
	  maskColors[i] = obj1.getInt();
	} else if (obj1.isReal()) {
	  error(errSyntaxError, -1, "Mask entry should be an integer but it's a real, trying to use it");
	  maskColors[i] = (int) obj1.getReal();
	} else {
	  error(errSyntaxError, -1, "Mask entry should be an integer but it's of type {0:d}", obj1.getType());
	  obj1.free();
	  goto err1;
	}
	obj1.free();
      }
      haveColorKeyMask = gTrue;
    } else if (maskObj.isStream()) {
      // explicit mask
      if (inlineImg) {
	goto err1;
      }
      maskStr = maskObj.getStream();
      maskDict = maskObj.streamGetDict();
      maskDict->lookup("Width", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("W", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskWidth = obj1.getInt();
      obj1.free();
      maskDict->lookup("Height", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("H", &obj1);
      }
      if (!obj1.isInt()) {
	goto err2;
      }
      maskHeight = obj1.getInt();
      obj1.free();
      maskDict->lookup("Interpolate", &obj1);
      if (obj1.isNull()) {
        obj1.free();
	maskDict->lookup("I", &obj1);
      }
      if (obj1.isBool())
        maskInterpolate = obj1.getBool();
      else
        maskInterpolate = gFalse;
      obj1.free();
      maskDict->lookup("ImageMask", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("IM", &obj1);
      }
      if (!obj1.isBool() || !obj1.getBool()) {
	goto err2;
      }
      obj1.free();
      maskInvert = gFalse;
      maskDict->lookup("Decode", &obj1);
      if (obj1.isNull()) {
	obj1.free();
	maskDict->lookup("D", &obj1);
      }
      if (obj1.isArray()) {
	obj1.arrayGet(0, &obj2);
	// Table 4.39 says /Decode must be [1 0] or [0 1]. Adobe
	// accepts [1.0 0.0] as well.
	if (obj2.isNum() && obj2.getNum() >= 0.9) {
	  maskInvert = gTrue;
	}
	obj2.free();
      } else if (!obj1.isNull()) {
	goto err2;
      }
      obj1.free();
      haveExplicitMask = gTrue;
    }

    // if drawing is disabled, skip over inline image data
    if (!ocState || !out->needNonText()) {
      str->reset();
      n = height * ((width * colorMap->getNumPixelComps() *
		     colorMap->getBits() + 7) / 8);
      for (i = 0; i < n; ++i) {
	str->getChar();
      }
      str->close();

    // draw it
    } else {
      if (haveSoftMask) {
	out->drawSoftMaskedImage(state, ref, str, width, height, colorMap, interpolate,
				 maskStr, maskWidth, maskHeight, maskColorMap, maskInterpolate);
	delete maskColorMap;
      } else if (haveExplicitMask) {
	out->drawMaskedImage(state, ref, str, width, height, colorMap, interpolate,
			     maskStr, maskWidth, maskHeight, maskInvert, maskInterpolate);
      } else {
	out->drawImage(state, ref, str, width, height, colorMap, interpolate,
		       haveColorKeyMask ? maskColors : (int *)NULL, inlineImg);
      }
    }
    delete colorMap;

    maskObj.free();
    smaskObj.free();
  }

  if ((i = width * height) > 1000) {
    i = 1000;
  }
  updateLevel += i;

  return;

 err2:
  obj1.free();
 err1:
  error(errSyntaxError, getPos(), "Bad image parameters");
}

GBool Gfx::checkTransparencyGroup(Dict *resDict) {
  // check the effect of compositing objects as a group:
  // look for ExtGState entries with ca != 1 or CA != 1 or BM != normal
  Object extGStates;
  GBool transpGroup = gFalse;
  double opac;

  if (resDict == NULL)
    return gFalse;
  pushResources(resDict);
  resDict->lookup("ExtGState", &extGStates);
  if (extGStates.isDict()) {
    Dict *dict = extGStates.getDict();
    for (int i = 0; i < dict->getLength() && !transpGroup; i++) {
      Object obj1, obj2;
      GfxBlendMode mode;

      if (res->lookupGState(dict->getKey(i), &obj1) && obj1.isDict()) {
        if (!obj1.dictLookup("BM", &obj2)->isNull()) {
          if (state->parseBlendMode(&obj2, &mode)) {
            if (mode != gfxBlendNormal)
              transpGroup = gTrue;
          } else {
            error(errSyntaxError, getPos(), "Invalid blend mode in ExtGState");
          }
        }
        obj2.free();
        if (obj1.dictLookup("ca", &obj2)->isNum()) {
          opac = obj2.getNum();
          opac = opac < 0 ? 0 : opac > 1 ? 1 : opac;
          if (opac != 1)
            transpGroup = gTrue;
        }
        obj2.free();
        if (obj1.dictLookup("CA", &obj2)->isNum()) {
          opac = obj2.getNum();
          opac = opac < 0 ? 0 : opac > 1 ? 1 : opac;
          if (opac != 1)
            transpGroup = gTrue;
        }
        obj2.free();
        // alpha is shape
        if (!transpGroup && obj1.dictLookup("AIS", &obj2)->isBool()) {
          transpGroup = obj2.getBool();
        }
        obj2.free();
        // soft mask
        if (!transpGroup && !obj1.dictLookup("SMask", &obj2)->isNull()) {
          if (!obj2.isName("None")) {
            transpGroup = gTrue;
          }
        }
        obj2.free();
      }
      obj1.free();
    }
  }
  extGStates.free();
  popResources();
  return transpGroup;
}

void Gfx::doForm(Object *str) {
  Dict *dict;
  GBool transpGroup, isolated, knockout;
  GfxColorSpace *blendingColorSpace;
  Object matrixObj, bboxObj;
  double m[6], bbox[4];
  Object resObj;
  Dict *resDict;
  GBool ocSaved;
  Object obj1, obj2, obj3;
  int i;

  // check for excessive recursion
  if (formDepth > 100) {
    return;
  }

  // get stream dict
  dict = str->streamGetDict();

  // check form type
  dict->lookup("FormType", &obj1);
  if (!(obj1.isNull() || (obj1.isInt() && obj1.getInt() == 1))) {
    error(errSyntaxError, getPos(), "Unknown form type");
  }
  obj1.free();

  // check for optional content key
  ocSaved = ocState;
  dict->lookupNF("OC", &obj1);
  if (catalog->getOptContentConfig() && !catalog->getOptContentConfig()->optContentIsVisible(&obj1)) {
    obj1.free();
    if (out->needCharCount()) {
      ocState = gFalse;
    } else {
      return;
    }
  }
  obj1.free();

  // get bounding box
  dict->lookup("BBox", &bboxObj);
  if (!bboxObj.isArray()) {
    bboxObj.free();
    error(errSyntaxError, getPos(), "Bad form bounding box");
    ocState = ocSaved;
    return;
  }
  for (i = 0; i < 4; ++i) {
    bboxObj.arrayGet(i, &obj1);
    if (likely(obj1.isNum())) {
      bbox[i] = obj1.getNum();
      obj1.free();
    } else {
      obj1.free();
      error(errSyntaxError, getPos(), "Bad form bounding box value");
      return;
    }
  }
  bboxObj.free();

  // get matrix
  dict->lookup("Matrix", &matrixObj);
  if (matrixObj.isArray()) {
    for (i = 0; i < 6; ++i) {
      matrixObj.arrayGet(i, &obj1);
      if (likely(obj1.isNum())) m[i] = obj1.getNum();
      else m[i] = 0;
      obj1.free();
    }
  } else {
    m[0] = 1; m[1] = 0;
    m[2] = 0; m[3] = 1;
    m[4] = 0; m[5] = 0;
  }
  matrixObj.free();

  // get resources
  dict->lookup("Resources", &resObj);
  resDict = resObj.isDict() ? resObj.getDict() : (Dict *)NULL;

  // check for a transparency group
  transpGroup = isolated = knockout = gFalse;
  blendingColorSpace = NULL;
  if (dict->lookup("Group", &obj1)->isDict()) {
    if (obj1.dictLookup("S", &obj2)->isName("Transparency")) {
      if (!obj1.dictLookup("CS", &obj3)->isNull()) {
	blendingColorSpace = GfxColorSpace::parse(&obj3, out, state);
      }
      obj3.free();
      if (obj1.dictLookup("I", &obj3)->isBool()) {
	isolated = obj3.getBool();
      }
      obj3.free();
      if (obj1.dictLookup("K", &obj3)->isBool()) {
	knockout = obj3.getBool();
      }
      obj3.free();
      transpGroup = isolated || out->checkTransparencyGroup(state, knockout) || checkTransparencyGroup(resDict);
    }
    obj2.free();
  }
  obj1.free();

  // draw it
  ++formDepth;
  drawForm(str, resDict, m, bbox,
	  transpGroup, gFalse, blendingColorSpace, isolated, knockout);
  --formDepth;

  if (blendingColorSpace) {
    delete blendingColorSpace;
  }
  resObj.free();

  ocState = ocSaved;
}

void Gfx::drawForm(Object *str, Dict *resDict, double *matrix, double *bbox,
		  GBool transpGroup, GBool softMask,
		  GfxColorSpace *blendingColorSpace,
		  GBool isolated, GBool knockout,
		  GBool alpha, Function *transferFunc,
		  GfxColor *backdropColor) {
  Parser *oldParser;
  GfxState *savedState;
  double oldBaseMatrix[6];
  int i;

  // push new resources on stack
  pushResources(resDict);

  // save current graphics state
  savedState = saveStateStack();

  // kill any pre-existing path
  state->clearPath();

  // save current parser
  oldParser = parser;

  // set form transformation matrix
  state->concatCTM(matrix[0], matrix[1], matrix[2],
		   matrix[3], matrix[4], matrix[5]);
  out->updateCTM(state, matrix[0], matrix[1], matrix[2],
		 matrix[3], matrix[4], matrix[5]);

  // set form bounding box
  state->moveTo(bbox[0], bbox[1]);
  state->lineTo(bbox[2], bbox[1]);
  state->lineTo(bbox[2], bbox[3]);
  state->lineTo(bbox[0], bbox[3]);
  state->closePath();
  state->clip();
  out->clip(state);
  state->clearPath();

  if (softMask || transpGroup) {
    if (state->getBlendMode() != gfxBlendNormal) {
      state->setBlendMode(gfxBlendNormal);
      out->updateBlendMode(state);
    }
    if (state->getFillOpacity() != 1) {
      state->setFillOpacity(1);
      out->updateFillOpacity(state);
    }
    if (state->getStrokeOpacity() != 1) {
      state->setStrokeOpacity(1);
      out->updateStrokeOpacity(state);
    }
    out->clearSoftMask(state);
    out->beginTransparencyGroup(state, bbox, blendingColorSpace,
				isolated, knockout, softMask);
  }

  // set new base matrix
  for (i = 0; i < 6; ++i) {
    oldBaseMatrix[i] = baseMatrix[i];
    baseMatrix[i] = state->getCTM()[i];
  }

  GfxState *stateBefore = state;

  // draw the form
  display(str, gFalse);
  
  if (stateBefore != state) {
    if (state->isParentState(stateBefore)) {
      error(errSyntaxError, -1, "There's a form with more q than Q, trying to fix");
      while (stateBefore != state) {
        restoreState();
      }
    } else {
      error(errSyntaxError, -1, "There's a form with more Q than q");
    }
  }

  if (softMask || transpGroup) {
    out->endTransparencyGroup(state);
  }

  // restore base matrix
  for (i = 0; i < 6; ++i) {
    baseMatrix[i] = oldBaseMatrix[i];
  }

  // restore parser
  parser = oldParser;

  // restore graphics state
  restoreStateStack(savedState);

  // pop resource stack
  popResources();

  if (softMask) {
    out->setSoftMask(state, bbox, alpha, transferFunc, backdropColor);
  } else if (transpGroup) {
    out->paintTransparencyGroup(state, bbox);
  }

  return;
}

//------------------------------------------------------------------------
// in-line image operators
//------------------------------------------------------------------------

void Gfx::opBeginImage(Object args[], int numArgs) {
  Stream *str;
  int c1, c2;

  // NB: this function is run even if ocState is false -- doImage() is
  // responsible for skipping over the inline image data

  // build dict/stream
  str = buildImageStream();

  // display the image
  if (str) {
    doImage(NULL, str, gTrue);
  
    // skip 'EI' tag
    c1 = str->getUndecodedStream()->getChar();
    c2 = str->getUndecodedStream()->getChar();
    while (!(c1 == 'E' && c2 == 'I') && c2 != EOF) {
      c1 = c2;
      c2 = str->getUndecodedStream()->getChar();
    }
    delete str;
  }
}

Stream *Gfx::buildImageStream() {
  Object dict;
  Object obj;
  char *key;
  Stream *str;

  // build dictionary
  dict.initDict(xref);
  parser->getObj(&obj);
  while (!obj.isCmd("ID") && !obj.isEOF()) {
    if (!obj.isName()) {
      error(errSyntaxError, getPos(), "Inline image dictionary key must be a name object");
      obj.free();
    } else {
      key = copyString(obj.getName());
      obj.free();
      parser->getObj(&obj);
      if (obj.isEOF() || obj.isError()) {
	gfree(key);
	break;
      }
      dict.dictAdd(key, &obj);
    }
    parser->getObj(&obj);
  }
  if (obj.isEOF()) {
    error(errSyntaxError, getPos(), "End of file in inline image");
    obj.free();
    dict.free();
    return NULL;
  }
  obj.free();

  // make stream
  if (parser->getStream()) {
    str = new EmbedStream(parser->getStream(), &dict, gFalse, 0);
    str = str->addFilters(&dict);
  } else {
    str = NULL;
    dict.free();
  }

  return str;
}

void Gfx::opImageData(Object args[], int numArgs) {
  error(errInternal, getPos(), "Got 'ID' operator");
}

void Gfx::opEndImage(Object args[], int numArgs) {
  error(errInternal, getPos(), "Got 'EI' operator");
}

//------------------------------------------------------------------------
// type 3 font operators
//------------------------------------------------------------------------

void Gfx::opSetCharWidth(Object args[], int numArgs) {
  out->type3D0(state, args[0].getNum(), args[1].getNum());
}

void Gfx::opSetCacheDevice(Object args[], int numArgs) {
  out->type3D1(state, args[0].getNum(), args[1].getNum(),
	       args[2].getNum(), args[3].getNum(),
	       args[4].getNum(), args[5].getNum());
}

//------------------------------------------------------------------------
// compatibility operators
//------------------------------------------------------------------------

void Gfx::opBeginIgnoreUndef(Object args[], int numArgs) {
  ++ignoreUndef;
}

void Gfx::opEndIgnoreUndef(Object args[], int numArgs) {
  if (ignoreUndef > 0)
    --ignoreUndef;
}

//------------------------------------------------------------------------
// marked content operators
//------------------------------------------------------------------------

enum GfxMarkedContentKind {
  gfxMCOptionalContent,
  gfxMCActualText,
  gfxMCOther
};

struct MarkedContentStack {
  GfxMarkedContentKind kind;
  GBool ocSuppressed;       // are we ignoring content based on OptionalContent?
  MarkedContentStack *next; // next object on stack
};

void Gfx::popMarkedContent() {
  MarkedContentStack *mc = mcStack;
  mcStack = mc->next;
  delete mc;
}

void Gfx::pushMarkedContent() {
  MarkedContentStack *mc = new MarkedContentStack();
  mc->ocSuppressed = gFalse;
  mc->kind = gfxMCOther;
  mc->next = mcStack;
  mcStack = mc;
}

GBool Gfx::contentIsHidden() {
  MarkedContentStack *mc = mcStack;
  bool hidden = mc && mc->ocSuppressed;
  while (!hidden && mc && mc->next) {
    mc = mc->next;
    hidden = mc->ocSuppressed;
  }
  return hidden;
}

void Gfx::opBeginMarkedContent(Object args[], int numArgs) {
  // push a new stack entry
  pushMarkedContent();
  
  OCGs *contentConfig = catalog->getOptContentConfig();
  char* name0 = args[0].getName();
  if ( strncmp( name0, "OC", 2) == 0 && contentConfig) {
    if ( numArgs >= 2 ) {
      if (!args[1].isName()) {
	error(errSyntaxError, getPos(), "Unexpected MC Type: {0:d}", args[1].getType());
      }
      char* name1 = args[1].getName();
      Object markedContent;
      MarkedContentStack *mc = mcStack;
      mc->kind = gfxMCOptionalContent;
      if ( res->lookupMarkedContentNF( name1, &markedContent ) ) {
        bool visible = contentConfig->optContentIsVisible(&markedContent);
        mc->ocSuppressed = !(visible);
      } else {
	error(errSyntaxError, getPos(), "DID NOT find {0:s}", name1);
      }
      markedContent.free();
    } else {
      error(errSyntaxError, getPos(), "insufficient arguments for Marked Content");
    }
  } else if (args[0].isName("Span") && numArgs == 2 && args[1].isDict()) {
    Object obj;
    if (args[1].dictLookup("ActualText", &obj)->isString()) {
      out->beginActualText(state, obj.getString());
      MarkedContentStack *mc = mcStack;
      mc->kind = gfxMCActualText;
    }
    obj.free();
  }

  if (printCommands) {
    printf("  marked content: %s ", args[0].getName());
    if (numArgs == 2)
      args[1].print(stdout);
    printf("\n");
    fflush(stdout);
  }
  ocState = !contentIsHidden();
  
  if (numArgs == 2 && args[1].isDict()) {
    out->beginMarkedContent(args[0].getName(), args[1].getDict());
  } else if(numArgs == 1) {
    out->beginMarkedContent(args[0].getName(), NULL);
  }
}

void Gfx::opEndMarkedContent(Object args[], int numArgs) {
  if (!mcStack) {
    error(errSyntaxWarning, getPos(), "Mismatched EMC operator");
    return;
  }

  MarkedContentStack *mc = mcStack;
  GfxMarkedContentKind mcKind = mc->kind;

  // pop the stack
  popMarkedContent();

  if (mcKind == gfxMCActualText)
    out->endActualText(state);
  ocState = !contentIsHidden();
  
  out->endMarkedContent(state);
}

void Gfx::opMarkPoint(Object args[], int numArgs) {
  if (printCommands) {
    printf("  mark point: %s ", args[0].getName());
    if (numArgs == 2)
      args[1].print(stdout);
    printf("\n");
    fflush(stdout);
  }

  if(numArgs == 2 && args[1].isDict()) {
    out->markPoint(args[0].getName(),args[1].getDict());
  } else {
    out->markPoint(args[0].getName());
  }

}

//------------------------------------------------------------------------
// misc
//------------------------------------------------------------------------

struct GfxStackStateSaver {
  GfxStackStateSaver(Gfx *gfx) : gfx(gfx) {
    gfx->saveState();
  }

  ~GfxStackStateSaver() {
    gfx->restoreState();
  }

  Gfx * const gfx;
};

void Gfx::drawAnnot(Object *str, AnnotBorder *border, AnnotColor *aColor,
		    double xMin, double yMin, double xMax, double yMax, int rotate) {
  Dict *dict, *resDict;
  Object matrixObj, bboxObj, resObj, obj1;
  double formXMin, formYMin, formXMax, formYMax;
  double x, y, sx, sy, tx, ty;
  double m[6], bbox[4];
  double r, g, b;
  GfxColor color;
  double *dash, *dash2;
  int dashLength;
  int i;

  // this function assumes that we are in the default user space,
  // i.e., baseMatrix = ctm

  // if the bounding box has zero width or height, don't draw anything
  // at all
  if (xMin == xMax || yMin == yMax) {
    return;
  }

  // saves gfx state and automatically restores it on return
  GfxStackStateSaver stackStateSaver(this);

  // Rotation around the topleft corner (for the NoRotate flag)
  if (rotate != 0) {
    const double angle_rad = rotate * M_PI / 180;
    const double c = cos(angle_rad);
    const double s = sin(angle_rad);

    // (xMin, yMax) is the pivot
    const double unrotateMTX[6] = {
        +c, -s,
        +s, +c,
        -c*xMin - s*yMax + xMin, -c*yMax + s*xMin + yMax
    };

    state->concatCTM(unrotateMTX[0], unrotateMTX[1], unrotateMTX[2],
                     unrotateMTX[3], unrotateMTX[4], unrotateMTX[5]);
    out->updateCTM(state, unrotateMTX[0], unrotateMTX[1], unrotateMTX[2],
                          unrotateMTX[3], unrotateMTX[4], unrotateMTX[5]);
  }

  // draw the appearance stream (if there is one)
  if (str->isStream()) {

    // get stream dict
    dict = str->streamGetDict();

    // get the form bounding box
    dict->lookup("BBox", &bboxObj);
    if (!bboxObj.isArray()) {
      bboxObj.free();
      error(errSyntaxError, getPos(), "Bad form bounding box");
      return;
    }
    for (i = 0; i < 4; ++i) {
      bboxObj.arrayGet(i, &obj1);
      if (likely(obj1.isNum())) {
        bbox[i] = obj1.getNum();
        obj1.free();
      } else {
        obj1.free();
        error(errSyntaxError, getPos(), "Bad form bounding box value");
        return;
      }
    }
    bboxObj.free();

    // get the form matrix
    dict->lookup("Matrix", &matrixObj);
    if (matrixObj.isArray() && matrixObj.arrayGetLength() >= 6) {
      for (i = 0; i < 6; ++i) {
	matrixObj.arrayGet(i, &obj1);
	m[i] = obj1.getNum();
	obj1.free();
      }
    } else {
      m[0] = 1; m[1] = 0;
      m[2] = 0; m[3] = 1;
      m[4] = 0; m[5] = 0;
    }
    matrixObj.free();

    // transform the four corners of the form bbox to default user
    // space, and construct the transformed bbox
    x = bbox[0] * m[0] + bbox[1] * m[2] + m[4];
    y = bbox[0] * m[1] + bbox[1] * m[3] + m[5];
    formXMin = formXMax = x;
    formYMin = formYMax = y;
    x = bbox[0] * m[0] + bbox[3] * m[2] + m[4];
    y = bbox[0] * m[1] + bbox[3] * m[3] + m[5];
    if (x < formXMin) {
      formXMin = x;
    } else if (x > formXMax) {
      formXMax = x;
    }
    if (y < formYMin) {
      formYMin = y;
    } else if (y > formYMax) {
      formYMax = y;
    }
    x = bbox[2] * m[0] + bbox[1] * m[2] + m[4];
    y = bbox[2] * m[1] + bbox[1] * m[3] + m[5];
    if (x < formXMin) {
      formXMin = x;
    } else if (x > formXMax) {
      formXMax = x;
    }
    if (y < formYMin) {
      formYMin = y;
    } else if (y > formYMax) {
      formYMax = y;
    }
    x = bbox[2] * m[0] + bbox[3] * m[2] + m[4];
    y = bbox[2] * m[1] + bbox[3] * m[3] + m[5];
    if (x < formXMin) {
      formXMin = x;
    } else if (x > formXMax) {
      formXMax = x;
    }
    if (y < formYMin) {
      formYMin = y;
    } else if (y > formYMax) {
      formYMax = y;
    }

    // construct a mapping matrix, [sx 0  0], which maps the transformed
    //                             [0  sy 0]
    //                             [tx ty 1]
    // bbox to the annotation rectangle
    if (formXMin == formXMax) {
      // this shouldn't happen
      sx = 1;
    } else {
      sx = (xMax - xMin) / (formXMax - formXMin);
    }
    if (formYMin == formYMax) {
      // this shouldn't happen
      sy = 1;
    } else {
      sy = (yMax - yMin) / (formYMax - formYMin);
    }
    tx = -formXMin * sx + xMin;
    ty = -formYMin * sy + yMin;

    // the final transform matrix is (form matrix) * (mapping matrix)
    m[0] *= sx;
    m[1] *= sy;
    m[2] *= sx;
    m[3] *= sy;
    m[4] = m[4] * sx + tx;
    m[5] = m[5] * sy + ty;

    // get the resources
    dict->lookup("Resources", &resObj);
    resDict = resObj.isDict() ? resObj.getDict() : (Dict *)NULL;

    // draw it
    drawForm(str, resDict, m, bbox);

    resObj.free();
  }

  // draw the border
  if (border && border->getWidth() > 0) {
    if (state->getStrokeColorSpace()->getMode() != csDeviceRGB) {
      state->setStrokePattern(NULL);
      state->setStrokeColorSpace(new GfxDeviceRGBColorSpace());
      out->updateStrokeColorSpace(state);
    }
    if (aColor && (aColor->getSpace() == AnnotColor::colorRGB)) {
      const double *values = aColor->getValues();
      r = values[0];
      g = values[1];
      b = values[2];
    } else {
      r = g = b = 0;
    };
    color.c[0] = dblToCol(r);
    color.c[1] = dblToCol(g);
    color.c[2] = dblToCol(b);
    state->setStrokeColor(&color);
    out->updateStrokeColor(state);
    state->setLineWidth(border->getWidth());
    out->updateLineWidth(state);
    dashLength = border->getDashLength();
    dash = border->getDash();
    if (border->getStyle() == AnnotBorder::borderDashed && dashLength > 0) {
      dash2 = (double *)gmallocn(dashLength, sizeof(double));
      memcpy(dash2, dash, dashLength * sizeof(double));
      state->setLineDash(dash2, dashLength, 0);
      out->updateLineDash(state);
    }
    //~ this doesn't currently handle the beveled and engraved styles
    state->clearPath();
    state->moveTo(xMin, yMin);
    state->lineTo(xMax, yMin);
    if (border->getStyle() != AnnotBorder::borderUnderlined) {
      state->lineTo(xMax, yMax);
      state->lineTo(xMin, yMax);
      state->closePath();
    }
    out->stroke(state);
  }
}

int Gfx::bottomGuard() {
    return stateGuards[stateGuards.size()-1];
}

void Gfx::pushStateGuard() {
    stateGuards.push_back(stackHeight);
}

void Gfx::popStateGuard() {
    while (stackHeight > bottomGuard() && state->hasSaves())
	restoreState();
    stateGuards.pop_back();
}

void Gfx::saveState() {
  out->saveState(state);
  state = state->save();
  stackHeight++;
}

void Gfx::restoreState() {
  if (stackHeight <= bottomGuard() || !state->hasSaves()) {
    error(errSyntaxError, -1, "Restoring state when no valid states to pop");
    commandAborted = gTrue;
    return;
  }
  state = state->restore();
  out->restoreState(state);
  stackHeight--;
}

// Create a new state stack, and initialize it with a copy of the
// current state.
GfxState *Gfx::saveStateStack() {
  GfxState *oldState;

  out->saveState(state);
  oldState = state;
  state = state->copy(gTrue);
  return oldState;
}

// Switch back to the previous state stack.
void Gfx::restoreStateStack(GfxState *oldState) {
  while (state->hasSaves()) {
    restoreState();
  }
  delete state;
  state = oldState;
  out->restoreState(state);
}

void Gfx::pushResources(Dict *resDict) {
  res = new GfxResources(xref, resDict, res);
}

void Gfx::popResources() {
  GfxResources *resPtr;

  resPtr = res->getNext();
  delete res;
  res = resPtr;
}
