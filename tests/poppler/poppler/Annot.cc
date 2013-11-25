//========================================================================
//
// Annot.cc
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2007, 2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2013 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007, 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2008, 2011 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008 Hugo Mercier <hmercier31@gmail.com>
// Copyright (C) 2009 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2011, 2013 José Aliste <jaliste@src.gnome.org>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Tobias Koenig <tokoe@kdab.com>
// Copyright (C) 2013 Peter Breitenlohner <peb@mppmu.mpg.de>
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

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "goo/gmem.h"
#include "goo/gstrtod.h"
#include "GooList.h"
#include "Error.h"
#include "Object.h"
#include "Catalog.h"
#include "Gfx.h"
#include "Lexer.h"
#include "PDFDoc.h"
#include "Page.h"
#include "Annot.h"
#include "GfxFont.h"
#include "CharCodeToUnicode.h"
#include "PDFDocEncoding.h"
#include "Form.h"
#include "Error.h"
#include "XRef.h"
#include "Movie.h"
#include "OptionalContent.h"
#include "Sound.h"
#include "FileSpec.h"
#include "DateInfo.h"
#include "Link.h"
#include <string.h>

#if MULTITHREADED
#  define annotLocker()   MutexLocker locker(&mutex)
#  define annotCondLocker(X)  MutexLocker locker(&mutex, (X))
#else
#  define annotLocker()
#  define annotCondLocker(X)
#endif

#define fieldFlagReadOnly           0x00000001
#define fieldFlagRequired           0x00000002
#define fieldFlagNoExport           0x00000004
#define fieldFlagMultiline          0x00001000
#define fieldFlagPassword           0x00002000
#define fieldFlagNoToggleToOff      0x00004000
#define fieldFlagRadio              0x00008000
#define fieldFlagPushbutton         0x00010000
#define fieldFlagCombo              0x00020000
#define fieldFlagEdit               0x00040000
#define fieldFlagSort               0x00080000
#define fieldFlagFileSelect         0x00100000
#define fieldFlagMultiSelect        0x00200000
#define fieldFlagDoNotSpellCheck    0x00400000
#define fieldFlagDoNotScroll        0x00800000
#define fieldFlagComb               0x01000000
#define fieldFlagRichText           0x02000000
#define fieldFlagRadiosInUnison     0x02000000
#define fieldFlagCommitOnSelChange  0x04000000

#define fieldQuadLeft   0
#define fieldQuadCenter 1
#define fieldQuadRight  2

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle 0.55228475

AnnotLineEndingStyle parseAnnotLineEndingStyle(GooString *string) {
  if (string != NULL) {
    if (!string->cmp("Square")) {
      return annotLineEndingSquare;
    } else if (!string->cmp("Circle")) {
      return annotLineEndingCircle;
    } else if (!string->cmp("Diamond")) {
      return annotLineEndingDiamond;
    } else if (!string->cmp("OpenArrow")) {
      return annotLineEndingOpenArrow;
    } else if (!string->cmp("ClosedArrow")) {
      return annotLineEndingClosedArrow;
    } else if (!string->cmp("Butt")) {
      return annotLineEndingButt;
    } else if (!string->cmp("ROpenArrow")) {
      return annotLineEndingROpenArrow;
    } else if (!string->cmp("RClosedArrow")) {
      return annotLineEndingRClosedArrow;
    } else if (!string->cmp("Slash")) {
      return annotLineEndingSlash;
    } else {
      return annotLineEndingNone;
    }
  } else {
    return annotLineEndingNone;
  }  
}

const char* convertAnnotLineEndingStyle(AnnotLineEndingStyle style) {
  switch (style) {
    case annotLineEndingSquare:
      return "Square";
    case annotLineEndingCircle:
      return "Circle";
    case annotLineEndingDiamond:
      return "Diamond";
    case annotLineEndingOpenArrow:
      return "OpenArrow";
    case annotLineEndingClosedArrow:
      return "ClosedArrow";
    case annotLineEndingButt:
      return "Butt";
    case annotLineEndingROpenArrow:
      return "ROpenArrow";
    case annotLineEndingRClosedArrow:
      return "RClosedArrow";
    case annotLineEndingSlash:
      return "Slash";
    default:
      return "None";
  }
}

static AnnotExternalDataType parseAnnotExternalData(Dict* dict) {
  Object obj1;
  AnnotExternalDataType type;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    const char *typeName = obj1.getName();

    if (!strcmp(typeName, "Markup3D")) {
      type = annotExternalDataMarkup3D;
    } else {
      type = annotExternalDataMarkupUnknown;
    }
  } else {
    type = annotExternalDataMarkupUnknown;
  }
  obj1.free();

  return type;
}

PDFRectangle *parseDiffRectangle(Array *array, PDFRectangle *rect) {
  PDFRectangle *newRect = NULL;
  if (array->getLength() == 4) {
    // deltas
    Object obj1;
    double dx1 = (array->get(0, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();
    double dy1 = (array->get(1, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();
    double dx2 = (array->get(2, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();
    double dy2 = (array->get(3, &obj1)->isNum() ? obj1.getNum() : 0);
    obj1.free();

    // checking that the numbers are valid (i.e. >= 0),
    // and that applying the differences still give us a valid rect
    if (dx1 >= 0 && dy1 >= 0 && dx2 >= 0 && dy2
        && (rect->x2 - rect->x1 - dx1 - dx2) >= 0
        && (rect->y2 - rect->y1 - dy1 - dy2) >= 0) {
      newRect = new PDFRectangle();
      newRect->x1 = rect->x1 + dx1;
      newRect->y1 = rect->y1 + dy1;
      newRect->x2 = rect->x2 - dx2;
      newRect->y2 = rect->y2 - dy2;
    }
  }
  return newRect;
}

static LinkAction* getAdditionalAction(Annot::AdditionalActionsType type, Object *additionalActions, PDFDoc *doc) {
  Object additionalActionsObject;
  LinkAction *linkAction = NULL;

  if (additionalActions->fetch(doc->getXRef(), &additionalActionsObject)->isDict()) {
    const char *key = (type == Annot::actionCursorEntering ? "E" :
                       type == Annot::actionCursorLeaving ?  "X" :
                       type == Annot::actionMousePressed ?   "D" :
                       type == Annot::actionMouseReleased ?  "U" :
                       type == Annot::actionFocusIn ?       "Fo" :
                       type == Annot::actionFocusOut ?      "BI" :
                       type == Annot::actionPageOpening ?   "PO" :
                       type == Annot::actionPageClosing ?   "PC" :
                       type == Annot::actionPageVisible ?   "PV" :
                       type == Annot::actionPageInvisible ? "PI" : NULL);

    Object actionObject;

    if (additionalActionsObject.dictLookup(key, &actionObject)->isDict())
      linkAction = LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
    actionObject.free();
  }

  additionalActionsObject.free();

  return linkAction;
}

static LinkAction* getFormAdditionalAction(Annot::FormAdditionalActionsType type, Object *additionalActions, PDFDoc *doc) {
  Object additionalActionsObject;
  LinkAction *linkAction = NULL;

  if (additionalActions->fetch(doc->getXRef(), &additionalActionsObject)->isDict()) {
    const char *key = (type == Annot::actionFieldModified ?  "K" :
                       type == Annot::actionFormatField ?    "F" :
                       type == Annot::actionValidateField ?  "V" :
                       type == Annot::actionCalculateField ? "C" : NULL);

    Object actionObject;

    if (additionalActionsObject.dictLookup(key, &actionObject)->isDict())
      linkAction = LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
    actionObject.free();
  }

  additionalActionsObject.free();

  return linkAction;
}

//------------------------------------------------------------------------
// AnnotBorderEffect
//------------------------------------------------------------------------

AnnotBorderEffect::AnnotBorderEffect(Dict *dict) {
  Object obj1;

  if (dict->lookup("S", &obj1)->isName()) {
    const char *effectName = obj1.getName();

    if (!strcmp(effectName, "C"))
      effectType = borderEffectCloudy;
    else
      effectType = borderEffectNoEffect;
  } else {
    effectType = borderEffectNoEffect;
  }
  obj1.free();

  if ((dict->lookup("I", &obj1)->isNum()) && effectType == borderEffectCloudy) {
    intensity = obj1.getNum();
  } else {
    intensity = 0;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotPath
//------------------------------------------------------------------------

AnnotPath::AnnotPath() {
  coords = NULL;
  coordsLength = 0;
}

AnnotPath::AnnotPath(Array *array) {
  coords = NULL;
  coordsLength = 0;
  parsePathArray(array);
}

AnnotPath::AnnotPath(AnnotCoord **coords, int coordsLength) {
  this->coords = coords;
  this->coordsLength = coordsLength;
}

AnnotPath::~AnnotPath() {
  if (coords) {
    for (int i = 0; i < coordsLength; ++i)
      delete coords[i];
    gfree(coords);
  }
}

double AnnotPath::getX(int coord) const {
  if (coord >= 0 && coord < coordsLength)
    return coords[coord]->getX();
  return 0;
}

double AnnotPath::getY(int coord) const {
  if (coord >= 0 && coord < coordsLength)
    return coords[coord]->getY();
  return 0;
}

AnnotCoord *AnnotPath::getCoord(int coord) const {
  if (coord >= 0 && coord < coordsLength)
    return coords[coord];
  return NULL;
}

void AnnotPath::parsePathArray(Array *array) {
  int tempLength;
  AnnotCoord **tempCoords;
  GBool correct = gTrue;

  if (array->getLength() % 2) {
    error(errSyntaxError, -1, "Bad Annot Path");
    return;
  }

  tempLength = array->getLength() / 2;
  tempCoords = (AnnotCoord **) gmallocn (tempLength, sizeof(AnnotCoord *));
  memset(tempCoords, 0, tempLength * sizeof(AnnotCoord *));
  for (int i = 0; i < tempLength && correct; i++) {
    Object obj1;
    double x = 0, y = 0;

    if (array->get(i * 2, &obj1)->isNum()) {
      x = obj1.getNum();
    } else {
      correct = gFalse;
    }
    obj1.free();

    if (array->get((i * 2) + 1, &obj1)->isNum()) {
      y = obj1.getNum();
    } else {
      correct = gFalse;
    }
    obj1.free();

    if (!correct) {
      for (int j = i - 1; j >= 0; j--)
        delete tempCoords[j];
      gfree (tempCoords);
      return;
    }

    tempCoords[i] = new AnnotCoord(x, y);
  }

  coords = tempCoords;
  coordsLength = tempLength;
}

//------------------------------------------------------------------------
// AnnotCalloutLine
//------------------------------------------------------------------------

AnnotCalloutLine::AnnotCalloutLine(double x1, double y1, double x2, double y2)
    :  coord1(x1, y1), coord2(x2, y2) {
}

//------------------------------------------------------------------------
// AnnotCalloutMultiLine
//------------------------------------------------------------------------

AnnotCalloutMultiLine::AnnotCalloutMultiLine(double x1, double y1, double x2,
                                             double y2, double x3, double y3)
    : AnnotCalloutLine(x1, y1, x2, y2), coord3(x3, y3) {
}

//------------------------------------------------------------------------
// AnnotQuadrilateral
//------------------------------------------------------------------------

AnnotQuadrilaterals::AnnotQuadrilaterals(Array *array, PDFRectangle *rect) {
  int arrayLength = array->getLength();
  GBool correct = gTrue;
  int quadsLength = 0;
  AnnotQuadrilateral **quads;
  double quadArray[8];

  // default values
  quadrilaterals = NULL;
  quadrilateralsLength = 0;

  if ((arrayLength % 8) == 0) {
    int i;

    quadsLength = arrayLength / 8;
    quads = (AnnotQuadrilateral **) gmallocn
        ((quadsLength), sizeof(AnnotQuadrilateral *));
    memset(quads, 0, quadsLength * sizeof(AnnotQuadrilateral *));

    for (i = 0; i < quadsLength; i++) {
      for (int j = 0; j < 8; j++) {
        Object obj;
        if (array->get(i * 8 + j, &obj)->isNum()) {
          quadArray[j] = obj.getNum();
        } else {
            correct = gFalse;
	    obj.free();
	    error (errSyntaxError, -1, "Invalid QuadPoint in annot");
	    break;
        }
        obj.free();
      }

      if (!correct)
        break;

      quads[i] = new AnnotQuadrilateral(quadArray[0], quadArray[1],
					quadArray[2], quadArray[3],
					quadArray[4], quadArray[5],
					quadArray[6], quadArray[7]);
    }

    if (correct) {
      quadrilateralsLength = quadsLength;
      quadrilaterals = quads;
    } else {
      for (int j = 0; j < i; j++)
        delete quads[j];
      gfree (quads);
    }
  }
}

AnnotQuadrilaterals::AnnotQuadrilaterals(AnnotQuadrilaterals::AnnotQuadrilateral **quads, int quadsLength) {
  quadrilaterals = quads;
  quadrilateralsLength = quadsLength;
}

AnnotQuadrilaterals::~AnnotQuadrilaterals() {
  if (quadrilaterals) {
    for(int i = 0; i < quadrilateralsLength; i++)
      delete quadrilaterals[i];

    gfree (quadrilaterals);
  }
}

double AnnotQuadrilaterals::getX1(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord1.getX();
  return 0;
}

double AnnotQuadrilaterals::getY1(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord1.getY();
  return 0;
}

double AnnotQuadrilaterals::getX2(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord2.getX();
  return 0;
}

double AnnotQuadrilaterals::getY2(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord2.getY();
  return 0;
}

double AnnotQuadrilaterals::getX3(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord3.getX();
  return 0;
}

double AnnotQuadrilaterals::getY3(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord3.getY();
  return 0;
}

double AnnotQuadrilaterals::getX4(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord4.getX();
  return 0;
}

double AnnotQuadrilaterals::getY4(int quadrilateral) {
  if (quadrilateral >= 0  && quadrilateral < quadrilateralsLength)
    return quadrilaterals[quadrilateral]->coord4.getY();
  return 0;
}

AnnotQuadrilaterals::AnnotQuadrilateral::AnnotQuadrilateral(double x1, double y1,
    double x2, double y2, double x3, double y3, double x4, double y4)
    : coord1(x1, y1), coord2(x2, y2), coord3(x3, y3), coord4(x4, y4) {
}

//------------------------------------------------------------------------
// AnnotBorder
//------------------------------------------------------------------------
AnnotBorder::AnnotBorder() {
  type = typeUnknown;
  width = 1;
  dashLength = 0;
  dash = NULL;
  style = borderSolid;
}

GBool AnnotBorder::parseDashArray(Object *dashObj) {
  GBool correct = gTrue;
  int tempLength = dashObj->arrayGetLength();
  double *tempDash = (double *) gmallocn (tempLength, sizeof (double));

  // TODO: check not all zero (Line Dash Pattern Page 217 PDF 8.1)
  for (int i = 0; i < tempLength && i < DASH_LIMIT && correct; i++) {
    Object obj1;

    if (dashObj->arrayGet(i, &obj1)->isNum()) {
      tempDash[i] = obj1.getNum();

      correct = tempDash[i] >= 0;
      obj1.free();
    }
  }

  if (correct) {
    dashLength = tempLength;
    dash = tempDash;
    style = borderDashed;
  } else {
    gfree (tempDash);
  }

  return correct;
}

AnnotBorder::~AnnotBorder() {
  if (dash)
    gfree (dash); 
}
  
//------------------------------------------------------------------------
// AnnotBorderArray
//------------------------------------------------------------------------

AnnotBorderArray::AnnotBorderArray() {
  type = typeArray;
  horizontalCorner = 0;
  verticalCorner = 0;
}

AnnotBorderArray::AnnotBorderArray(Array *array) {
  Object obj1;
  int arrayLength = array->getLength();

  GBool correct = gTrue;
  if (arrayLength == 3 || arrayLength == 4) {
    // implementation note 81 in Appendix H.

    if (array->get(0, &obj1)->isNum())
      horizontalCorner = obj1.getNum();
    else
      correct = gFalse;
    obj1.free();

    if (array->get(1, &obj1)->isNum())
      verticalCorner = obj1.getNum();
    else
      correct = gFalse;
    obj1.free();

    if (array->get(2, &obj1)->isNum())
      width = obj1.getNum();
    else
      correct = gFalse;
    obj1.free();

    if (arrayLength == 4) {
      if (array->get(3, &obj1)->isArray())
        correct = parseDashArray(&obj1);
      else
        correct = gFalse;
      obj1.free();
    }
  } else {
    correct = gFalse;
  }
  
  if (!correct) {
    width = 0;
  }
}

void AnnotBorderArray::writeToObject(XRef *xref, Object *obj1) const {
  Object obj2;

  obj1->initArray(xref);
  obj1->arrayAdd(obj2.initReal( horizontalCorner ));
  obj1->arrayAdd(obj2.initReal( verticalCorner ));
  obj1->arrayAdd(obj2.initReal( width ));
  // TODO: Dash array
}

//------------------------------------------------------------------------
// AnnotBorderBS
//------------------------------------------------------------------------

AnnotBorderBS::AnnotBorderBS() {
  type = typeBS;
}

AnnotBorderBS::AnnotBorderBS(Dict *dict) {
  Object obj1, obj2;

  // acroread 8 seems to need both W and S entries for
  // any border to be drawn, even though the spec
  // doesn't claim anything of that sort. We follow
  // that behaviour by veryifying both entries exist
  // otherwise we set the borderWidth to 0
  // --jrmuizel
  dict->lookup("W", &obj1);
  dict->lookup("S", &obj2);
  if (obj1.isNum() && obj2.isName()) {
    const char *styleName = obj2.getName();

    width = obj1.getNum();

    if (!strcmp(styleName, "S")) {
      style = borderSolid;
    } else if (!strcmp(styleName, "D")) {
      style = borderDashed;
    } else if (!strcmp(styleName, "B")) {
      style = borderBeveled;
    } else if (!strcmp(styleName, "I")) {
      style = borderInset;
    } else if (!strcmp(styleName, "U")) {
      style = borderUnderlined;
    } else {
      style = borderSolid;
    }
  } else {
    width = 0;
  }
  obj2.free();
  obj1.free();

  if (style == borderDashed) {
    if (dict->lookup("D", &obj1)->isArray())
      parseDashArray(&obj1);
    obj1.free();

    if (!dash) {
      dashLength = 1;
      dash = (double *) gmallocn (dashLength, sizeof (double));
      dash[0] = 3;
    }
  }
}

//------------------------------------------------------------------------
// AnnotColor
//------------------------------------------------------------------------

AnnotColor::AnnotColor() {
  length = 0;
}

AnnotColor::AnnotColor(double gray) {
  length = 1;

  values[0] = gray;
}

AnnotColor::AnnotColor(double r, double g, double b) {
  length = 3;

  values[0] = r;
  values[1] = g;
  values[2] = b;
}

AnnotColor::AnnotColor(double c, double m, double y, double k) {
  length = 4;

  values[0] = c;
  values[1] = m;
  values[2] = y;
  values[3] = k;
}

// If <adjust> is +1, color is brightened;
// if <adjust> is -1, color is darkened;
// otherwise color is not modified.
AnnotColor::AnnotColor(Array *array, int adjust) {
  int i;

  length = array->getLength();
  if (length > 4)
    length = 4;

  for (i = 0; i < length; i++) {
    Object obj1;

    if (array->get(i, &obj1)->isNum()) {
      values[i] = obj1.getNum();

      if (values[i] < 0 || values[i] > 1)
        values[i] = 0;
    } else {
      values[i] = 0;
    }
    obj1.free();
  }

  if (adjust != 0)
    adjustColor(adjust);
}

void AnnotColor::adjustColor(int adjust) {
  int i;

  if (length == 4) {
    adjust = -adjust;
  }
  if (adjust > 0) {
    for (i = 0; i < length; ++i) {
      values[i] = 0.5 * values[i] + 0.5;
    }
  } else if (adjust < 0) {
    for (i = 0; i < length; ++i) {
      values[i] = 0.5 * values[i];
    }
  }
}

void AnnotColor::writeToObject(XRef *xref, Object *obj1) const {
  Object obj2;
  int i;

  if (length == 0) {
    obj1->initNull(); // Transparent (no color)
  } else {
    obj1->initArray(xref);
    for (i = 0; i < length; ++i)
      obj1->arrayAdd( obj2.initReal( values[i] ) );
  }
}

//------------------------------------------------------------------------
// AnnotIconFit
//------------------------------------------------------------------------

AnnotIconFit::AnnotIconFit(Dict* dict) {
  Object obj1;

  if (dict->lookup("SW", &obj1)->isName()) {
    const char *scaleName = obj1.getName();

    if(!strcmp(scaleName, "B")) {
      scaleWhen = scaleBigger;
    } else if(!strcmp(scaleName, "S")) {
      scaleWhen = scaleSmaller;
    } else if(!strcmp(scaleName, "N")) {
      scaleWhen = scaleNever;
    } else {
      scaleWhen = scaleAlways;
    }
  } else {
    scaleWhen = scaleAlways;
  }
  obj1.free();

  if (dict->lookup("S", &obj1)->isName()) {
    const char *scaleName = obj1.getName();

    if(!strcmp(scaleName, "A")) {
      scale = scaleAnamorphic;
    } else {
      scale = scaleProportional;
    }
  } else {
    scale = scaleProportional;
  }
  obj1.free();

  if (dict->lookup("A", &obj1)->isArray() && obj1.arrayGetLength() == 2) {
    Object obj2;
    (obj1.arrayGet(0, &obj2)->isNum() ? left = obj2.getNum() : left = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? bottom = obj2.getNum() : bottom = 0);
    obj2.free();

    if (left < 0 || left > 1)
      left = 0.5;

    if (bottom < 0 || bottom > 1)
      bottom = 0.5;

  } else {
    left = bottom = 0.5;
  }
  obj1.free();

  if (dict->lookup("FB", &obj1)->isBool()) {
    fullyBounds = obj1.getBool();
  } else {
    fullyBounds = gFalse;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotAppearance
//------------------------------------------------------------------------

AnnotAppearance::AnnotAppearance(PDFDoc *docA, Object *dict) {
  assert(dict->isDict());
  doc = docA;
  xref = docA->getXRef();
  dict->copy(&appearDict);
}

AnnotAppearance::~AnnotAppearance() {
  appearDict.free();
}

void AnnotAppearance::getAppearanceStream(AnnotAppearanceType type, const char *state, Object *dest) {
  Object apData, stream;
  apData.initNull();

  // Obtain dictionary or stream associated to appearance type
  switch (type) {
  case appearRollover:
    if (appearDict.dictLookupNF("R", &apData)->isNull())
      appearDict.dictLookupNF("N", &apData);
    break;
  case appearDown:
    if (appearDict.dictLookupNF("D", &apData)->isNull())
      appearDict.dictLookupNF("N", &apData);
    break;
  case appearNormal:
    appearDict.dictLookupNF("N", &apData);
    break;
  }

  dest->initNull();
  if (apData.isDict() && state)
    apData.dictLookupNF(state, dest);
  else if (apData.isRef())
    apData.copy(dest);
  apData.free();
}

GooString * AnnotAppearance::getStateKey(int i) {
  Object obj1;
  GooString * res = NULL;
  if (appearDict.dictLookupNF("N", &obj1)->isDict())
    res = new GooString(obj1.dictGetKey(i));
  obj1.free();
  return res;
}

int AnnotAppearance::getNumStates() {
  Object obj1;
  int res = 0;
  if (appearDict.dictLookupNF("N", &obj1)->isDict())
    res = obj1.dictGetLength();
  obj1.free();
  return res;
}

// Test if stateObj (a Ref or a Dict) points to the specified stream
GBool AnnotAppearance::referencesStream(Object *stateObj, Ref refToStream) {
  if (stateObj->isRef()) {
    Ref r = stateObj->getRef();
    if (r.num == refToStream.num && r.gen == refToStream.gen) {
      return gTrue;
    }
  } else if (stateObj->isDict()) { // Test each value
    const int size = stateObj->dictGetLength();
    for (int i = 0; i < size; ++i) {
      Object obj1;
      stateObj->dictGetValNF(i, &obj1);
      if (obj1.isRef()) {
        Ref r = obj1.getRef();
        if (r.num == refToStream.num && r.gen == refToStream.gen) {
          return gTrue;
        }
      }
      obj1.free();
    }
  }
  return gFalse; // Not found
}

// Test if this AnnotAppearance references the specified stream
GBool AnnotAppearance::referencesStream(Ref refToStream) {
  Object obj1;
  GBool found;

  // Scan each state's ref/subdictionary
  appearDict.dictLookupNF("N", &obj1);
  found = referencesStream(&obj1, refToStream);
  obj1.free();
  if (found)
    return gTrue;

  appearDict.dictLookupNF("R", &obj1);
  found = referencesStream(&obj1, refToStream);
  obj1.free();
  if (found)
    return gTrue;

  appearDict.dictLookupNF("D", &obj1);
  found = referencesStream(&obj1, refToStream);
  obj1.free();
  return found;
}

// If this is the only annotation in the document that references the
// specified appearance stream, remove the appearance stream
void AnnotAppearance::removeStream(Ref refToStream) {
  const int lastpage = doc->getNumPages();
  for (int pg = 1; pg <= lastpage; ++pg) { // Scan all annotations in the document
    Page *page = doc->getPage(pg);
    if (!page) {
      error(errSyntaxError, -1, "Failed check for shared annotation stream at page {0:d}", pg);
      continue;
    }
    Annots *annots = page->getAnnots();
    for (int i = 0; i < annots->getNumAnnots(); ++i) {
      AnnotAppearance *annotAp = annots->getAnnot(i)->getAppearStreams();
      if (annotAp && annotAp != this && annotAp->referencesStream(refToStream)) {
        return; // Another annotation points to the stream -> Don't delete it
      }
    }
  }

  // TODO: stream resources (e.g. font), AP name tree
  xref->removeIndirectObject(refToStream);
}

// Removes stream if obj is a Ref, or removes pointed streams if obj is a Dict
void AnnotAppearance::removeStateStreams(Object *obj1) {
  if (obj1->isRef()) {
    removeStream(obj1->getRef());
  } else if (obj1->isDict()) {
    const int size = obj1->dictGetLength();
    for (int i = 0; i < size; ++i) {
      Object obj2;
      obj1->dictGetValNF(i, &obj2);
      if (obj2.isRef()) {
        removeStream(obj2.getRef());
      }
      obj2.free();
    }
  }
}

void AnnotAppearance::removeAllStreams() {
  Object obj1;
  appearDict.dictLookupNF("N", &obj1);
  removeStateStreams(&obj1);
  obj1.free();
  appearDict.dictLookupNF("R", &obj1);
  removeStateStreams(&obj1);
  obj1.free();
  appearDict.dictLookupNF("D", &obj1);
  removeStateStreams(&obj1);
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotAppearanceCharacs
//------------------------------------------------------------------------

AnnotAppearanceCharacs::AnnotAppearanceCharacs(Dict *dict) {
  Object obj1;

  if (dict->lookup("R", &obj1)->isInt()) {
    rotation = obj1.getInt();
  } else {
    rotation = 0;
  }
  obj1.free();

  if (dict->lookup("BC", &obj1)->isArray()) {
    borderColor = new AnnotColor(obj1.getArray());
  } else {
    borderColor = NULL;
  }
  obj1.free();

  if (dict->lookup("BG", &obj1)->isArray()) {
    backColor = new AnnotColor(obj1.getArray());
  } else {
    backColor = NULL;
  }
  obj1.free();

  if (dict->lookup("CA", &obj1)->isString()) {
    normalCaption = new GooString(obj1.getString());
  } else {
    normalCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("RC", &obj1)->isString()) {
    rolloverCaption = new GooString(obj1.getString());
  } else {
    rolloverCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("AC", &obj1)->isString()) {
    alternateCaption = new GooString(obj1.getString());
  } else {
    alternateCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("IF", &obj1)->isDict()) {
    iconFit = new AnnotIconFit(obj1.getDict());
  } else {
    iconFit = NULL;
  }
  obj1.free();

  if (dict->lookup("TP", &obj1)->isInt()) {
    position = (AnnotAppearanceCharacsTextPos) obj1.getInt();
  } else {
    position = captionNoIcon;
  }
  obj1.free();
}

AnnotAppearanceCharacs::~AnnotAppearanceCharacs() {
  if (borderColor)
    delete borderColor;

  if (backColor)
    delete backColor;

  if (normalCaption)
    delete normalCaption;

  if (rolloverCaption)
    delete rolloverCaption;

  if (alternateCaption)
    delete alternateCaption;

  if (iconFit)
    delete iconFit;
}

//------------------------------------------------------------------------
// AnnotAppearanceBBox
//------------------------------------------------------------------------

AnnotAppearanceBBox::AnnotAppearanceBBox(PDFRectangle *rect) {
  origX = rect->x1;
  origY = rect->y1;
  borderWidth = 0;

  // Initially set the same size as rect
  minX = 0;
  minY = 0;
  maxX = rect->x2 - rect->x1;
  maxY = rect->y2 - rect->y1;
}

void AnnotAppearanceBBox::extendTo(double x, double y) {
  if (x < minX) {
    minX = x;
  } else if (x > maxX) {
    maxX = x;
  }
  if (y < minY) {
    minY = y;
  } else if (y > maxY) {
    maxY = y;
  }
}

void AnnotAppearanceBBox::getBBoxRect(double bbox[4]) const {
  bbox[0] = minX - borderWidth;
  bbox[1] = minY - borderWidth;
  bbox[2] = maxX + borderWidth;
  bbox[3] = maxY + borderWidth;
}

double AnnotAppearanceBBox::getPageXMin() const {
  return origX + minX - borderWidth;
}

double AnnotAppearanceBBox::getPageYMin() const {
  return origY + minY - borderWidth;
}

double AnnotAppearanceBBox::getPageXMax() const {
  return origX + maxX + borderWidth;
}

double AnnotAppearanceBBox::getPageYMax() const {
  return origY + maxY + borderWidth;
}

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

Annot::Annot(PDFDoc *docA, PDFRectangle *rectA) {
  Object obj1;

  refCnt = 1;
  flags = flagUnknown;
  type = typeUnknown;

  obj1.initArray (docA->getXRef());
  Object obj2;
  obj1.arrayAdd (obj2.initReal (rectA->x1));
  obj1.arrayAdd (obj2.initReal (rectA->y1));
  obj1.arrayAdd (obj2.initReal (rectA->x2));
  obj1.arrayAdd (obj2.initReal (rectA->y2));
  obj2.free ();

  annotObj.initDict (docA->getXRef());
  annotObj.dictSet ("Type", obj2.initName ("Annot"));
  annotObj.dictSet ("Rect", &obj1);
  // obj1 is owned by the dict

  ref = docA->getXRef()->addIndirectObject (&annotObj);

  initialize (docA, annotObj.getDict());
}

Annot::Annot(PDFDoc *docA, Dict *dict) {
  refCnt = 1;
  hasRef = false;
  flags = flagUnknown;
  type = typeUnknown;
  annotObj.initDict (dict);
  initialize (docA, dict);
}

Annot::Annot(PDFDoc *docA, Dict *dict, Object *obj) {
  refCnt = 1;
  if (obj->isRef()) {
    hasRef = gTrue;
    ref = obj->getRef();
  } else {
    hasRef = gFalse;
  }
  flags = flagUnknown;
  type = typeUnknown;
  annotObj.initDict (dict);
  initialize (docA, dict);
}

void Annot::initialize(PDFDoc *docA, Dict *dict) {
  Object apObj, asObj, obj1, obj2;

  ok = gTrue;
  doc = docA;
  xref = doc->getXRef();
  appearStreams = NULL;
  appearBBox = NULL;
  appearState = NULL;
  appearBuf = NULL;
  fontSize = 0;

  appearance.initNull();

  //----- parse the rectangle
  rect = new PDFRectangle();
  if (dict->lookup("Rect", &obj1)->isArray() && obj1.arrayGetLength() == 4) {
    Object obj2;
    (obj1.arrayGet(0, &obj2)->isNum() ? rect->x1 = obj2.getNum() : rect->x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? rect->y1 = obj2.getNum() : rect->y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? rect->x2 = obj2.getNum() : rect->x2 = 1);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? rect->y2 = obj2.getNum() : rect->y2 = 1);
    obj2.free();

    if (rect->x1 > rect->x2) {
      double t = rect->x1;
      rect->x1 = rect->x2;
      rect->x2 = t;
    }

    if (rect->y1 > rect->y2) {
      double t = rect->y1;
      rect->y1 = rect->y2;
      rect->y2 = t;
    }
  } else {
    rect->x1 = rect->y1 = 0;
    rect->x2 = rect->y2 = 1;
    error(errSyntaxError, -1, "Bad bounding box for annotation");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Contents", &obj1)->isString()) {
    contents = obj1.getString()->copy();
  } else {
    contents = NULL;
  }
  obj1.free();

  // Note: This value is overwritten by Annots ctor
  if (dict->lookupNF("P", &obj1)->isRef()) {
    Ref ref = obj1.getRef();

    page = doc->getCatalog()->findPage (ref.num, ref.gen);
  } else {
    page = 0;
  }
  obj1.free();

  if (dict->lookup("NM", &obj1)->isString()) {
    name = obj1.getString()->copy();
  } else {
    name = NULL;
  }
  obj1.free();

  if (dict->lookup("M", &obj1)->isString()) {
    modified = obj1.getString()->copy();
  } else {
    modified = NULL;
  }
  obj1.free();

  //----- get the flags
  if (dict->lookup("F", &obj1)->isInt()) {
    flags |= obj1.getInt();
  } else {
    flags = flagUnknown;
  }
  obj1.free();

  //----- get the annotation appearance dictionary
  dict->lookup("AP", &apObj);
  if (apObj.isDict()) {
    appearStreams = new AnnotAppearance(doc, &apObj);
  }
  apObj.free();

  //----- get the appearance state
  dict->lookup("AS", &asObj);
  if (asObj.isName()) {
    appearState = new GooString(asObj.getName());
  } else if (appearStreams && appearStreams->getNumStates() != 0) {
    error (errSyntaxError, -1, "Invalid or missing AS value in annotation containing one or more appearance subdictionaries");
    // AS value is required in this case, but if the
    // N dictionary contains only one entry
    // take it as default appearance.
    if (appearStreams->getNumStates() == 1) {
      appearState = appearStreams->getStateKey(0);
    }
  }
  if (!appearState) {
    appearState = new GooString("Off");
  }
  asObj.free();

  //----- get the annotation appearance
  if (appearStreams) {
    appearStreams->getAppearanceStream(AnnotAppearance::appearNormal, appearState->getCString(), &appearance);
  }

  //----- parse the border style
  if (dict->lookup("BS", &obj1)->isDict()) {
    border = new AnnotBorderBS(obj1.getDict());
  } else {
    obj1.free();

    if (dict->lookup("Border", &obj1)->isArray())
      border = new AnnotBorderArray(obj1.getArray());
    else
      // Adobe draws no border at all if the last element is of
      // the wrong type.
      border = NULL;
  }
  obj1.free();

  if (dict->lookup("C", &obj1)->isArray()) {
    color = new AnnotColor(obj1.getArray());
  } else {
    color = NULL;
  }
  obj1.free();

  if (dict->lookup("StructParent", &obj1)->isInt()) {
    treeKey = obj1.getInt();
  } else {
    treeKey = 0;
  }
  obj1.free();

  dict->lookupNF("OC", &oc);

#if MULTITHREADED
  gInitMutex(&mutex);
#endif
}

void Annot::getRect(double *x1, double *y1, double *x2, double *y2) const {
  *x1 = rect->x1;
  *y1 = rect->y1;
  *x2 = rect->x2;
  *y2 = rect->y2;
}

void Annot::setRect(PDFRectangle *rect) {
    setRect(rect->x1, rect->y1, rect->x2, rect->y2);
}

void Annot::setRect(double x1, double y1, double x2, double y2) {
  Object obj1, obj2;

  if (x1 < x2) {
    rect->x1 = x1;
    rect->x2 = x2;
  } else {
    rect->x1 = x2;
    rect->x2 = x1;
  }

  if (y1 < y2) {
    rect->y1 = y1;
    rect->y2 = y2;
  } else {
    rect->y1 = y2;
    rect->y2 = y1;
  }

  obj1.initArray (xref);
  obj1.arrayAdd (obj2.initReal (rect->x1));
  obj1.arrayAdd (obj2.initReal (rect->y1));
  obj1.arrayAdd (obj2.initReal (rect->x2));
  obj1.arrayAdd (obj2.initReal (rect->y2));

  update("Rect", &obj1);
  invalidateAppearance();
}

GBool Annot::inRect(double x, double y) const {
  return rect->contains(x, y);
}

void Annot::update(const char *key, Object *value) {
  annotLocker();
  /* Set M to current time, unless we are updating M itself */
  if (strcmp(key, "M") != 0) {
    delete modified;
    modified = timeToDateString(NULL);

    Object obj1;
    obj1.initString (modified->copy());
    annotObj.dictSet("M", &obj1);
  }

  annotObj.dictSet(const_cast<char*>(key), value);
  
  xref->setModifiedObject(&annotObj, ref);
}

void Annot::setContents(GooString *new_content) {
  annotLocker();
  delete contents;

  if (new_content) {
    contents = new GooString(new_content);
    //append the unicode marker <FE FF> if needed	
    if (!contents->hasUnicodeMarker()) {
      contents->insert(0, 0xff);
      contents->insert(0, 0xfe);
    }
  } else {
    contents = new GooString();
  }
  
  Object obj1;
  obj1.initString(contents->copy());
  update ("Contents", &obj1);
}

void Annot::setName(GooString *new_name) {
  annotLocker();
  delete name;

  if (new_name) {
    name = new GooString(new_name);
  } else {
    name = new GooString();
  }

  Object obj1;
  obj1.initString(name->copy());
  update ("NM", &obj1);
}

void Annot::setModified(GooString *new_modified) {
  annotLocker();
  delete modified;

  if (new_modified)
    modified = new GooString(new_modified);
  else
    modified = new GooString();

  Object obj1;
  obj1.initString(modified->copy());
  update ("M", &obj1);
}

void Annot::setFlags(Guint new_flags) {
  annotLocker();
  Object obj1;
  flags = new_flags;
  obj1.initInt(flags);
  update ("F", &obj1);
}

void Annot::setBorder(AnnotBorderArray *new_border) {
  annotLocker();
  delete border;

  if (new_border) {
    Object obj1;
    new_border->writeToObject(xref, &obj1);
    update ("Border", &obj1);
    border = new_border;
  } else {
    border = NULL;
  }
  invalidateAppearance();
}

void Annot::setColor(AnnotColor *new_color) {
  annotLocker();
  delete color;

  if (new_color) {
    Object obj1;
    new_color->writeToObject(xref, &obj1);
    update ("C", &obj1);
    color = new_color;
  } else {
    color = NULL;
  }
  invalidateAppearance();
}

void Annot::setPage(int pageIndex, GBool updateP) {
  annotLocker();
  Page *pageobj = doc->getPage(pageIndex);
  Object obj1;

  if (pageobj) {
    Ref pageRef = pageobj->getRef();
    obj1.initRef(pageRef.num, pageRef.gen);
    page = pageIndex;
  } else {
    obj1.initNull();
    page = 0;
  }

  if (updateP) {
    update("P", &obj1);
  }
}

void Annot::setAppearanceState(const char *state) {
  annotLocker();
  if (!state)
    return;

  delete appearState;
  appearState = new GooString(state);

  delete appearBBox;
  appearBBox = NULL;

  Object obj1;
  obj1.initName(state);
  update ("AS", &obj1);

  // The appearance state determines the current appearance stream
  appearance.free();
  if (appearStreams) {
    appearStreams->getAppearanceStream(AnnotAppearance::appearNormal, appearState->getCString(), &appearance);
  } else {
    appearance.initNull();
  }
}

void Annot::invalidateAppearance() {
  annotLocker();
  if (appearStreams) { // Remove existing appearance streams
    appearStreams->removeAllStreams();
  }
  delete appearStreams;
  appearStreams = NULL;

  delete appearState;
  appearState = NULL;

  delete appearBBox;
  appearBBox = NULL;

  appearance.free();
  appearance.initNull();

  Object obj1, obj2;
  obj1.initNull();
  if (!annotObj.dictLookup("AP", &obj2)->isNull())
    update ("AP", &obj1); // Remove AP
  obj2.free();

  if (!annotObj.dictLookup("AS", &obj2)->isNull())
    update ("AS", &obj1); // Remove AS
  obj2.free();
}

double Annot::getXMin() {
  return rect->x1;
}

double Annot::getYMin() {
  return rect->y1;
}

double Annot::getXMax() {
  return rect->x2;
}

double Annot::getYMax() {
  return rect->y2;
}

void Annot::readArrayNum(Object *pdfArray, int key, double *value) {
  Object valueObject;

  pdfArray->arrayGet(key, &valueObject);
  if (valueObject.isNum()) {
    *value = valueObject.getNum();
  } else {
    *value = 0;
    ok = gFalse;
  }
  valueObject.free();
}

void Annot::removeReferencedObjects() {
  // Remove appearance streams (if any)
  invalidateAppearance();
}

void Annot::incRefCnt() {
  annotLocker();
  refCnt++;
}

void Annot::decRefCnt() {
#if MULTITHREADED
  gLockMutex(&mutex);
#endif
  if (--refCnt == 0) {
#if MULTITHREADED
    gUnlockMutex(&mutex);
#endif
    delete this;
    return;
  }
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
}

Annot::~Annot() {
  annotObj.free();
  
  delete rect;
  
  if (contents)
    delete contents;

  if (name)
    delete name;

  if (modified)
    delete modified;

  delete appearStreams;
  delete appearBBox;
  appearance.free();

  if (appearState)
    delete appearState;

  if (border)
    delete border;

  if (color)
    delete color;

  oc.free();

#if MULTITHREADED
    gDestroyMutex(&mutex);
#endif
}

void Annot::setColor(AnnotColor *color, GBool fill) {
  const double *values = color->getValues();

  switch (color->getSpace()) {
  case AnnotColor::colorCMYK:
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:c}\n",
		       values[0], values[1], values[2], values[3],
		       fill ? 'k' : 'K');
    break;
  case AnnotColor::colorRGB:
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:s}\n",
		       values[0], values[1], values[2],
		       fill ? "rg" : "RG");
    break;
  case AnnotColor::colorGray:
    appearBuf->appendf("{0:.2f} {1:c}\n",
		       values[0],
		       fill ? 'g' : 'G');
    break;
  case AnnotColor::colorTransparent:
  default:
    break;
  }
}

// Draw an (approximate) circle of radius <r> centered at (<cx>, <cy>).
// If <fill> is true, the circle is filled; otherwise it is stroked.
void Annot::drawCircle(double cx, double cy, double r, GBool fill) {
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx + r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + r, cy + bezierCircle * r,
      cx + bezierCircle * r, cy + r,
      cx, cy + r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - bezierCircle * r, cy + r,
      cx - r, cy + bezierCircle * r,
      cx - r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - r, cy - bezierCircle * r,
      cx - bezierCircle * r, cy - r,
      cx, cy - r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + bezierCircle * r, cy - r,
      cx + r, cy - bezierCircle * r,
      cx + r, cy);
  appearBuf->append(fill ? "f\n" : "s\n");
}

// Draw the top-left half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleTopLeft(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx + r2, cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + (1 - bezierCircle) * r2,
      cy + (1 + bezierCircle) * r2,
      cx - (1 - bezierCircle) * r2,
      cy + (1 + bezierCircle) * r2,
      cx - r2,
      cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - (1 + bezierCircle) * r2,
      cy + (1 - bezierCircle) * r2,
      cx - (1 + bezierCircle) * r2,
      cy - (1 - bezierCircle) * r2,
      cx - r2,
      cy - r2);
  appearBuf->append("S\n");
}

// Draw the bottom-right half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleBottomRight(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx - r2, cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - (1 - bezierCircle) * r2,
      cy - (1 + bezierCircle) * r2,
      cx + (1 - bezierCircle) * r2,
      cy - (1 + bezierCircle) * r2,
      cx + r2,
      cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + (1 + bezierCircle) * r2,
      cy - (1 - bezierCircle) * r2,
      cx + (1 + bezierCircle) * r2,
      cy + (1 - bezierCircle) * r2,
      cx + r2,
      cy + r2);
  appearBuf->append("S\n");
}

void Annot::createForm(double *bbox, GBool transparencyGroup, Object *resDict, Object *aStream) {
  Object obj1, obj2;
  Object appearDict;

  appearDict.initDict(xref);
  appearDict.dictSet("Length", obj1.initInt(appearBuf->getLength()));
  appearDict.dictSet("Subtype", obj1.initName("Form"));
  obj1.initArray(xref);
  obj1.arrayAdd(obj2.initReal(bbox[0]));
  obj1.arrayAdd(obj2.initReal(bbox[1]));
  obj1.arrayAdd(obj2.initReal(bbox[2]));
  obj1.arrayAdd(obj2.initReal(bbox[3]));
  appearDict.dictSet("BBox", &obj1);
  if (transparencyGroup) {
    Object transDict;
    transDict.initDict(xref);
    transDict.dictSet("S", obj1.initName("Transparency"));
    appearDict.dictSet("Group", &transDict);
  }
  if (resDict)
    appearDict.dictSet("Resources", resDict);

  MemStream *mStream = new MemStream(copyString(appearBuf->getCString()), 0,
				     appearBuf->getLength(), &appearDict);
  mStream->setNeedFree(gTrue);
  aStream->initStream(mStream);
}

void Annot::createResourcesDict(const char *formName, Object *formStream,
				const char *stateName,
				double opacity, const char *blendMode,
				Object *resDict) {
  Object gsDict, stateDict, formDict, obj1;

  gsDict.initDict(xref);
  if (opacity != 1) {
    gsDict.dictSet("CA", obj1.initReal(opacity));
    gsDict.dictSet("ca", obj1.initReal(opacity));
  }
  if (blendMode)
    gsDict.dictSet("BM", obj1.initName(blendMode));
  stateDict.initDict(xref);
  stateDict.dictSet(stateName, &gsDict);
  formDict.initDict(xref);
  formDict.dictSet(formName, formStream);

  resDict->initDict(xref);
  resDict->dictSet("ExtGState", &stateDict);
  resDict->dictSet("XObject", &formDict);
}

Object *Annot::getAppearanceResDict(Object *dest) {
  Object obj1, obj2;

  dest->initNull(); // Default value

  // Fetch appearance's resource dict (if any)
  appearance.fetch(xref, &obj1);
  if (obj1.isStream()) {
    obj1.streamGetDict()->lookup("Resources", &obj2);
    if (obj2.isDict()) {
      obj2.copy(dest);
    }
    obj2.free();
  }
  obj1.free();

  return dest;
}

GBool Annot::isVisible(GBool printing) {
  // check the flags
  if ((flags & flagHidden) ||
      (printing && !(flags & flagPrint)) ||
      (!printing && (flags & flagNoView))) {
    return gFalse;
  }

  // check the OC
  OCGs *optContentConfig = doc->getCatalog()->getOptContentConfig();
  if (optContentConfig) {
    if (! optContentConfig->optContentIsVisible(&oc))
      return gFalse;
  }

  return gTrue;
}

int Annot::getRotation() const
{
  Page *pageobj = doc->getPage(page);
  assert(pageobj != NULL);

  if (flags & flagNoRotate) {
    return (360 - pageobj->getRotate()) % 360;
  } else {
    return 0;
  }
}

void Annot::draw(Gfx *gfx, GBool printing) {
  Object obj;

  annotLocker();
  if (!isVisible (printing))
    return;

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
      rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  obj.free();
}

//------------------------------------------------------------------------
// AnnotPopup
//------------------------------------------------------------------------

AnnotPopup::AnnotPopup(PDFDoc *docA, PDFRectangle *rect) :
    Annot(docA, rect) {
  Object obj1;

  type = typePopup;

  annotObj.dictSet ("Subtype", obj1.initName ("Popup"));
  initialize (docA, annotObj.getDict());
}

AnnotPopup::AnnotPopup(PDFDoc *docA, Dict *dict, Object *obj) :
    Annot(docA, dict, obj) {
  type = typePopup;
  initialize(docA, dict);
}

AnnotPopup::~AnnotPopup() {
  parent.free();
}

void AnnotPopup::initialize(PDFDoc *docA, Dict *dict) {
  Object obj1;

  if (!dict->lookupNF("Parent", &parent)->isRef()) {
    parent.initNull();
  }

  if (dict->lookup("Open", &obj1)->isBool()) {
    open = obj1.getBool();
  } else {
    open = gFalse;
  }
  obj1.free();
}

void AnnotPopup::setParent(Object *parentA) {
  parentA->copy(&parent);
  update ("Parent", &parent);
}

void AnnotPopup::setParent(Annot *parentA) {
  Ref parentRef = parentA->getRef();
  parent.initRef(parentRef.num, parentRef.gen);
  update ("Parent", &parent);
}

void AnnotPopup::setOpen(GBool openA) {
  Object obj1;

  open = openA;
  obj1.initBool(open);
  update ("Open", &obj1);
}

//------------------------------------------------------------------------
// AnnotMarkup
//------------------------------------------------------------------------
AnnotMarkup::AnnotMarkup(PDFDoc *docA, PDFRectangle *rect) :
    Annot(docA, rect) {
  initialize(docA, annotObj.getDict(), &annotObj);
}

AnnotMarkup::AnnotMarkup(PDFDoc *docA, Dict *dict, Object *obj) :
    Annot(docA, dict, obj) {
  initialize(docA, dict, obj);
}

AnnotMarkup::~AnnotMarkup() {
  if (label)
    delete label;

  if (popup)
    delete popup;

  if (date)
    delete date;

  if (subject)
    delete subject;
}

void AnnotMarkup::initialize(PDFDoc *docA, Dict *dict, Object *obj) {
  Object obj1, obj2;

  if (dict->lookup("T", &obj1)->isString()) {
    label = obj1.getString()->copy();
  } else {
    label = NULL;
  }
  obj1.free();

  if (dict->lookup("Popup", &obj1)->isDict() && dict->lookupNF("Popup", &obj2)->isRef()) {
    popup = new AnnotPopup(docA, obj1.getDict(), &obj2);
  } else {
    popup = NULL;
  }
  obj1.free();

  if (dict->lookup("CA", &obj1)->isNum()) {
    opacity = obj1.getNum();
  } else {
    opacity = 1.0;
  }
  obj1.free();

  if (dict->lookup("CreationDate", &obj1)->isString()) {
    date = obj1.getString()->copy();
  } else {
    date = NULL;
  }
  obj1.free();

  if (dict->lookupNF("IRT", &obj1)->isRef()) {
    inReplyTo = obj1.getRef();
  } else {
    inReplyTo.num = 0;
    inReplyTo.gen = 0;
  }
  obj1.free();

  if (dict->lookup("Subj", &obj1)->isString()) {
    subject = obj1.getString()->copy();
  } else {
    subject = NULL;
  }
  obj1.free();

  if (dict->lookup("RT", &obj1)->isName()) {
    const char *replyName = obj1.getName();

    if (!strcmp(replyName, "R")) {
      replyTo = replyTypeR;
    } else if (!strcmp(replyName, "Group")) {
      replyTo = replyTypeGroup;
    } else {
      replyTo = replyTypeR;
    }
  } else {
    replyTo = replyTypeR;
  }
  obj1.free();

  if (dict->lookup("ExData", &obj1)->isDict()) {
    exData = parseAnnotExternalData(obj1.getDict());
  } else {
    exData = annotExternalDataMarkupUnknown;
  }
  obj1.free();
}

void AnnotMarkup::setLabel(GooString *new_label) {
  delete label;

  if (new_label) {
    label = new GooString(new_label);
    //append the unicode marker <FE FF> if needed
    if (!label->hasUnicodeMarker()) {
      label->insert(0, 0xff);
      label->insert(0, 0xfe);
    }
  } else {
    label = new GooString();
  }

  Object obj1;
  obj1.initString(label->copy());
  update ("T", &obj1);
}

void AnnotMarkup::setPopup(AnnotPopup *new_popup) {
  delete popup;

  if (new_popup) {
    Object obj1;
    Ref popupRef = new_popup->getRef();

    obj1.initRef (popupRef.num, popupRef.gen);
    update ("Popup", &obj1);

    new_popup->setParent(this);
    popup = new_popup;
  } else {
    popup = NULL;
  }
}

void AnnotMarkup::setOpacity(double opacityA) {
  Object obj1;

  opacity = opacityA;
  obj1.initReal(opacity);
  update ("CA", &obj1);
  invalidateAppearance();
}

void AnnotMarkup::setDate(GooString *new_date) {
  delete date;

  if (new_date)
    date = new GooString(new_date);
  else
    date = new GooString();

  Object obj1;
  obj1.initString(date->copy());
  update ("CreationDate", &obj1);
}

void AnnotMarkup::removeReferencedObjects() {
  Page *pageobj = doc->getPage(page);
  assert(pageobj != NULL); // We're called when removing an annot from a page

  // Remove popup
  if (popup) {
    pageobj->removeAnnot(popup);
  }

  Annot::removeReferencedObjects();
}

//------------------------------------------------------------------------
// AnnotText
//------------------------------------------------------------------------

AnnotText::AnnotText(PDFDoc *docA, PDFRectangle *rect) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeText;
  flags |= flagNoZoom | flagNoRotate;

  annotObj.dictSet ("Subtype", obj1.initName ("Text"));
  initialize (docA, annotObj.getDict());
}

AnnotText::AnnotText(PDFDoc *docA, Dict *dict, Object *obj) :
    AnnotMarkup(docA, dict, obj) {

  type = typeText;
  flags |= flagNoZoom | flagNoRotate;
  initialize (docA, dict);
}

AnnotText::~AnnotText() {
  delete icon;
}

void AnnotText::initialize(PDFDoc *docA, Dict *dict) {
  Object obj1;

  if (dict->lookup("Open", &obj1)->isBool())
    open = obj1.getBool();
  else
    open = gFalse;
  obj1.free();

  if (dict->lookup("Name", &obj1)->isName()) {
    icon = new GooString(obj1.getName());
  } else {
    icon = new GooString("Note");
  }
  obj1.free();

  if (dict->lookup("StateModel", &obj1)->isString()) {
    Object obj2;
    GooString *modelName = obj1.getString();

    if (dict->lookup("State", &obj2)->isString()) {
      GooString *stateName = obj2.getString();

      if (!stateName->cmp("Marked")) {
        state = stateMarked;
      } else if (!stateName->cmp("Unmarked")) {
        state = stateUnmarked;
      } else if (!stateName->cmp("Accepted")) {
        state = stateAccepted;
      } else if (!stateName->cmp("Rejected")) {
        state = stateRejected;
      } else if (!stateName->cmp("Cancelled")) {
        state = stateCancelled;
      } else if (!stateName->cmp("Completed")) {
        state = stateCompleted;
      } else if (!stateName->cmp("None")) {
        state = stateNone;
      } else {
        state = stateUnknown;
      }
    } else {
      state = stateUnknown;
    }
    obj2.free();

    if (!modelName->cmp("Marked")) {
      switch (state) {
        case stateUnknown:
          state = stateMarked;
          break;
        case stateAccepted:
        case stateRejected:
        case stateCancelled:
        case stateCompleted:
        case stateNone:
          state = stateUnknown;
          break;
        default:
          break;
      }
    } else if (!modelName->cmp("Review")) {
      switch (state) {
        case stateUnknown:
          state = stateNone;
          break;
        case stateMarked:
        case stateUnmarked:
          state = stateUnknown;
          break;
        default:
          break;
      }
    } else {
      state = stateUnknown;
    }
  } else {
    state = stateUnknown;
  }
  obj1.free();
}

void AnnotText::setOpen(GBool openA) {
  Object obj1;

  open = openA;
  obj1.initBool(open);
  update ("Open", &obj1);
}

void AnnotText::setIcon(GooString *new_icon) {
  if (new_icon && icon->cmp(new_icon) == 0)
    return;

  delete icon;

  if (new_icon) {
    icon = new GooString (new_icon);
  } else {
    icon = new GooString("Note");
  }

  Object obj1;
  obj1.initName (icon->getCString());
  update("Name", &obj1);
  invalidateAppearance();
}

#define ANNOT_TEXT_AP_NOTE                                                    \
  "3.602 24 m 20.398 24 l 22.387 24 24 22.387 24 20.398 c 24 3.602 l 24\n"    \
  "1.613 22.387 0 20.398 0 c 3.602 0 l 1.613 0 0 1.613 0 3.602 c 0 20.398\n"  \
  "l 0 22.387 1.613 24 3.602 24 c h\n"                                        \
  "3.602 24 m f\n"                                                            \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                       \
  "1 J\n"                                                                     \
  "1 j\n"                                                                     \
  "[] 0.0 d\n"                                                                \
  "4 M 9 18 m 4 18 l 4 7 4 4 6 3 c 20 3 l 18 4 18 7 18 18 c 17 18 l S\n"      \
  "1.5 w\n"                                                                   \
  "0 j\n"                                                                     \
  "10 16 m 14 21 l S\n"                                                       \
  "1.85625 w\n"                                                               \
  "1 j\n"                                                                     \
  "15.07 20.523 m 15.07 19.672 14.379 18.977 13.523 18.977 c 12.672 18.977\n" \
  "11.977 19.672 11.977 20.523 c 11.977 21.379 12.672 22.07 13.523 22.07 c\n" \
  "14.379 22.07 15.07 21.379 15.07 20.523 c h\n"                              \
  "15.07 20.523 m S\n"                                                        \
  "1 w\n"                                                                     \
  "0 j\n"                                                                     \
  "6.5 13.5 m 15.5 13.5 l S\n"                                                \
  "6.5 10.5 m 13.5 10.5 l S\n"                                                \
  "6.801 7.5 m 15.5 7.5 l S\n"                                                \
  "0.729412 0.741176 0.713725 RG 2 w\n"                                       \
  "1 j\n"                                                                     \
  "9 19 m 4 19 l 4 8 4 5 6 4 c 20 4 l 18 5 18 8 18 19 c 17 19 l S\n"          \
  "1.5 w\n"                                                                   \
  "0 j\n"                                                                     \
  "10 17 m 14 22 l S\n"                                                       \
  "1.85625 w\n"                                                               \
  "1 j\n"                                                                     \
  "15.07 21.523 m 15.07 20.672 14.379 19.977 13.523 19.977 c 12.672 19.977\n" \
  "11.977 20.672 11.977 21.523 c 11.977 22.379 12.672 23.07 13.523 23.07 c\n" \
  "14.379 23.07 15.07 22.379 15.07 21.523 c h\n"                              \
  "15.07 21.523 m S\n"                                                        \
  "1 w\n"                                                                     \
  "0 j\n"                                                                     \
  "6.5 14.5 m 15.5 14.5 l S\n"                                                \
  "6.5 11.5 m 13.5 11.5 l S\n"                                                \
  "6.801 8.5 m 15.5 8.5 l S\n"

#define ANNOT_TEXT_AP_COMMENT                                                   \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"      \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"    \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                          \
  "4.301 23 m f\n"                                                              \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                         \
  "0 J\n"                                                                       \
  "1 j\n"                                                                       \
  "[] 0.0 d\n"                                                                  \
  "4 M 8 20 m 16 20 l 18.363 20 20 18.215 20 16 c 20 13 l 20 10.785 18.363 9\n" \
  "16 9 c 13 9 l 8 3 l 8 9 l 8 9 l 5.637 9 4 10.785 4 13 c 4 16 l 4 18.215\n"   \
  "5.637 20 8 20 c h\n"                                                         \
  "8 20 m S\n"                                                                  \
  "0.729412 0.741176 0.713725 RG 8 21 m 16 21 l 18.363 21 20 19.215 20 17\n"    \
  "c 20 14 l 20 11.785 18.363 10\n"                                             \
  "16 10 c 13 10 l 8 4 l 8 10 l 8 10 l 5.637 10 4 11.785 4 14 c 4 17 l 4\n"     \
  "19.215 5.637 21 8 21 c h\n"                                                  \
  "8 21 m S\n"

#define ANNOT_TEXT_AP_KEY                                                    \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"   \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n" \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                       \
  "4.301 23 m f\n"                                                           \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                      \
  "1 J\n"                                                                    \
  "0 j\n"                                                                    \
  "[] 0.0 d\n"                                                               \
  "4 M 11.895 18.754 m 13.926 20.625 17.09 20.496 18.961 18.465 c 20.832\n"  \
  "16.434 20.699 13.27 18.668 11.398 c 17.164 10.016 15.043 9.746 13.281\n"  \
  "10.516 c 12.473 9.324 l 11.281 10.078 l 9.547 8.664 l 9.008 6.496 l\n"    \
  "7.059 6.059 l 6.34 4.121 l 5.543 3.668 l 3.375 4.207 l 2.938 6.156 l\n"   \
  "10.57 13.457 l 9.949 15.277 10.391 17.367 11.895 18.754 c h\n"            \
  "11.895 18.754 m S\n"                                                      \
  "1.5 w\n"                                                                  \
  "16.059 15.586 m 16.523 15.078 17.316 15.043 17.824 15.512 c 18.332\n"     \
  "15.98 18.363 16.77 17.895 17.277 c 17.43 17.785 16.637 17.816 16.129\n"   \
  "17.352 c 15.621 16.883 15.59 16.094 16.059 15.586 c h\n"                  \
  "16.059 15.586 m S\n"                                                      \
  "0.729412 0.741176 0.713725 RG 2 w\n"                                      \
  "11.895 19.754 m 13.926 21.625 17.09 21.496 18.961 19.465 c 20.832\n"      \
  "17.434 20.699 14.27 18.668 12.398 c 17.164 11.016 15.043 10.746 13.281\n" \
  "11.516 c 12.473 10.324 l 11.281 11.078 l 9.547 9.664 l 9.008 7.496 l\n"   \
  "7.059 7.059 l 6.34 5.121 l 5.543 4.668 l 3.375 5.207 l 2.938 7.156 l\n"   \
  "10.57 14.457 l 9.949 16.277 10.391 18.367 11.895 19.754 c h\n"            \
  "11.895 19.754 m S\n"                                                      \
  "1.5 w\n"                                                                  \
  "16.059 16.586 m 16.523 16.078 17.316 16.043 17.824 16.512 c 18.332\n"     \
  "16.98 18.363 17.77 17.895 18.277 c 17.43 18.785 16.637 18.816 16.129\n"   \
  "18.352 c 15.621 17.883 15.59 17.094 16.059 16.586 c h\n"                  \
  "16.059 16.586 m S\n"

#define ANNOT_TEXT_AP_HELP                                                        \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"        \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"      \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                            \
  "4.301 23 m f\n"                                                                \
  "0.533333 0.541176 0.521569 RG 2.5 w\n"                                         \
  "1 J\n"                                                                         \
  "1 j\n"                                                                         \
  "[] 0.0 d\n"                                                                    \
  "4 M 8.289 16.488 m 8.824 17.828 10.043 18.773 11.473 18.965 c 12.902 19.156\n" \
  "14.328 18.559 15.195 17.406 c 16.062 16.254 16.242 14.723 15.664 13.398\n"     \
  "c S\n"                                                                         \
  "0 j\n"                                                                         \
  "12 8 m 12 12 16 11 16 15 c S\n"                                                \
  "1.539286 w\n"                                                                  \
  "1 j\n"                                                                         \
  "q 1 0 0 -0.999991 0 24 cm\n"                                                   \
  "12.684 20.891 m 12.473 21.258 12.004 21.395 11.629 21.196 c 11.254\n"          \
  "20.992 11.105 20.531 11.297 20.149 c 11.488 19.77 11.945 19.61 12.332\n"       \
  "19.789 c 12.719 19.969 12.891 20.426 12.719 20.817 c S Q\n"                    \
  "0.729412 0.741176 0.713725 RG 2.5 w\n"                                         \
  "8.289 17.488 m 9.109 19.539 11.438 20.535 13.488 19.711 c 15.539 18.891\n"     \
  "16.535 16.562 15.711 14.512 c 15.699 14.473 15.684 14.438 15.664 14.398\n"     \
  "c S\n"                                                                         \
  "0 j\n"                                                                         \
  "12 9 m 12 13 16 12 16 16 c S\n"                                                \
  "1.539286 w\n"                                                                  \
  "1 j\n"                                                                         \
  "q 1 0 0 -0.999991 0 24 cm\n"                                                   \
  "12.684 19.891 m 12.473 20.258 12.004 20.395 11.629 20.195 c 11.254\n"          \
  "19.992 11.105 19.531 11.297 19.149 c 11.488 18.77 11.945 18.61 12.332\n"       \
  "18.789 c 12.719 18.969 12.891 19.426 12.719 19.817 c S Q\n"

#define ANNOT_TEXT_AP_NEW_PARAGRAPH                                               \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"        \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"      \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                            \
  "4.301 23 m f\n"                                                                \
  "0.533333 0.541176 0.521569 RG 4 w\n"                                           \
  "0 J\n"                                                                         \
  "2 j\n"                                                                         \
  "[] 0.0 d\n"                                                                    \
  "4 M q 1 0 0 -1 0 24 cm\n"                                                      \
  "9.211 11.988 m 8.449 12.07 7.711 11.707 7.305 11.059 c 6.898 10.41\n"          \
  "6.898 9.59 7.305 8.941 c 7.711 8.293 8.449 7.93 9.211 8.012 c S Q\n"           \
  "1.004413 w\n"                                                                  \
  "1 J\n"                                                                         \
  "1 j\n"                                                                         \
  "q 1 0 0 -0.991232 0 24 cm\n"                                                   \
  "18.07 11.511 m 15.113 10.014 l 12.199 11.602 l 12.711 8.323 l 10.301\n"        \
  "6.045 l 13.574 5.517 l 14.996 2.522 l 16.512 5.474 l 19.801 5.899 l\n"         \
  "17.461 8.252 l 18.07 11.511 l h\n"                                             \
  "18.07 11.511 m S Q\n"                                                          \
  "2 w\n"                                                                         \
  "0 j\n"                                                                         \
  "11 17 m 10 17 l 10 3 l S\n"                                                    \
  "14 3 m 14 13 l S\n"                                                            \
  "0.729412 0.741176 0.713725 RG 4 w\n"                                           \
  "0 J\n"                                                                         \
  "2 j\n"                                                                         \
  "q 1 0 0 -1 0 24 cm\n"                                                          \
  "9.211 10.988 m 8.109 11.105 7.125 10.309 7.012 9.211 c 6.895 8.109\n"          \
  "7.691 7.125 8.789 7.012 c 8.93 6.996 9.07 6.996 9.211 7.012 c S Q\n"           \
  "1.004413 w\n"                                                                  \
  "1 J\n"                                                                         \
  "1 j\n"                                                                         \
  "q 1 0 0 -0.991232 0 24 cm\n"                                                   \
  "18.07 10.502 m 15.113 9.005 l 12.199 10.593 l 12.711 7.314 l 10.301\n"         \
  "5.036 l 13.574 4.508 l 14.996 1.513 l 16.512 4.465 l 19.801 4.891 l\n"         \
  "17.461 7.243 l 18.07 10.502 l h\n"                                             \
  "18.07 10.502 m S Q\n"                                                          \
  "2 w\n"                                                                         \
  "0 j\n"                                                                         \
  "11 18 m 10 18 l 10 4 l S\n"                                                    \
  "14 4 m 14 14 l S\n"

#define ANNOT_TEXT_AP_PARAGRAPH                                              \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"   \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n" \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                       \
  "4.301 23 m f\n"                                                           \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                      \
  "1 J\n"                                                                    \
  "1 j\n"                                                                    \
  "[] 0.0 d\n"                                                               \
  "4 M 15 3 m 15 18 l 11 18 l 11 3 l S\n"                                    \
  "4 w\n"                                                                    \
  "q 1 0 0 -1 0 24 cm\n"                                                     \
  "9.777 10.988 m 8.746 10.871 7.973 9.988 8 8.949 c 8.027 7.91 8.844\n"     \
  "7.066 9.879 7.004 c S Q\n"                                                \
  "0.729412 0.741176 0.713725 RG 2 w\n"                                      \
  "15 4 m 15 19 l 11 19 l 11 4 l S\n"                                        \
  "4 w\n"                                                                    \
  "q 1 0 0 -1 0 24 cm\n"                                                     \
  "9.777 9.988 m 8.746 9.871 7.973 8.988 8 7.949 c 8.027 6.91 8.844 6.066\n" \
  "9.879 6.004 c S Q\n"

#define ANNOT_TEXT_AP_INSERT                                                 \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"   \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n" \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                       \
  "4.301 23 m f\n"                                                           \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                      \
  "1 J\n"                                                                    \
  "0 j\n"                                                                    \
  "[] 0.0 d\n"                                                               \
  "4 M 12 18.012 m 20 18 l S\n"                                              \
  "9 10 m 17 10 l S\n"                                                       \
  "12 14.012 m 20 14 l S\n"                                                  \
  "12 6.012 m 20 6.012 l S\n"                                                \
  "4 12 m 6 10 l 4 8 l S\n"                                                  \
  "4 12 m 4 8 l S\n"                                                         \
  "0.729412 0.741176 0.713725 RG 12 19.012 m 20 19 l S\n"                    \
  "9 11 m 17 11 l S\n"                                                       \
  "12 15.012 m 20 15 l S\n"                                                  \
  "12 7.012 m 20 7.012 l S\n"                                                \
  "4 13 m 6 11 l 4 9 l S\n"                                                  \
  "4 13 m 4 9 l S\n"

#define ANNOT_TEXT_AP_CROSS                                                  \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"   \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n" \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                       \
  "4.301 23 m f\n"                                                           \
  "0.533333 0.541176 0.521569 RG 2.5 w\n"                                    \
  "1 J\n"                                                                    \
  "0 j\n"                                                                    \
  "[] 0.0 d\n"                                                               \
  "4 M 18 5 m 6 17 l S\n"                                                    \
  "6 5 m 18 17 l S\n"                                                        \
  "0.729412 0.741176 0.713725 RG 18 6 m 6 18 l S\n"                          \
  "6 6 m 18 18 l S\n"

#define ANNOT_TEXT_AP_CIRCLE                                                      \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"        \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"      \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                            \
  "4.301 23 m f\n"                                                                \
  "0.533333 0.541176 0.521569 RG 2.5 w\n"                                         \
  "1 J\n"                                                                         \
  "1 j\n"                                                                         \
  "[] 0.0 d\n"                                                                    \
  "4 M 19.5 11.5 m 19.5 7.359 16.141 4 12 4 c 7.859 4 4.5 7.359 4.5 11.5 c 4.5\n" \
  "15.641 7.859 19 12 19 c 16.141 19 19.5 15.641 19.5 11.5 c h\n"                 \
  "19.5 11.5 m S\n"                                                               \
  "0.729412 0.741176 0.713725 RG 19.5 12.5 m 19.5 8.359 16.141 5 12 5 c\n"        \
  "7.859 5 4.5 8.359 4.5 12.5 c 4.5\n"                                            \
  "16.641 7.859 20 12 20 c 16.141 20 19.5 16.641 19.5 12.5 c h\n"                 \
  "19.5 12.5 m S\n"

void AnnotText::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    ca = opacity;

    appearBuf = new GooString ();

    appearBuf->append ("q\n");
    if (color)
      setColor(color, gTrue);
    else
      appearBuf->append ("1 1 1 rg\n");
    if (!icon->cmp("Note"))
      appearBuf->append (ANNOT_TEXT_AP_NOTE);
    else if (!icon->cmp("Comment"))
      appearBuf->append (ANNOT_TEXT_AP_COMMENT);
    else if (!icon->cmp("Key"))
      appearBuf->append (ANNOT_TEXT_AP_KEY);
    else if (!icon->cmp("Help"))
      appearBuf->append (ANNOT_TEXT_AP_HELP);
    else if (!icon->cmp("NewParagraph"))
      appearBuf->append (ANNOT_TEXT_AP_NEW_PARAGRAPH);
    else if (!icon->cmp("Paragraph"))
      appearBuf->append (ANNOT_TEXT_AP_PARAGRAPH);
    else if (!icon->cmp("Insert"))
      appearBuf->append (ANNOT_TEXT_AP_INSERT);
    else if (!icon->cmp("Cross"))
      appearBuf->append (ANNOT_TEXT_AP_CROSS);
    else if (!icon->cmp("Circle"))
      appearBuf->append (ANNOT_TEXT_AP_CIRCLE);
    appearBuf->append ("Q\n");

    // Force 24x24 rectangle
    PDFRectangle fixedRect(rect->x1, rect->y2 - 24, rect->x1 + 24, rect->y2);
    appearBBox = new AnnotAppearanceBBox(&fixedRect);
    double bbox[4];
    appearBBox->getBBoxRect(bbox);
    if (ca == 1) {
      createForm(bbox, gFalse, NULL, &appearance);
    } else {
      Object aStream, resDict;

      createForm(bbox, gTrue, NULL, &aStream);
      delete appearBuf;

      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
      createForm(bbox, gFalse, &resDict, &appearance);
    }
    delete appearBuf;
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  if (appearBBox) {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   appearBBox->getPageXMin(), appearBBox->getPageYMin(),
                   appearBBox->getPageXMax(), appearBBox->getPageYMax(),
                   getRotation());
  } else {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  }
  obj.free();
}

//------------------------------------------------------------------------
// AnnotLink
//------------------------------------------------------------------------
AnnotLink::AnnotLink(PDFDoc *docA, PDFRectangle *rect) :
    Annot(docA, rect) {
  Object obj1;

  type = typeLink;
  annotObj.dictSet ("Subtype", obj1.initName ("Link"));
  initialize (docA, annotObj.getDict());
}

AnnotLink::AnnotLink(PDFDoc *docA, Dict *dict, Object *obj) :
    Annot(docA, dict, obj) {

  type = typeLink;
  initialize (docA, dict);
}

AnnotLink::~AnnotLink() {
  delete action;
  /*
  if (uriAction)
    delete uriAction;
  */
  if (quadrilaterals)
    delete quadrilaterals;
}

void AnnotLink::initialize(PDFDoc *docA, Dict *dict) {
  Object obj1;

  action = NULL;

  // look for destination
  if (!dict->lookup("Dest", &obj1)->isNull()) {
    action = LinkAction::parseDest(&obj1);
  // look for action
  } else {
    obj1.free();
    if (dict->lookup("A", &obj1)->isDict()) {
      action = LinkAction::parseAction(&obj1, doc->getCatalog()->getBaseURI());
    }
  }
  obj1.free();

  if (dict->lookup("H", &obj1)->isName()) {
    const char *effect = obj1.getName();

    if (!strcmp(effect, "N")) {
      linkEffect = effectNone;
    } else if (!strcmp(effect, "I")) {
      linkEffect = effectInvert;
    } else if (!strcmp(effect, "O")) {
      linkEffect = effectOutline;
    } else if (!strcmp(effect, "P")) {
      linkEffect = effectPush;
    } else {
      linkEffect = effectInvert;
    }
  } else {
    linkEffect = effectInvert;
  }
  obj1.free();
  /*
  if (dict->lookup("PA", &obj1)->isDict()) {
    uriAction = NULL;
  } else {
    uriAction = NULL;
  }
  obj1.free();
  */
  if (dict->lookup("QuadPoints", &obj1)->isArray()) {
    quadrilaterals = new AnnotQuadrilaterals(obj1.getArray(), rect);
  } else {
    quadrilaterals = NULL;
  }
  obj1.free();
}

void AnnotLink::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

  annotLocker();
  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  obj.free();
}

//------------------------------------------------------------------------
// AnnotFreeText
//------------------------------------------------------------------------
AnnotFreeText::AnnotFreeText(PDFDoc *docA, PDFRectangle *rect, GooString *da) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeFreeText;

  annotObj.dictSet ("Subtype", obj1.initName ("FreeText"));

  Object obj2;
  obj2.initString (da->copy());
  annotObj.dictSet("DA", &obj2);

  initialize (docA, annotObj.getDict());
}

AnnotFreeText::AnnotFreeText(PDFDoc *docA, Dict *dict, Object *obj) :
    AnnotMarkup(docA, dict, obj) {
  type = typeFreeText;
  initialize(docA, dict);
}

AnnotFreeText::~AnnotFreeText() {
  delete appearanceString;

  if (styleString)
    delete styleString;

  if (calloutLine)
    delete calloutLine;

  if (borderEffect)
    delete borderEffect;

  if (rectangle)
    delete rectangle;
}

void AnnotFreeText::initialize(PDFDoc *docA, Dict *dict) {
  Object obj1;

  if (dict->lookup("DA", &obj1)->isString()) {
    appearanceString = obj1.getString()->copy();
  } else {
    appearanceString = new GooString();
    error(errSyntaxError, -1, "Bad appearance for annotation");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Q", &obj1)->isInt()) {
    quadding = (AnnotFreeTextQuadding) obj1.getInt();
  } else {
    quadding = quaddingLeftJustified;
  }
  obj1.free();

  if (dict->lookup("DS", &obj1)->isString()) {
    styleString = obj1.getString()->copy();
  } else {
    styleString = NULL;
  }
  obj1.free();

  if (dict->lookup("CL", &obj1)->isArray() && obj1.arrayGetLength() >= 4) {
    double x1, y1, x2, y2;
    Object obj2;

    (obj1.arrayGet(0, &obj2)->isNum() ? x1 = obj2.getNum() : x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? y1 = obj2.getNum() : y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? x2 = obj2.getNum() : x2 = 0);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? y2 = obj2.getNum() : y2 = 0);
    obj2.free();

    if (obj1.arrayGetLength() == 6) {
      double x3, y3;
      (obj1.arrayGet(4, &obj2)->isNum() ? x3 = obj2.getNum() : x3 = 0);
      obj2.free();
      (obj1.arrayGet(5, &obj2)->isNum() ? y3 = obj2.getNum() : y3 = 0);
      obj2.free();
      calloutLine = new AnnotCalloutMultiLine(x1, y1, x2, y2, x3, y3);
    } else {
      calloutLine = new AnnotCalloutLine(x1, y1, x2, y2);
    }
  } else {
    calloutLine = NULL;
  }
  obj1.free();

  if (dict->lookup("IT", &obj1)->isName()) {
    const char *intentName = obj1.getName();

    if (!strcmp(intentName, "FreeText")) {
      intent = intentFreeText;
    } else if (!strcmp(intentName, "FreeTextCallout")) {
      intent = intentFreeTextCallout;
    } else if (!strcmp(intentName, "FreeTextTypeWriter")) {
      intent = intentFreeTextTypeWriter;
    } else {
      intent = intentFreeText;
    }
  } else {
    intent = intentFreeText;
  }
  obj1.free();

  if (dict->lookup("BE", &obj1)->isDict()) {
    borderEffect = new AnnotBorderEffect(obj1.getDict());
  } else {
    borderEffect = NULL;
  }
  obj1.free();

  if (dict->lookup("RD", &obj1)->isArray()) {
    rectangle = parseDiffRectangle(obj1.getArray(), rect);
  } else {
    rectangle = NULL;
  }
  obj1.free();

  if (dict->lookup("LE", &obj1)->isName()) {
    GooString styleName(obj1.getName());
    endStyle = parseAnnotLineEndingStyle(&styleName);
  } else {
    endStyle = annotLineEndingNone;
  }
  obj1.free();
}

void AnnotFreeText::setContents(GooString *new_content) {
  Annot::setContents(new_content);
  invalidateAppearance();
}

void AnnotFreeText::setAppearanceString(GooString *new_string) {
  delete appearanceString;

  if (new_string) {
    appearanceString = new GooString(new_string);
  } else {
    appearanceString = new GooString();
  }

  Object obj1;
  obj1.initString(appearanceString->copy());
  update ("DA", &obj1);
  invalidateAppearance();
}

void AnnotFreeText::setQuadding(AnnotFreeTextQuadding new_quadding) {
  Object obj1;
  quadding = new_quadding;
  obj1.initInt((int)quadding);
  update ("Q", &obj1);
  invalidateAppearance();
}

void AnnotFreeText::setStyleString(GooString *new_string) {
  delete styleString;

  if (new_string) {
    styleString = new GooString(new_string);
    //append the unicode marker <FE FF> if needed
    if (!styleString->hasUnicodeMarker()) {
      styleString->insert(0, 0xff);
      styleString->insert(0, 0xfe);
    }
  } else {
    styleString = new GooString();
  }

  Object obj1;
  obj1.initString(styleString->copy());
  update ("DS", &obj1);
}

void AnnotFreeText::setCalloutLine(AnnotCalloutLine *line) {
  delete calloutLine;

  Object obj1;
  if (line == NULL) {
    obj1.initNull();
    calloutLine = NULL;
  } else {
    double x1 = line->getX1(), y1 = line->getY1();
    double x2 = line->getX2(), y2 = line->getY2();
    Object obj2;
    obj1.initArray(xref);
    obj1.arrayAdd( obj2.initReal(x1) );
    obj1.arrayAdd( obj2.initReal(y1) );
    obj1.arrayAdd( obj2.initReal(x2) );
    obj1.arrayAdd( obj2.initReal(y2) );

    AnnotCalloutMultiLine *mline = dynamic_cast<AnnotCalloutMultiLine*>(line);
    if (mline) {
      double x3 = mline->getX3(), y3 = mline->getY3();
      obj1.arrayAdd( obj2.initReal(x3) );
      obj1.arrayAdd( obj2.initReal(y3) );
      calloutLine = new AnnotCalloutMultiLine(x1, y1, x2, y2, x3, y3);
    } else {
      calloutLine = new AnnotCalloutLine(x1, y1, x2, y2);
    }
  }

  update("CL", &obj1);
  invalidateAppearance();
}

void AnnotFreeText::setIntent(AnnotFreeTextIntent new_intent) {
  Object obj1;

  intent = new_intent;
  if (new_intent == intentFreeText)
    obj1.initName("FreeText");
  else if (new_intent == intentFreeTextCallout)
    obj1.initName("FreeTextCallout");
  else // intentFreeTextTypeWriter
    obj1.initName("FreeTextTypeWriter");
  update ("IT", &obj1);
}

static GfxFont * createAnnotDrawFont(XRef * xref, Object *fontResDict)
{
  Ref dummyRef = { -1, -1 };

  Object baseFontObj, subtypeObj, encodingObj;
  baseFontObj.initName("Helvetica");
  subtypeObj.initName("Type0");
  encodingObj.initName("WinAnsiEncoding");

  Object fontDictObj;
  Dict *fontDict = new Dict(xref);
  fontDict->decRef();
  fontDict->add(copyString("BaseFont"), &baseFontObj);
  fontDict->add(copyString("Subtype"), &subtypeObj);
  fontDict->add(copyString("Encoding"), &encodingObj);
  fontDictObj.initDict(fontDict);

  Object fontsDictObj;
  Dict *fontsDict = new Dict(xref);
  fontsDict->decRef();
  fontsDict->add(copyString("AnnotDrawFont"), &fontDictObj);
  fontsDictObj.initDict(fontsDict);

  Dict *dict = new Dict(xref);
  dict->add(copyString("Font"), &fontsDictObj);

  fontResDict->initDict(dict);
  return GfxFont::makeFont(xref, "AnnotDrawFont", dummyRef, fontDict);
}

void AnnotFreeText::parseAppearanceString(GooString *da, double &fontsize, AnnotColor* &fontcolor) {
  fontsize = -1;
  fontcolor = NULL;
  if (da) {
    GooList * daToks = new GooList();
    int j, i = 0;

    // Tokenize
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
        ++i;
      }
      if (i < da->getLength()) {
        for (j = i + 1; j < da->getLength() && !Lexer::isSpace(da->getChar(j)); ++j) {
        }
        daToks->append(new GooString(da, i, j - i));
        i = j;
      }
    }

    // Scan backwards: we are looking for the last set value
    for (i = daToks->getLength()-1; i >= 0; --i) {
      if (fontsize == -1) {
        if (!((GooString *)daToks->get(i))->cmp("Tf") && i >= 2) {
            // TODO: Font name
            fontsize = gatof(( (GooString *)daToks->get(i-1) )->getCString());
        }
      }
      if (fontcolor == NULL) {
        if (!((GooString *)daToks->get(i))->cmp("g") && i >= 1) {
          fontcolor = new AnnotColor(gatof(( (GooString *)daToks->get(i-1) )->getCString()));
        } else if (!((GooString *)daToks->get(i))->cmp("rg") && i >= 3) {
          fontcolor = new AnnotColor(gatof(( (GooString *)daToks->get(i-3) )->getCString()),
                                     gatof(( (GooString *)daToks->get(i-2) )->getCString()),
                                     gatof(( (GooString *)daToks->get(i-1) )->getCString()));
        } else if (!((GooString *)daToks->get(i))->cmp("k") && i >= 4) {
          fontcolor = new AnnotColor(gatof(( (GooString *)daToks->get(i-4) )->getCString()),
                                     gatof(( (GooString *)daToks->get(i-3) )->getCString()),
                                     gatof(( (GooString *)daToks->get(i-2) )->getCString()),
                                     gatof(( (GooString *)daToks->get(i-1) )->getCString()));
        }
      }
    }
    deleteGooList(daToks, GooString);
  }
}

void AnnotFreeText::generateFreeTextAppearance()
{
  double borderWidth, ca = opacity;

  appearBuf = new GooString ();
  appearBuf->append ("q\n");
  
  if (border) {
    int i, dashLength;
    double *dash;
    borderWidth = border->getWidth();

    switch (border->getStyle()) {
    case AnnotBorder::borderDashed:
      appearBuf->append("[");
      dashLength = border->getDashLength();
      dash = border->getDash();
      for (i = 0; i < dashLength; ++i)
        appearBuf->appendf(" {0:.2f}", dash[i]);
      appearBuf->append(" ] 0 d\n");
      break;
    default:
      appearBuf->append("[] 0 d\n");
      break;
    }
    appearBuf->appendf("{0:.2f} w\n", borderWidth);
  } else {
    borderWidth = 0; // No border
  }

  // Box size
  const double width = rect->x2 - rect->x1;
  const double height = rect->y2 - rect->y1;

  // Parse some properties from the appearance string
  double fontsize;
  AnnotColor *fontcolor;
  parseAppearanceString(appearanceString, fontsize, fontcolor);
  // Default values
  if (fontsize <= 0)
    fontsize = 10;
  if (fontcolor == NULL)
    fontcolor = new AnnotColor(0, 0, 0); // Black

  // Draw box
  GBool doFill = (color && color->getSpace() != AnnotColor::colorTransparent);
  GBool doStroke = (borderWidth != 0);
  if (doFill || doStroke) {
    if (doStroke) {
      setColor(fontcolor, gFalse); // Border color: same as font color
    }
    appearBuf->appendf ("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re\n", borderWidth/2, width-borderWidth, height-borderWidth);
    if (doFill) {
      setColor(color, gTrue);
      appearBuf->append(doStroke ? "B\n" : "f\n");
    } else {
      appearBuf->append("S\n");
    }
  }

  // Setup text clipping
  const double textmargin = borderWidth * 2;
  const double textwidth = width - 2*textmargin;
  appearBuf->appendf ("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n", textmargin, textwidth, height - 2*textmargin);

  Object fontResDict;
  GfxFont *font = createAnnotDrawFont(xref, &fontResDict);

  // Set font state
  setColor(fontcolor, gTrue);
  appearBuf->appendf ("BT 1 0 0 1 {0:.2f} {1:.2f} Tm\n", textmargin, height - textmargin - fontsize * font->getDescent());
  appearBuf->appendf ("/AnnotDrawFont {0:.2f} Tf\n", fontsize);

  int i = 0;
  double xposPrev = 0;
  while (i < contents->getLength()) {
    GooString out;
    double linewidth, xpos;
    layoutText(contents, &out, &i, font, &linewidth, textwidth/fontsize, NULL, gFalse);
    linewidth *= fontsize;
    switch (quadding) {
    case quaddingCentered:
      xpos = (textwidth - linewidth) / 2;
      break;
    case quaddingRightJustified:
      xpos = textwidth - linewidth;
      break;
    default: // quaddingLeftJustified:
      xpos = 0;
      break;
    }
    appearBuf->appendf("{0:.2f} {1:.2f} Td\n", xpos - xposPrev, -fontsize);
    writeString(&out, appearBuf);
    appearBuf->append("Tj\n");
    xposPrev = xpos;
  }

  font->decRefCnt();
  delete fontcolor;
  appearBuf->append ("ET Q\n");

  double bbox[4];
  bbox[0] = bbox[1] = 0;
  bbox[2] = rect->x2 - rect->x1;
  bbox[3] = rect->y2 - rect->y1;

  if (ca == 1) {
    createForm(bbox, gFalse, &fontResDict, &appearance);
  } else {
    Object aStream, resDict;

    createForm(bbox, gTrue, &fontResDict, &aStream);
    delete appearBuf;

    appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
    createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
    createForm(bbox, gFalse, &resDict, &appearance);
  }
  delete appearBuf;
}

void AnnotFreeText::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    generateFreeTextAppearance();
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                 rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  obj.free();
}

// Before retrieving the res dict, regenerate the appearance stream if needed,
// because AnnotFreeText::draw needs to store font info in the res dict
Object *AnnotFreeText::getAppearanceResDict(Object *dest) {
  if (appearance.isNull()) {
    generateFreeTextAppearance();
  }
  return Annot::getAppearanceResDict(dest);
}

//------------------------------------------------------------------------
// AnnotLine
//------------------------------------------------------------------------

AnnotLine::AnnotLine(PDFDoc *docA, PDFRectangle *rect) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeLine;
  annotObj.dictSet ("Subtype", obj1.initName ("Line"));

  initialize (docA, annotObj.getDict());
}

AnnotLine::AnnotLine(PDFDoc *docA, Dict *dict, Object *obj) :
    AnnotMarkup(docA, dict, obj) {
  type = typeLine;
  initialize(docA, dict);
}

AnnotLine::~AnnotLine() {
  delete coord1;
  delete coord2;

  if (interiorColor)
    delete interiorColor;

  if (measure)
    delete measure;
}

void AnnotLine::initialize(PDFDoc *docA, Dict *dict) {
  Object obj1;

  if (dict->lookup("L", &obj1)->isArray() && obj1.arrayGetLength() == 4) {
    Object obj2;
    double x1, y1, x2, y2;

    (obj1.arrayGet(0, &obj2)->isNum() ? x1 = obj2.getNum() : x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? y1 = obj2.getNum() : y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? x2 = obj2.getNum() : x2 = 0);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? y2 = obj2.getNum() : y2 = 0);
    obj2.free();

    coord1 = new AnnotCoord(x1, y1);
    coord2 = new AnnotCoord(x2, y2);
  } else {
    coord1 = new AnnotCoord();
    coord2 = new AnnotCoord();
  }
  obj1.free();

  if (dict->lookup("LE", &obj1)->isArray() && obj1.arrayGetLength() == 2) {
    Object obj2;

    if(obj1.arrayGet(0, &obj2)->isString())
      startStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      startStyle = annotLineEndingNone;
    obj2.free();

    if(obj1.arrayGet(1, &obj2)->isString())
      endStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      endStyle = annotLineEndingNone;
    obj2.free();

  } else {
    startStyle = endStyle = annotLineEndingNone;
  }
  obj1.free();

  if (dict->lookup("IC", &obj1)->isArray()) {
    interiorColor = new AnnotColor(obj1.getArray());
  } else {
    interiorColor = NULL;
  }
  obj1.free();

  if (dict->lookup("LL", &obj1)->isNum()) {
    leaderLineLength = obj1.getNum();
  } else {
    leaderLineLength = 0;
  }
  obj1.free();

  if (dict->lookup("LLE", &obj1)->isNum()) {
    leaderLineExtension = obj1.getNum();

    if (leaderLineExtension < 0)
      leaderLineExtension = 0;
  } else {
    leaderLineExtension = 0;
  }
  obj1.free();

  if (dict->lookup("Cap", &obj1)->isBool()) {
    caption = obj1.getBool();
  } else {
    caption = gFalse;
  }
  obj1.free();

  if (dict->lookup("IT", &obj1)->isName()) {
    const char *intentName = obj1.getName();

    if(!strcmp(intentName, "LineArrow")) {
      intent = intentLineArrow;
    } else if(!strcmp(intentName, "LineDimension")) {
      intent = intentLineDimension;
    } else {
      intent = intentLineArrow;
    }
  } else {
    intent = intentLineArrow;
  }
  obj1.free();

  if (dict->lookup("LLO", &obj1)->isNum()) {
    leaderLineOffset = obj1.getNum();

    if (leaderLineOffset < 0)
      leaderLineOffset = 0;
  } else {
    leaderLineOffset = 0;
  }
  obj1.free();

  if (dict->lookup("CP", &obj1)->isName()) {
    const char *captionName = obj1.getName();

    if(!strcmp(captionName, "Inline")) {
      captionPos = captionPosInline;
    } else if(!strcmp(captionName, "Top")) {
      captionPos = captionPosTop;
    } else {
      captionPos = captionPosInline;
    }
  } else {
    captionPos = captionPosInline;
  }
  obj1.free();

  if (dict->lookup("Measure", &obj1)->isDict()) {
    measure = NULL;
  } else {
    measure = NULL;
  }
  obj1.free();

  if ((dict->lookup("CO", &obj1)->isArray()) && (obj1.arrayGetLength() == 2)) {
    Object obj2;

    (obj1.arrayGet(0, &obj2)->isNum() ? captionTextHorizontal = obj2.getNum() :
      captionTextHorizontal = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? captionTextVertical = obj2.getNum() :
      captionTextVertical = 0);
    obj2.free();
  } else {
    captionTextHorizontal = captionTextVertical = 0;
  }
  obj1.free();
}

void AnnotLine::setContents(GooString *new_content) {
  Annot::setContents(new_content);
  if (caption)
    invalidateAppearance();
}

void AnnotLine::setVertices(double x1, double y1, double x2, double y2) {
  Object obj1, obj2;

  delete coord1;
  coord1 = new AnnotCoord(x1, y1);
  delete coord2;
  coord2 = new AnnotCoord(x2, y2);

  obj1.initArray(xref);
  obj1.arrayAdd( obj2.initReal(x1) );
  obj1.arrayAdd( obj2.initReal(y1) );
  obj1.arrayAdd( obj2.initReal(x2) );
  obj1.arrayAdd( obj2.initReal(y2) );

  update("L", &obj1);
  invalidateAppearance();
}

void AnnotLine::setStartEndStyle(AnnotLineEndingStyle start, AnnotLineEndingStyle end) {
  Object obj1, obj2;

  startStyle = start;
  endStyle = end;

  obj1.initArray(xref);
  obj1.arrayAdd( obj2.initName(convertAnnotLineEndingStyle( startStyle )) );
  obj1.arrayAdd( obj2.initName(convertAnnotLineEndingStyle( endStyle )) );

  update("LE", &obj1);
  invalidateAppearance();
}

void AnnotLine::setInteriorColor(AnnotColor *new_color) {
  delete interiorColor;

  if (new_color) {
    Object obj1;
    new_color->writeToObject(xref, &obj1);
    update ("IC", &obj1);
    interiorColor = new_color;
  } else {
    interiorColor = NULL;
  }
  invalidateAppearance();
}

void AnnotLine::setLeaderLineLength(double len) {
  Object obj1;

  leaderLineLength = len;
  obj1.initReal(len);
  update ("LL", &obj1);
  invalidateAppearance();
}

void AnnotLine::setLeaderLineExtension(double len) {
  Object obj1;

  leaderLineExtension = len;
  obj1.initReal(len);
  update ("LLE", &obj1);

  // LL is required if LLE is present
  obj1.initReal(leaderLineLength);
  update ("LL", &obj1);
  invalidateAppearance();
}

void AnnotLine::setCaption(bool new_cap) {
  Object obj1;

  caption = new_cap;
  obj1.initBool(new_cap);
  update ("Cap", &obj1);
  invalidateAppearance();
}

void AnnotLine::setIntent(AnnotLineIntent new_intent) {
  Object obj1;

  intent = new_intent;
  if (new_intent == intentLineArrow)
    obj1.initName("LineArrow");
  else // intentLineDimension
    obj1.initName("LineDimension");
  update ("IT", &obj1);
}

void AnnotLine::generateLineAppearance()
{
  double borderWidth, ca = opacity;

  appearBBox = new AnnotAppearanceBBox(rect);
  appearBuf = new GooString ();
  appearBuf->append ("q\n");
  if (color) {
    setColor(color, gFalse);
  }

  if (border) {
    int i, dashLength;
    double *dash;

    switch (border->getStyle()) {
    case AnnotBorder::borderDashed:
      appearBuf->append("[");
      dashLength = border->getDashLength();
      dash = border->getDash();
      for (i = 0; i < dashLength; ++i)
        appearBuf->appendf(" {0:.2f}", dash[i]);
      appearBuf->append(" ] 0 d\n");
      break;
    default:
      appearBuf->append("[] 0 d\n");
      break;
    }
    borderWidth = border->getWidth();
    appearBuf->appendf("{0:.2f} w\n", borderWidth);
    appearBBox->setBorderWidth(borderWidth);
  } else {
    borderWidth = 0;
  }

  const double x1 = coord1->getX();
  const double y1 = coord1->getY();
  const double x2 = coord2->getX();
  const double y2 = coord2->getY();

  // Main segment length
  const double main_len = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

  // Main segment becomes positive x direction, coord1 becomes (0,0)
  Matrix matr;
  const double angle = atan2(y2 - y1, x2 - x1);
  matr.m[0] = matr.m[3] = cos(angle);
  matr.m[1] = sin(angle);
  matr.m[2] = -matr.m[1];
  matr.m[4] = x1-rect->x1;
  matr.m[5] = y1-rect->y1;

  double tx, ty, captionwidth = 0, captionheight = 0;
  AnnotLineCaptionPos actualCaptionPos = captionPos;
  const double fontsize = 9;
  const double captionhmargin = 2; // Left and right margin (inline caption only)
  const double captionmaxwidth = main_len - 2 * captionhmargin;

  Object fontResDict;
  GfxFont *font;

  // Calculate caption width and height
  if (caption) {
    font = createAnnotDrawFont(xref, &fontResDict);
    int lines = 0;
    int i = 0;
    while (i < contents->getLength()) {
      GooString out;
      double linewidth;
      layoutText(contents, &out, &i, font, &linewidth, 0, NULL, gFalse);
      linewidth *= fontsize;
      if (linewidth > captionwidth) {
        captionwidth = linewidth;
      }
      ++lines;
    }
    captionheight = lines * fontsize;
    // If text is longer than available space, turn into captionPosTop
    if (captionwidth > captionmaxwidth) {
      actualCaptionPos = captionPosTop;
    }
  } else {
    fontResDict.initNull();
    font = NULL;
  }

  // Draw main segment
  matr.transform (0, leaderLineLength, &tx, &ty);
  appearBuf->appendf ("{0:.2f} {1:.2f} m\n", tx, ty);
  appearBBox->extendTo (tx, ty);

  if (captionwidth != 0 && actualCaptionPos == captionPosInline) { // Break in the middle
    matr.transform ((main_len-captionwidth)/2 - captionhmargin, leaderLineLength, &tx, &ty);
    appearBuf->appendf ("{0:.2f} {1:.2f} l S\n", tx, ty);

    matr.transform ((main_len+captionwidth)/2 + captionhmargin, leaderLineLength, &tx, &ty);
    appearBuf->appendf ("{0:.2f} {1:.2f} m\n", tx, ty);
  }

  matr.transform (main_len, leaderLineLength, &tx, &ty);
  appearBuf->appendf ("{0:.2f} {1:.2f} l S\n", tx, ty);
  appearBBox->extendTo (tx, ty);

  // TODO: Line endings

  // Draw caption text
  if (caption) {
    double tlx = (main_len - captionwidth) / 2, tly; // Top-left coords
    if (actualCaptionPos == captionPosInline) {
      tly = leaderLineLength + captionheight / 2;
    } else {
      tly = leaderLineLength + captionheight + 2*borderWidth;
    }

    tlx += captionTextHorizontal;
    tly += captionTextVertical;

    // Adjust bounding box
    matr.transform (tlx, tly-captionheight, &tx, &ty);
    appearBBox->extendTo (tx, ty);
    matr.transform (tlx+captionwidth, tly-captionheight, &tx, &ty);
    appearBBox->extendTo (tx, ty);
    matr.transform (tlx+captionwidth, tly, &tx, &ty);
    appearBBox->extendTo (tx, ty);
    matr.transform (tlx, tly, &tx, &ty);
    appearBBox->extendTo (tx, ty);

    // Setup text state (reusing transformed top-left coord)
    appearBuf->appendf ("0 g BT /AnnotDrawFont {0:.2f} Tf\n", fontsize); // Font color: black
    appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} Tm\n",
                        matr.m[0], matr.m[1], matr.m[2], matr.m[3], tx, ty);
    appearBuf->appendf ("0 {0:.2f} Td\n",  -fontsize * font->getDescent());
    // Draw text
    int i = 0;
    double xposPrev = 0;
    while (i < contents->getLength()) {
      GooString out;
      double linewidth, xpos;
      layoutText(contents, &out, &i, font, &linewidth, 0, NULL, gFalse);
      linewidth *= fontsize;
      xpos = (captionwidth - linewidth) / 2;
      appearBuf->appendf("{0:.2f} {1:.2f} Td\n", xpos - xposPrev, -fontsize);
      writeString(&out, appearBuf);
      appearBuf->append ("Tj\n");
      xposPrev = xpos;
    }
    appearBuf->append ("ET\n");
    font->decRefCnt();
  }

  // Draw leader lines
  double ll_len = fabs(leaderLineLength) + leaderLineExtension;
  double sign = leaderLineLength >= 0 ? 1 : -1;
  if (ll_len != 0) {
    matr.transform (0, 0, &tx, &ty);
    appearBuf->appendf ("{0:.2f} {1:.2f} m\n", tx, ty);
    appearBBox->extendTo (tx, ty);
    matr.transform (0, sign*ll_len, &tx, &ty);
    appearBuf->appendf ("{0:.2f} {1:.2f} l S\n", tx, ty);
    appearBBox->extendTo (tx, ty);

    matr.transform (main_len, 0, &tx, &ty);
    appearBuf->appendf ("{0:.2f} {1:.2f} m\n", tx, ty);
    appearBBox->extendTo (tx, ty);
    matr.transform (main_len, sign*ll_len, &tx, &ty);
    appearBuf->appendf ("{0:.2f} {1:.2f} l S\n", tx, ty);
    appearBBox->extendTo (tx, ty);
  }

  appearBuf->append ("Q\n");

  double bbox[4];
  appearBBox->getBBoxRect(bbox);
  if (ca == 1) {
    createForm(bbox, gFalse, &fontResDict, &appearance);
  } else {
    Object aStream, resDict;

    createForm(bbox, gTrue, &fontResDict, &aStream);
    delete appearBuf;

    appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
    createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
    createForm(bbox, gFalse, &resDict, &appearance);
  }
  delete appearBuf;
}

void AnnotLine::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    generateLineAppearance();
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  if (appearBBox) {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   appearBBox->getPageXMin(), appearBBox->getPageYMin(),
                   appearBBox->getPageXMax(), appearBBox->getPageYMax(),
                   getRotation());
  } else {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  }
  obj.free();
}

// Before retrieving the res dict, regenerate the appearance stream if needed,
// because AnnotLine::draw may need to store font info in the res dict
Object *AnnotLine::getAppearanceResDict(Object *dest) {
  if (appearance.isNull()) {
    generateLineAppearance();
  }
  return Annot::getAppearanceResDict(dest);
}

//------------------------------------------------------------------------
// AnnotTextMarkup
//------------------------------------------------------------------------
AnnotTextMarkup::AnnotTextMarkup(PDFDoc *docA, PDFRectangle *rect, AnnotSubtype subType) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  switch (subType) {
    case typeHighlight:
      annotObj.dictSet ("Subtype", obj1.initName ("Highlight"));
      break;
    case typeUnderline:
      annotObj.dictSet ("Subtype", obj1.initName ("Underline"));
      break;
    case typeSquiggly:
      annotObj.dictSet ("Subtype", obj1.initName ("Squiggly"));
      break;
    case typeStrikeOut:
      annotObj.dictSet ("Subtype", obj1.initName ("StrikeOut"));
      break;
    default:
      assert (0 && "Invalid subtype for AnnotTextMarkup\n");
  }

  // Store dummy quadrilateral with null coordinates
  Object obj2, obj3;
  obj2.initArray (doc->getXRef());
  for (int i = 0; i < 4*2; ++i) {
    obj2.arrayAdd (obj3.initReal (0));
  }
  annotObj.dictSet ("QuadPoints", &obj2);

  initialize(docA, annotObj.getDict());
}

AnnotTextMarkup::AnnotTextMarkup(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  // the real type will be read in initialize()
  type = typeHighlight;
  initialize(docA, dict);
}

void AnnotTextMarkup::initialize(PDFDoc *docA, Dict *dict) {
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("Highlight")) {
      type = typeHighlight;
    } else if (!typeName.cmp("Underline")) {
      type = typeUnderline;
    } else if (!typeName.cmp("Squiggly")) {
      type = typeSquiggly;
    } else if (!typeName.cmp("StrikeOut")) {
      type = typeStrikeOut;
    }
  }
  obj1.free();

  if(dict->lookup("QuadPoints", &obj1)->isArray()) {
    quadrilaterals = new AnnotQuadrilaterals(obj1.getArray(), rect);
  } else {
    error(errSyntaxError, -1, "Bad Annot Text Markup QuadPoints");
    quadrilaterals = NULL;
    ok = gFalse;
  }
  obj1.free();
}

AnnotTextMarkup::~AnnotTextMarkup() {
  if(quadrilaterals) {
    delete quadrilaterals;
  }
}

void AnnotTextMarkup::setType(AnnotSubtype new_type) {
  Object obj1;

  switch (new_type) {
    case typeHighlight:
      obj1.initName("Highlight");
      break;
    case typeUnderline:
      obj1.initName("Underline");
      break;
    case typeSquiggly:
      obj1.initName("Squiggly");
      break;
    case typeStrikeOut:
      obj1.initName("StrikeOut");
      break;
    default:
      assert(!"Invalid subtype");
  }

  type = new_type;
  update("Subtype", &obj1);
  invalidateAppearance();
}

void AnnotTextMarkup::setQuadrilaterals(AnnotQuadrilaterals *quadPoints) {
  Object obj1, obj2;
  obj1.initArray (xref);

  for (int i = 0; i < quadPoints->getQuadrilateralsLength(); ++i) {
    obj1.arrayAdd (obj2.initReal (quadPoints->getX1(i)));
    obj1.arrayAdd (obj2.initReal (quadPoints->getY1(i)));
    obj1.arrayAdd (obj2.initReal (quadPoints->getX2(i)));
    obj1.arrayAdd (obj2.initReal (quadPoints->getY2(i)));
    obj1.arrayAdd (obj2.initReal (quadPoints->getX3(i)));
    obj1.arrayAdd (obj2.initReal (quadPoints->getY3(i)));
    obj1.arrayAdd (obj2.initReal (quadPoints->getX4(i)));
    obj1.arrayAdd (obj2.initReal (quadPoints->getY4(i)));
  }

  quadrilaterals = new AnnotQuadrilaterals(obj1.getArray(), rect);

  annotObj.dictSet ("QuadPoints", &obj1);
  invalidateAppearance();
}

void AnnotTextMarkup::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;
  int i;
  Object obj1, obj2;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull() || type == typeHighlight) {
    GBool blendMultiply = gTrue;
    ca = opacity;

    appearBuf = new GooString ();
    appearBuf->append ("q\n");

    /* Adjust BBox */
    delete appearBBox;
    appearBBox = new AnnotAppearanceBBox(rect);
    for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
      appearBBox->extendTo (quadrilaterals->getX1(i) - rect->x1, quadrilaterals->getY1(i) - rect->y1);
      appearBBox->extendTo (quadrilaterals->getX2(i) - rect->x1, quadrilaterals->getY2(i) - rect->y1);
      appearBBox->extendTo (quadrilaterals->getX3(i) - rect->x1, quadrilaterals->getY3(i) - rect->y1);
      appearBBox->extendTo (quadrilaterals->getX4(i) - rect->x1, quadrilaterals->getY4(i) - rect->y1);
    }

    switch (type) {
    case typeUnderline:
      if (color) {
        setColor(color, gFalse);
      }
      appearBuf->append ("[] 0 d 1 w\n");

      for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
	double x3, y3, x4, y4;

	x3 = quadrilaterals->getX3(i);
	y3 = quadrilaterals->getY3(i);
	x4 = quadrilaterals->getX4(i);
	y4 = quadrilaterals->getY4(i);

	appearBuf->appendf ("{0:.2f} {1:.2f} m\n", x3, y3);
	appearBuf->appendf ("{0:.2f} {1:.2f} l\n", x4, y4);
	appearBuf->append ("S\n");
      }
      break;
    case typeStrikeOut:
      if (color) {
        setColor(color, gFalse);
      }
      blendMultiply = gFalse;
      appearBuf->append ("[] 0 d 1 w\n");

      for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
	double x1, y1, x2, y2;
	double x3, y3, x4, y4;

	x1 = quadrilaterals->getX1(i);
	y1 = quadrilaterals->getY1(i);
	x2 = quadrilaterals->getX2(i);
	y2 = quadrilaterals->getY2(i);

	x3 = quadrilaterals->getX3(i);
	y3 = quadrilaterals->getY3(i);
	x4 = quadrilaterals->getX4(i);
	y4 = quadrilaterals->getY4(i);

	appearBuf->appendf ("{0:.2f} {1:.2f} m\n", (x1+x3)/2., (y1+y3)/2.);
	appearBuf->appendf ("{0:.2f} {1:.2f} l\n", (x2+x4)/2., (y2+y4)/2.);
	appearBuf->append ("S\n");
      }
      break;
    case typeSquiggly:
      if (color) {
        setColor(color, gFalse);
      }
      appearBuf->append ("[] 0 d 1 w\n");

      for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
        double x1, y1, x2, y3;
        double h6;

        x1 = quadrilaterals->getX1(i);
        y1 = quadrilaterals->getY1(i);
        x2 = quadrilaterals->getX2(i);
        y3 = quadrilaterals->getY3(i);
        h6 = (y1 - y3) / 6.0;

        appearBuf->appendf ("{0:.2f} {1:.2f} m\n", x1, y3+h6);
        bool down = false;
        do {
          down = !down; // Zigzag line
          x1 += 2;
          appearBuf->appendf ("{0:.2f} {1:.2f} l\n", x1, y3 + (down ? 0 : h6));
        } while (x1 < x2);
        appearBuf->append ("S\n");
      }
      break;
    default:
    case typeHighlight:
      appearance.free();

      if (color)
        setColor(color, gTrue);

      double biggestBorder = 0;
      for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
        double x1, y1, x2, y2, x3, y3, x4, y4;
	double h4;

	x1 = quadrilaterals->getX1(i);
	y1 = quadrilaterals->getY1(i);
	x2 = quadrilaterals->getX2(i);
	y2 = quadrilaterals->getY2(i);
	x3 = quadrilaterals->getX3(i);
	y3 = quadrilaterals->getY3(i);
	x4 = quadrilaterals->getX4(i);
	y4 = quadrilaterals->getY4(i);
	h4 = fabs(y1 - y3) / 4.0;

	if (h4 > biggestBorder) {
	  biggestBorder = h4;
	}

	appearBuf->appendf ("{0:.2f} {1:.2f} m\n", x3, y3);
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x3 - h4, y3 + h4, x1 - h4, y1 - h4, x1, y1);
	appearBuf->appendf ("{0:.2f} {1:.2f} l\n", x2, y2);
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x2 + h4, y2 - h4, x4 + h4, y4 + h4, x4, y4);
	appearBuf->append ("f\n");
      }
      appearBBox->setBorderWidth(biggestBorder);
      break;
    }
    appearBuf->append ("Q\n");

    Object aStream, resDict;
    double bbox[4];
    bbox[0] = appearBBox->getPageXMin();
    bbox[1] = appearBBox->getPageYMin();
    bbox[2] = appearBBox->getPageXMax();
    bbox[3] = appearBBox->getPageYMax();
    createForm(bbox, gTrue, NULL, &aStream);
    delete appearBuf;

    appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
    createResourcesDict("Fm0", &aStream, "GS0", 1, blendMultiply ? "Multiply" : NULL, &resDict);
    if (ca == 1) {
      createForm(bbox, gFalse, &resDict, &appearance);
    } else {
      createForm(bbox, gTrue, &resDict, &aStream);
      delete appearBuf;

      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
      createForm(bbox, gFalse, &resDict, &appearance);
    }
    delete appearBuf;
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  if (appearBBox) {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   appearBBox->getPageXMin(), appearBBox->getPageYMin(),
                   appearBBox->getPageXMax(), appearBBox->getPageYMax(),
                   getRotation());
  } else {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  }
  obj.free();
}

//------------------------------------------------------------------------
// AnnotWidget
//------------------------------------------------------------------------

AnnotWidget::AnnotWidget(PDFDoc *docA, Dict *dict, Object *obj) :
    Annot(docA, dict, obj) {
  type = typeWidget;
  field = NULL;
  initialize(docA, dict);
}

AnnotWidget::AnnotWidget(PDFDoc *docA, Dict *dict, Object *obj, FormField *fieldA) :
    Annot(docA, dict, obj) {
  type = typeWidget;
  field = fieldA;
  initialize(docA, dict);
}

AnnotWidget::~AnnotWidget() {
  if (appearCharacs)
    delete appearCharacs;
  
  if (action)
    delete action;

  additionalActions.free();

  if (parent)
    delete parent;
}

void AnnotWidget::initialize(PDFDoc *docA, Dict *dict) {
  Object obj1;

  form = doc->getCatalog()->getForm();

  if(dict->lookup("H", &obj1)->isName()) {
    const char *modeName = obj1.getName();

    if(!strcmp(modeName, "N")) {
      mode = highlightModeNone;
    } else if(!strcmp(modeName, "O")) {
      mode = highlightModeOutline;
    } else if(!strcmp(modeName, "P") || !strcmp(modeName, "T")) {
      mode = highlightModePush;
    } else {
      mode = highlightModeInvert;
    }
  } else {
    mode = highlightModeInvert;
  }
  obj1.free();

  if(dict->lookup("MK", &obj1)->isDict()) {
    appearCharacs = new AnnotAppearanceCharacs(obj1.getDict());
  } else {
    appearCharacs = NULL;
  }
  obj1.free();

  action = NULL;
  if(dict->lookup("A", &obj1)->isDict()) {
    action = LinkAction::parseAction(&obj1, doc->getCatalog()->getBaseURI());
  }
  obj1.free();

  dict->lookupNF("AA", &additionalActions);

  if(dict->lookup("Parent", &obj1)->isDict()) {
    parent = NULL;
  } else {
    parent = NULL;
  }
  obj1.free();

  updatedAppearanceStream.num = updatedAppearanceStream.gen = -1;
}

LinkAction* AnnotWidget::getAdditionalAction(AdditionalActionsType type)
{
  return ::getAdditionalAction(type, &additionalActions, doc);
}

LinkAction* AnnotWidget::getFormAdditionalAction(FormAdditionalActionsType type)
{
  return ::getFormAdditionalAction(type, &additionalActions, doc);
}

// Grand unified handler for preparing text strings to be drawn into form
// fields.  Takes as input a text string (in PDFDocEncoding or UTF-16).
// Converts some or all of this string to the appropriate encoding for the
// specified font, and computes the width of the text.  Can optionally stop
// converting when a specified width has been reached, to perform line-breaking
// for multi-line fields.
//
// Parameters:
//   text: input text string to convert
//   outBuf: buffer for writing re-encoded string
//   i: index at which to start converting; will be updated to point just after
//      last character processed
//   font: the font which will be used for display
//   width: computed width (unscaled by font size) will be stored here
//   widthLimit: if non-zero, stop converting to keep width under this value
//      (should be scaled down by font size)
//   charCount: count of number of characters will be stored here
//   noReencode: if set, do not try to translate the character encoding
//      (useful for Zapf Dingbats or other unusual encodings)
//      can only be used with simple fonts, not CID-keyed fonts
//
// TODO: Handle surrogate pairs in UTF-16.
//       Should be able to generate output for any CID-keyed font.
//       Doesn't handle vertical fonts--should it?
void Annot::layoutText(GooString *text, GooString *outBuf, int *i,
                             GfxFont *font, double *width, double widthLimit,
                             int *charCount, GBool noReencode)
{
  CharCode c;
  Unicode uChar, *uAux;
  double w = 0.0;
  int uLen, n;
  double dx, dy, ox, oy;
  GBool unicode = text->hasUnicodeMarker();
  GBool spacePrev;              // previous character was a space

  // State for backtracking when more text has been processed than fits within
  // widthLimit.  We track for both input (text) and output (outBuf) the offset
  // to the first character to discard.
  //
  // We keep track of several points:
  //   1 - end of previous completed word which fits
  //   2 - previous character which fit
  int last_i1, last_i2, last_o1, last_o2;

  if (unicode && text->getLength() % 2 != 0) {
    error(errSyntaxError, -1, "AnnotWidget::layoutText, bad unicode string");
    return;
  }

  // skip Unicode marker on string if needed
  if (unicode && *i == 0)
    *i = 2;

  // Start decoding and copying characters, until either:
  //   we reach the end of the string
  //   we reach the maximum width
  //   we reach a newline character
  // As we copy characters, keep track of the last full word to fit, so that we
  // can backtrack if we exceed the maximum width.
  last_i1 = last_i2 = *i;
  last_o1 = last_o2 = 0;
  spacePrev = gFalse;
  outBuf->clear();

  while (*i < text->getLength()) {
    last_i2 = *i;
    last_o2 = outBuf->getLength();

    if (unicode) {
      uChar = (unsigned char)(text->getChar(*i)) << 8;
      uChar += (unsigned char)(text->getChar(*i + 1));
      *i += 2;
    } else {
      if (noReencode)
        uChar = text->getChar(*i) & 0xff;
      else
        uChar = pdfDocEncoding[text->getChar(*i) & 0xff];
      *i += 1;
    }

    // Explicit line break?
    if (uChar == '\r' || uChar == '\n') {
      // Treat a <CR><LF> sequence as a single line break
      if (uChar == '\r' && *i < text->getLength()) {
        if (unicode && text->getChar(*i) == '\0'
            && text->getChar(*i + 1) == '\n')
          *i += 2;
        else if (!unicode && text->getChar(*i) == '\n')
          *i += 1;
      }

      break;
    }

    if (noReencode) {
      outBuf->append(uChar);
    } else {
      CharCodeToUnicode *ccToUnicode = font->getToUnicode();
      if (!ccToUnicode) {
        // This assumes an identity CMap.
        outBuf->append((uChar >> 8) & 0xff);
        outBuf->append(uChar & 0xff);
      } else if (ccToUnicode->mapToCharCode(&uChar, &c, 1)) {
        ccToUnicode->decRefCnt();
        if (font->isCIDFont()) {
          // TODO: This assumes an identity CMap.  It should be extended to
          // handle the general case.
          outBuf->append((c >> 8) & 0xff);
          outBuf->append(c & 0xff);
        } else {
          // 8-bit font
          outBuf->append(c);
        }
      } else {
        ccToUnicode->decRefCnt();
        error(errSyntaxError, -1, "AnnotWidget::layoutText, cannot convert U+{0:04uX}", uChar);
      }
    }

    // If we see a space, then we have a linebreak opportunity.
    if (uChar == ' ') {
      last_i1 = *i;
      if (!spacePrev)
        last_o1 = last_o2;
      spacePrev = gTrue;
    } else {
      spacePrev = gFalse;
    }

    // Compute width of character just output
    if (outBuf->getLength() > last_o2) {
      dx = 0.0;
      font->getNextChar(outBuf->getCString() + last_o2,
                        outBuf->getLength() - last_o2,
                        &c, &uAux, &uLen, &dx, &dy, &ox, &oy);
      w += dx;
    }

    // Current line over-full now?
    if (widthLimit > 0.0 && w > widthLimit) {
      if (last_o1 > 0) {
        // Back up to the previous word which fit, if there was a previous
        // word.
        *i = last_i1;
        outBuf->del(last_o1, outBuf->getLength() - last_o1);
      } else if (last_o2 > 0) {
        // Otherwise, back up to the previous character (in the only word on
        // this line)
        *i = last_i2;
        outBuf->del(last_o2, outBuf->getLength() - last_o2);
      } else {
        // Otherwise, we were trying to fit the first character; include it
        // anyway even if it overflows the space--no updates to make.
      }
      break;
    }
  }

  // If splitting input into lines because we reached the width limit, then
  // consume any remaining trailing spaces that would go on this line from the
  // input.  If in doing so we reach a newline, consume that also.  This code
  // won't run if we stopped at a newline above, since in that case w <=
  // widthLimit still.
  if (widthLimit > 0.0 && w > widthLimit) {
    if (unicode) {
      while (*i < text->getLength()
             && text->getChar(*i) == '\0' && text->getChar(*i + 1) == ' ')
        *i += 2;
      if (*i < text->getLength()
          && text->getChar(*i) == '\0' && text->getChar(*i + 1) == '\r')
        *i += 2;
      if (*i < text->getLength()
          && text->getChar(*i) == '\0' && text->getChar(*i + 1) == '\n')
        *i += 2;
    } else {
      while (*i < text->getLength() && text->getChar(*i) == ' ')
        *i += 1;
      if (*i < text->getLength() && text->getChar(*i) == '\r')
        *i += 1;
      if (*i < text->getLength() && text->getChar(*i) == '\n')
        *i += 1;
    }
  }

  // Compute the actual width and character count of the final string, based on
  // breakpoint, if this information is requested by the caller.
  if (width != NULL || charCount != NULL) {
    char *s = outBuf->getCString();
    int len = outBuf->getLength();

    if (width != NULL)
      *width = 0.0;
    if (charCount != NULL)
      *charCount = 0;

    while (len > 0) {
      dx = 0.0;
      n = font->getNextChar(s, len, &c, &uAux, &uLen, &dx, &dy, &ox, &oy);

      if (n == 0) {
        break;
      }

      if (width != NULL)
        *width += dx;
      if (charCount != NULL)
        *charCount += 1;

      s += n;
      len -= n;
    }
  }
}

// Copy the given string to appearBuf, adding parentheses around it and
// escaping characters as appropriate.
void Annot::writeString(GooString *str, GooString *appearBuf)
{
  char c;
  int i;

  appearBuf->append('(');

  for (i = 0; i < str->getLength(); ++i) {
    c = str->getChar(i);
    if (c == '(' || c == ')' || c == '\\') {
      appearBuf->append('\\');
      appearBuf->append(c);
    } else if (c < 0x20) {
      appearBuf->appendf("\\{0:03o}", (unsigned char)c);
    } else {
      appearBuf->append(c);
    }
  }

  appearBuf->append(')');
}

// Draw the variable text or caption for a field.
void AnnotWidget::drawText(GooString *text, GooString *da, GfxResources *resources,
    GBool multiline, int comb, int quadding,
    GBool txField, GBool forceZapfDingbats,
    GBool password) {
  GooList *daToks;
  GooString *tok, *convertedText;
  GfxFont *font;
  double dx, dy;
  double fontSize, fontSize2, borderWidth, x, xPrev, y, w, wMax;
  int tfPos, tmPos, i, j;
  int rot;
  GBool freeText = gFalse;      // true if text should be freed before return
  GBool freeFont = gFalse;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)
  
  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GooList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
        ++i;
      }
      if (i < da->getLength()) {
        for (j = i + 1;
            j < da->getLength() && !Lexer::isSpace(da->getChar(j));
            ++j) ;
        daToks->append(new GooString(da, i, j - i));
        i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GooString *)daToks->get(i))->cmp("Tf")) {
        tfPos = i - 2;
      } else if (i >= 6 && !((GooString *)daToks->get(i))->cmp("Tm")) {
        tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // force ZapfDingbats
  if (forceZapfDingbats) {
    if (tfPos >= 0) {
      tok = (GooString *)daToks->get(tfPos);
      if (tok->cmp("/ZaDb")) {
        tok->clear();
        tok->append("/ZaDb");
      }
    }
  }
  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GooString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!resources || !(font = resources->lookupFont(tok->getCString() + 1))) {
        if (forceZapfDingbats) {
          // We are forcing ZaDb but the font does not exist
          // so create a fake one
          Ref r; // dummy Ref, it's not used at all in this codepath
          r.num = -1;
          r.gen = -1;
          Dict *d = new Dict(xref);
          font = new Gfx8BitFont(xref, "ZaDb", r, new GooString("ZapfDingbats"), fontType1, r, d);
          delete d;
          freeFont = gTrue;
          addDingbatsResource = gTrue;
        } else {
          error(errSyntaxError, -1, "Unknown font in field's DA string");
        }
      }
    } else {
      error(errSyntaxError, -1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = gatof(tok->getCString());
  } else {
    error(errSyntaxError, -1, "Missing 'Tf' operator in field's DA string");
  }
  if (!font) {
    if (daToks) {
      deleteGooList(daToks, GooString);
    }
    return;
  }

  // get the border width
  borderWidth = border ? border->getWidth() : 0;

  // for a password field, replace all characters with asterisks
  if (password) {
    int len;
    if (text->hasUnicodeMarker())
      len = (text->getLength() - 2) / 2;
    else
      len = text->getLength();

    text = new GooString;
    for (i = 0; i < len; ++i)
      text->append('*');
    freeText = gTrue;
  }

  convertedText = new GooString;

  // setup
  if (txField) {
    appearBuf->append("/Tx BMC\n");
  }
  appearBuf->append("q\n");
  rot = appearCharacs ? appearCharacs->getRotation() : 0;
  switch (rot) {
  case 90:
    appearBuf->appendf("0 1 -1 0 {0:.2f} 0 cm\n", rect->x2 - rect->x1);
    dx = rect->y2 - rect->y1;
    dy = rect->x2 - rect->x1;
    break;
  case 180:
    appearBuf->appendf("-1 0 0 -1 {0:.2f} {1:.2f} cm\n",
		       rect->x2 - rect->x1, rect->y2 - rect->y1);
    dx = rect->x2 - rect->y2;
    dy = rect->y2 - rect->y1;
    break;
  case 270:
    appearBuf->appendf("0 -1 1 0 0 {0:.2f} cm\n", rect->y2 - rect->y1);
    dx = rect->y2 - rect->y1;
    dy = rect->x2 - rect->x1;
    break;
  default: // assume rot == 0
    dx = rect->x2 - rect->x1;
    dy = rect->y2 - rect->y1;
    break;
  }
  appearBuf->append("BT\n");
  // multi-line text
  if (multiline) {
    // note: the comb flag is ignored in multiline mode

    wMax = dx - 2 * borderWidth - 4;

    // compute font autosize
    if (fontSize == 0) {
      for (fontSize = 20; fontSize > 1; --fontSize) {
        y = dy - 3;
        i = 0;
        while (i < text->getLength()) {
          layoutText(text, convertedText, &i, font, &w, wMax / fontSize, NULL,
                     forceZapfDingbats);
          y -= fontSize;
        }
        // approximate the descender for the last line
        if (y >= 0.33 * fontSize) {
          break;
        }
      }
      if (tfPos >= 0) {
        tok = (GooString *)daToks->get(tfPos + 1);
        tok->clear();
        tok->appendf("{0:.2f}", fontSize);
      }
    }

    // starting y coordinate
    // (note: each line of text starts with a Td operator that moves
    // down a line)
    y = dy - 3;

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GooString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->append('0');
      tok = (GooString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (i = 0; i < daToks->getLength(); ++i) {
        appearBuf->append((GooString *)daToks->get(i))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 0 {0:.2f} Tm\n", y);
    }

    // write a series of lines of text
    i = 0;
    xPrev = 0;
    while (i < text->getLength()) {
      layoutText(text, convertedText, &i, font, &w, wMax / fontSize, NULL,
                 forceZapfDingbats);
      w *= fontSize;

      // compute text start position
      switch (quadding) {
      case quaddingLeftJustified:
      default:
        x = borderWidth + 2;
        break;
      case quaddingCentered:
        x = (dx - w) / 2;
        break;
      case quaddingRightJustified:
        x = dx - borderWidth - 2 - w;
        break;
      }

      // draw the line
      appearBuf->appendf("{0:.2f} {1:.2f} Td\n", x - xPrev, -fontSize);
      writeString(convertedText, appearBuf);
      appearBuf->append(" Tj\n");

      // next line
      xPrev = x;
    }

    // single-line text
  } else {
    //~ replace newlines with spaces? - what does Acrobat do?

    // comb formatting
    if (comb > 0) {
      int charCount;

      // compute comb spacing
      w = (dx - 2 * borderWidth) / comb;

      // compute font autosize
      if (fontSize == 0) {
        fontSize = dy - 2 * borderWidth;
        if (w < fontSize) {
          fontSize = w;
        }
        fontSize = floor(fontSize);
        if (tfPos >= 0) {
          tok = (GooString *)daToks->get(tfPos + 1);
          tok->clear();
          tok->appendf("{0:.2f}", fontSize);
        }
      }

      i = 0;
      layoutText(text, convertedText, &i, font, NULL, 0.0, &charCount,
                 forceZapfDingbats);
      if (charCount > comb)
        charCount = comb;

      // compute starting text cell
      switch (quadding) {
      case quaddingLeftJustified:
      default:
          x = borderWidth;
        break;
      case quaddingCentered:
        x = borderWidth + (comb - charCount) / 2 * w;
        break;
      case quaddingRightJustified:
        x = borderWidth + (comb - charCount) * w;
        break;
      }
      y = 0.5 * dy - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
        tok = (GooString *)daToks->get(tmPos + 4);
        tok->clear();
        tok->appendf("{0:.2f}", x);
        tok = (GooString *)daToks->get(tmPos + 5);
        tok->clear();
        tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
        for (i = 0; i < daToks->getLength(); ++i) {
          appearBuf->append((GooString *)daToks->get(i))->append(' ');
        }
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
        appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }

      // write the text string
      char *s = convertedText->getCString();
      int len = convertedText->getLength();
      i = 0;
      xPrev = w;                // so that first character is placed properly
      while (i < comb && len > 0) {
        CharCode code;
        Unicode *uAux;
        int uLen, n;
        double dx, dy, ox, oy;

        dx = 0.0;
        n = font->getNextChar(s, len, &code, &uAux, &uLen, &dx, &dy, &ox, &oy);
        dx *= fontSize;

        // center each character within its cell, by advancing the text
        // position the appropriate amount relative to the start of the
        // previous character
        x = 0.5 * (w - dx);
        appearBuf->appendf("{0:.2f} 0 Td\n", x - xPrev + w);

        GooString *charBuf = new GooString(s, n);
        writeString(charBuf, appearBuf);
        appearBuf->append(" Tj\n");
        delete charBuf;

        i++;
        s += n;
        len -= n;
        xPrev = x;
      }

      // regular (non-comb) formatting
    } else {
      i = 0;
      layoutText(text, convertedText, &i, font, &w, 0.0, NULL,
                 forceZapfDingbats);

      // compute font autosize
      if (fontSize == 0) {
        fontSize = dy - 2 * borderWidth;
        fontSize2 = (dx - 4 - 2 * borderWidth) / w;
        if (fontSize2 < fontSize) {
          fontSize = fontSize2;
        }
        fontSize = floor(fontSize);
        if (tfPos >= 0) {
          tok = (GooString *)daToks->get(tfPos + 1);
          tok->clear();
          tok->appendf("{0:.2f}", fontSize);
        }
      }

      // compute text start position
      w *= fontSize;
      switch (quadding) {
      case quaddingLeftJustified:
      default:
        x = borderWidth + 2;
        break;
      case quaddingCentered:
        x = (dx - w) / 2;
        break;
      case quaddingRightJustified:
        x = dx - borderWidth - 2 - w;
        break;
      }
      y = 0.5 * dy - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
        tok = (GooString *)daToks->get(tmPos + 4);
        tok->clear();
        tok->appendf("{0:.2f}", x);
        tok = (GooString *)daToks->get(tmPos + 5);
        tok->clear();
        tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
        for (i = 0; i < daToks->getLength(); ++i) {
          appearBuf->append((GooString *)daToks->get(i))->append(' ');
        }
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
        appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }

      // write the text string
      writeString(convertedText, appearBuf);
      appearBuf->append(" Tj\n");
    }
  }
  // cleanup
  appearBuf->append("ET\n");
  appearBuf->append("Q\n");
  if (txField) {
    appearBuf->append("EMC\n");
  }
  if (daToks) {
    deleteGooList(daToks, GooString);
  }
  if (freeText) {
    delete text;
  }
  delete convertedText;
  if (freeFont) {
    font->decRefCnt();
  }
}

// Draw the variable text or caption for a field.
void AnnotWidget::drawListBox(FormFieldChoice *fieldChoice,
			      GooString *da, GfxResources *resources, int quadding) {
  GooList *daToks;
  GooString *tok, *convertedText;
  GfxFont *font;
  double fontSize, fontSize2, borderWidth, x, y, w, wMax;
  int tfPos, tmPos, i, j;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)

  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GooList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
	++i;
       }
      if (i < da->getLength()) {
	for (j = i + 1;
	     j < da->getLength() && !Lexer::isSpace(da->getChar(j));
	     ++j) ;
	daToks->append(new GooString(da, i, j - i));
	i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GooString *)daToks->get(i))->cmp("Tf")) {
	tfPos = i - 2;
      } else if (i >= 6 && !((GooString *)daToks->get(i))->cmp("Tm")) {
	tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GooString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!resources || !(font = resources->lookupFont(tok->getCString() + 1))) {
        error(errSyntaxError, -1, "Unknown font in field's DA string");
      }
    } else {
      error(errSyntaxError, -1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = gatof(tok->getCString());
  } else {
    error(errSyntaxError, -1, "Missing 'Tf' operator in field's DA string");
  }
  if (!font) {
    if (daToks) {
      deleteGooList(daToks, GooString);
    }
    return;
  }

  convertedText = new GooString;

  // get the border width
  borderWidth = border ? border->getWidth() : 0;

  // compute font autosize
  if (fontSize == 0) {
    wMax = 0;
    for (i = 0; i < fieldChoice->getNumChoices(); ++i) {
      j = 0;
      if (fieldChoice->getChoice(i) == NULL) {
        error(errSyntaxError, -1, "Invalid annotation listbox");
        if (daToks) {
          deleteGooList(daToks, GooString);
        }
        delete convertedText;
        return;
      }
      layoutText(fieldChoice->getChoice(i), convertedText, &j, font, &w, 0.0, NULL, gFalse);
      if (w > wMax) {
        wMax = w;
      }
    }
    fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
    fontSize2 = (rect->x2 - rect->x1 - 4 - 2 * borderWidth) / wMax;
    if (fontSize2 < fontSize) {
      fontSize = fontSize2;
    }
    fontSize = floor(fontSize);
    if (tfPos >= 0) {
      tok = (GooString *)daToks->get(tfPos + 1);
      tok->clear();
      tok->appendf("{0:.2f}", fontSize);
    }
  }
  // draw the text
  y = rect->y2 - rect->y1 - 1.1 * fontSize;
  for (i = fieldChoice->getTopIndex(); i < fieldChoice->getNumChoices(); ++i) {
    // setup
    appearBuf->append("q\n");

    // draw the background if selected
    if (fieldChoice->isSelected(i)) {
      appearBuf->append("0 g f\n");
      appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} re f\n",
          borderWidth,
          y - 0.2 * fontSize,
          rect->x2 - rect->x1 - 2 * borderWidth,
          1.1 * fontSize);
    }

    // setup
    appearBuf->append("BT\n");

    // compute text width and start position
    j = 0;
    layoutText(fieldChoice->getChoice(i), convertedText, &j, font, &w, 0.0, NULL, gFalse);
    w *= fontSize;
    switch (quadding) {
    case quaddingLeftJustified:
    default:
      x = borderWidth + 2;
      break;
    case quaddingCentered:
      x = (rect->x2 - rect->x1 - w) / 2;
      break;
    case quaddingRightJustified:
      x = rect->x2 - rect->x1 - borderWidth - 2 - w;
      break;
    }

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GooString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->appendf("{0:.2f}", x);
      tok = (GooString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (j = 0; j < daToks->getLength(); ++j) {
        appearBuf->append((GooString *)daToks->get(j))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
    }

    // change the text color if selected
    if (fieldChoice->isSelected(i)) {
      appearBuf->append("1 g\n");
    }

    // write the text string
    writeString(convertedText, appearBuf);
    appearBuf->append(" Tj\n");

    // cleanup
    appearBuf->append("ET\n");
    appearBuf->append("Q\n");

    // next line
    y -= 1.1 * fontSize;
  }

  if (daToks) {
    deleteGooList(daToks, GooString);
  }

  delete convertedText;
}

void AnnotWidget::drawBorder() {
  int dashLength;
  double *dash;
  AnnotColor adjustedColor;
  double w = border->getWidth();

  AnnotColor *aColor = appearCharacs->getBorderColor();
  if (!aColor)
    aColor = appearCharacs->getBackColor();
  if (!aColor)
    return;

  double dx = rect->x2 - rect->x1;
  double dy = rect->y2 - rect->y1;

  // radio buttons with no caption have a round border
  GBool hasCaption = appearCharacs->getNormalCaption() != NULL;
  if (field->getType() == formButton &&
      static_cast<FormFieldButton*>(field)->getButtonType() == formButtonRadio && !hasCaption) {
    double r = 0.5 * (dx < dy ? dx : dy);
    switch (border->getStyle()) {
    case AnnotBorder::borderDashed:
      appearBuf->append("[");
      dashLength = border->getDashLength();
      dash = border->getDash();
      for (int i = 0; i < dashLength; ++i) {
        appearBuf->appendf(" {0:.2f}", dash[i]);
      }
      appearBuf->append("] 0 d\n");
      // fall through to the solid case
    case AnnotBorder::borderSolid:
    case AnnotBorder::borderUnderlined:
      appearBuf->appendf("{0:.2f} w\n", w);
      setColor(aColor, gFalse);
      drawCircle(0.5 * dx, 0.5 * dy, r - 0.5 * w, gFalse);
      break;
    case AnnotBorder::borderBeveled:
    case AnnotBorder::borderInset:
      appearBuf->appendf("{0:.2f} w\n", 0.5 * w);
      setColor(aColor, gFalse);
      drawCircle(0.5 * dx, 0.5 * dy, r - 0.25 * w, gFalse);
      adjustedColor = AnnotColor(*aColor);
      adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
      setColor(&adjustedColor, gFalse);
      drawCircleTopLeft(0.5 * dx, 0.5 * dy, r - 0.75 * w);
      adjustedColor = AnnotColor(*aColor);
      adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
      setColor(&adjustedColor, gFalse);
      drawCircleBottomRight(0.5 * dx, 0.5 * dy, r - 0.75 * w);
      break;
    }
  } else {
    switch (border->getStyle()) {
    case AnnotBorder::borderDashed:
      appearBuf->append("[");
      dashLength = border->getDashLength();
      dash = border->getDash();
      for (int i = 0; i < dashLength; ++i) {
        appearBuf->appendf(" {0:.2f}", dash[i]);
      }
      appearBuf->append("] 0 d\n");
      // fall through to the solid case
    case AnnotBorder::borderSolid:
      appearBuf->appendf("{0:.2f} w\n", w);
      setColor(aColor, gFalse);
      appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re s\n",
                         0.5 * w, dx - w, dy - w);
      break;
    case AnnotBorder::borderBeveled:
    case AnnotBorder::borderInset:
      adjustedColor = AnnotColor(*aColor);
      adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
      setColor(&adjustedColor, gTrue);
      appearBuf->append("0 0 m\n");
      appearBuf->appendf("0 {0:.2f} l\n", dy);
      appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
      appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
      appearBuf->appendf("{0:.2f} {1:.2f} l\n", w, dy - w);
      appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
      appearBuf->append("f\n");
      adjustedColor = AnnotColor(*aColor);
      adjustedColor.adjustColor(border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
      setColor(&adjustedColor, gTrue);
      appearBuf->append("0 0 m\n");
      appearBuf->appendf("{0:.2f} 0 l\n", dx);
      appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
      appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
      appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, w);
      appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
      appearBuf->append("f\n");
      break;
    case AnnotBorder::borderUnderlined:
      appearBuf->appendf("{0:.2f} w\n", w);
      setColor(aColor, gFalse);
      appearBuf->appendf("0 0 m {0:.2f} 0 l s\n", dx);
      break;
    }

    // clip to the inside of the border
    appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n",
                       w, dx - 2 * w, dy - 2 * w);
  }
}

void AnnotWidget::drawFormFieldButton(GfxResources *resources, GooString *da) {
  GooString *caption = NULL;
  if (appearCharacs)
    caption = appearCharacs->getNormalCaption();

  switch (static_cast<FormFieldButton *>(field)->getButtonType()) {
  case formButtonRadio: {
    //~ Acrobat doesn't draw a caption if there is no AP dict (?)
    if (appearState && appearState->cmp("Off") != 0) {
      if (caption) {
        drawText(caption, da, resources, gFalse, 0, fieldQuadCenter,
                 gFalse, gTrue);
      } else if (appearCharacs) {
        AnnotColor *aColor = appearCharacs->getBorderColor();
        if (aColor) {
          double dx = rect->x2 - rect->x1;
          double dy = rect->y2 - rect->y1;
          setColor(aColor, gTrue);
          drawCircle(0.5 * dx, 0.5 * dy, 0.2 * (dx < dy ? dx : dy), gTrue);
        }
      }
    }
  }
    break;
  case formButtonPush:
    if (caption)
      drawText(caption, da, resources, gFalse, 0, fieldQuadCenter, gFalse, gFalse);
    break;
  case formButtonCheck:
    if (appearState && appearState->cmp("Off") != 0) {
      if (!caption) {
        GooString checkMark("3");
        drawText(&checkMark, da, resources, gFalse, 0, fieldQuadCenter, gFalse, gTrue);
      } else {
        drawText(caption, da, resources, gFalse, 0, fieldQuadCenter, gFalse, gTrue);
      }
    }
    break;
  }
}

void AnnotWidget::drawFormFieldText(GfxResources *resources, GooString *da) {
  VariableTextQuadding quadding;
  GooString *contents;
  FormFieldText *fieldText = static_cast<FormFieldText *>(field);

  contents = fieldText->getContent();
  if (contents) {
    quadding = field->hasTextQuadding() ? field->getTextQuadding() : form->getTextQuadding();

    int comb = 0;
    if (fieldText->isComb())
      comb = fieldText->getMaxLen();

    drawText(contents, da, resources,
             fieldText->isMultiline(), comb, quadding, gTrue, gFalse, fieldText->isPassword());
  }
}

void AnnotWidget::drawFormFieldChoice(GfxResources *resources, GooString *da) {
  GooString *selected;
  VariableTextQuadding quadding;
  FormFieldChoice *fieldChoice = static_cast<FormFieldChoice *>(field);

  quadding = field->hasTextQuadding() ? field->getTextQuadding() : form->getTextQuadding();

  if (fieldChoice->isCombo()) {
    selected = fieldChoice->getSelectedChoice();
    if (selected) {
      drawText(selected, da, resources, gFalse, 0, quadding, gTrue, gFalse);
      //~ Acrobat draws a popup icon on the right side
    }
  // list box
  } else {
    drawListBox(fieldChoice, da, resources, quadding);
  }
}

void AnnotWidget::generateFieldAppearance() {
  Object appearDict, obj1, obj2;
  GfxResources *resources;
  MemStream *appearStream;
  GooString *da;

  appearBuf = new GooString ();

  // draw the background
  if (appearCharacs) {
    AnnotColor *aColor = appearCharacs->getBackColor();
    if (aColor) {
      setColor(aColor, gTrue);
      appearBuf->appendf("0 0 {0:.2f} {1:.2f} re f\n",
                         rect->x2 - rect->x1, rect->y2 - rect->y1);
    }
  }

  // draw the border
  if (appearCharacs && border && border->getWidth() > 0)
    drawBorder();

  da = field->getDefaultAppearance();
  if (!da)
    da = form->getDefaultAppearance();

  resources = form->getDefaultResources();

  // draw the field contents
  switch (field->getType()) {
  case formButton:
    drawFormFieldButton(resources, da);
    break;
  case formText:
    drawFormFieldText(resources, da);
    break;
  case formChoice:
    drawFormFieldChoice(resources, da);
    break;
  case formSignature:
    //~unimp
    break;
  case formUndef:
  default:
    error(errSyntaxError, -1, "Unknown field type");
  }

  // build the appearance stream dictionary
  appearDict.initDict(xref);
  appearDict.dictAdd(copyString("Length"),
      obj1.initInt(appearBuf->getLength()));
  appearDict.dictAdd(copyString("Subtype"), obj1.initName("Form"));
  obj1.initArray(xref);
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(rect->x2 - rect->x1));
  obj1.arrayAdd(obj2.initReal(rect->y2 - rect->y1));
  appearDict.dictAdd(copyString("BBox"), &obj1);

  // set the resource dictionary
  Object *resDict = form->getDefaultResourcesObj();
  if (resDict->isDict()) {
    appearDict.dictAdd(copyString("Resources"), resDict->copy(&obj1));
  }

  // build the appearance stream
  appearStream = new MemStream(copyString(appearBuf->getCString()), 0,
      appearBuf->getLength(), &appearDict);
  appearance.free();
  appearance.initStream(appearStream);
  delete appearBuf;

  appearStream->setNeedFree(gTrue);
}

void AnnotWidget::updateAppearanceStream()
{
  // If this the first time updateAppearanceStream() is called on this widget,
  // destroy the AP dictionary because we are going to create a new one.
  if (updatedAppearanceStream.num == -1) {
    invalidateAppearance(); // Delete AP dictionary and all referenced streams
  }

  // There's no need to create a new appearance stream if NeedAppearances is
  // set, because it will be ignored next time anyway.
  if (form && form->getNeedAppearances())
    return;

  // Create the new appearance
  generateFieldAppearance();

  // Fetch the appearance stream we've just created
  Object obj1;
  appearance.fetch(xref, &obj1);

  // If this the first time updateAppearanceStream() is called on this widget,
  // create a new AP dictionary containing the new appearance stream.
  // Otherwise, just update the stream we had created previously.
  if (updatedAppearanceStream.num == -1) {
    // Write the appearance stream
    updatedAppearanceStream = xref->addIndirectObject(&obj1);
    obj1.free();

    // Write the AP dictionary
    Object obj2;
    obj1.initDict(xref);
    obj1.dictAdd(copyString("N"), obj2.initRef(updatedAppearanceStream.num, updatedAppearanceStream.gen));
    update("AP", &obj1);

    // Update our internal pointers to the appearance dictionary
    appearStreams = new AnnotAppearance(doc, &obj1);
  } else {
    // Replace the existing appearance stream
    xref->setModifiedObject(&obj1, updatedAppearanceStream);
    obj1.free();
  }
}

void AnnotWidget::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

  annotLocker();
  addDingbatsResource = gFalse;

  // Only construct the appearance stream when
  // - annot doesn't have an AP or
  // - NeedAppearances is true
  if (field) {
    if (appearance.isNull() || (form && form->getNeedAppearances()))
      generateFieldAppearance();
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  if (addDingbatsResource) {
    // We are forcing ZaDb but the font does not exist
    // so create a fake one
    Object baseFontObj, subtypeObj;
    baseFontObj.initName("ZapfDingbats");
    subtypeObj.initName("Type1");

    Object fontDictObj;
    Dict *fontDict = new Dict(gfx->getXRef());
    fontDict->decRef();
    fontDict->add(copyString("BaseFont"), &baseFontObj);
    fontDict->add(copyString("Subtype"), &subtypeObj);
    fontDictObj.initDict(fontDict);

    Object fontsDictObj;
    Dict *fontsDict = new Dict(gfx->getXRef());
    fontsDict->decRef();
    fontsDict->add(copyString("ZaDb"), &fontDictObj);
    fontsDictObj.initDict(fontsDict);

    Dict *dict = new Dict(gfx->getXRef());
    dict->add(copyString("Font"), &fontsDictObj);
    gfx->pushResources(dict);
    delete dict;
  }
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  if (addDingbatsResource) {
    gfx->popResources();
  }
  obj.free();
}


//------------------------------------------------------------------------
// AnnotMovie
//------------------------------------------------------------------------
AnnotMovie::AnnotMovie(PDFDoc *docA, PDFRectangle *rect, Movie *movieA) :
    Annot(docA, rect) {
  Object obj1;

  type = typeMovie;
  annotObj.dictSet ("Subtype", obj1.initName ("Movie"));

  movie = movieA->copy();
  // TODO: create movie dict from movieA

  initialize(docA, annotObj.getDict());
}

AnnotMovie::AnnotMovie(PDFDoc *docA, Dict *dict, Object *obj) :
  Annot(docA, dict, obj) {
  type = typeMovie;
  initialize(docA, dict);
}

AnnotMovie::~AnnotMovie() {
  if (title)
    delete title;
  delete movie;
}

void AnnotMovie::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  if (dict->lookup("T", &obj1)->isString()) {
    title = obj1.getString()->copy();
  } else {
    title = NULL;
  }
  obj1.free();

  Object movieDict;
  if (dict->lookup("Movie", &movieDict)->isDict()) {
    Object obj2;
    dict->lookup("A", &obj2);
    if (obj2.isDict())
      movie = new Movie (&movieDict, &obj2);
    else
      movie = new Movie (&movieDict);
    if (!movie->isOk()) {
      delete movie;
      movie = NULL;
      ok = gFalse;
    }
    obj2.free();
  } else {
    error(errSyntaxError, -1, "Bad Annot Movie");
    movie = NULL;
    ok = gFalse;
  }
  movieDict.free();
}

void AnnotMovie::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull() && movie->getShowPoster()) {
    int width, height;
    Object poster;
    movie->getPoster(&poster);
    movie->getAspect(&width, &height);

    if (width != -1 && height != -1 && !poster.isNone()) {
      MemStream *mStream;

      appearBuf = new GooString ();
      appearBuf->append ("q\n");
      appearBuf->appendf ("{0:d} 0 0 {1:d} 0 0 cm\n", width, height);
      appearBuf->append ("/MImg Do\n");
      appearBuf->append ("Q\n");

      Object imgDict;
      imgDict.initDict(gfx->getXRef());
      imgDict.dictSet ("MImg", &poster);

      Object resDict;
      resDict.initDict(gfx->getXRef());
      resDict.dictSet ("XObject", &imgDict);

      Object formDict, obj1, obj2;
      formDict.initDict(gfx->getXRef());
      formDict.dictSet("Length", obj1.initInt(appearBuf->getLength()));
      formDict.dictSet("Subtype", obj1.initName("Form"));
      formDict.dictSet("Name", obj1.initName("FRM"));
      obj1.initArray(gfx->getXRef());
      obj1.arrayAdd(obj2.initInt(0));
      obj1.arrayAdd(obj2.initInt(0));
      obj1.arrayAdd(obj2.initInt(width));
      obj1.arrayAdd(obj2.initInt(height));
      formDict.dictSet("BBox", &obj1);
      obj1.initArray(gfx->getXRef());
      obj1.arrayAdd(obj2.initInt(1));
      obj1.arrayAdd(obj2.initInt(0));
      obj1.arrayAdd(obj2.initInt(0));
      obj1.arrayAdd(obj2.initInt(1));
      obj1.arrayAdd(obj2.initInt(-width / 2));
      obj1.arrayAdd(obj2.initInt(-height / 2));
      formDict.dictSet("Matrix", &obj1);
      formDict.dictSet("Resources", &resDict);

      Object aStream;
      mStream = new MemStream(copyString(appearBuf->getCString()), 0,
			      appearBuf->getLength(), &formDict);
      mStream->setNeedFree(gTrue);
      aStream.initStream(mStream);
      delete appearBuf;

      Object objDict;
      objDict.initDict(gfx->getXRef());
      objDict.dictSet ("FRM", &aStream);

      resDict.initDict(gfx->getXRef());
      resDict.dictSet ("XObject", &objDict);

      appearBuf = new GooString ();
      appearBuf->append ("q\n");
      appearBuf->appendf ("0 0 {0:d} {1:d} re W n\n", width, height);
      appearBuf->append ("q\n");
      appearBuf->appendf ("0 0 {0:d} {1:d} re W n\n", width, height);
      appearBuf->appendf ("1 0 0 1 {0:d} {1:d} cm\n", width / 2, height / 2);
      appearBuf->append ("/FRM Do\n");
      appearBuf->append ("Q\n");
      appearBuf->append ("Q\n");

      double bbox[4];
      bbox[0] = bbox[1] = 0;
      bbox[2] = width;
      bbox[3] = height;
      createForm(bbox, gFalse, &resDict, &appearance);
      delete appearBuf;
    }
    poster.free();
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  obj.free();
}

//------------------------------------------------------------------------
// AnnotScreen
//------------------------------------------------------------------------
AnnotScreen::AnnotScreen(PDFDoc *docA, PDFRectangle *rect) :
    Annot(docA, rect) {
  Object obj1;

  type = typeScreen;

  annotObj.dictSet ("Subtype", obj1.initName ("Screen"));
  initialize(docA, annotObj.getDict());
}

AnnotScreen::AnnotScreen(PDFDoc *docA, Dict *dict, Object *obj) :
  Annot(docA, dict, obj) {
  type = typeScreen;
  initialize(docA, dict);
}

AnnotScreen::~AnnotScreen() {
  if (title)
    delete title;
  if (appearCharacs)
    delete appearCharacs;
  if (action)
    delete action;

  additionalActions.free();
}

void AnnotScreen::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  title = NULL;
  if (dict->lookup("T", &obj1)->isString()) {
    title = obj1.getString()->copy();
  }
  obj1.free();

  action = NULL;
  if (dict->lookup("A", &obj1)->isDict()) {
    action = LinkAction::parseAction(&obj1, doc->getCatalog()->getBaseURI());
    if (action->getKind() == actionRendition && page == 0) {
      error (errSyntaxError, -1, "Invalid Rendition action: associated screen annotation without P");
      delete action;
      action = NULL;
      ok = gFalse;
    }
  }
  obj1.free();

  dict->lookupNF("AA", &additionalActions);

  appearCharacs = NULL;
  if(dict->lookup("MK", &obj1)->isDict()) {
    appearCharacs = new AnnotAppearanceCharacs(obj1.getDict());
  }
  obj1.free();
}

LinkAction* AnnotScreen::getAdditionalAction(AdditionalActionsType type)
{
  if (type == actionFocusIn || type == actionFocusOut) // not defined for screen annotation
    return NULL;

  return ::getAdditionalAction(type, &additionalActions, doc);
}

//------------------------------------------------------------------------
// AnnotStamp
//------------------------------------------------------------------------
AnnotStamp::AnnotStamp(PDFDoc *docA, PDFRectangle *rect) :
  AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeStamp;
  annotObj.dictSet ("Subtype", obj1.initName ("Stamp"));
  initialize(docA, annotObj.getDict());
}

AnnotStamp::AnnotStamp(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  type = typeStamp;
  initialize(docA, dict);
}

AnnotStamp::~AnnotStamp() {
  delete icon;
}

void AnnotStamp::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  if (dict->lookup("Name", &obj1)->isName()) {
    icon = new GooString(obj1.getName());
  } else {
    icon = new GooString("Draft");
  }
  obj1.free();

}

void AnnotStamp::setIcon(GooString *new_icon) {
  delete icon;

  if (new_icon) {
    icon = new GooString (new_icon);
  } else {
    icon = new GooString();
  }

  Object obj1;
  obj1.initName (icon->getCString());
  update("Name", &obj1);
  invalidateAppearance();
}

//------------------------------------------------------------------------
// AnnotGeometry
//------------------------------------------------------------------------
AnnotGeometry::AnnotGeometry(PDFDoc *docA, PDFRectangle *rect, AnnotSubtype subType) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  switch (subType) {
    case typeSquare:
      annotObj.dictSet ("Subtype", obj1.initName ("Square"));
      break;
    case typeCircle:
      annotObj.dictSet ("Subtype", obj1.initName ("Circle"));
      break;
    default:
      assert (0 && "Invalid subtype for AnnotGeometry\n");
  }

  initialize(docA, annotObj.getDict());
}

AnnotGeometry::AnnotGeometry(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  // the real type will be read in initialize()
  type = typeSquare;
  initialize(docA, dict);
}

AnnotGeometry::~AnnotGeometry() {
  delete interiorColor;
  delete borderEffect;
  delete geometryRect;
}

void AnnotGeometry::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("Square")) {
      type = typeSquare;
    } else if (!typeName.cmp("Circle")) {
      type = typeCircle;
    }
  }
  obj1.free();

  if (dict->lookup("IC", &obj1)->isArray()) {
    interiorColor = new AnnotColor(obj1.getArray());
  } else {
    interiorColor = NULL;
  }
  obj1.free();

  if (dict->lookup("BE", &obj1)->isDict()) {
    borderEffect = new AnnotBorderEffect(obj1.getDict());
  } else {
    borderEffect = NULL;
  }
  obj1.free();

  geometryRect = NULL;
  if (dict->lookup("RD", &obj1)->isArray()) {
    geometryRect = parseDiffRectangle(obj1.getArray(), rect);
  }
  obj1.free();

}

void AnnotGeometry::setType(AnnotSubtype new_type) {
  Object obj1;

  switch (new_type) {
    case typeSquare:
      obj1.initName("Square");
      break;
    case typeCircle:
      obj1.initName("Circle");
      break;
    default:
      assert(!"Invalid subtype");
  }

  type = new_type;
  update("Subtype", &obj1);
  invalidateAppearance();
}

void AnnotGeometry::setInteriorColor(AnnotColor *new_color) {
  delete interiorColor;

  if (new_color) {
    Object obj1;
    new_color->writeToObject(xref, &obj1);
    update ("IC", &obj1);
    interiorColor = new_color;
  } else {
    interiorColor = NULL;
  }
  invalidateAppearance();
}

void AnnotGeometry::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    ca = opacity;

    appearBuf = new GooString ();
    appearBuf->append ("q\n");
    if (color)
      setColor(color, gFalse);

    if (border) {
      int i, dashLength;
      double *dash;
      double borderWidth = border->getWidth();

      switch (border->getStyle()) {
      case AnnotBorder::borderDashed:
        appearBuf->append("[");
	dashLength = border->getDashLength();
	dash = border->getDash();
	for (i = 0; i < dashLength; ++i)
	  appearBuf->appendf(" {0:.2f}", dash[i]);
	appearBuf->append(" ] 0 d\n");
	break;
      default:
        appearBuf->append("[] 0 d\n");
        break;
      }
      appearBuf->appendf("{0:.2f} w\n", borderWidth);

      if (interiorColor)
        setColor(interiorColor, gTrue);

      if (type == typeSquare) {
        appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} re\n",
			    borderWidth / 2.0, borderWidth / 2.0,
			    (rect->x2 - rect->x1) - borderWidth,
			    (rect->y2 - rect->y1) - borderWidth);
      } else {
        double width, height;
	double b;
	double x1, y1, x2, y2, x3, y3;

	width = rect->x2 - rect->x1;
	height = rect->y2 - rect->y1;
	b = borderWidth / 2.0;

	x1 = b;
	y1 = height / 2.0;
	appearBuf->appendf ("{0:.2f} {1:.2f} m\n", x1, y1);

	y1 += height / 4.0;
	x2 = width / 4.0;
	y2 = height - b;
	x3 = width / 2.0;
	y3 = y2;
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x1, y1, x2, y2, x3, y3);
	x2 = width - b;
	y2 = y1;
	x1 = x3 + (width / 4.0);
	y1 = y3;
	x3 = x2;
	y3 = height / 2.0;
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x1, y1, x2, y2, x3, y3);

	x2 = x1;
	y2 = b;
	x1 = x3;
	y1 = height / 4.0;
	x3 = width / 2.0;
	y3 = b;
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x1, y1, x2, y2, x3, y3);

	x2 = b;
	y2 = y1;
	x1 = width / 4.0;
	y1 = b;
	x3 = b;
	y3 = height / 2.0;
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x1, y1, x2, y2, x3, y3);

      }

      if (interiorColor && interiorColor->getSpace() != AnnotColor::colorTransparent)
        appearBuf->append ("b\n");
      else
        appearBuf->append ("S\n");
    }
    appearBuf->append ("Q\n");

    double bbox[4];
    bbox[0] = bbox[1] = 0;
    bbox[2] = rect->x2 - rect->x1;
    bbox[3] = rect->y2 - rect->y1;
    if (ca == 1) {
      createForm(bbox, gFalse, NULL, &appearance);
    } else {
      Object aStream;

      createForm(bbox, gTrue, NULL, &aStream);
      delete appearBuf;

      Object resDict;
      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
      createForm(bbox, gFalse, &resDict, &appearance);
    }
    delete appearBuf;
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  obj.free();
}

//------------------------------------------------------------------------
// AnnotPolygon
//------------------------------------------------------------------------
AnnotPolygon::AnnotPolygon(PDFDoc *docA, PDFRectangle *rect, AnnotSubtype subType) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  switch (subType) {
    case typePolygon:
      annotObj.dictSet ("Subtype", obj1.initName ("Polygon"));
      break;
    case typePolyLine:
      annotObj.dictSet ("Subtype", obj1.initName ("PolyLine"));
      break;
    default:
      assert (0 && "Invalid subtype for AnnotGeometry\n");
  }

  // Store dummy path with one null vertex only
  Object obj2, obj3;
  obj2.initArray (doc->getXRef());
  obj2.arrayAdd (obj3.initReal (0));
  obj2.arrayAdd (obj3.initReal (0));
  annotObj.dictSet ("Vertices", &obj2);

  initialize(docA, annotObj.getDict());
}

AnnotPolygon::AnnotPolygon(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  // the real type will be read in initialize()
  type = typePolygon;
  initialize(docA, dict);
}

AnnotPolygon::~AnnotPolygon() {
  delete vertices;

  if (interiorColor)
    delete interiorColor;

  if (borderEffect)
    delete borderEffect;
}

void AnnotPolygon::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("Polygon")) {
      type = typePolygon;
    } else if (!typeName.cmp("PolyLine")) {
      type = typePolyLine;
    }
  }
  obj1.free();

  if (dict->lookup("Vertices", &obj1)->isArray()) {
    vertices = new AnnotPath(obj1.getArray());
  } else {
    vertices = new AnnotPath();
    error(errSyntaxError, -1, "Bad Annot Polygon Vertices");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("LE", &obj1)->isArray() && obj1.arrayGetLength() == 2) {
    Object obj2;

    if(obj1.arrayGet(0, &obj2)->isString())
      startStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      startStyle = annotLineEndingNone;
    obj2.free();

    if(obj1.arrayGet(1, &obj2)->isString())
      endStyle = parseAnnotLineEndingStyle(obj2.getString());
    else
      endStyle = annotLineEndingNone;
    obj2.free();

  } else {
    startStyle = endStyle = annotLineEndingNone;
  }
  obj1.free();

  if (dict->lookup("IC", &obj1)->isArray()) {
    interiorColor = new AnnotColor(obj1.getArray());
  } else {
    interiorColor = NULL;
  }
  obj1.free();

  if (dict->lookup("BE", &obj1)->isDict()) {
    borderEffect = new AnnotBorderEffect(obj1.getDict());
  } else {
    borderEffect = NULL;
  }
  obj1.free();

  if (dict->lookup("IT", &obj1)->isName()) {
    const char *intentName = obj1.getName();

    if(!strcmp(intentName, "PolygonCloud")) {
      intent = polygonCloud;
    } else if(!strcmp(intentName, "PolyLineDimension")) {
      intent = polylineDimension;
    } else {
      intent = polygonDimension;
    }
  } else {
    intent = polygonCloud;
  }
  obj1.free();
}

void AnnotPolygon::setType(AnnotSubtype new_type) {
  Object obj1;

  switch (new_type) {
    case typePolygon:
      obj1.initName("Polygon");
      break;
    case typePolyLine:
      obj1.initName("PolyLine");
      break;
    default:
      assert(!"Invalid subtype");
  }

  type = new_type;
  update("Subtype", &obj1);
  invalidateAppearance();
}

void AnnotPolygon::setVertices(AnnotPath *path) {
  Object obj1, obj2;
  delete vertices;

  obj1.initArray(xref);

  for (int i = 0; i < path->getCoordsLength(); i++) {
    obj1.arrayAdd (obj2.initReal (path->getX(i)));
    obj1.arrayAdd (obj2.initReal (path->getY(i)));
  }

  vertices = new AnnotPath(obj1.getArray());

  update("Vertices", &obj1);
  invalidateAppearance();
}

void AnnotPolygon::setStartEndStyle(AnnotLineEndingStyle start, AnnotLineEndingStyle end) {
  Object obj1, obj2;

  startStyle = start;
  endStyle = end;

  obj1.initArray(xref);
  obj1.arrayAdd( obj2.initName(convertAnnotLineEndingStyle( startStyle )) );
  obj1.arrayAdd( obj2.initName(convertAnnotLineEndingStyle( endStyle )) );

  update("LE", &obj1);
  invalidateAppearance();
}

void AnnotPolygon::setInteriorColor(AnnotColor *new_color) {
  delete interiorColor;

  if (new_color) {
    Object obj1;
    new_color->writeToObject(xref, &obj1);
    update ("IC", &obj1);
    interiorColor = new_color;
  } else {
    interiorColor = NULL;
  }
  invalidateAppearance();
}

void AnnotPolygon::setIntent(AnnotPolygonIntent new_intent) {
  Object obj1;

  intent = new_intent;
  if (new_intent == polygonCloud)
    obj1.initName("PolygonCloud");
  else if (new_intent == polylineDimension)
    obj1.initName("PolyLineDimension");
  else // polygonDimension
    obj1.initName("PolygonDimension");
  update ("IT", &obj1);
}

void AnnotPolygon::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    appearBBox = new AnnotAppearanceBBox(rect);
    ca = opacity;

    appearBuf = new GooString ();
    appearBuf->append ("q\n");

    if (color) {
      setColor(color, gFalse);
    }

    if (border) {
      int i, dashLength;
      double *dash;

      switch (border->getStyle()) {
      case AnnotBorder::borderDashed:
        appearBuf->append("[");
        dashLength = border->getDashLength();
        dash = border->getDash();
        for (i = 0; i < dashLength; ++i)
          appearBuf->appendf(" {0:.2f}", dash[i]);
        appearBuf->append(" ] 0 d\n");
        break;
      default:
        appearBuf->append("[] 0 d\n");
        break;
      }
      appearBuf->appendf("{0:.2f} w\n", border->getWidth());
      appearBBox->setBorderWidth(border->getWidth());
    }

    if (interiorColor) {
      setColor(interiorColor, gTrue);
    }

    if (vertices->getCoordsLength() != 0) {
      appearBuf->appendf ("{0:.2f} {1:.2f} m\n", vertices->getX(0) - rect->x1, vertices->getY(0) - rect->y1);
      appearBBox->extendTo (vertices->getX(0) - rect->x1, vertices->getY(0) - rect->y1);

      for (int i = 1; i < vertices->getCoordsLength(); ++i) {
        appearBuf->appendf ("{0:.2f} {1:.2f} l\n", vertices->getX(i) - rect->x1, vertices->getY(i) - rect->y1);
        appearBBox->extendTo (vertices->getX(i) - rect->x1, vertices->getY(i) - rect->y1);
      }

      if (type == typePolygon) {
        if (interiorColor && interiorColor->getSpace() != AnnotColor::colorTransparent) {
          appearBuf->append ("b\n");
        } else {
          appearBuf->append ("s\n");
        }
      } else {
        appearBuf->append ("S\n");
      }
    }

    appearBuf->append ("Q\n");

    double bbox[4];
    appearBBox->getBBoxRect(bbox);
    if (ca == 1) {
      createForm(bbox, gFalse, NULL, &appearance);
    } else {
      Object aStream, resDict;

      createForm(bbox, gTrue, NULL, &aStream);
      delete appearBuf;

      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
      createForm(bbox, gFalse, &resDict, &appearance);
    }
    delete appearBuf;
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  if (appearBBox) {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   appearBBox->getPageXMin(), appearBBox->getPageYMin(),
                   appearBBox->getPageXMax(), appearBBox->getPageYMax(),
                   getRotation());
  } else {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  }
  obj.free();
}

//------------------------------------------------------------------------
// AnnotCaret
//------------------------------------------------------------------------
AnnotCaret::AnnotCaret(PDFDoc *docA, PDFRectangle *rect) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeCaret;

  annotObj.dictSet ("Subtype", obj1.initName ("Caret"));
  initialize(docA, annotObj.getDict());
}

AnnotCaret::AnnotCaret(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  type = typeCaret;
  initialize(docA, dict);
}

AnnotCaret::~AnnotCaret() {
  delete caretRect;
}

void AnnotCaret::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  symbol = symbolNone;
  if (dict->lookup("Sy", &obj1)->isName()) {
    GooString typeName(obj1.getName());
    if (!typeName.cmp("P")) {
      symbol = symbolP;
    } else if (!typeName.cmp("None")) {
      symbol = symbolNone;
    }
  }
  obj1.free();

  if (dict->lookup("RD", &obj1)->isArray()) {
    caretRect = parseDiffRectangle(obj1.getArray(), rect);
  } else {
    caretRect = NULL;
  }
  obj1.free();

}

void AnnotCaret::setSymbol(AnnotCaretSymbol new_symbol) {
  Object obj1;
  obj1.initName( new_symbol == symbolP ? "P" : "None" );
  symbol = new_symbol;
  update("Sy", &obj1);
  invalidateAppearance();
}

//------------------------------------------------------------------------
// AnnotInk
//------------------------------------------------------------------------
AnnotInk::AnnotInk(PDFDoc *docA, PDFRectangle *rect) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeInk;

  annotObj.dictSet ("Subtype", obj1.initName ("Ink"));

  // Store dummy path with one null vertex only
  Object obj2, obj3, obj4;
  obj2.initArray (doc->getXRef());
  obj2.arrayAdd (obj3.initArray (doc->getXRef()));
  obj3.arrayAdd (obj4.initReal (0));
  obj3.arrayAdd (obj4.initReal (0));
  annotObj.dictSet ("InkList", &obj2);

  initialize(docA, annotObj.getDict());
}

AnnotInk::AnnotInk(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  type = typeInk;
  initialize(docA, dict);
}

AnnotInk::~AnnotInk() {
  freeInkList();
}

void AnnotInk::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  if (dict->lookup("InkList", &obj1)->isArray()) {
    parseInkList(obj1.getArray());
  } else {
    inkListLength = 0;
    inkList = NULL;
    error(errSyntaxError, -1, "Bad Annot Ink List");
    ok = gFalse;
  }
  obj1.free();
}

void AnnotInk::writeInkList(AnnotPath **paths, int n_paths, Array *dest_array) {
  Object obj1, obj2;
  for (int i = 0; i < n_paths; ++i) {
    AnnotPath *path = paths[i];
    obj1.initArray (xref);
    for (int j = 0; j < path->getCoordsLength(); ++j) {
      obj1.arrayAdd (obj2.initReal (path->getX(j)));
      obj1.arrayAdd (obj2.initReal (path->getY(j)));
    }
    dest_array->add (&obj1);
  }
}

void AnnotInk::parseInkList(Array *array) {
  inkListLength = array->getLength();
  inkList = (AnnotPath **) gmallocn ((inkListLength), sizeof(AnnotPath *));
  memset(inkList, 0, inkListLength * sizeof(AnnotPath *));
  for (int i = 0; i < inkListLength; i++) {
    Object obj2;
    if (array->get(i, &obj2)->isArray())
      inkList[i] = new AnnotPath(obj2.getArray());
    obj2.free();
  }
}

void AnnotInk::freeInkList() {
  if (inkList) {
    for (int i = 0; i < inkListLength; ++i)
      delete inkList[i];
    gfree(inkList);
  }
}

void AnnotInk::setInkList(AnnotPath **paths, int n_paths) {
  Object obj1;

  freeInkList();

  obj1.initArray (xref);
  writeInkList(paths, n_paths, obj1.getArray());

  parseInkList(obj1.getArray());
  annotObj.dictSet ("InkList", &obj1);
  invalidateAppearance();
}

void AnnotInk::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    appearBBox = new AnnotAppearanceBBox(rect);
    ca = opacity;

    appearBuf = new GooString ();
    appearBuf->append ("q\n");

    if (color) {
      setColor(color, gFalse);
    }

    if (border) {
      appearBuf->appendf("{0:.2f} w\n", border->getWidth());
      appearBBox->setBorderWidth(border->getWidth());
    }

    for (int i = 0; i < inkListLength; ++i) {
      const AnnotPath * path = inkList[i];
      if (path->getCoordsLength() != 0) {
        appearBuf->appendf ("{0:.2f} {1:.2f} m\n", path->getX(0) - rect->x1, path->getY(0) - rect->y1);
        appearBBox->extendTo (path->getX(0) - rect->x1, path->getY(0) - rect->y1);

        for (int j = 1; j < path->getCoordsLength(); ++j) {
          appearBuf->appendf ("{0:.2f} {1:.2f} l\n", path->getX(j) - rect->x1, path->getY(j) - rect->y1);
          appearBBox->extendTo (path->getX(j) - rect->x1, path->getY(j) - rect->y1);
        }

        appearBuf->append ("S\n");
      }
    }

    appearBuf->append ("Q\n");

    double bbox[4];
    appearBBox->getBBoxRect(bbox);
    if (ca == 1) {
      createForm(bbox, gFalse, NULL, &appearance);
    } else {
      Object aStream, resDict;

      createForm(bbox, gTrue, NULL, &aStream);
      delete appearBuf;

      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
      createForm(bbox, gFalse, &resDict, &appearance);
    }
    delete appearBuf;
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  if (appearBBox) {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   appearBBox->getPageXMin(), appearBBox->getPageYMin(),
                   appearBBox->getPageXMax(), appearBBox->getPageYMax(),
                   getRotation());
  } else {
    gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
                   rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  }
  obj.free();
}

//------------------------------------------------------------------------
// AnnotFileAttachment
//------------------------------------------------------------------------
AnnotFileAttachment::AnnotFileAttachment(PDFDoc *docA, PDFRectangle *rect, GooString *filename) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeFileAttachment;

  annotObj.dictSet ("Subtype", obj1.initName ("FileAttachment"));

  Object obj2;
  obj2.initString(filename->copy());
  annotObj.dictSet ("FS", &obj2);

  initialize(docA, annotObj.getDict());
}

AnnotFileAttachment::AnnotFileAttachment(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  type = typeFileAttachment;
  initialize(docA, dict);
}

AnnotFileAttachment::~AnnotFileAttachment() {
  file.free();

  if (name)
    delete name;
}

void AnnotFileAttachment::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  if (dict->lookup("FS", &obj1)->isDict() || dict->lookup("FS", &obj1)->isString()) {
    obj1.copy(&file);
  } else {
    error(errSyntaxError, -1, "Bad Annot File Attachment");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Name", &obj1)->isName()) {
    name = new GooString(obj1.getName());
  } else {
    name = new GooString("PushPin");
  }
  obj1.free();
}

#define ANNOT_FILE_ATTACHMENT_AP_PUSHPIN                                         \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"       \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"     \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                           \
  "4.301 23 m f\n"                                                               \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                          \
  "1 J\n"                                                                        \
  "1 j\n"                                                                        \
  "[] 0.0 d\n"                                                                   \
  "4 M 5 4 m 6 5 l S\n"                                                          \
  "2 w\n"                                                                        \
  "11 14 m 9 12 l 6 12 l 13 5 l 13 8 l 15 10 l 18 11 l 20 11 l 12 19 l 12\n"     \
  "17 l 11 14 l h\n"                                                             \
  "11 14 m S\n"                                                                  \
  "3 w\n"                                                                        \
  "6 5 m 9 8 l S\n"                                                              \
  "0.729412 0.741176 0.713725 RG 2 w\n"                                          \
  "5 5 m 6 6 l S\n"                                                              \
  "2 w\n"                                                                        \
  "11 15 m 9 13 l 6 13 l 13 6 l 13 9 l 15 11 l 18 12 l 20 12 l 12 20 l 12\n"     \
  "18 l 11 15 l h\n"                                                             \
  "11 15 m S\n"                                                                  \
  "3 w\n"                                                                        \
  "6 6 m 9 9 l S\n"

#define ANNOT_FILE_ATTACHMENT_AP_PAPERCLIP                                       \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"       \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"     \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                           \
  "4.301 23 m f\n"                                                               \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                          \
  "1 J\n"                                                                        \
  "1 j\n"                                                                        \
  "[] 0.0 d\n"                                                                   \
  "4 M 16.645 12.035 m 12.418 7.707 l 10.902 6.559 6.402 11.203 8.09 12.562 c\n" \
  "14.133 18.578 l 14.949 19.387 16.867 19.184 17.539 18.465 c 20.551\n"         \
  "15.23 l 21.191 14.66 21.336 12.887 20.426 12.102 c 13.18 4.824 l 12.18\n"     \
  "3.82 6.25 2.566 4.324 4.461 c 3 6.395 3.383 11.438 4.711 12.801 c 9.648\n"    \
  "17.887 l S\n"                                                                 \
  "0.729412 0.741176 0.713725 RG 16.645 13.035 m 12.418 8.707 l\n"               \
  "10.902 7.559 6.402 12.203 8.09 13.562 c\n"                                    \
  "14.133 19.578 l 14.949 20.387 16.867 20.184 17.539 19.465 c 20.551\n"         \
  "16.23 l 21.191 15.66 21.336 13.887 20.426 13.102 c 13.18 5.824 l 12.18\n"     \
  "4.82 6.25 3.566 4.324 5.461 c 3 7.395 3.383 12.438 4.711 13.801 c 9.648\n"    \
  "18.887 l S\n"

#define ANNOT_FILE_ATTACHMENT_AP_GRAPH                                           \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"       \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"     \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                           \
  "4.301 23 m f\n"                                                               \
  "0.533333 0.541176 0.521569 RG 1 w\n"                                          \
  "1 J\n"                                                                        \
  "0 j\n"                                                                        \
  "[] 0.0 d\n"                                                                   \
  "4 M 18.5 15.5 m 18.5 13.086 l 16.086 15.5 l 18.5 15.5 l h\n"                  \
  "18.5 15.5 m S\n"                                                              \
  "7 7 m 10 11 l 13 9 l 18 15 l S\n"                                             \
  "0.729412 0.741176 0.713725 RG 7 8 m 10 12 l 13 10 l 18 16 l S\n"              \
  "18.5 16.5 m 18.5 14.086 l 16.086 16.5 l 18.5 16.5 l h\n"                      \
  "18.5 16.5 m S\n"                                                              \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                          \
  "1 j\n"                                                                        \
  "3 19 m 3 3 l 21 3 l S\n"                                                      \
  "0.729412 0.741176 0.713725 RG 3 20 m 3 4 l 21 4 l S\n"

#define ANNOT_FILE_ATTACHMENT_AP_TAG                                             \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"       \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"     \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                           \
  "4.301 23 m f\n"                                                               \
  "0.533333 0.541176 0.521569 RG 0.999781 w\n"                                   \
  "1 J\n"                                                                        \
  "1 j\n"                                                                        \
  "[] 0.0 d\n"                                                                   \
  "4 M q 1 0 0 -1 0 24 cm\n"                                                     \
  "8.492 8.707 m 8.492 9.535 7.82 10.207 6.992 10.207 c 6.164 10.207 5.492\n"    \
  "9.535 5.492 8.707 c 5.492 7.879 6.164 7.207 6.992 7.207 c 7.82 7.207\n"       \
  "8.492 7.879 8.492 8.707 c h\n"                                                \
  "8.492 8.707 m S Q\n"                                                          \
  "2 w\n"                                                                        \
  "20.078 11.414 m 20.891 10.602 20.785 9.293 20.078 8.586 c 14.422 2.93 l\n"    \
  "13.715 2.223 12.301 2.223 11.594 2.93 c 3.816 10.707 l 3.109 11.414\n"        \
  "2.402 17.781 3.816 19.195 c 5.23 20.609 11.594 19.902 12.301 19.195 c\n"      \
  "20.078 11.414 l h\n"                                                          \
  "20.078 11.414 m S\n"                                                          \
  "0.729412 0.741176 0.713725 RG 20.078 12.414 m\n"                              \
  "20.891 11.605 20.785 10.293 20.078 9.586 c 14.422 3.93 l\n"                   \
  "13.715 3.223 12.301 3.223 11.594 3.93 c 3.816 11.707 l 3.109 12.414\n"        \
  "2.402 18.781 3.816 20.195 c 5.23 21.609 11.594 20.902 12.301 20.195 c\n"      \
  "20.078 12.414 l h\n"                                                          \
  "20.078 12.414 m S\n"                                                          \
  "0.533333 0.541176 0.521569 RG 1 w\n"                                          \
  "0 j\n"                                                                        \
  "11.949 13.184 m 16.191 8.941 l S\n"                                           \
  "0.729412 0.741176 0.713725 RG 11.949 14.184 m 16.191 9.941 l S\n"             \
  "0.533333 0.541176 0.521569 RG 14.07 6.82 m 9.828 11.062 l S\n"                \
  "0.729412 0.741176 0.713725 RG 14.07 7.82 m 9.828 12.062 l S\n"                \
  "0.533333 0.541176 0.521569 RG 6.93 15.141 m 8 20 14.27 20.5 16 20.5 c\n"      \
  "18.094 20.504 19.5 20 19.5 18 c 19.5 16.699 20.91 16.418 22.5 16.5 c S\n"     \
  "0.729412 0.741176 0.713725 RG 0.999781 w\n"                                   \
  "1 j\n"                                                                        \
  "q 1 0 0 -1 0 24 cm\n"                                                         \
  "8.492 7.707 m 8.492 8.535 7.82 9.207 6.992 9.207 c 6.164 9.207 5.492\n"       \
  "8.535 5.492 7.707 c 5.492 6.879 6.164 6.207 6.992 6.207 c 7.82 6.207\n"       \
  "8.492 6.879 8.492 7.707 c h\n"                                                \
  "8.492 7.707 m S Q\n"                                                          \
  "1 w\n"                                                                        \
  "0 j\n"                                                                        \
  "6.93 16.141 m 8 21 14.27 21.5 16 21.5 c 18.094 21.504 19.5 21 19.5 19 c\n"    \
  "19.5 17.699 20.91 17.418 22.5 17.5 c S\n"

void AnnotFileAttachment::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    ca = opacity;

    appearBuf = new GooString ();

    appearBuf->append ("q\n");
    if (color)
      setColor(color, gTrue);
    else
      appearBuf->append ("1 1 1 rg\n");
    if (!name->cmp("PushPin"))
      appearBuf->append (ANNOT_FILE_ATTACHMENT_AP_PUSHPIN);
    else if (!name->cmp("Paperclip"))
      appearBuf->append (ANNOT_FILE_ATTACHMENT_AP_PAPERCLIP);
    else if (!name->cmp("Graph"))
      appearBuf->append (ANNOT_FILE_ATTACHMENT_AP_GRAPH);
    else if (!name->cmp("Tag"))
      appearBuf->append (ANNOT_FILE_ATTACHMENT_AP_TAG);
    appearBuf->append ("Q\n");

    double bbox[4];
    bbox[0] = bbox[1] = 0;
    bbox[2] = bbox[3] = 24;
    if (ca == 1) {
      createForm (bbox, gFalse, NULL, &appearance);
    } else {
      Object aStream;

      createForm (bbox, gTrue, NULL, &aStream);
      delete appearBuf;

      Object resDict;
      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
      createForm(bbox, gFalse, &resDict, &appearance);
    }
    delete appearBuf;
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  obj.free();
}

//------------------------------------------------------------------------
// AnnotSound
//------------------------------------------------------------------------
AnnotSound::AnnotSound(PDFDoc *docA, PDFRectangle *rect, Sound *soundA) :
    AnnotMarkup(docA, rect) {
  Object obj1;

  type = typeSound;

  annotObj.dictSet ("Subtype", obj1.initName ("Sound"));

  Object obj2;
  Stream *str = soundA->getStream();
  obj2.initStream (str);
  str->incRef(); //FIXME: initStream should do this?
  annotObj.dictSet ("Sound", &obj2);

  initialize(docA, annotObj.getDict());
}

AnnotSound::AnnotSound(PDFDoc *docA, Dict *dict, Object *obj) :
  AnnotMarkup(docA, dict, obj) {
  type = typeSound;
  initialize(docA, dict);
}

AnnotSound::~AnnotSound() {
  delete sound;

  delete name;
}

void AnnotSound::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  sound = Sound::parseSound(dict->lookup("Sound", &obj1));
  if (!sound) {
    error(errSyntaxError, -1, "Bad Annot Sound");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Name", &obj1)->isName()) {
    name = new GooString(obj1.getName());
  } else {
    name = new GooString("Speaker");
  }
  obj1.free();
}

#define ANNOT_SOUND_AP_SPEAKER                                               \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"   \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n" \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                       \
  "4.301 23 m f\n"                                                           \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                      \
  "0 J\n"                                                                    \
  "1 j\n"                                                                    \
  "[] 0.0 d\n"                                                               \
  "4 M 4 14 m 4.086 8.043 l 7 8 l 11 4 l 11 18 l 7 14 l 4 14 l h\n"          \
  "4 14 m S\n"                                                               \
  "1 w\n"                                                                    \
  "1 J\n"                                                                    \
  "0 j\n"                                                                    \
  "13.699 15.398 m 14.699 13.398 14.699 9.398 13.699 7.398 c S\n"            \
  "18.199 19.398 m 21.199 17.398 21.199 5.398 18.199 3.398 c S\n"            \
  "16 17.398 m 18 16.398 18 7.398 16 5.398 c S\n"                            \
  "0.729412 0.741176 0.713725 RG 2 w\n"                                      \
  "0 J\n"                                                                    \
  "1 j\n"                                                                    \
  "4 15 m 4.086 9.043 l 7 9 l 11 5 l 11 19 l 7 15 l 4 15 l h\n"              \
  "4 15 m S\n"                                                               \
  "1 w\n"                                                                    \
  "1 J\n"                                                                    \
  "0 j\n"                                                                    \
  "13.699 16 m 14.699 14 14.699 10 13.699 8 c S\n"                           \
  "18.199 20 m 21.199 18 21.199 6 18.199 4 c S\n"                            \
  "16 18 m 18 17 18 8 16 6 c S\n"

#define ANNOT_SOUND_AP_MIC                                                        \
  "4.301 23 m 19.699 23 l 21.523 23 23 21.523 23 19.699 c 23 4.301 l 23\n"        \
  "2.477 21.523 1 19.699 1 c 4.301 1 l 2.477 1 1 2.477 1 4.301 c 1 19.699\n"      \
  "l 1 21.523 2.477 23 4.301 23 c h\n"                                            \
  "4.301 23 m f\n"                                                                \
  "0.533333 0.541176 0.521569 RG 2 w\n"                                           \
  "1 J\n"                                                                         \
  "0 j\n"                                                                         \
  "[] 0.0 d\n"                                                                    \
  "4 M 12 20 m 12 20 l 13.656 20 15 18.656 15 17 c 15 13 l 15 11.344 13.656 10\n" \
  "12 10 c 12 10 l 10.344 10 9 11.344 9 13 c 9 17 l 9 18.656 10.344 20 12\n"      \
  "20 c h\n"                                                                      \
  "12 20 m S\n"                                                                   \
  "1 w\n"                                                                         \
  "17.5 14.5 m 17.5 11.973 l 17.5 8.941 15.047 6.5 12 6.5 c 8.953 6.5 6.5\n"      \
  "8.941 6.5 11.973 c 6.5 14.5 l S\n"                                             \
  "2 w\n"                                                                         \
  "0 J\n"                                                                         \
  "12 6.52 m 12 3 l S\n"                                                          \
  "1 J\n"                                                                         \
  "8 3 m 16 3 l S\n"                                                              \
  "0.729412 0.741176 0.713725 RG 12 21 m 12 21 l 13.656 21 15 19.656 15 18 c\n"   \
  "15 14 l 15 12.344 13.656 11 12 11 c 12 11 l 10.344 11 9 12.344 9 14 c\n"       \
  "9 18 l 9 19.656 10.344 21 12 21 c h\n"                                         \
  "12 21 m S\n"                                                                   \
  "1 w\n"                                                                         \
  "17.5 15.5 m 17.5 12.973 l 17.5 9.941 15.047 7.5 12 7.5 c 8.953 7.5 6.5\n"      \
  "9.941 6.5 12.973 c 6.5 15.5 l S\n"                                             \
  "2 w\n"                                                                         \
  "0 J\n"                                                                         \
  "12 7.52 m 12 4 l S\n"                                                          \
  "1 J\n"                                                                         \
  "8 4 m 16 4 l S\n"

void AnnotSound::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

  annotLocker();
  if (appearance.isNull()) {
    ca = opacity;

    appearBuf = new GooString ();

    appearBuf->append ("q\n");
    if (color)
      setColor(color, gTrue);
    else
      appearBuf->append ("1 1 1 rg\n");
    if (!name->cmp("Speaker"))
      appearBuf->append (ANNOT_SOUND_AP_SPEAKER);
    else if (!name->cmp("Mic"))
      appearBuf->append (ANNOT_SOUND_AP_MIC);
    appearBuf->append ("Q\n");

    double bbox[4];
    bbox[0] = bbox[1] = 0;
    bbox[2] = bbox[3] = 24;
    if (ca == 1) {
      createForm(bbox, gFalse, NULL, &appearance);
    } else {
      Object aStream, resDict;

      createForm(bbox, gTrue, NULL, &aStream);
      delete appearBuf;

      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", ca, NULL, &resDict);
      createForm(bbox, gFalse, &resDict, &appearance);
    }
    delete appearBuf;
  }

  // draw the appearance stream
  appearance.fetch(gfx->getXRef(), &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rect->x2, rect->y2, getRotation());
  obj.free();
}

//------------------------------------------------------------------------
// Annot3D
//------------------------------------------------------------------------
Annot3D::Annot3D(PDFDoc *docA, PDFRectangle *rect) :
    Annot(docA, rect) {
  Object obj1;

  type = type3D;

  annotObj.dictSet ("Subtype", obj1.initName ("3D"));

  initialize(docA, annotObj.getDict());
}

Annot3D::Annot3D(PDFDoc *docA, Dict *dict, Object *obj) :
  Annot(docA, dict, obj) {
  type = type3D;
  initialize(docA, dict);
}

Annot3D::~Annot3D() {
  if (activation)
    delete activation;
}

void Annot3D::initialize(PDFDoc *docA, Dict* dict) {
  Object obj1;

  if (dict->lookup("3DA", &obj1)->isDict()) {
    activation = new Activation(obj1.getDict());
  } else {
    activation = NULL;
  }
  obj1.free();
}

Annot3D::Activation::Activation(Dict *dict) {
  Object obj1;

  if (dict->lookup("A", &obj1)->isName()) {
    const char *name = obj1.getName();

    if(!strcmp(name, "PO")) {
      aTrigger = aTriggerPageOpened;
    } else if(!strcmp(name, "PV")) {
      aTrigger = aTriggerPageVisible;
    } else if(!strcmp(name, "XA")) {
      aTrigger = aTriggerUserAction;
    } else {
      aTrigger = aTriggerUnknown;
    }
  } else {
    aTrigger = aTriggerUnknown;
  }
  obj1.free();

  if(dict->lookup("AIS", &obj1)->isName()) {
    const char *name = obj1.getName();

    if(!strcmp(name, "I")) {
      aState = aStateEnabled;
    } else if(!strcmp(name, "L")) {
      aState = aStateDisabled;
    } else {
      aState = aStateUnknown;
    }
  } else {
    aState = aStateUnknown;
  }
  obj1.free();

  if(dict->lookup("D", &obj1)->isName()) {
    const char *name = obj1.getName();

    if(!strcmp(name, "PC")) {
      dTrigger = dTriggerPageClosed;
    } else if(!strcmp(name, "PI")) {
      dTrigger = dTriggerPageInvisible;
    } else if(!strcmp(name, "XD")) {
      dTrigger = dTriggerUserAction;
    } else {
      dTrigger = dTriggerUnknown;
    }
  } else {
    dTrigger = dTriggerUnknown;
  }
  obj1.free();

  if(dict->lookup("DIS", &obj1)->isName()) {
    const char *name = obj1.getName();

    if(!strcmp(name, "U")) {
      dState = dStateUninstantiaded;
    } else if(!strcmp(name, "I")) {
      dState = dStateInstantiated;
    } else if(!strcmp(name, "L")) {
      dState = dStateLive;
    } else {
      dState = dStateUnknown;
    }
  } else {
    dState = dStateUnknown;
  }
  obj1.free();

  if (dict->lookup("TB", &obj1)->isBool()) {
    displayToolbar = obj1.getBool();
  } else {
    displayToolbar = gTrue;
  }
  obj1.free();

  if (dict->lookup("NP", &obj1)->isBool()) {
    displayNavigation = obj1.getBool();
  } else {
    displayNavigation = gFalse;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

Annots::Annots(PDFDoc *docA, int page, Object *annotsObj) {
  Annot *annot;
  Object obj1;
  int i;

  doc = docA;
  annots = NULL;
  size = 0;
  nAnnots = 0;

  if (annotsObj->isArray()) {
    for (i = 0; i < annotsObj->arrayGetLength(); ++i) {
      //get the Ref to this annot and pass it to Annot constructor 
      //this way, it'll be possible for the annot to retrieve the corresponding
      //form widget
      Object obj2;
      if (annotsObj->arrayGet(i, &obj1)->isDict()) {
        annotsObj->arrayGetNF(i, &obj2);
        annot = createAnnot (obj1.getDict(), &obj2);
        if (annot) {
          if (annot->isOk()) {
            annot->setPage(page, gFalse); // Don't change /P
            appendAnnot(annot);
          }
          annot->decRefCnt();
        }
      }
      obj2.free();
      obj1.free();
    }
  }
}

void Annots::appendAnnot(Annot *annot) {
  if (annot && annot->isOk()) {
    if (nAnnots >= size) {
      size += 16;
      annots = (Annot **)greallocn(annots, size, sizeof(Annot *));
    }
    annots[nAnnots++] = annot;
    annot->incRefCnt();
  }
}

GBool Annots::removeAnnot(Annot *annot) {
  int idx = -1;
  // Search annot and determine its index
  for (int i = 0; idx == -1 && i < nAnnots; i++) {
    if (annots[i] == annot) {
      idx = i;
    }
  }
  if (idx == -1) {
    return gFalse;
  } else {
    --nAnnots;
    memmove( annots + idx, annots + idx + 1, sizeof(annots[0]) * (nAnnots - idx) );
    annot->decRefCnt();
    return gTrue;
  }
}

Annot *Annots::createAnnot(Dict* dict, Object *obj) {
  Annot *annot = NULL;
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    const char *typeName = obj1.getName();

    if (!strcmp(typeName, "Text")) {
      annot = new AnnotText(doc, dict, obj);
    } else if (!strcmp(typeName, "Link")) {
      annot = new AnnotLink(doc, dict, obj);
    } else if (!strcmp(typeName, "FreeText")) {
      annot = new AnnotFreeText(doc, dict, obj);
    } else if (!strcmp(typeName, "Line")) {
      annot = new AnnotLine(doc, dict, obj);
    } else if (!strcmp(typeName, "Square")) {
      annot = new AnnotGeometry(doc, dict, obj);
    } else if (!strcmp(typeName, "Circle")) {
      annot = new AnnotGeometry(doc, dict, obj);
    } else if (!strcmp(typeName, "Polygon")) {
      annot = new AnnotPolygon(doc, dict, obj);
    } else if (!strcmp(typeName, "PolyLine")) {
      annot = new AnnotPolygon(doc, dict, obj);
    } else if (!strcmp(typeName, "Highlight")) {
      annot = new AnnotTextMarkup(doc, dict, obj);
    } else if (!strcmp(typeName, "Underline")) {
      annot = new AnnotTextMarkup(doc, dict, obj);
    } else if (!strcmp(typeName, "Squiggly")) {
      annot = new AnnotTextMarkup(doc, dict, obj);
    } else if (!strcmp(typeName, "StrikeOut")) {
      annot = new AnnotTextMarkup(doc, dict, obj);
    } else if (!strcmp(typeName, "Stamp")) {
      annot = new AnnotStamp(doc, dict, obj);
    } else if (!strcmp(typeName, "Caret")) {
      annot = new AnnotCaret(doc, dict, obj);
    } else if (!strcmp(typeName, "Ink")) {
      annot = new AnnotInk(doc, dict, obj);
    } else if (!strcmp(typeName, "FileAttachment")) {
      annot = new AnnotFileAttachment(doc, dict, obj);
    } else if (!strcmp(typeName, "Sound")) {
      annot = new AnnotSound(doc, dict, obj);
    } else if(!strcmp(typeName, "Movie")) {
      annot = new AnnotMovie(doc, dict, obj);
    } else if(!strcmp(typeName, "Widget")) {
      // Find the annot in forms
      if (obj->isRef()) {
        Form *form = doc->getCatalog()->getForm();
        if (form) {
          FormWidget *widget = form->findWidgetByRef(obj->getRef());
          if (widget) {
            annot = widget->getWidgetAnnotation();
            annot->incRefCnt();
          }
        }
      }
      if (!annot)
        annot = new AnnotWidget(doc, dict, obj);
    } else if(!strcmp(typeName, "Screen")) {
      annot = new AnnotScreen(doc, dict, obj);
    } else if(!strcmp(typeName, "PrinterMark")) {
      annot = new Annot(doc, dict, obj);
    } else if (!strcmp(typeName, "TrapNet")) {
      annot = new Annot(doc, dict, obj);
    } else if (!strcmp(typeName, "Watermark")) {
      annot = new Annot(doc, dict, obj);
    } else if (!strcmp(typeName, "3D")) {
      annot = new Annot3D(doc, dict, obj);
    } else if (!strcmp(typeName, "Popup")) {
      /* Popup annots are already handled by markup annots
       * Here we only care about popup annots without a
       * markup annotation associated
       */
      Object obj2;

      if (dict->lookup("Parent", &obj2)->isNull())
        annot = new AnnotPopup(doc, dict, obj);
      else
        annot = NULL;
      
      obj2.free();
    } else {
      annot = new Annot(doc, dict, obj);
    }
  }
  obj1.free();

  return annot;
}

Annot *Annots::findAnnot(Ref *ref) {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    if (annots[i]->match(ref)) {
      return annots[i];
    }
  }
  return NULL;
}


Annots::~Annots() {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    annots[i]->decRefCnt();
  }
  gfree(annots);
}
