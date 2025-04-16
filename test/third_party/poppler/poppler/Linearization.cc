//========================================================================
//
// Linearization.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
//
//========================================================================

#include "Linearization.h"
#include "Parser.h"
#include "Lexer.h"

//------------------------------------------------------------------------
// Linearization
//------------------------------------------------------------------------

Linearization::Linearization (BaseStream *str)
{
  Parser *parser;
  Object obj1, obj2, obj3, obj5;

  linDict.initNull();

  str->reset();
  obj1.initNull();
  parser = new Parser(NULL,
      new Lexer(NULL, str->makeSubStream(str->getStart(), gFalse, 0, &obj1)),
      gFalse);
  parser->getObj(&obj1);
  parser->getObj(&obj2);
  parser->getObj(&obj3);
  parser->getObj(&linDict);
  if (obj1.isInt() && obj2.isInt() && obj3.isCmd("obj") && linDict.isDict()) {
    linDict.dictLookup("Linearized", &obj5);
    if (!(obj5.isNum() && obj5.getNum() > 0)) {
       linDict.free();
       linDict.initNull();
    }
    obj5.free();
  }
  obj3.free();
  obj2.free();
  obj1.free();
  delete parser;
}

Linearization:: ~Linearization()
{
  linDict.free();
}

Guint Linearization::getLength()
{
  if (!linDict.isDict()) return 0;

  int length;
  if (linDict.getDict()->lookupInt("L", NULL, &length) &&
      length > 0) {
    return length;
  } else {
    error(-1, "Length in linearization table is invalid");
    return 0;
  }
}

Guint Linearization::getHintsOffset()
{
  int hintsOffset;

  Object obj1, obj2;
  if (linDict.isDict() &&
      linDict.dictLookup("H", &obj1)->isArray() &&
      obj1.arrayGetLength()>=2 &&
      obj1.arrayGet(0, &obj2)->isInt() &&
      obj2.getInt() > 0) {
    hintsOffset = obj2.getInt();
  } else {
    error(-1, "Hints table offset in linearization table is invalid");
    hintsOffset = 0;
  }
  obj2.free();
  obj1.free();

  return hintsOffset;
}

Guint Linearization::getHintsLength()
{
  int hintsLength;

  Object obj1, obj2;
  if (linDict.isDict() &&
      linDict.dictLookup("H", &obj1)->isArray() &&
      obj1.arrayGetLength()>=2 &&
      obj1.arrayGet(1, &obj2)->isInt() &&
      obj2.getInt() > 0) {
    hintsLength = obj2.getInt();
  } else {
    error(-1, "Hints table length in linearization table is invalid");
    hintsLength = 0;
  }
  obj2.free();
  obj1.free();

  return hintsLength;
}

Guint Linearization::getHintsOffset2()
{
  int hintsOffset2 = 0; // default to 0

  Object obj1, obj2;
  if (linDict.isDict() &&
      linDict.dictLookup("H", &obj1)->isArray() &&
      obj1.arrayGetLength()>=4) {
    if (obj1.arrayGet(2, &obj2)->isInt() &&
        obj2.getInt() > 0) {
      hintsOffset2 = obj2.getInt();
    } else {
      error(-1, "Second hints table offset in linearization table is invalid");
      hintsOffset2 = 0;
    }
  }
  obj2.free();
  obj1.free();

  return hintsOffset2;
}

Guint Linearization::getHintsLength2()
{
  int hintsLength2 = 0; // default to 0

  Object obj1, obj2;
  if (linDict.isDict() &&
      linDict.dictLookup("H", &obj1)->isArray() &&
      obj1.arrayGetLength()>=4) {
    if (obj1.arrayGet(3, &obj2)->isInt() &&
        obj2.getInt() > 0) {
      hintsLength2 = obj2.getInt();
    } else {
      error(-1, "Second hints table length in linearization table is invalid");
      hintsLength2 = 0;
    }
  }
  obj2.free();
  obj1.free();

  return hintsLength2;
}

int Linearization::getObjectNumberFirst()
{
  int objectNumberFirst = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("O", NULL, &objectNumberFirst) &&
      objectNumberFirst > 0) {
    return objectNumberFirst;
  } else {
    error(-1, "Object number of first page in linearization table is invalid");
    return 0;
  }
}

Guint Linearization::getEndFirst()
{
  int pageEndFirst = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("E", NULL, &pageEndFirst) &&
      pageEndFirst > 0) {
    return pageEndFirst;
  } else {
    error(-1, "First page end offset in linearization table is invalid");
    return 0;
  }
}

int Linearization::getNumPages()
{
  int numPages = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("N", NULL, &numPages) &&
      numPages > 0) {
    return numPages;
  } else {
    error(-1, "Page count in linearization table is invalid");
    return 0;
  }
}

Guint Linearization::getMainXRefEntriesOffset()
{
  int mainXRefEntriesOffset = 0;
  if (linDict.isDict() &&
      linDict.getDict()->lookupInt("T", NULL, &mainXRefEntriesOffset) &&
      mainXRefEntriesOffset > 0) {
    return mainXRefEntriesOffset;
  } else {
    error(-1, "Main Xref offset in linearization table is invalid");
    return 0;
  }
}

int Linearization::getPageFirst()
{
  int pageFirst = 0; // Optional, defaults to 0.

  if (linDict.isDict()) {
    linDict.getDict()->lookupInt("P", NULL, &pageFirst);
  }

  if (pageFirst < 0) {
    error(-1, "First page in linearization table is invalid");
    return 0;
  }

  return pageFirst;
}


