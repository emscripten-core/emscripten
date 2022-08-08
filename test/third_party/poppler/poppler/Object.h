//========================================================================
//
// Object.h
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
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Kees Cook <kees@outflux.net>
// Copyright (C) 2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Jakub Wilk <ubanus@users.sf.net>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef OBJECT_H
#define OBJECT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <set>
#include <stdio.h>
#include <string.h>
#include "goo/gtypes.h"
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooLikely.h"
#include "Error.h"

#define OBJECT_TYPE_CHECK(wanted_type) \
    if (unlikely(type != wanted_type)) { \
        error(0, (char *) "Call to Object where the object was type %d, " \
                 "not the expected type %d", type, wanted_type); \
        abort(); \
    }

#define OBJECT_2TYPES_CHECK(wanted_type1, wanted_type2) \
    if (unlikely(type != wanted_type1) && unlikely(type != wanted_type2)) { \
        error(0, (char *) "Call to Object where the object was type %d, " \
                 "not the expected type %d or %d", type, wanted_type1, wanted_type2); \
        abort(); \
    }

class XRef;
class Array;
class Dict;
class Stream;

//------------------------------------------------------------------------
// Ref
//------------------------------------------------------------------------

struct Ref {
  int num;			// object number
  int gen;			// generation number
};

//------------------------------------------------------------------------
// object types
//------------------------------------------------------------------------

enum ObjType {
  // simple objects
  objBool,			// boolean
  objInt,			// integer
  objReal,			// real
  objString,			// string
  objName,			// name
  objNull,			// null

  // complex objects
  objArray,			// array
  objDict,			// dictionary
  objStream,			// stream
  objRef,			// indirect reference

  // special objects
  objCmd,			// command name
  objError,			// error return from Lexer
  objEOF,			// end of file return from Lexer
  objNone,			// uninitialized object

  // poppler-only objects
  objUint			// overflown integer that still fits in a unsigned integer
};

#define numObjTypes 15		// total number of object types

//------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------

#ifdef DEBUG_MEM
#define initObj(t) zeroUnion(); ++numAlloc[type = t]
#else
#define initObj(t) zeroUnion(); type = t
#endif

class Object {
public:
  // clear the anonymous union as best we can -- clear at least a pointer
  // XXX Emscripten: Also null out ref.gen
  void zeroUnion() { this->name = NULL; this->ref.gen = 0; }

  // Default constructor.
  Object():
    type(objNone) { zeroUnion(); }

  // Initialize an object.
  Object *initBool(GBool boolnA)
    { initObj(objBool); booln = boolnA; return this; }
  Object *initInt(int intgA)
    { initObj(objInt); intg = intgA; return this; }
  Object *initReal(double realA)
    { initObj(objReal); real = realA; return this; }
  Object *initString(GooString *stringA)
    { initObj(objString); string = stringA; return this; }
  Object *initName(char *nameA)
    { initObj(objName); name = copyString(nameA); return this; }
  Object *initNull()
    { initObj(objNull); return this; }
  Object *initArray(XRef *xref);
  Object *initDict(XRef *xref);
  Object *initDict(Dict *dictA);
  Object *initStream(Stream *streamA);
  Object *initRef(int numA, int genA)
    { initObj(objRef); ref.num = numA; ref.gen = genA; return this; }
  Object *initCmd(char *cmdA)
    { initObj(objCmd); cmd = copyString(cmdA); return this; }
  Object *initError()
    { initObj(objError); return this; }
  Object *initEOF()
    { initObj(objEOF); return this; }
  Object *initUint(unsigned int uintgA)
    { initObj(objUint); uintg = uintgA; return this; }

  // Copy an object.
  Object *copy(Object *obj);
  Object *shallowCopy(Object *obj) {
    *obj = *this;
    return obj;
  }

  // If object is a Ref, fetch and return the referenced object.
  // Otherwise, return a copy of the object.
  Object *fetch(XRef *xref, Object *obj, std::set<int> *fetchOriginatorNums = NULL);

  // Free object contents.
  void free();

  // Type checking.
  ObjType getType() { return type; }
  GBool isBool() { return type == objBool; }
  GBool isInt() { return type == objInt; }
  GBool isReal() { return type == objReal; }
  GBool isNum() { return type == objInt || type == objReal; }
  GBool isString() { return type == objString; }
  GBool isName() { return type == objName; }
  GBool isNull() { return type == objNull; }
  GBool isArray() { return type == objArray; }
  GBool isDict() { return type == objDict; }
  GBool isStream() { return type == objStream; }
  GBool isRef() { return type == objRef; }
  GBool isCmd() { return type == objCmd; }
  GBool isError() { return type == objError; }
  GBool isEOF() { return type == objEOF; }
  GBool isNone() { return type == objNone; }
  GBool isUint() { return type == objUint; }

  // Special type checking.
  GBool isName(char *nameA)
    { return type == objName && !strcmp(name, nameA); }
  GBool isDict(char *dictType);
  GBool isStream(char *dictType);
  GBool isCmd(char *cmdA)
    { return type == objCmd && !strcmp(cmd, cmdA); }

  // Accessors.
  GBool getBool() { OBJECT_TYPE_CHECK(objBool); return booln; }
  int getInt() { OBJECT_TYPE_CHECK(objInt); return intg; }
  double getReal() { OBJECT_TYPE_CHECK(objReal); return real; }
  double getNum() { OBJECT_2TYPES_CHECK(objInt, objReal); return type == objInt ? (double)intg : real; }
  GooString *getString() { OBJECT_TYPE_CHECK(objString); return string; }
  char *getName() { OBJECT_TYPE_CHECK(objName); return name; }
  Array *getArray() { OBJECT_TYPE_CHECK(objArray); return array; }
  Dict *getDict() { OBJECT_TYPE_CHECK(objDict); return dict; }
  Stream *getStream() { OBJECT_TYPE_CHECK(objStream); return stream; }
  Ref getRef() { OBJECT_TYPE_CHECK(objRef); return ref; }
  int getRefNum() { OBJECT_TYPE_CHECK(objRef); return ref.num; }
  int getRefGen() { OBJECT_TYPE_CHECK(objRef); return ref.gen; }
  char *getCmd() { OBJECT_TYPE_CHECK(objCmd); return cmd; }
  unsigned int getUint() { OBJECT_TYPE_CHECK(objUint); return uintg; }

  // Array accessors.
  int arrayGetLength();
  void arrayAdd(Object *elem);
  Object *arrayGet(int i, Object *obj);
  Object *arrayGetNF(int i, Object *obj);

  // Dict accessors.
  int dictGetLength();
  void dictAdd(char *key, Object *val);
  void dictSet(char *key, Object *val);
  GBool dictIs(char *dictType);
  Object *dictLookup(char *key, Object *obj, std::set<int> *fetchOriginatorNums = NULL);
  Object *dictLookupNF(char *key, Object *obj);
  char *dictGetKey(int i);
  Object *dictGetVal(int i, Object *obj);
  Object *dictGetValNF(int i, Object *obj);

  // Stream accessors.
  GBool streamIs(char *dictType);
  void streamReset();
  void streamClose();
  int streamGetChar();
  int streamGetChars(int nChars, Guchar *buffer);
  int streamLookChar();
  char *streamGetLine(char *buf, int size);
  Guint streamGetPos();
  void streamSetPos(Guint pos, int dir = 0);
  Dict *streamGetDict();

  // Output.
  char *getTypeName();
  void print(FILE *f = stdout);

  // Memory testing.
  static void memCheck(FILE *f);

private:

  ObjType type;			// object type
  union {			// value for each type:
    GBool booln;		//   boolean
    int intg;			//   integer
    unsigned int uintg;		//   unsigned integer
    double real;		//   real
    GooString *string;		//   string
    char *name;			//   name
    Array *array;		//   array
    Dict *dict;			//   dictionary
    Stream *stream;		//   stream
    Ref ref;			//   indirect reference
    char *cmd;			//   command
  };

#ifdef DEBUG_MEM
  static int			// number of each type of object
    numAlloc[numObjTypes];	//   currently allocated
#endif
};

//------------------------------------------------------------------------
// Array accessors.
//------------------------------------------------------------------------

#include "Array.h"

inline int Object::arrayGetLength()
  { OBJECT_TYPE_CHECK(objArray); return array->getLength(); }

inline void Object::arrayAdd(Object *elem)
  { OBJECT_TYPE_CHECK(objArray); array->add(elem); }

inline Object *Object::arrayGet(int i, Object *obj)
  { OBJECT_TYPE_CHECK(objArray); return array->get(i, obj); }

inline Object *Object::arrayGetNF(int i, Object *obj)
  { OBJECT_TYPE_CHECK(objArray); return array->getNF(i, obj); }

//------------------------------------------------------------------------
// Dict accessors.
//------------------------------------------------------------------------

#include "Dict.h"

inline int Object::dictGetLength()
  { OBJECT_TYPE_CHECK(objDict); return dict->getLength(); }

inline void Object::dictAdd(char *key, Object *val)
  { OBJECT_TYPE_CHECK(objDict); dict->add(key, val); }

inline void Object::dictSet(char *key, Object *val)
 	{ OBJECT_TYPE_CHECK(objDict); dict->set(key, val); }

inline GBool Object::dictIs(char *dictType)
  { OBJECT_TYPE_CHECK(objDict); return dict->is(dictType); }

inline GBool Object::isDict(char *dictType)
  { return type == objDict && dictIs(dictType); }

inline Object *Object::dictLookup(char *key, Object *obj, std::set<int> *fetchOriginatorNums)
  { OBJECT_TYPE_CHECK(objDict); return dict->lookup(key, obj, fetchOriginatorNums); }

inline Object *Object::dictLookupNF(char *key, Object *obj)
  { OBJECT_TYPE_CHECK(objDict); return dict->lookupNF(key, obj); }

inline char *Object::dictGetKey(int i)
  { OBJECT_TYPE_CHECK(objDict); return dict->getKey(i); }

inline Object *Object::dictGetVal(int i, Object *obj)
  { OBJECT_TYPE_CHECK(objDict); return dict->getVal(i, obj); }

inline Object *Object::dictGetValNF(int i, Object *obj)
  { OBJECT_TYPE_CHECK(objDict); return dict->getValNF(i, obj); }

//------------------------------------------------------------------------
// Stream accessors.
//------------------------------------------------------------------------

#include "Stream.h"

inline GBool Object::streamIs(char *dictType)
  { OBJECT_TYPE_CHECK(objStream); return stream->getDict()->is(dictType); }

inline GBool Object::isStream(char *dictType)
  { return type == objStream && streamIs(dictType); }

inline void Object::streamReset()
  { OBJECT_TYPE_CHECK(objStream); stream->reset(); }

inline void Object::streamClose()
  { OBJECT_TYPE_CHECK(objStream); stream->close(); }

inline int Object::streamGetChar()
  { OBJECT_TYPE_CHECK(objStream); return stream->getChar(); }

inline int Object::streamGetChars(int nChars, Guchar *buffer)
  { OBJECT_TYPE_CHECK(objStream); return stream->doGetChars(nChars, buffer); }

inline int Object::streamLookChar()
  { OBJECT_TYPE_CHECK(objStream); return stream->lookChar(); }

inline char *Object::streamGetLine(char *buf, int size)
  { OBJECT_TYPE_CHECK(objStream); return stream->getLine(buf, size); }

inline Guint Object::streamGetPos()
  { OBJECT_TYPE_CHECK(objStream); return stream->getPos(); }

inline void Object::streamSetPos(Guint pos, int dir)
  { OBJECT_TYPE_CHECK(objStream); stream->setPos(pos, dir); }

inline Dict *Object::streamGetDict()
  { OBJECT_TYPE_CHECK(objStream); return stream->getDict(); }

#endif
