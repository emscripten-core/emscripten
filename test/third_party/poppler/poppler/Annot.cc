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
// Copyright (C) 2007-2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007, 2008 Iñigo Martínez <inigomartinez@gmail.com>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008 Hugo Mercier <hmercier31@gmail.com>
// Copyright (C) 2009 Ilya Gorenbein <igorenbein@finjan.com>
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
#include "Annot.h"
#include "GfxFont.h"
#include "CharCodeToUnicode.h"
#include "PDFDocEncoding.h"
#include "Form.h"
#include "Error.h"
#include "Page.h"
#include "XRef.h"
#include "Movie.h"
#include "OptionalContent.h"
#include "Sound.h"
#include "FileSpec.h"
#include "DateInfo.h"
#include "Link.h"
#include <string.h>

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

// Ensures that x is between the limits set by low and high.
// If low is greater than high the result is undefined.
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

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

static AnnotExternalDataType parseAnnotExternalData(Dict* dict) {
  Object obj1;
  AnnotExternalDataType type;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString *typeName = new GooString(obj1.getName());

    if (!typeName->cmp("Markup3D")) {
      type = annotExternalDataMarkup3D;
    } else {
      type = annotExternalDataMarkupUnknown;
    }
    delete typeName;
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

//------------------------------------------------------------------------
// AnnotBorderEffect
//------------------------------------------------------------------------

AnnotBorderEffect::AnnotBorderEffect(Dict *dict) {
  Object obj1;

  if (dict->lookup("S", &obj1)->isName()) {
    GooString *effectName = new GooString(obj1.getName());

    if (!effectName->cmp("C"))
      effectType = borderEffectCloudy;
    else
      effectType = borderEffectNoEffect;
    delete effectName;
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
    error(-1, "Bad Annot Path");
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
          if (j % 2 == 1)
	    quadArray[j] = CLAMP (obj.getNum(), rect->y1, rect->y2);
          else
	    quadArray[j] = CLAMP (obj.getNum(), rect->x1, rect->x2);
        } else {
            correct = gFalse;
	    obj.free();
	    error (-1, "Invalid QuadPoint in annot");
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

    // TODO: check not all zero ? (Line Dash Pattern Page 217 PDF 8.1)
    if (arrayLength == 4) {
      if (array->get(3, &obj1)->isArray()) {
        Array *dashPattern = obj1.getArray();
        int tempLength = dashPattern->getLength();
        double *tempDash = (double *) gmallocn (tempLength, sizeof (double));

        for(int i = 0; i < tempLength && i < DASH_LIMIT && correct; i++) {

          if (dashPattern->get(i, &obj1)->isNum()) {
            tempDash[i] = obj1.getNum();

            if (tempDash[i] < 0)
              correct = gFalse;

          } else {
            correct = gFalse;
          }
          obj1.free();
        }

        if (correct) {
          dashLength = tempLength;
          dash = tempDash;
          style = borderDashed;
        } else {
          gfree (tempDash);
        }
      } else {
        correct = gFalse;
      }
      obj1.free();
    }
  } else {
    correct = gFalse;
  }
  
  if (!correct) {
    width = 0;
  }
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
    GooString *styleName = new GooString(obj2.getName());

    width = obj1.getNum();

    if (!styleName->cmp("S")) {
      style = borderSolid;
    } else if (!styleName->cmp("D")) {
      style = borderDashed;
    } else if (!styleName->cmp("B")) {
      style = borderBeveled;
    } else if (!styleName->cmp("I")) {
      style = borderInset;
    } else if (!styleName->cmp("U")) {
      style = borderUnderlined;
    } else {
      style = borderSolid;
    }
    delete styleName;
  } else {
    width = 0;
  }
  obj2.free();
  obj1.free();

  // TODO: check not all zero (Line Dash Pattern Page 217 PDF 8.1)
  if (dict->lookup("D", &obj1)->isArray()) {
    GBool correct = gTrue;
    int tempLength = obj1.arrayGetLength();
    double *tempDash = (double *) gmallocn (tempLength, sizeof (double));

    for(int i = 0; i < tempLength && correct; i++) {
      Object obj2;

      if (obj1.arrayGet(i, &obj2)->isNum()) {
        tempDash[i] = obj2.getNum();

        if (tempDash[i] < 0)
          correct = gFalse;
      } else {
        correct = gFalse;
      }
      obj2.free();
    }

    if (correct) {
      dashLength = tempLength;
      dash = tempDash;
      style = borderDashed;
    } else {
      gfree (tempDash);
    }

  }

  if (!dash) {
    dashLength = 1;
    dash = (double *) gmallocn (dashLength, sizeof (double));
    dash[0] = 3;
  }
  obj1.free();
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

//------------------------------------------------------------------------
// AnnotBorderStyle
//------------------------------------------------------------------------

AnnotBorderStyle::AnnotBorderStyle(AnnotBorderType typeA, double widthA,
				   double *dashA, int dashLengthA,
				   double rA, double gA, double bA) {
  type = typeA;
  width = widthA;
  dash = dashA;
  dashLength = dashLengthA;
  r = rA;
  g = gA;
  b = bA;
}

AnnotBorderStyle::~AnnotBorderStyle() {
  if (dash) {
    gfree(dash);
  }
}

//------------------------------------------------------------------------
// AnnotIconFit
//------------------------------------------------------------------------

AnnotIconFit::AnnotIconFit(Dict* dict) {
  Object obj1;

  if (dict->lookup("SW", &obj1)->isName()) {
    GooString *scaleName = new GooString(obj1.getName());

    if(!scaleName->cmp("B")) {
      scaleWhen = scaleBigger;
    } else if(!scaleName->cmp("S")) {
      scaleWhen = scaleSmaller;
    } else if(!scaleName->cmp("N")) {
      scaleWhen = scaleNever;
    } else {
      scaleWhen = scaleAlways;
    }
    delete scaleName;
  } else {
    scaleWhen = scaleAlways;
  }
  obj1.free();

  if (dict->lookup("S", &obj1)->isName()) {
    GooString *scaleName = new GooString(obj1.getName());

    if(!scaleName->cmp("A")) {
      scale = scaleAnamorphic;
    } else {
      scale = scaleProportional;
    }
    delete scaleName;
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

  if (dict->lookup("CA", &obj1)->isName()) {
    normalCaption = new GooString(obj1.getName());
  } else {
    normalCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("RC", &obj1)->isName()) {
    rolloverCaption = new GooString(obj1.getName());
  } else {
    rolloverCaption = NULL;
  }
  obj1.free();

  if (dict->lookup("AC", &obj1)->isName()) {
    alternateCaption = new GooString(obj1.getName());
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
// Annot
//------------------------------------------------------------------------

Annot::Annot(XRef *xrefA, PDFRectangle *rectA, Catalog *catalog) {
  Object obj1;

  flags = flagUnknown;
  type = typeUnknown;

  obj1.initArray (xrefA);
  Object obj2;
  obj1.arrayAdd (obj2.initReal (rectA->x1));
  obj1.arrayAdd (obj2.initReal (rectA->y1));
  obj1.arrayAdd (obj2.initReal (rectA->x2));
  obj1.arrayAdd (obj2.initReal (rectA->y2));
  obj2.free ();

  annotObj.initDict (xrefA);
  annotObj.dictSet ("Type", obj2.initName ("Annot"));
  annotObj.dictSet ("Rect", &obj1);
  // obj1 is owned by the dict

  ref = xrefA->addIndirectObject (&annotObj);

  initialize (xrefA, annotObj.getDict(), catalog);
}

Annot::Annot(XRef *xrefA, Dict *dict, Catalog* catalog) {
  hasRef = false;
  flags = flagUnknown;
  type = typeUnknown;
  annotObj.initDict (dict);
  initialize (xrefA, dict, catalog);
}

Annot::Annot(XRef *xrefA, Dict *dict, Catalog* catalog, Object *obj) {
  if (obj->isRef()) {
    hasRef = gTrue;
    ref = obj->getRef();
  } else {
    hasRef = gFalse;
  }
  flags = flagUnknown;
  type = typeUnknown;
  annotObj.initDict (dict);
  initialize (xrefA, dict, catalog);
}

void Annot::initialize(XRef *xrefA, Dict *dict, Catalog *catalog) {
  Object asObj, obj1, obj2, obj3;

  appRef.num = 0;
  appRef.gen = 65535;
  ok = gTrue;
  xref = xrefA;
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
    error(-1, "Bad bounding box for annotation");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Contents", &obj1)->isString()) {
    contents = obj1.getString()->copy();
  } else {
    contents = NULL;
  }
  obj1.free();

  if (dict->lookupNF("P", &obj1)->isRef()) {
    Ref ref = obj1.getRef();

    page = catalog ? catalog->findPage (ref.num, ref.gen) : -1;
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

  if (dict->lookup("AP", &obj1)->isDict()) {
    Object obj2;

    if (dict->lookup("AS", &obj2)->isName()) {
      Object obj3;

      appearState = new GooString(obj2.getName());
      if (obj1.dictLookup("N", &obj3)->isDict()) {
        Object obj4;

        if (obj3.dictLookupNF(appearState->getCString(), &obj4)->isRef()) {
          obj4.copy(&appearance);
        } else {
          obj4.free();
          if (obj3.dictLookupNF("Off", &obj4)->isRef()) {
            obj4.copy(&appearance);
          }
        } 
        obj4.free();
      }
      obj3.free();
    } else {
      obj2.free();

      appearState = NULL;
      if (obj1.dictLookupNF("N", &obj2)->isRef()) {
        obj2.copy(&appearance);
      }
    }
    obj2.free();
  } else {
    appearState = NULL;
  }
  obj1.free();

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

  optContentConfig = catalog ? catalog->getOptContentConfig() : NULL;
  dict->lookupNF("OC", &oc);
  if (!oc.isRef() && !oc.isNull()) {
    error (-1, "Annotation OC value not null or dict: %i", oc.getType());
  }
}

void Annot::update(const char *key, Object *value) {
  /* Set M to current time */
  delete modified;
  modified = timeToDateString(NULL);

  Object obj1;
  obj1.initString (modified->copy());
  annotObj.dictSet("M", &obj1);

  annotObj.dictSet(const_cast<char*>(key), value);
  
  xref->setModifiedObject(&annotObj, ref);
}

void Annot::setContents(GooString *new_content) {
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

void Annot::setColor(AnnotColor *new_color) {
  delete color;

  if (new_color) {
    Object obj1, obj2;
    const double *values = new_color->getValues();

    obj1.initArray(xref);
    for (int i = 0; i < (int)new_color->getSpace(); i++)
      obj1.arrayAdd(obj2.initReal (values[i]));
    update ("C", &obj1);
    color = new_color;
  } else {
    color = NULL;
  }
}

void Annot::setPage(Ref *pageRef, int pageIndex)
{
  Object obj1;

  obj1.initRef(pageRef->num, pageRef->gen);
  update("P", &obj1);
  page = pageIndex;
}

double Annot::getXMin() {
  return rect->x1;
}

double Annot::getYMin() {
  return rect->y1;
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

Annot::~Annot() {
  annotObj.free();
  
  delete rect;
  
  if (contents)
    delete contents;

  if (name)
    delete name;

  if (modified)
    delete modified;

  appearance.free();

  if (appearState)
    delete appearState;

  if (border)
    delete border;

  if (color)
    delete color;

  oc.free();
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

void Annot::createResourcesDict(char *formName, Object *formStream,
				char *stateName,
				double opacity,	char *blendMode,
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

GBool Annot::isVisible(GBool printing) {
  // check the flags
  if ((flags & flagHidden) ||
      (printing && !(flags & flagPrint)) ||
      (!printing && (flags & flagNoView))) {
    return gFalse;
  }

  // check the OC
  if (optContentConfig && oc.isRef()) {
    if (! optContentConfig->optContentIsVisible(&oc))
      return gFalse;
  }

  return gTrue;
}

void Annot::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
      rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotPopup
//------------------------------------------------------------------------

AnnotPopup::AnnotPopup(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typePopup;

  annotObj.dictSet ("Subtype", obj1.initName ("Popup"));
  initialize (xrefA, annotObj.getDict(), catalog);
}

AnnotPopup::AnnotPopup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {
  type = typePopup;
  initialize(xrefA, dict, catalog);
}

AnnotPopup::~AnnotPopup() {
  parent.free();
}

void AnnotPopup::initialize(XRef *xrefA, Dict *dict, Catalog *catalog) {
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
AnnotMarkup::AnnotMarkup(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  initialize(xrefA, annotObj.getDict(), catalog, &annotObj);
}

AnnotMarkup::AnnotMarkup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {
  initialize(xrefA, dict, catalog, obj);
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

void AnnotMarkup::initialize(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) {
  Object obj1;

  if (dict->lookup("T", &obj1)->isString()) {
    label = obj1.getString()->copy();
  } else {
    label = NULL;
  }
  obj1.free();

  if (dict->lookup("Popup", &obj1)->isDict()) {
    popup = new AnnotPopup(xrefA, obj1.getDict(), catalog, obj);
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
    GooString *replyName = new GooString(obj1.getName());

    if (!replyName->cmp("R")) {
      replyTo = replyTypeR;
    } else if (!replyName->cmp("Group")) {
      replyTo = replyTypeGroup;
    } else {
      replyTo = replyTypeR;
    }
    delete replyName;
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
}

//------------------------------------------------------------------------
// AnnotText
//------------------------------------------------------------------------

AnnotText::AnnotText(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeText;
  flags |= flagNoZoom | flagNoRotate;

  annotObj.dictSet ("Subtype", obj1.initName ("Text"));
  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotText::AnnotText(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    AnnotMarkup(xrefA, dict, catalog, obj) {

  type = typeText;
  flags |= flagNoZoom | flagNoRotate;
  initialize (xrefA, catalog, dict);
}

AnnotText::~AnnotText() {
  delete icon;
}

void AnnotText::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
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

  double rectx2 = rect->x2;
  double recty2 = rect->y2;
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

    rectx2 = rect->x1 + 24;
    recty2 = rect->y1 + 24;
  }

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rectx2, recty2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotLink
//------------------------------------------------------------------------
AnnotLink::AnnotLink(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typeLink;
  annotObj.dictSet ("Subtype", obj1.initName ("Link"));
  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotLink::AnnotLink(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {

  type = typeLink;
  initialize (xrefA, catalog, dict);
}

AnnotLink::~AnnotLink() {
  /*
  if (actionDict)
    delete actionDict;
  */
  dest.free();
  /*
  if (uriAction)
    delete uriAction;
  */
  if (quadrilaterals)
    delete quadrilaterals;
}

void AnnotLink::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;
  /*
  if (dict->lookup("A", &obj1)->isDict()) {
    actionDict = NULL;
  } else {
    actionDict = NULL;
  }
  obj1.free();
  */
  dict->lookup("Dest", &dest);
  if (dict->lookup("H", &obj1)->isName()) {
    GooString *effect = new GooString(obj1.getName());

    if (!effect->cmp("N")) {
      linkEffect = effectNone;
    } else if (!effect->cmp("I")) {
      linkEffect = effectInvert;
    } else if (!effect->cmp("O")) {
      linkEffect = effectOutline;
    } else if (!effect->cmp("P")) {
      linkEffect = effectPush;
    } else {
      linkEffect = effectInvert;
    }
    delete effect;
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

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotFreeText
//------------------------------------------------------------------------
AnnotFreeText::AnnotFreeText(XRef *xrefA, PDFRectangle *rect, GooString *da, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeFreeText;

  annotObj.dictSet ("Subtype", obj1.initName ("FreeText"));

  Object obj2;
  obj2.initString (da->copy());
  annotObj.dictSet("DA", &obj2);

  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotFreeText::AnnotFreeText(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeFreeText;
  initialize(xrefA, catalog, dict);
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

void AnnotFreeText::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if (dict->lookup("DA", &obj1)->isString()) {
    appearanceString = obj1.getString()->copy();
  } else {
    appearanceString = new GooString();
    error(-1, "Bad appearance for annotation");
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
    GooString *intentName = new GooString(obj1.getName());

    if (!intentName->cmp("FreeText")) {
      intent = intentFreeText;
    } else if (!intentName->cmp("FreeTextCallout")) {
      intent = intentFreeTextCallout;
    } else if (!intentName->cmp("FreeTextTypeWriter")) {
      intent = intentFreeTextTypeWriter;
    } else {
      intent = intentFreeText;
    }
    delete intentName;
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
    GooString *styleName = new GooString(obj1.getName());
    endStyle = parseAnnotLineEndingStyle(styleName);
    delete styleName;
  } else {
    endStyle = annotLineEndingNone;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotLine
//------------------------------------------------------------------------

AnnotLine::AnnotLine(XRef *xrefA, PDFRectangle *rect, PDFRectangle *lRect, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeLine;
  annotObj.dictSet ("Subtype", obj1.initName ("Line"));

  Object obj2, obj3;
  obj2.initArray (xrefA);
  obj2.arrayAdd (obj3.initReal (lRect->x1));
  obj2.arrayAdd (obj3.initReal (lRect->y1));
  obj2.arrayAdd (obj3.initReal (lRect->x2));
  obj2.arrayAdd (obj3.initReal (lRect->y2));
  annotObj.dictSet ("L", &obj2);

  initialize (xrefA, catalog, annotObj.getDict());
}

AnnotLine::AnnotLine(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeLine;
  initialize(xrefA, catalog, dict);
}

AnnotLine::~AnnotLine() {
  delete coord1;
  delete coord2;

  if (interiorColor)
    delete interiorColor;

  if (measure)
    delete measure;
}

void AnnotLine::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
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
    GooString *intentName = new GooString(obj1.getName());

    if(!intentName->cmp("LineArrow")) {
      intent = intentLineArrow;
    } else if(!intentName->cmp("LineDimension")) {
      intent = intentLineDimension;
    } else {
      intent = intentLineArrow;
    }
    delete intentName;
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
    GooString *captionName = new GooString(obj1.getName());

    if(!captionName->cmp("Inline")) {
      captionPos = captionPosInline;
    } else if(!captionName->cmp("Top")) {
      captionPos = captionPosTop;
    } else {
      captionPos = captionPosInline;
    }
    delete captionName;
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

void AnnotLine::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

  /* Some documents like pdf_commenting_new.pdf,
   * have y1 = y2 but line_width > 0, acroread
   * renders the lines in such cases even though
   * the annot bbox is empty. We adjust the bbox here
   * to avoid having an empty bbox so that lines
   * are rendered
   */
  if (rect->y1 == rect->y2)
    rect->y2 += border ? border->getWidth() : 1;

  if (appearance.isNull()) {
    ca = opacity;

    appearBuf = new GooString ();
    appearBuf->append ("q\n");
    if (color)
      setColor(color, gFalse);

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
    }
    appearBuf->appendf ("{0:.2f} {1:.2f} m\n", coord1->getX() - rect->x1, coord1->getY() - rect->y1);
    appearBuf->appendf ("{0:.2f} {1:.2f} l\n", coord2->getX() - rect->x1, coord2->getY() - rect->y1);
    // TODO: Line ending, caption, leader lines
    appearBuf->append ("S\n");
    appearBuf->append ("Q\n");

    double bbox[4];
    bbox[0] = bbox[1] = 0;
    bbox[2] = rect->x2 - rect->x1;
    bbox[3] = rect->y2 - rect->y1;
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
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotTextMarkup
//------------------------------------------------------------------------
AnnotTextMarkup::AnnotTextMarkup(XRef *xrefA, PDFRectangle *rect, AnnotSubtype subType,
				 AnnotQuadrilaterals *quadPoints, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
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

  Object obj2;
  obj2.initArray (xrefA);

  for (int i = 0; i < quadPoints->getQuadrilateralsLength(); ++i) {
    Object obj3;

    obj2.arrayAdd (obj3.initReal (quadPoints->getX1(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY1(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getX2(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY2(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getX3(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY3(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getX4(i)));
    obj2.arrayAdd (obj3.initReal (quadPoints->getY4(i)));
  }

  annotObj.dictSet ("QuadPoints", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotTextMarkup::AnnotTextMarkup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  // the real type will be read in initialize()
  type = typeHighlight;
  initialize(xrefA, catalog, dict);
}

void AnnotTextMarkup::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
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
    error(-1, "Bad Annot Text Markup QuadPoints");
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



void AnnotTextMarkup::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;
  int i;
  Object obj1, obj2;

  if (!isVisible (printing))
    return;

  if (appearance.isNull() || type == typeHighlight) {
    ca = opacity;

    appearBuf = new GooString ();

    switch (type) {
    case typeUnderline:
      if (color) {
        setColor(color, gFalse);
	setColor(color, gTrue);
      }

      for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
        double x1, y1, x2, y2, x3, y3;
	double x, y;

	x1 = quadrilaterals->getX1(i);
	y1 = quadrilaterals->getY1(i);
	x2 = quadrilaterals->getX2(i);
	y2 = quadrilaterals->getY2(i);
	x3 = quadrilaterals->getX3(i);
	y3 = quadrilaterals->getY3(i);

	x = x1 - rect->x1;
	y = y3 - rect->y1;
	appearBuf->append ("[]0 d 2 w\n");
	appearBuf->appendf ("{0:.2f} {1:.2f} m\n", x, y);
	appearBuf->appendf ("{0:.2f} {1:.2f} l\n", x + (x2 - x1), y);
	appearBuf->append ("S\n");
      }
      break;
    case typeStrikeOut:
      if (color) {
        setColor(color, gFalse);
	setColor(color, gTrue);
      }

      for (i = 0; i < quadrilaterals->getQuadrilateralsLength(); ++i) {
        double x1, y1, x2, y2, x3, y3;
	double x, y;
	double h2;

	x1 = quadrilaterals->getX1(i);
	y1 = quadrilaterals->getY1(i);
	x2 = quadrilaterals->getX2(i);
	y2 = quadrilaterals->getY2(i);
	x3 = quadrilaterals->getX3(i);
	y3 = quadrilaterals->getY3(i);
	h2 = (y1 - y3) / 2.0;

	x = x1 - rect->x1;
	y = (y3 - rect->y1) + h2;
	appearBuf->append ("[]0 d 2 w\n");
	appearBuf->appendf ("{0:.2f} {1:.2f} m\n", x, y);
	appearBuf->appendf ("{0:.2f} {1:.2f} l\n", x + (x2 - x1), y);
	appearBuf->append ("S\n");
      }
      break;
    case typeSquiggly:
      // TODO
    default:
    case typeHighlight:
      appearance.free();

      if (color)
        setColor(color, gTrue);

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
	h4 = abs(y1 - y3) / 4.0;

	appearBuf->appendf ("{0:.2f} {1:.2f} m\n", x3, y3);
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x3 - h4, y3 + h4, x1 - h4, y1 - h4, x1, y1);
	appearBuf->appendf ("{0:.2f} {1:.2f} l\n", x2, y2);
	appearBuf->appendf ("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
			    x2 + h4, y2 - h4, x4 + h4, y4 + h4, x4, y4);
	appearBuf->append ("f\n");
      }

      Object aStream, resDict;
      double bbox[4];
      bbox[0] = rect->x1;
      bbox[1] = rect->y1;
      bbox[2] = rect->x2;
      bbox[3] = rect->y2;
      createForm(bbox, gTrue, NULL, &aStream);
      delete appearBuf;

      appearBuf = new GooString ("/GS0 gs\n/Fm0 Do");
      createResourcesDict("Fm0", &aStream, "GS0", 1, "Multiply", &resDict);
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
      break;
    }
  }

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotWidget
//------------------------------------------------------------------------

AnnotWidget::AnnotWidget(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, dict, catalog, obj) {
  type = typeWidget;
  widget = NULL;
  initialize(xrefA, catalog, dict);
}

AnnotWidget::~AnnotWidget() {
  if (appearCharacs)
    delete appearCharacs;
  
  if (action)
    delete action;
    
  if (additionActions)
    delete additionActions;
    
  if (parent)
    delete parent;
}

void AnnotWidget::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if ((form = catalog->getForm ())) {
    widget = form->findWidgetByRef (ref);

    // check if field apperances need to be regenerated
    // Only text or choice fields needs to have appearance regenerated
    // see section 8.6.2 "Variable Text" of PDFReference
    regen = gFalse;
    if (widget != NULL && (widget->getType () == formText || widget->getType () == formChoice)) {
      regen = form->getNeedAppearances ();
    }
  }

  // If field doesn't have an AP we'll have to generate it
  if (appearance.isNone () || appearance.isNull ())
    regen = gTrue;

  if(dict->lookup("H", &obj1)->isName()) {
    GooString *modeName = new GooString(obj1.getName());

    if(!modeName->cmp("N")) {
      mode = highlightModeNone;
    } else if(!modeName->cmp("O")) {
      mode = highlightModeOutline;
    } else if(!modeName->cmp("P") || !modeName->cmp("T")) {
      mode = highlightModePush;
    } else {
      mode = highlightModeInvert;
    }
    delete modeName;
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

  if(dict->lookup("A", &obj1)->isDict()) {
    action = NULL;
  } else {
    action = NULL;
  }
  obj1.free();

  if(dict->lookup("AA", &obj1)->isDict()) {
    additionActions = NULL;
  } else {
    additionActions = NULL;
  }
  obj1.free();

  if(dict->lookup("Parent", &obj1)->isDict()) {
    parent = NULL;
  } else {
    parent = NULL;
  }
  obj1.free();
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
void AnnotWidget::layoutText(GooString *text, GooString *outBuf, int *i,
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
    error(-1, "AnnotWidget::layoutText, bad unicode string");
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
        fprintf(stderr,
                "warning: layoutText: cannot convert U+%04X\n", uChar);
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
void AnnotWidget::writeString(GooString *str, GooString *appearBuf)
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
void AnnotWidget::drawText(GooString *text, GooString *da, GfxFontDict *fontDict,
    GBool multiline, int comb, int quadding,
    GBool txField, GBool forceZapfDingbats,
    GBool password) {
  GooList *daToks;
  GooString *tok, *convertedText;
  GfxFont *font;
  double fontSize, fontSize2, borderWidth, x, xPrev, y, w, wMax;
  int tfPos, tmPos, i, j;
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
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
        if (forceZapfDingbats) {
          // We are forcing ZaDb but the font does not exist
          // so create a fake one
          Ref r; // dummy Ref, it's not used at all in this codepath
          r.num = 0;
          r.gen = 0;
          Dict *d = new Dict(xref);
          font = new Gfx8BitFont(xref, "ZaDb", r, new GooString("ZapfDingbats"), fontType1, d);
          delete d;
          freeFont = gTrue;
          addDingbatsResource = gTrue;
        } else {
          error(-1, "Unknown font in field's DA string");
        }
      }
    } else {
      error(-1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = gatof(tok->getCString());
  } else {
    error(-1, "Missing 'Tf' operator in field's DA string");
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
  appearBuf->append("BT\n");
  // multi-line text
  if (multiline) {
    // note: the comb flag is ignored in multiline mode

    wMax = rect->x2 - rect->x1 - 2 * borderWidth - 4;

    // compute font autosize
    if (fontSize == 0) {
      for (fontSize = 20; fontSize > 1; --fontSize) {
        y = rect->y2 - rect->y1;
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
    y = rect->y2 - rect->y1;

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
        case fieldQuadLeft:
        default:
          x = borderWidth + 2;
          break;
        case fieldQuadCenter:
          x = (rect->x2 - rect->x1 - w) / 2;
          break;
        case fieldQuadRight:
          x = rect->x2 - rect->x1 - borderWidth - 2 - w;
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
      w = (rect->x2 - rect->x1 - 2 * borderWidth) / comb;

      // compute font autosize
      if (fontSize == 0) {
        fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
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
        case fieldQuadLeft:
        default:
          x = borderWidth;
          break;
        case fieldQuadCenter:
          x = borderWidth + (comb - charCount) / 2 * w;
          break;
        case fieldQuadRight:
          x = borderWidth + (comb - charCount) * w;
          break;
      }
      y = 0.5 * (rect->y2 - rect->y1) - 0.4 * fontSize;

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
        fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
        fontSize2 = (rect->x2 - rect->x1 - 4 - 2 * borderWidth) / w;
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
        case fieldQuadLeft:
        default:
          x = borderWidth + 2;
          break;
        case fieldQuadCenter:
          x = (rect->x2 - rect->x1 - w) / 2;
          break;
        case fieldQuadRight:
          x = rect->x2 - rect->x1 - borderWidth - 2 - w;
          break;
      }
      y = 0.5 * (rect->y2 - rect->y1) - 0.4 * fontSize;

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
void AnnotWidget::drawListBox(GooString **text, GBool *selection,
			      int nOptions, int topIdx,
			      GooString *da, GfxFontDict *fontDict, int quadding) {
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
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
        error(-1, "Unknown font in field's DA string");
      }
    } else {
      error(-1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = gatof(tok->getCString());
  } else {
    error(-1, "Missing 'Tf' operator in field's DA string");
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
    for (i = 0; i < nOptions; ++i) {
      j = 0;
      layoutText(text[i], convertedText, &j, font, &w, 0.0, NULL, gFalse);
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
  for (i = topIdx; i < nOptions; ++i) {
    // setup
    appearBuf->append("q\n");

    // draw the background if selected
    if (selection[i]) {
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
    layoutText(text[i], convertedText, &j, font, &w, 0.0, NULL, gFalse);
    w *= fontSize;
    switch (quadding) {
      case fieldQuadLeft:
      default:
        x = borderWidth + 2;
        break;
      case fieldQuadCenter:
        x = (rect->x2 - rect->x1 - w) / 2;
        break;
      case fieldQuadRight:
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
    if (selection[i]) {
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

void AnnotWidget::generateFieldAppearance() {
  Object mkObj, ftObj, appearDict, drObj, obj1, obj2, obj3;
  Dict *field;
  Dict *annot;
  Dict *acroForm;
  Dict *mkDict;
  MemStream *appearStream;
  GfxFontDict *fontDict;
  GBool hasCaption;
  double w, dx, dy, r;
  double *dash;
  GooString *caption, *da;
  GooString **text;
  GBool *selection;
  int dashLength, ff, quadding, comb, nOptions, topIdx, i, j;
  GBool modified;
  AnnotColor aColor;

  if (widget == NULL || !widget->getField () || !widget->getField ()->getObj ()->isDict ())
    return;

  field = widget->getField ()->getObj ()->getDict ();
  annot = widget->getObj ()->getDict ();
  acroForm = form->getObj ()->getDict ();
  
  // do not regenerate appearence if widget has not changed
  modified = widget->isModified ();

  // only regenerate when it doesn't have an AP or
  // it already has an AP but widget has been modified
  if (!regen && !modified) {
    return;
  }

  appearBuf = new GooString ();
  // get the appearance characteristics (MK) dictionary
  if (annot->lookup("MK", &mkObj)->isDict()) {
    mkDict = mkObj.getDict();
  } else {
    mkDict = NULL;
  }
  // draw the background
  if (mkDict) {
    if (mkDict->lookup("BG", &obj1)->isArray() &&
        obj1.arrayGetLength() > 0) {
      AnnotColor aColor = AnnotColor (obj1.getArray());
      setColor(&aColor, gTrue);
      appearBuf->appendf("0 0 {0:.2f} {1:.2f} re f\n",
          rect->x2 - rect->x1, rect->y2 - rect->y1);
    }
    obj1.free();
  }

  // get the field type
  Form::fieldLookup(field, "FT", &ftObj);

  // get the field flags (Ff) value
  if (Form::fieldLookup(field, "Ff", &obj1)->isInt()) {
    ff = obj1.getInt();
  } else {
    ff = 0;
  }
  obj1.free();

  // draw the border
  if (mkDict && border) {
    w = border->getWidth();
    if (w > 0) {
      mkDict->lookup("BC", &obj1);
      if (!(obj1.isArray() && obj1.arrayGetLength() > 0)) {
        mkDict->lookup("BG", &obj1);
      }
      if (obj1.isArray() && obj1.arrayGetLength() > 0) {
        dx = rect->x2 - rect->x1;
        dy = rect->y2 - rect->y1;

        // radio buttons with no caption have a round border
        hasCaption = mkDict->lookup("CA", &obj2)->isString();
        obj2.free();
        if (ftObj.isName("Btn") && (ff & fieldFlagRadio) && !hasCaption) {
          r = 0.5 * (dx < dy ? dx : dy);
          switch (border->getStyle()) {
            case AnnotBorder::borderDashed:
              appearBuf->append("[");
              dashLength = border->getDashLength();
              dash = border->getDash();
              for (i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
              }
              appearBuf->append("] 0 d\n");
              // fall through to the solid case
            case AnnotBorder::borderSolid:
            case AnnotBorder::borderUnderlined:
              appearBuf->appendf("{0:.2f} w\n", w);
	      aColor = AnnotColor (obj1.getArray());
              setColor(&aColor, gFalse);
              drawCircle(0.5 * dx, 0.5 * dy, r - 0.5 * w, gFalse);
              break;
            case AnnotBorder::borderBeveled:
            case AnnotBorder::borderInset:
              appearBuf->appendf("{0:.2f} w\n", 0.5 * w);
	      aColor = AnnotColor (obj1.getArray());
              setColor(&aColor, gFalse);
              drawCircle(0.5 * dx, 0.5 * dy, r - 0.25 * w, gFalse);
	      aColor = AnnotColor (obj1.getArray(),
				   border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
              setColor(&aColor, gFalse);
              drawCircleTopLeft(0.5 * dx, 0.5 * dy, r - 0.75 * w);
	      aColor = AnnotColor (obj1.getArray(),
				   border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
              setColor(&aColor, gFalse);
              drawCircleBottomRight(0.5 * dx, 0.5 * dy, r - 0.75 * w);
              break;
          }

        } else {
          switch (border->getStyle()) {
            case AnnotBorder::borderDashed:
              appearBuf->append("[");
              dashLength = border->getDashLength();
              dash = border->getDash();
              for (i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
              }
              appearBuf->append("] 0 d\n");
              // fall through to the solid case
            case AnnotBorder::borderSolid:
              appearBuf->appendf("{0:.2f} w\n", w);
	      aColor = AnnotColor (obj1.getArray());
              setColor(&aColor, gFalse);
              appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re s\n",
                  0.5 * w, dx - w, dy - w);
              break;
            case AnnotBorder::borderBeveled:
            case AnnotBorder::borderInset:
	      aColor = AnnotColor (obj1.getArray(),
				   border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
	      setColor(&aColor, gTrue);
              appearBuf->append("0 0 m\n");
              appearBuf->appendf("0 {0:.2f} l\n", dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", w, dy - w);
              appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
              appearBuf->append("f\n");
	      aColor = AnnotColor (obj1.getArray(),
				   border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
              setColor(&aColor, gTrue);
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
	      aColor = AnnotColor (obj1.getArray());
              setColor(&aColor, gFalse);
              appearBuf->appendf("0 0 m {0:.2f} 0 l s\n", dx);
              break;
          }

          // clip to the inside of the border
          appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n",
              w, dx - 2 * w, dy - 2 * w);
        }
      }
      obj1.free();
    }
  }

  // get the resource dictionary
  acroForm->lookup("DR", &drObj);

  // build the font dictionary
  if (drObj.isDict() && drObj.dictLookup("Font", &obj1)->isDict()) {
    fontDict = new GfxFontDict(xref, NULL, obj1.getDict());
  } else {
    fontDict = NULL;
  }
  obj1.free();

  // get the default appearance string
  if (Form::fieldLookup(field, "DA", &obj1)->isNull()) {
    obj1.free();
    acroForm->lookup("DA", &obj1);
  }
  if (obj1.isString()) {
    da = obj1.getString()->copy();
    //TODO: look for a font size / name HERE
    // => create a function
  } else {
    da = NULL;
  }
  obj1.free();

  // draw the field contents
  if (ftObj.isName("Btn")) {
    caption = NULL;
    if (mkDict) {
      if (mkDict->lookup("CA", &obj1)->isString()) {
        caption = obj1.getString()->copy();
      }
      obj1.free();
    }
    // radio button
    if (ff & fieldFlagRadio) {
      //~ Acrobat doesn't draw a caption if there is no AP dict (?)
      if (Form::fieldLookup(field, "V", &obj1)->isName()) {
        if (annot->lookup("AS", &obj2)->isName(obj1.getName()) &&
	    strcmp (obj1.getName(), "Off") != 0) {
          if (caption) {
            drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
                gFalse, gTrue);
          } else {
            if (mkDict) {
              if (mkDict->lookup("BC", &obj3)->isArray() &&
                  obj3.arrayGetLength() > 0) {
                dx = rect->x2 - rect->x1;
                dy = rect->y2 - rect->y1;
		aColor = AnnotColor (obj3.getArray());
                setColor(&aColor, gTrue);
                drawCircle(0.5 * dx, 0.5 * dy, 0.2 * (dx < dy ? dx : dy),
                    gTrue);
              }
              obj3.free();
            }
          }
        }
        obj2.free();
      }
      obj1.free();
      // pushbutton
    } else if (ff & fieldFlagPushbutton) {
      if (caption) {
        drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
            gFalse, gFalse);
      }
      // checkbox
    } else {
      if (annot->lookup("AS", &obj1)->isName() &&
          strcmp(obj1.getName(), "Off") != 0) {
        if (!caption) {
          caption = new GooString("3"); // ZapfDingbats checkmark
        }
        drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
            gFalse, gTrue);
      }
      obj1.free();
    }
    if (caption) {
      delete caption;
    }
  } else if (ftObj.isName("Tx")) {
    if (Form::fieldLookup(field, "V", &obj1)->isString()) {
      if (Form::fieldLookup(field, "Q", &obj2)->isInt()) {
        quadding = obj2.getInt();
      } else {
        quadding = fieldQuadLeft;
      }
      obj2.free();
      comb = 0;
      if (ff & fieldFlagComb) {
        if (Form::fieldLookup(field, "MaxLen", &obj2)->isInt()) {
          comb = obj2.getInt();
        }
        obj2.free();
      }
      drawText(obj1.getString(), da, fontDict,
          ff & fieldFlagMultiline, comb, quadding, gTrue, gFalse, ff & fieldFlagPassword);
    }
    obj1.free();
  } else if (ftObj.isName("Ch")) {
    if (Form::fieldLookup(field, "Q", &obj1)->isInt()) {
      quadding = obj1.getInt();
    } else {
      quadding = fieldQuadLeft;
    }
    obj1.free();
    // combo box
    if (ff & fieldFlagCombo) {
      if (Form::fieldLookup(field, "V", &obj1)->isString()) {
        drawText(obj1.getString(), da, fontDict,
            gFalse, 0, quadding, gTrue, gFalse);
        //~ Acrobat draws a popup icon on the right side
      }
      obj1.free();
      // list box
    } else {
      if (field->lookup("Opt", &obj1)->isArray()) {
        nOptions = obj1.arrayGetLength();
        // get the option text
        text = (GooString **)gmallocn(nOptions, sizeof(GooString *));
        for (i = 0; i < nOptions; ++i) {
          text[i] = NULL;
          obj1.arrayGet(i, &obj2);
          if (obj2.isString()) {
            text[i] = obj2.getString()->copy();
          } else if (obj2.isArray() && obj2.arrayGetLength() == 2) {
            if (obj2.arrayGet(1, &obj3)->isString()) {
              text[i] = obj3.getString()->copy();
            }
            obj3.free();
          }
          obj2.free();
          if (!text[i]) {
            text[i] = new GooString();
          }
        }
        // get the selected option(s)
        selection = (GBool *)gmallocn(nOptions, sizeof(GBool));
        //~ need to use the I field in addition to the V field
	Form::fieldLookup(field, "V", &obj2);
        for (i = 0; i < nOptions; ++i) {
          selection[i] = gFalse;
          if (obj2.isString()) {
            if (!obj2.getString()->cmp(text[i])) {
              selection[i] = gTrue;
            }
          } else if (obj2.isArray()) {
            for (j = 0; j < obj2.arrayGetLength(); ++j) {
              if (obj2.arrayGet(j, &obj3)->isString() &&
                  !obj3.getString()->cmp(text[i])) {
                selection[i] = gTrue;
              }
              obj3.free();
            }
          }
        }
        obj2.free();
        // get the top index
        if (field->lookup("TI", &obj2)->isInt()) {
          topIdx = obj2.getInt();
        } else {
          topIdx = 0;
        }
        obj2.free();
        // draw the text
        drawListBox(text, selection, nOptions, topIdx, da, fontDict, quadding);
        for (i = 0; i < nOptions; ++i) {
          delete text[i];
        }
        gfree(text);
        gfree(selection);
      }
      obj1.free();
    }
  } else if (ftObj.isName("Sig")) {
    //~unimp
  } else {
    error(-1, "Unknown field type");
  }

  if (da) {
    delete da;
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
  if (drObj.isDict()) {
    appearDict.dictAdd(copyString("Resources"), drObj.copy(&obj1));
  }
  drObj.free();

  // build the appearance stream
  appearStream = new MemStream(strdup(appearBuf->getCString()), 0,
      appearBuf->getLength(), &appearDict);
  appearance.free();
  appearance.initStream(appearStream);
  delete appearBuf;

  appearStream->setNeedFree(gTrue);

  if (widget->isModified()) {
    //create a new object that will contains the new appearance
    
    //if we already have a N entry in our AP dict, reuse it
    if (annot->lookup("AP", &obj1)->isDict() &&
        obj1.dictLookupNF("N", &obj2)->isRef()) {
      appRef = obj2.getRef();
    }

    obj2.free();
    obj1.free();

    // this annot doesn't have an AP yet, create one
    if (appRef.num == 0)
      appRef = xref->addIndirectObject(&appearance);
    else // since we reuse the already existing AP, we have to notify the xref about this update
      xref->setModifiedObject(&appearance, appRef);

    // update object's AP and AS
    Object apObj;
    apObj.initDict(xref);

    Object oaRef;
    oaRef.initRef(appRef.num, appRef.gen);

    apObj.dictSet("N", &oaRef);
    annot->set("AP", &apObj);
    Dict* d = new Dict(annot);
    d->decRef();
    Object dictObj;
    dictObj.initDict(d);

    xref->setModifiedObject(&dictObj, ref);
    dictObj.free();
  }

  if (fontDict) {
    delete fontDict;
  }
  ftObj.free();
  mkObj.free();
}


void AnnotWidget::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

  addDingbatsResource = gFalse;
  generateFieldAppearance ();

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  if (addDingbatsResource) {
    // We are forcing ZaDb but the font does not exist
    // so create a fake one
    Object baseFontObj, subtypeObj;
    baseFontObj.initName("ZapfDingbats");
    subtypeObj.initName("Type1");

    Object fontDictObj;
    Dict *fontDict = new Dict(xref);
    fontDict->decRef();
    fontDict->add(copyString("BaseFont"), &baseFontObj);
    fontDict->add(copyString("Subtype"), &subtypeObj);
    fontDictObj.initDict(fontDict);

    Object fontsDictObj;
    Dict *fontsDict = new Dict(xref);
    fontsDict->decRef();
    fontsDict->add(copyString("ZaDb"), &fontDictObj);
    fontsDictObj.initDict(fontsDict);

    Dict *dict = new Dict(xref);
    dict->add(copyString("Font"), &fontsDictObj);
    gfx->pushResources(dict);
    delete dict;
  }
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  if (addDingbatsResource) {
    gfx->popResources();
  }
  obj.free();
}


//------------------------------------------------------------------------
// AnnotMovie
//------------------------------------------------------------------------
AnnotMovie::AnnotMovie(XRef *xrefA, PDFRectangle *rect, Movie *movieA, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typeMovie;
  annotObj.dictSet ("Subtype", obj1.initName ("Movie"));

  movie = movieA->copy();
  // TODO: create movie dict from movieA

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotMovie::AnnotMovie(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  Annot(xrefA, dict, catalog, obj) {
  type = typeMovie;
  initialize(xrefA, catalog, dict);
}

AnnotMovie::~AnnotMovie() {
  if (title)
    delete title;
  delete movie;
}

void AnnotMovie::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
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
    error(-1, "Bad Annot Movie");
    movie = NULL;
    ok = gFalse;
  }
  movieDict.free();
}

void AnnotMovie::draw(Gfx *gfx, GBool printing) {
  Object obj;

  if (!isVisible (printing))
    return;

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
      imgDict.initDict(xref);
      imgDict.dictSet ("MImg", &poster);

      Object resDict;
      resDict.initDict(xref);
      resDict.dictSet ("XObject", &imgDict);

      Object formDict, obj1, obj2;
      formDict.initDict(xref);
      formDict.dictSet("Length", obj1.initInt(appearBuf->getLength()));
      formDict.dictSet("Subtype", obj1.initName("Form"));
      formDict.dictSet("Name", obj1.initName("FRM"));
      obj1.initArray(xref);
      obj1.arrayAdd(obj2.initInt(0));
      obj1.arrayAdd(obj2.initInt(0));
      obj1.arrayAdd(obj2.initInt(width));
      obj1.arrayAdd(obj2.initInt(height));
      formDict.dictSet("BBox", &obj1);
      obj1.initArray(xref);
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
      objDict.initDict(xref);
      objDict.dictSet ("FRM", &aStream);

      resDict.initDict(xref);
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
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotScreen
//------------------------------------------------------------------------
AnnotScreen::AnnotScreen(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = typeScreen;

  annotObj.dictSet ("Subtype", obj1.initName ("Screen"));
  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotScreen::AnnotScreen(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  Annot(xrefA, dict, catalog, obj) {
  type = typeScreen;
  initialize(xrefA, catalog, dict);
}

AnnotScreen::~AnnotScreen() {
  if (title)
    delete title;
  if (appearCharacs)
    delete appearCharacs;
  if (action)
    delete action;

  additionAction.free();
}

void AnnotScreen::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  title = NULL;
  if (dict->lookup("T", &obj1)->isString()) {
    title = obj1.getString()->copy();
  }
  obj1.free();

  action = NULL;
  if (dict->lookup("A", &obj1)->isDict()) {
    action = LinkAction::parseAction(&obj1, catalog->getBaseURI());
    if (action->getKind() == actionRendition && page == 0) {
      error (-1, "Invalid Rendition action: associated screen annotation without P");
      delete action;
      action = NULL;
      ok = gFalse;
    }
  }

  dict->lookup("AA", &additionAction);

  appearCharacs = NULL;
  if(dict->lookup("MK", &obj1)->isDict()) {
    appearCharacs = new AnnotAppearanceCharacs(obj1.getDict());
  }
  obj1.free();

}

//------------------------------------------------------------------------
// AnnotStamp
//------------------------------------------------------------------------
AnnotStamp::AnnotStamp(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
  AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeStamp;
  annotObj.dictSet ("Subtype", obj1.initName ("Stamp"));
  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotStamp::AnnotStamp(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeStamp;
  initialize(xrefA, catalog, dict);
}

AnnotStamp::~AnnotStamp() {
  delete icon;
}

void AnnotStamp::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("Name", &obj1)->isName()) {
    icon = new GooString(obj1.getName());
  } else {
    icon = new GooString("Draft");
  }
  obj1.free();

}

//------------------------------------------------------------------------
// AnnotGeometry
//------------------------------------------------------------------------
AnnotGeometry::AnnotGeometry(XRef *xrefA, PDFRectangle *rect, AnnotSubtype subType, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
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

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotGeometry::AnnotGeometry(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  // the real type will be read in initialize()
  type = typeSquare;
  initialize(xrefA, catalog, dict);
}

AnnotGeometry::~AnnotGeometry() {
  delete interiorColor;
  delete borderEffect;
  delete geometryRect;
}

void AnnotGeometry::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
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

void AnnotGeometry::draw(Gfx *gfx, GBool printing) {
  Object obj;
  double ca = 1;

  if (!isVisible (printing))
    return;

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

      if (interiorColor)
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
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, (AnnotBorder *)NULL, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotPolygon
//------------------------------------------------------------------------
AnnotPolygon::AnnotPolygon(XRef *xrefA, PDFRectangle *rect, AnnotSubtype subType,
			   AnnotPath *path, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
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

  Object obj2;
  obj2.initArray (xrefA);

  for (int i = 0; i < path->getCoordsLength(); ++i) {
    Object obj3;

    obj2.arrayAdd (obj3.initReal (path->getX(i)));
    obj2.arrayAdd (obj3.initReal (path->getY(i)));
  }

  annotObj.dictSet ("Vertices", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotPolygon::AnnotPolygon(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  // the real type will be read in initialize()
  type = typePolygon;
  initialize(xrefA, catalog, dict);
}

AnnotPolygon::~AnnotPolygon() {
  delete vertices;

  if (interiorColor)
    delete interiorColor;

  if (borderEffect)
    delete borderEffect;
}

void AnnotPolygon::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
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
    error(-1, "Bad Annot Polygon Vertices");
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
    GooString *intentName = new GooString(obj1.getName());

    if(!intentName->cmp("PolygonCloud")) {
      intent = polygonCloud;
    } else if(!intentName->cmp("PolyLineDimension")) {
      intent = polylineDimension;
    } else {
      intent = polygonDimension;
    }
    delete intentName;
  } else {
    intent = polygonCloud;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotCaret
//------------------------------------------------------------------------
AnnotCaret::AnnotCaret(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeCaret;

  annotObj.dictSet ("Subtype", obj1.initName ("Caret"));
  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotCaret::AnnotCaret(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeCaret;
  initialize(xrefA, catalog, dict);
}

AnnotCaret::~AnnotCaret() {
  delete caretRect;
}

void AnnotCaret::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
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

//------------------------------------------------------------------------
// AnnotInk
//------------------------------------------------------------------------
AnnotInk::AnnotInk(XRef *xrefA, PDFRectangle *rect, AnnotPath **paths, int n_paths, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeInk;

  annotObj.dictSet ("Subtype", obj1.initName ("Ink"));

  Object obj2;
  obj2.initArray (xrefA);

  for (int i = 0; i < n_paths; ++i) {
    AnnotPath *path = paths[i];
    Object obj3;
    obj3.initArray (xrefA);

    for (int j = 0; j < path->getCoordsLength(); ++j) {
      Object obj4;

      obj3.arrayAdd (obj4.initReal (path->getX(j)));
      obj3.arrayAdd (obj4.initReal (path->getY(j)));
    }

    obj2.arrayAdd (&obj3);
  }

  annotObj.dictSet ("InkList", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotInk::AnnotInk(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeInk;
  initialize(xrefA, catalog, dict);
}

AnnotInk::~AnnotInk() {
  if (inkList) {
    for (int i = 0; i < inkListLength; ++i)
      delete inkList[i];
    gfree(inkList);
  }
}

void AnnotInk::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("InkList", &obj1)->isArray()) {
    Array *array = obj1.getArray();
    inkListLength = array->getLength();
    inkList = (AnnotPath **) gmallocn ((inkListLength), sizeof(AnnotPath *));
    memset(inkList, 0, inkListLength * sizeof(AnnotPath *));
    for (int i = 0; i < inkListLength; i++) {
      Object obj2;
      if (array->get(i, &obj2)->isArray())
        inkList[i] = new AnnotPath(obj2.getArray());
      obj2.free();
    }
  } else {
    inkListLength = 0;
    inkList = NULL;
    error(-1, "Bad Annot Ink List");
    ok = gFalse;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotFileAttachment
//------------------------------------------------------------------------
AnnotFileAttachment::AnnotFileAttachment(XRef *xrefA, PDFRectangle *rect, GooString *filename, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeFileAttachment;

  annotObj.dictSet ("Subtype", obj1.initName ("FileAttachment"));

  Object obj2;
  obj2.initString(filename->copy());
  annotObj.dictSet ("FS", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotFileAttachment::AnnotFileAttachment(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeFileAttachment;
  initialize(xrefA, catalog, dict);
}

AnnotFileAttachment::~AnnotFileAttachment() {
  file.free();

  if (name)
    delete name;
}

void AnnotFileAttachment::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  if (dict->lookup("FS", &obj1)->isDict() || dict->lookup("FS", &obj1)->isString()) {
    obj1.copy(&file);
  } else {
    error(-1, "Bad Annot File Attachment");
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
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotSound
//------------------------------------------------------------------------
AnnotSound::AnnotSound(XRef *xrefA, PDFRectangle *rect, Sound *soundA, Catalog *catalog) :
    AnnotMarkup(xrefA, rect, catalog) {
  Object obj1;

  type = typeSound;

  annotObj.dictSet ("Subtype", obj1.initName ("Sound"));

  Object obj2;
  Stream *str = soundA->getStream();
  obj2.initStream (str);
  str->incRef(); //FIXME: initStream should do this?
  annotObj.dictSet ("Sound", &obj2);

  initialize(xrefA, catalog, annotObj.getDict());
}

AnnotSound::AnnotSound(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  AnnotMarkup(xrefA, dict, catalog, obj) {
  type = typeSound;
  initialize(xrefA, catalog, dict);
}

AnnotSound::~AnnotSound() {
  delete sound;

  delete name;
}

void AnnotSound::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
  Object obj1;

  sound = Sound::parseSound(dict->lookup("Sound", &obj1));
  if (!sound) {
    error(-1, "Bad Annot Sound");
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
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, border, color,
		 rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// Annot3D
//------------------------------------------------------------------------
Annot3D::Annot3D(XRef *xrefA, PDFRectangle *rect, Catalog *catalog) :
    Annot(xrefA, rect, catalog) {
  Object obj1;

  type = type3D;

  annotObj.dictSet ("Subtype", obj1.initName ("3D"));

  initialize(xrefA, catalog, annotObj.getDict());
}

Annot3D::Annot3D(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj) :
  Annot(xrefA, dict, catalog, obj) {
  type = type3D;
  initialize(xrefA, catalog, dict);
}

Annot3D::~Annot3D() {
  if (activation)
    delete activation;
}

void Annot3D::initialize(XRef *xrefA, Catalog *catalog, Dict* dict) {
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
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("PO")) {
      aTrigger = aTriggerPageOpened;
    } else if(!name->cmp("PV")) {
      aTrigger = aTriggerPageVisible;
    } else if(!name->cmp("XA")) {
      aTrigger = aTriggerUserAction;
    } else {
      aTrigger = aTriggerUnknown;
    }
    delete name;
  } else {
    aTrigger = aTriggerUnknown;
  }
  obj1.free();

  if(dict->lookup("AIS", &obj1)->isName()) {
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("I")) {
      aState = aStateEnabled;
    } else if(!name->cmp("L")) {
      aState = aStateDisabled;
    } else {
      aState = aStateUnknown;
    }
    delete name;
  } else {
    aState = aStateUnknown;
  }
  obj1.free();

  if(dict->lookup("D", &obj1)->isName()) {
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("PC")) {
      dTrigger = dTriggerPageClosed;
    } else if(!name->cmp("PI")) {
      dTrigger = dTriggerPageInvisible;
    } else if(!name->cmp("XD")) {
      dTrigger = dTriggerUserAction;
    } else {
      dTrigger = dTriggerUnknown;
    }
    delete name;
  } else {
    dTrigger = dTriggerUnknown;
  }
  obj1.free();

  if(dict->lookup("DIS", &obj1)->isName()) {
    GooString *name = new GooString(obj1.getName());

    if(!name->cmp("U")) {
      dState = dStateUninstantiaded;
    } else if(!name->cmp("I")) {
      dState = dStateInstantiated;
    } else if(!name->cmp("L")) {
      dState = dStateLive;
    } else {
      dState = dStateUnknown;
    }
    delete name;
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

Annots::Annots(XRef *xref, Catalog *catalog, Object *annotsObj) {
  Annot *annot;
  Object obj1;
  int size;
  int i;

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
        annot = createAnnot (xref, obj1.getDict(), catalog, &obj2);
        if (annot && annot->isOk()) {
          if (nAnnots >= size) {
            size += 16;
            annots = (Annot **)greallocn(annots, size, sizeof(Annot *));
          }
          annots[nAnnots++] = annot;
        } else {
          delete annot;
        }
      }
      obj2.free();
      obj1.free();
    }
  }
}

Annot *Annots::createAnnot(XRef *xref, Dict* dict, Catalog *catalog, Object *obj) {
  Annot *annot;
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString *typeName = new GooString(obj1.getName());

    if (!typeName->cmp("Text")) {
      annot = new AnnotText(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Link")) {
      annot = new AnnotLink(xref, dict, catalog, obj);
    } else if (!typeName->cmp("FreeText")) {
      annot = new AnnotFreeText(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Line")) {
      annot = new AnnotLine(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Square")) {
      annot = new AnnotGeometry(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Circle")) {
      annot = new AnnotGeometry(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Polygon")) {
      annot = new AnnotPolygon(xref, dict, catalog, obj);
    } else if (!typeName->cmp("PolyLine")) {
      annot = new AnnotPolygon(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Highlight")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Underline")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Squiggly")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("StrikeOut")) {
      annot = new AnnotTextMarkup(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Stamp")) {
      annot = new AnnotStamp(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Caret")) {
      annot = new AnnotCaret(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Ink")) {
      annot = new AnnotInk(xref, dict, catalog, obj);
    } else if (!typeName->cmp("FileAttachment")) {
      annot = new AnnotFileAttachment(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Sound")) {
      annot = new AnnotSound(xref, dict, catalog, obj);
    } else if(!typeName->cmp("Movie")) {
      annot = new AnnotMovie(xref, dict, catalog, obj);
    } else if(!typeName->cmp("Widget")) {
      annot = new AnnotWidget(xref, dict, catalog, obj);
    } else if(!typeName->cmp("Screen")) {
      annot = new AnnotScreen(xref, dict, catalog, obj);
    } else if(!typeName->cmp("PrinterMark")) {
      annot = new Annot(xref, dict, catalog, obj);
    } else if (!typeName->cmp("TrapNet")) {
      annot = new Annot(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Watermark")) {
      annot = new Annot(xref, dict, catalog, obj);
    } else if (!typeName->cmp("3D")) {
      annot = new Annot3D(xref, dict, catalog, obj);
    } else if (!typeName->cmp("Popup")) {
      /* Popup annots are already handled by markup annots
       * Here we only care about popup annots without a
       * markup annotation associated
       */
      Object obj2;

      if (dict->lookup("Parent", &obj2)->isNull())
        annot = new AnnotPopup(xref, dict, catalog, obj);
      else
        annot = NULL;
      
      obj2.free();
    } else {
      annot = new Annot(xref, dict, catalog, obj);
    }

    delete typeName;
  } else {
    annot = NULL;
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
    delete annots[i];
  }
  gfree(annots);
}
