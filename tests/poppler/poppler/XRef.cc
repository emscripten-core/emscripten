//========================================================================
//
// XRef.cc
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
// Copyright (C) 2005 Dan Sheridan <dan.sheridan@postman.org.uk>
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006, 2008, 2010, 2012, 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009, 2010 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@kabelmail.de>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Pino Toscano <pino@kde.org>
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
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include "goo/gfile.h"
#include "goo/gmem.h"
#include "Object.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"
#include "Dict.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "XRef.h"
#include "PopplerCache.h"

//------------------------------------------------------------------------
// Permission bits
// Note that the PDF spec uses 1 base (eg bit 3 is 1<<2)
//------------------------------------------------------------------------

#define permPrint         (1<<2)  // bit 3
#define permChange        (1<<3)  // bit 4
#define permCopy          (1<<4)  // bit 5
#define permNotes         (1<<5)  // bit 6
#define permFillForm      (1<<8)  // bit 9
#define permAccessibility (1<<9)  // bit 10
#define permAssemble      (1<<10) // bit 11
#define permHighResPrint  (1<<11) // bit 12
#define defPermFlags 0xfffc

#if MULTITHREADED
#  define xrefLocker()   MutexLocker locker(&mutex)
#  define xrefCondLocker(X)  MutexLocker locker(&mutex, (X))
#else
#  define xrefLocker()
#  define xrefCondLocker(X)
#endif

//------------------------------------------------------------------------
// ObjectStream
//------------------------------------------------------------------------

class ObjectStream {
public:

  // Create an object stream, using object number <objStrNum>,
  // generation 0.
  ObjectStream(XRef *xref, int objStrNumA, int recursion = 0);

  GBool isOk() { return ok; }

  ~ObjectStream();

  // Return the object number of this object stream.
  int getObjStrNum() { return objStrNum; }

  // Get the <objIdx>th object from this stream, which should be
  // object number <objNum>, generation 0.
  Object *getObject(int objIdx, int objNum, Object *obj);

private:

  int objStrNum;		// object number of the object stream
  int nObjects;			// number of objects in the stream
  Object *objs;			// the objects (length = nObjects)
  int *objNums;			// the object numbers (length = nObjects)
  GBool ok;
};

class ObjectStreamKey : public PopplerCacheKey
{
  public:
    ObjectStreamKey(int num) : objStrNum(num)
    {
    }

    bool operator==(const PopplerCacheKey &key) const
    {
      const ObjectStreamKey *k = static_cast<const ObjectStreamKey*>(&key);
      return objStrNum == k->objStrNum;
    }

    const int objStrNum;
};

class ObjectStreamItem : public PopplerCacheItem
{
  public:
    ObjectStreamItem(ObjectStream *objStr) : objStream(objStr)
    {
    }

    ~ObjectStreamItem()
    {
      delete objStream;
    }

    ObjectStream *objStream;
};

ObjectStream::ObjectStream(XRef *xref, int objStrNumA, int recursion) {
  Stream *str;
  Parser *parser;
  Goffset *offsets;
  Object objStr, obj1, obj2;
  Goffset first;
  int i;

  objStrNum = objStrNumA;
  nObjects = 0;
  objs = NULL;
  objNums = NULL;
  ok = gFalse;

  if (!xref->fetch(objStrNum, 0, &objStr, recursion)->isStream()) {
    goto err1;
  }

  if (!objStr.streamGetDict()->lookup("N", &obj1, recursion)->isInt()) {
    obj1.free();
    goto err1;
  }
  nObjects = obj1.getInt();
  obj1.free();
  if (nObjects <= 0) {
    goto err1;
  }

  objStr.streamGetDict()->lookup("First", &obj1, recursion);
  if (!obj1.isInt() && !obj1.isInt64()) {
    obj1.free();
    goto err1;
  }
  if (obj1.isInt())
    first = obj1.getInt();
  else
    first = obj1.getInt64();
  obj1.free();
  if (first < 0) {
    goto err1;
  }

  // this is an arbitrary limit to avoid integer overflow problems
  // in the 'new Object[nObjects]' call (Acrobat apparently limits
  // object streams to 100-200 objects)
  if (nObjects > 1000000) {
    error(errSyntaxError, -1, "Too many objects in an object stream");
    goto err1;
  }
  objs = new Object[nObjects];
  objNums = (int *)gmallocn(nObjects, sizeof(int));
  offsets = (Goffset *)gmallocn(nObjects, sizeof(Goffset));

  // parse the header: object numbers and offsets
  objStr.streamReset();
  obj1.initNull();
  str = new EmbedStream(objStr.getStream(), &obj1, gTrue, first);
  parser = new Parser(xref, new Lexer(xref, str), gFalse);
  for (i = 0; i < nObjects; ++i) {
    parser->getObj(&obj1);
    parser->getObj(&obj2);
    if (!obj1.isInt() || !(obj2.isInt() || obj2.isInt64())) {
      obj1.free();
      obj2.free();
      delete parser;
      gfree(offsets);
      goto err1;
    }
    objNums[i] = obj1.getInt();
    if (obj2.isInt())
      offsets[i] = obj2.getInt();
    else
      offsets[i] = obj2.getInt64();
    obj1.free();
    obj2.free();
    if (objNums[i] < 0 || offsets[i] < 0 ||
	(i > 0 && offsets[i] < offsets[i-1])) {
      delete parser;
      gfree(offsets);
      goto err1;
    }
  }
  while (str->getChar() != EOF) ;
  delete parser;

  // skip to the first object - this shouldn't be necessary because
  // the First key is supposed to be equal to offsets[0], but just in
  // case...
  for (Goffset pos = first; pos < offsets[0]; ++pos) {
    objStr.getStream()->getChar();
  }

  // parse the objects
  for (i = 0; i < nObjects; ++i) {
    obj1.initNull();
    if (i == nObjects - 1) {
      str = new EmbedStream(objStr.getStream(), &obj1, gFalse, 0);
    } else {
      str = new EmbedStream(objStr.getStream(), &obj1, gTrue,
			    offsets[i+1] - offsets[i]);
    }
    parser = new Parser(xref, new Lexer(xref, str), gFalse);
    parser->getObj(&objs[i]);
    while (str->getChar() != EOF) ;
    delete parser;
  }

  gfree(offsets);
  ok = gTrue;

 err1:
  objStr.free();
}

ObjectStream::~ObjectStream() {
  int i;

  if (objs) {
    for (i = 0; i < nObjects; ++i) {
      objs[i].free();
    }
    delete[] objs;
  }
  gfree(objNums);
}

Object *ObjectStream::getObject(int objIdx, int objNum, Object *obj) {
  if (objIdx < 0 || objIdx >= nObjects || objNum != objNums[objIdx]) {
    return obj->initNull();
  }
  return objs[objIdx].copy(obj);
}

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

void XRef::init() {
#if MULTITHREADED
  gInitMutex(&mutex);
#endif
  ok = gTrue;
  errCode = errNone;
  entries = NULL;
  capacity = 0;
  size = 0;
  streamEnds = NULL;
  streamEndsLen = 0;
  objStrs = new PopplerCache(5);
  mainXRefEntriesOffset = 0;
  xRefStream = gFalse;
  scannedSpecialFlags = gFalse;
  encrypted = gFalse;
  permFlags = defPermFlags;
  ownerPasswordOk = gFalse;
  rootNum = -1;
  strOwner = gFalse;
}

XRef::XRef() {
  init();
}

XRef::XRef(Object *trailerDictA) {
  init();

  if (trailerDictA->isDict())
    trailerDict.initDict(trailerDictA->getDict());
}

XRef::XRef(BaseStream *strA, Goffset pos, Goffset mainXRefEntriesOffsetA, GBool *wasReconstructed, GBool reconstruct) {
  Object obj;

  init();
  mainXRefEntriesOffset = mainXRefEntriesOffsetA;

  // read the trailer
  str = strA;
  start = str->getStart();
  prevXRefOffset = mainXRefOffset = pos;

  if (reconstruct && !(ok = constructXRef(wasReconstructed)))
  {
    errCode = errDamaged;
    return;
  }
  else
  {
    // if there was a problem with the 'startxref' position, try to
    // reconstruct the xref table
    if (prevXRefOffset == 0) {
      if (!(ok = constructXRef(wasReconstructed))) {
        errCode = errDamaged;
        return;
      }

    // read the xref table
    } else {
      std::vector<Goffset> followedXRefStm;
      readXRef(&prevXRefOffset, &followedXRefStm, NULL);

      // if there was a problem with the xref table,
      // try to reconstruct it
      if (!ok) {
        if (!(ok = constructXRef(wasReconstructed))) {
          errCode = errDamaged;
          return;
        }
      }
    }

    // set size to (at least) the size specified in trailer dict
    trailerDict.dictLookupNF("Size", &obj);
    if (!obj.isInt()) {
        error(errSyntaxWarning, -1, "No valid XRef size in trailer");
    } else {
      if (obj.getInt() > size) {
         if (resize(obj.getInt()) != obj.getInt()) {
            if (!(ok = constructXRef(wasReconstructed))) {
               obj.free();
               errCode = errDamaged;
               return;
            }
         }
      }
    }
    obj.free();

    // get the root dictionary (catalog) object
    trailerDict.dictLookupNF("Root", &obj);
    if (obj.isRef()) {
      rootNum = obj.getRefNum();
      rootGen = obj.getRefGen();
      obj.free();
    } else {
      obj.free();
      if (!(ok = constructXRef(wasReconstructed))) {
        errCode = errDamaged;
        return;
      }
    }
  }
  // now set the trailer dictionary's xref pointer so we can fetch
  // indirect objects from it
  trailerDict.getDict()->setXRef(this);
}

XRef::~XRef() {
  for(int i=0; i<size; i++) {
      entries[i].obj.free ();
  }
  gfree(entries);

  trailerDict.free();
  if (streamEnds) {
    gfree(streamEnds);
  }
  if (objStrs) {
    delete objStrs;
  }
  if (strOwner) {
    delete str;
  }
#if MULTITHREADED
  gDestroyMutex(&mutex);
#endif
}

XRef *XRef::copy() {
  XRef *xref = new XRef();
  xref->str = str->copy();
  xref->strOwner = gTrue;
  xref->encrypted = encrypted;
  xref->permFlags = permFlags;
  xref->ownerPasswordOk = ownerPasswordOk;
  xref->rootGen = rootGen;
  xref->rootNum = rootNum;

  xref->start = start;
  xref->prevXRefOffset = prevXRefOffset;
  xref->mainXRefEntriesOffset = mainXRefEntriesOffset;
  xref->xRefStream = xRefStream;
  trailerDict.copy(&xref->trailerDict);
  xref->encAlgorithm = encAlgorithm;
  xref->encRevision = encRevision;
  xref->encVersion = encVersion;
  xref->permFlags = permFlags;
  xref->keyLength = keyLength;
  xref->permFlags = permFlags;
  for (int i = 0; i < 32; i++) {
    xref->fileKey[i] = fileKey[i];
  }

  if (xref->reserve(size) == 0) {
    error(errSyntaxError, -1, "unable to allocate {0:d} entries", size);
    return NULL;
  }
  xref->size = size;
  for (int i = 0; i < size; ++i) {
    xref->entries[i].offset = entries[i].offset;
    xref->entries[i].type = entries[i].type;
    xref->entries[i].obj.initNull ();
    xref->entries[i].flags = entries[i].flags;
    xref->entries[i].gen = entries[i].gen;
  }
  xref->streamEndsLen = streamEndsLen;
  if (streamEndsLen  != 0) {
    xref->streamEnds = (Goffset *)gmalloc(streamEndsLen * sizeof(Goffset));
    for (int i = 0; i < streamEndsLen; i++) {
      xref->streamEnds[i] = streamEnds[i];
    }
  }
  return xref;
}

int XRef::reserve(int newSize)
{
  if (newSize > capacity) {

    int realNewSize;
    for (realNewSize = capacity ? 2 * capacity : 1024;
          newSize > realNewSize && realNewSize > 0;
          realNewSize <<= 1) ;
    if ((realNewSize < 0) ||
        (realNewSize >= INT_MAX / (int)sizeof(XRefEntry))) {
      return 0;
    }

    void *p = greallocn_checkoverflow(entries, realNewSize, sizeof(XRefEntry));
    if (p == NULL) {
      return 0;
    }

    entries = (XRefEntry *) p;
    capacity = realNewSize;

  }

  return capacity;
}

int XRef::resize(int newSize)
{
  if (newSize > size) {

    if (reserve(newSize) < newSize) return size;

    for (int i = size; i < newSize; ++i) {
      entries[i].offset = -1;
      entries[i].type = xrefEntryNone;
      entries[i].obj.initNull ();
      entries[i].flags = 0;
      entries[i].gen = 0;
    }
  } else {
    for (int i = newSize; i < size; i++) {
      entries[i].obj.free ();
    }
  }

  size = newSize;

  return size;
}

/* Read one xref table section.  Also reads the associated trailer
 * dictionary, and returns the prev pointer (if any).
 * Arguments:
 *   pos                Points to a Goffset containing the offset of the XRef
 *                      section to be read. If a prev pointer is found, *pos is
 *                      updated with its value
 *   followedXRefStm    Used in case of nested readXRef calls to spot circular
 *                      references in XRefStm pointers
 *   xrefStreamObjsNum  If not NULL, every time a XRef stream is encountered,
 *                      its object number is appended
 * Return value:
 *   gTrue if a prev pointer is found, otherwise gFalse
 */
GBool XRef::readXRef(Goffset *pos, std::vector<Goffset> *followedXRefStm, std::vector<int> *xrefStreamObjsNum) {
  Parser *parser;
  Object obj;
  GBool more;

  // start up a parser, parse one token
  obj.initNull();
  parser = new Parser(NULL,
	     new Lexer(NULL,
	       str->makeSubStream(start + *pos, gFalse, 0, &obj)),
	     gTrue);
  parser->getObj(&obj, gTrue);

  // parse an old-style xref table
  if (obj.isCmd("xref")) {
    obj.free();
    more = readXRefTable(parser, pos, followedXRefStm, xrefStreamObjsNum);

  // parse an xref stream
  } else if (obj.isInt()) {
    const int objNum = obj.getInt();
    obj.free();
    if (!parser->getObj(&obj, gTrue)->isInt()) {
      goto err1;
    }
    obj.free();
    if (!parser->getObj(&obj, gTrue)->isCmd("obj")) {
      goto err1;
    }
    obj.free();
    if (!parser->getObj(&obj)->isStream()) {
      goto err1;
    }
    if (trailerDict.isNone()) {
      xRefStream = gTrue;
    }
    if (xrefStreamObjsNum) {
      xrefStreamObjsNum->push_back(objNum);
    }
    more = readXRefStream(obj.getStream(), pos);
    obj.free();

  } else {
    goto err1;
  }

  delete parser;
  return more;

 err1:
  obj.free();
  delete parser;
  ok = gFalse;
  return gFalse;
}

GBool XRef::readXRefTable(Parser *parser, Goffset *pos, std::vector<Goffset> *followedXRefStm, std::vector<int> *xrefStreamObjsNum) {
  XRefEntry entry;
  GBool more;
  Object obj, obj2;
  Goffset pos2;
  int first, n, i;

  while (1) {
    parser->getObj(&obj, gTrue);
    if (obj.isCmd("trailer")) {
      obj.free();
      break;
    }
    if (!obj.isInt()) {
      goto err1;
    }
    first = obj.getInt();
    obj.free();
    if (!parser->getObj(&obj, gTrue)->isInt()) {
      goto err1;
    }
    n = obj.getInt();
    obj.free();
    if (first < 0 || n < 0 || first + n < 0) {
      goto err0;
    }
    if (first + n > size) {
      if (resize(first + n) != first + n) {
        error(errSyntaxError, -1, "Invalid 'obj' parameters'");
        goto err0;
      }
    }
    for (i = first; i < first + n; ++i) {
      parser->getObj(&obj, gTrue);
      if (obj.isInt()) {
	entry.offset = obj.getInt();
      } else if (obj.isInt64()) {
	entry.offset = obj.getInt64();
      } else {
	goto err1;
      }
      obj.free();
      if (!parser->getObj(&obj, gTrue)->isInt()) {
	goto err1;
      }
      entry.gen = obj.getInt();
      entry.obj.initNull ();
      entry.flags = 0;
      obj.free();
      parser->getObj(&obj, gTrue);
      if (obj.isCmd("n")) {
	entry.type = xrefEntryUncompressed;
      } else if (obj.isCmd("f")) {
	entry.type = xrefEntryFree;
      } else {
	goto err1;
      }
      obj.free();
      if (entries[i].offset == -1) {
	entries[i] = entry;
	// PDF files of patents from the IBM Intellectual Property
	// Network have a bug: the xref table claims to start at 1
	// instead of 0.
	if (i == 1 && first == 1 &&
	    entries[1].offset == 0 && entries[1].gen == 65535 &&
	    entries[1].type == xrefEntryFree) {
	  i = first = 0;
	  entries[0] = entries[1];
	  entries[1].offset = -1;
	}
      }
    }
  }

  // read the trailer dictionary
  if (!parser->getObj(&obj)->isDict()) {
    goto err1;
  }

  // get the 'Prev' pointer
  obj.getDict()->lookupNF("Prev", &obj2);
  if (obj2.isInt() || obj2.isInt64()) {
    if (obj2.isInt())
      pos2 = obj2.getInt();
    else
      pos2 = obj2.getInt64();
    if (pos2 != *pos) {
      *pos = pos2;
      more = gTrue;
    } else {
      error(errSyntaxWarning, -1, "Infinite loop in xref table");
      more = gFalse;
    }
  } else if (obj2.isRef()) {
    // certain buggy PDF generators generate "/Prev NNN 0 R" instead
    // of "/Prev NNN"
    pos2 = (Guint)obj2.getRefNum();
    if (pos2 != *pos) {
      *pos = pos2;
      more = gTrue;
    } else {
      error(errSyntaxWarning, -1, "Infinite loop in xref table");
      more = gFalse;
    }
  } else {
    more = gFalse;
  }
  obj2.free();

  // save the first trailer dictionary
  if (trailerDict.isNone()) {
    obj.copy(&trailerDict);
  }

  // check for an 'XRefStm' key
  obj.getDict()->lookup("XRefStm", &obj2);
  if (obj2.isInt() || obj2.isInt64()) {
    if (obj2.isInt())
      pos2 = obj2.getInt();
    else
      pos2 = obj2.getInt64();
    for (size_t i = 0; ok == gTrue && i < followedXRefStm->size(); ++i) {
      if (followedXRefStm->at(i) == pos2) {
        ok = gFalse;
      }
    }
    if (ok) {
      followedXRefStm->push_back(pos2);
      readXRef(&pos2, followedXRefStm, xrefStreamObjsNum);
    }
    if (!ok) {
      obj2.free();
      goto err1;
    }
  }
  obj2.free();

  obj.free();
  return more;

 err1:
  obj.free();
 err0:
  ok = gFalse;
  return gFalse;
}

GBool XRef::readXRefStream(Stream *xrefStr, Goffset *pos) {
  Dict *dict;
  int w[3];
  GBool more;
  Object obj, obj2, idx;
  int newSize, first, n, i;

  dict = xrefStr->getDict();

  if (!dict->lookupNF("Size", &obj)->isInt()) {
    goto err1;
  }
  newSize = obj.getInt();
  obj.free();
  if (newSize < 0) {
    goto err1;
  }
  if (newSize > size) {
    if (resize(newSize) != newSize) {
      error(errSyntaxError, -1, "Invalid 'size' parameter");
      goto err0;
    }
  }

  if (!dict->lookupNF("W", &obj)->isArray() ||
      obj.arrayGetLength() < 3) {
    goto err1;
  }
  for (i = 0; i < 3; ++i) {
    if (!obj.arrayGet(i, &obj2)->isInt()) {
      obj2.free();
      goto err1;
    }
    w[i] = obj2.getInt();
    obj2.free();
    if (w[i] < 0) {
      goto err1;
    }
  }
  obj.free();
  if (w[0] > (int)sizeof(int) || w[1] > (int)sizeof(long long) || w[2] > (int)sizeof(int)) {
    goto err1;
  }

  xrefStr->reset();
  dict->lookupNF("Index", &idx);
  if (idx.isArray()) {
    for (i = 0; i+1 < idx.arrayGetLength(); i += 2) {
      if (!idx.arrayGet(i, &obj)->isInt()) {
	idx.free();
	goto err1;
      }
      first = obj.getInt();
      obj.free();
      if (!idx.arrayGet(i+1, &obj)->isInt()) {
	idx.free();
	goto err1;
      }
      n = obj.getInt();
      obj.free();
      if (first < 0 || n < 0 ||
	  !readXRefStreamSection(xrefStr, w, first, n)) {
	idx.free();
	goto err0;
      }
    }
  } else {
    if (!readXRefStreamSection(xrefStr, w, 0, newSize)) {
      idx.free();
      goto err0;
    }
  }
  idx.free();

  dict->lookupNF("Prev", &obj);
  if (obj.isInt()) {
    *pos = obj.getInt();
    more = gTrue;
  } else if (obj.isInt64()) {
    *pos = obj.getInt64();
    more = gTrue;
  } else {
    more = gFalse;
  }
  obj.free();
  if (trailerDict.isNone()) {
    trailerDict.initDict(dict);
  }

  return more;

 err1:
  obj.free();
 err0:
  ok = gFalse;
  return gFalse;
}

GBool XRef::readXRefStreamSection(Stream *xrefStr, int *w, int first, int n) {
  unsigned long long offset;
  int type, gen, c, i, j;

  if (first + n < 0) {
    return gFalse;
  }
  if (first + n > size) {
    if (resize(first + n) != size) {
      error(errSyntaxError, -1, "Invalid 'size' inside xref table");
      return gFalse;
    }
    if (first + n > size) {
      error(errSyntaxError, -1, "Invalid 'first' or 'n' inside xref table");
      return gFalse;
    }
  }
  for (i = first; i < first + n; ++i) {
    if (w[0] == 0) {
      type = 1;
    } else {
      for (type = 0, j = 0; j < w[0]; ++j) {
	if ((c = xrefStr->getChar()) == EOF) {
	  return gFalse;
	}
	type = (type << 8) + c;
      }
    }
    for (offset = 0, j = 0; j < w[1]; ++j) {
      if ((c = xrefStr->getChar()) == EOF) {
	return gFalse;
      }
      offset = (offset << 8) + c;
    }
    if (offset > (unsigned long long)GoffsetMax()) {
      error(errSyntaxError, -1, "Offset inside xref table too large for fseek");
      return gFalse;
    }
    for (gen = 0, j = 0; j < w[2]; ++j) {
      if ((c = xrefStr->getChar()) == EOF) {
	return gFalse;
      }
      gen = (gen << 8) + c;
    }
    if (entries[i].offset == -1) {
      switch (type) {
      case 0:
	entries[i].offset = offset;
	entries[i].gen = gen;
	entries[i].type = xrefEntryFree;
	break;
      case 1:
	entries[i].offset = offset;
	entries[i].gen = gen;
	entries[i].type = xrefEntryUncompressed;
	break;
      case 2:
	entries[i].offset = offset;
	entries[i].gen = gen;
	entries[i].type = xrefEntryCompressed;
	break;
      default:
	return gFalse;
      }
    }
  }

  return gTrue;
}

// Attempt to construct an xref table for a damaged file.
GBool XRef::constructXRef(GBool *wasReconstructed, GBool needCatalogDict) {
  Parser *parser;
  Object newTrailerDict, obj;
  char buf[256];
  Goffset pos;
  int num, gen;
  int newSize;
  int streamEndsSize;
  char *p;
  GBool gotRoot;
  char* token = NULL;
  bool oneCycle = true;
  int offset = 0;

  gfree(entries);
  capacity = 0;
  size = 0;
  entries = NULL;

  gotRoot = gFalse;
  streamEndsLen = streamEndsSize = 0;

  if (wasReconstructed)
  {
    *wasReconstructed = true;
  }

  str->reset();
  while (1) {
    pos = str->getPos();
    if (!str->getLine(buf, 256)) {
      break;
    }
    p = buf;

    // skip whitespace
    while (*p && Lexer::isSpace(*p & 0xff)) ++p;

    oneCycle = true;
    offset = 0;

    while( ( token = strstr( p, "endobj" ) ) || oneCycle ) {
      oneCycle = false;

      if( token ) {
        oneCycle = true;
        token[0] = '\0'; 
        offset = token - p;
      }

      // got trailer dictionary
      if (!strncmp(p, "trailer", 7)) {
        obj.initNull();
        parser = new Parser(NULL,
		 new Lexer(NULL,
		   str->makeSubStream(pos + 7, gFalse, 0, &obj)),
		 gFalse);
        parser->getObj(&newTrailerDict);
        if (newTrailerDict.isDict()) {
	  newTrailerDict.dictLookupNF("Root", &obj);
	  if (obj.isRef() && (!gotRoot || !needCatalogDict) && rootNum != obj.getRefNum()) {
	    rootNum = obj.getRefNum();
	    rootGen = obj.getRefGen();
	    if (!trailerDict.isNone()) {
	      trailerDict.free();
	    }
	    newTrailerDict.copy(&trailerDict);
	    gotRoot = gTrue;
	  }
	  obj.free();
        }
        newTrailerDict.free();
        delete parser;

      // look for object
    } else if (isdigit(*p & 0xff)) {
      num = atoi(p);
      if (num > 0) {
	do {
	  ++p;
	} while (*p && isdigit(*p & 0xff));
	if (isspace(*p & 0xff)) {
	  do {
	    ++p;
	  } while (*p && isspace(*p & 0xff));
	  if (isdigit(*p & 0xff)) {
	    gen = atoi(p);
	    do {
	      ++p;
	    } while (*p && isdigit(*p & 0xff));
	    if (isspace(*p & 0xff)) {
	      do {
		++p;
	      } while (*p && isspace(*p & 0xff));
	      if (!strncmp(p, "obj", 3)) {
		if (num >= size) {
		  newSize = (num + 1 + 255) & ~255;
		  if (newSize < 0) {
		    error(errSyntaxError, -1, "Bad object number");
		    return gFalse;
		  }
		    if (resize(newSize) != newSize) {
		      error(errSyntaxError, -1, "Invalid 'obj' parameters");
		      return gFalse;
		    }
		  }
		  if (entries[num].type == xrefEntryFree ||
		      gen >= entries[num].gen) {
		    entries[num].offset = pos - start;
		    entries[num].gen = gen;
		    entries[num].type = xrefEntryUncompressed;
		}
	        }
	      }
	    }
	  }
        }

      } else if (!strncmp(p, "endstream", 9)) {
        if (streamEndsLen == streamEndsSize) {
	  streamEndsSize += 64;
          if (streamEndsSize >= INT_MAX / (int)sizeof(int)) {
            error(errSyntaxError, -1, "Invalid 'endstream' parameter.");
            return gFalse;
          }
	  streamEnds = (Goffset *)greallocn(streamEnds,
					streamEndsSize, sizeof(Goffset));
        }
        streamEnds[streamEndsLen++] = pos;
      }
      if( token ) {
        p = token + 6;// strlen( "endobj" ) = 6
        pos += offset + 6;// strlen( "endobj" ) = 6
        while (*p && Lexer::isSpace(*p & 0xff)) {
          ++p;
          ++pos;
        }
      }
    }
  }

  if (gotRoot)
    return gTrue;

  error(errSyntaxError, -1, "Couldn't find trailer dictionary");
  return gFalse;
}

void XRef::setEncryption(int permFlagsA, GBool ownerPasswordOkA,
			 Guchar *fileKeyA, int keyLengthA,
			 int encVersionA, int encRevisionA,
			 CryptAlgorithm encAlgorithmA) {
  int i;

  encrypted = gTrue;
  permFlags = permFlagsA;
  ownerPasswordOk = ownerPasswordOkA;
  if (keyLengthA <= 32) {
    keyLength = keyLengthA;
  } else {
    keyLength = 32;
  }
  for (i = 0; i < keyLength; ++i) {
    fileKey[i] = fileKeyA[i];
  }
  encVersion = encVersionA;
  encRevision = encRevisionA;
  encAlgorithm = encAlgorithmA;
}

void XRef::getEncryptionParameters(Guchar **fileKeyA, CryptAlgorithm *encAlgorithmA,
                              int *keyLengthA) {
  if (encrypted) {
    *fileKeyA = fileKey;
    *encAlgorithmA = encAlgorithm;
    *keyLengthA = keyLength;
  } else {
    // null encryption parameters
    *fileKeyA = NULL;
    *encAlgorithmA = cryptRC4;
    *keyLengthA = 0;
  }
}

GBool XRef::okToPrint(GBool ignoreOwnerPW) {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permPrint);
}

// we can print at high res if we are only doing security handler revision
// 2 (and we are allowed to print at all), or with security handler rev
// 3 and we are allowed to print, and bit 12 is set.
GBool XRef::okToPrintHighRes(GBool ignoreOwnerPW) {
  if (encrypted) {
    if (2 == encRevision) {
      return (okToPrint(ignoreOwnerPW));
    } else if (encRevision >= 3) {
      return (okToPrint(ignoreOwnerPW) && (permFlags & permHighResPrint));
    } else {
      // something weird - unknown security handler version
      return gFalse;
    }
  } else {
    return gTrue;
  }
}

GBool XRef::okToChange(GBool ignoreOwnerPW) {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permChange);
}

GBool XRef::okToCopy(GBool ignoreOwnerPW) {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permCopy);
}

GBool XRef::okToAddNotes(GBool ignoreOwnerPW) {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permNotes);
}

GBool XRef::okToFillForm(GBool ignoreOwnerPW) {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permFillForm);
}

GBool XRef::okToAccessibility(GBool ignoreOwnerPW) {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permAccessibility);
}

GBool XRef::okToAssemble(GBool ignoreOwnerPW) {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permAssemble);
}

Object *XRef::getCatalog(Object *catalog) {
  Object *obj = fetch(rootNum, rootGen, catalog);
  if (obj->isDict()) {
    return obj;
  }
  GBool wasReconstructed = false;
  GBool ok = constructXRef(&wasReconstructed, gTrue);
  return (ok) ? fetch(rootNum, rootGen, catalog) : obj;
}

Object *XRef::fetch(int num, int gen, Object *obj, int recursion) {
  XRefEntry *e;
  Parser *parser;
  Object obj1, obj2, obj3;

  xrefLocker();
  // check for bogus ref - this can happen in corrupted PDF files
  if (num < 0 || num >= size) {
    goto err;
  }

  e = getEntry(num);
  if(!e->obj.isNull ()) { //check for updated object
    obj = e->obj.copy(obj);
    return obj;
  }

  switch (e->type) {

  case xrefEntryUncompressed:
    if (e->gen != gen) {
      goto err;
    }
    obj1.initNull();
    parser = new Parser(this,
	       new Lexer(this,
		 str->makeSubStream(start + e->offset, gFalse, 0, &obj1)),
	       gTrue);
    parser->getObj(&obj1, recursion);
    parser->getObj(&obj2, recursion);
    parser->getObj(&obj3, recursion);
    if (!obj1.isInt() || obj1.getInt() != num ||
	!obj2.isInt() || obj2.getInt() != gen ||
	!obj3.isCmd("obj")) {
      // some buggy pdf have obj1234 for ints that represent 1234
      // try to recover here
      if (obj1.isInt() && obj1.getInt() == num &&
	  obj2.isInt() && obj2.getInt() == gen &&
	  obj3.isCmd()) {
	char *cmd = obj3.getCmd();
	if (strlen(cmd) > 3 &&
	    cmd[0] == 'o' &&
	    cmd[1] == 'b' &&
	    cmd[2] == 'j') {
	  char *end_ptr;
	  long longNumber = strtol(cmd + 3, &end_ptr, 0);
	  if (longNumber <= INT_MAX && longNumber >= INT_MIN && *end_ptr == '\0') {
	    int number = longNumber;
	    error(errSyntaxWarning, -1, "Cmd was not obj but {0:s}, assuming the creator meant obj {1:d}", cmd, number);
	    obj->initInt(number);
	    obj1.free();
	    obj2.free();
	    obj3.free();
	    delete parser;
	    break;
	  }
	}
      }
      obj1.free();
      obj2.free();
      obj3.free();
      delete parser;
      goto err;
    }
    parser->getObj(obj, gFalse, (encrypted && !e->getFlag(XRefEntry::Unencrypted)) ? fileKey : NULL,
		   encAlgorithm, keyLength, num, gen, recursion);
    obj1.free();
    obj2.free();
    obj3.free();
    delete parser;
    break;

  case xrefEntryCompressed:
  {
#if 0 // Adobe apparently ignores the generation number on compressed objects
    if (gen != 0) {
      goto err;
    }
#endif
    if (e->offset >= (Guint)size ||
	entries[e->offset].type != xrefEntryUncompressed) {
      error(errSyntaxError, -1, "Invalid object stream");
      goto err;
    }

    ObjectStream *objStr = NULL;
    ObjectStreamKey key(e->offset);
    PopplerCacheItem *item = objStrs->lookup(key);
    if (item) {
      ObjectStreamItem *it = static_cast<ObjectStreamItem *>(item);
      objStr = it->objStream;
    }

    if (!objStr) {
      objStr = new ObjectStream(this, e->offset, recursion + 1);
      if (!objStr->isOk()) {
	delete objStr;
	objStr = NULL;
	goto err;
      } else {
	// XRef could be reconstructed in constructor of ObjectStream:
	e = getEntry(num);
	ObjectStreamKey *newkey = new ObjectStreamKey(e->offset);
	ObjectStreamItem *newitem = new ObjectStreamItem(objStr);
	objStrs->put(newkey, newitem);
      }
    }
    objStr->getObject(e->gen, num, obj);
  }
  break;

  default:
    goto err;
  }
  
  return obj;

 err:
  return obj->initNull();
}

void XRef::lock() {
#if MULTITHREADED
  gLockMutex(&mutex);
#endif
}

void XRef::unlock() {
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
}

Object *XRef::getDocInfo(Object *obj) {
  return trailerDict.dictLookup("Info", obj);
}

// Added for the pdftex project.
Object *XRef::getDocInfoNF(Object *obj) {
  return trailerDict.dictLookupNF("Info", obj);
}

GBool XRef::getStreamEnd(Goffset streamStart, Goffset *streamEnd) {
  int a, b, m;

  if (streamEndsLen == 0 ||
      streamStart > streamEnds[streamEndsLen - 1]) {
    return gFalse;
  }

  a = -1;
  b = streamEndsLen - 1;
  // invariant: streamEnds[a] < streamStart <= streamEnds[b]
  while (b - a > 1) {
    m = (a + b) / 2;
    if (streamStart <= streamEnds[m]) {
      b = m;
    } else {
      a = m;
    }
  }
  *streamEnd = streamEnds[b];
  return gTrue;
}

int XRef::getNumEntry(Goffset offset)
{
  if (size > 0)
  {
    int res = 0;
    Goffset resOffset = getEntry(0)->offset;
    XRefEntry *e;
    for (int i = 1; i < size; ++i)
    {
      e = getEntry(i, gFalse);
      if (e->type != xrefEntryFree && e->offset < offset && e->offset >= resOffset)
      {
        res = i;
        resOffset = e->offset;
      }
    }
    return res;
  }
  else return -1;
}

void XRef::add(int num, int gen, Goffset offs, GBool used) {
  xrefLocker();
  if (num >= size) {
    if (num >= capacity) {
      entries = (XRefEntry *)greallocn(entries, num + 1, sizeof(XRefEntry));
      capacity = num + 1;
    }
    for (int i = size; i < num + 1; ++i) {
      entries[i].offset = -1;
      entries[i].type = xrefEntryFree;
      entries[i].obj.initNull ();
      entries[i].flags = 0;
      entries[i].gen = 0;
    }
    size = num + 1;
  }
  XRefEntry *e = getEntry(num);
  e->gen = gen;
  e->obj.initNull ();
  e->flags = 0;
  if (used) {
    e->type = xrefEntryUncompressed;
    e->offset = offs;
  } else {
    e->type = xrefEntryFree;
    e->offset = 0;
  }
}

void XRef::setModifiedObject (Object* o, Ref r) {
  xrefLocker();
  if (r.num < 0 || r.num >= size) {
    error(errInternal, -1,"XRef::setModifiedObject on unknown ref: {0:d}, {1:d}\n", r.num, r.gen);
    return;
  }
  XRefEntry *e = getEntry(r.num);
  e->obj.free();
  o->copy(&(e->obj));
  e->setFlag(XRefEntry::Updated, gTrue);
}

Ref XRef::addIndirectObject (Object* o) {
  int entryIndexToUse = -1;
  for (int i = 1; entryIndexToUse == -1 && i < size; ++i) {
    XRefEntry *e = getEntry(i, false /* complainIfMissing */);
    if (e->type == xrefEntryFree && e->gen != 65535) {
      entryIndexToUse = i;
    }
  }

  XRefEntry *e;
  if (entryIndexToUse == -1) {
    entryIndexToUse = size;
    add(entryIndexToUse, 0, 0, gFalse);
    e = getEntry(entryIndexToUse);
  } else {
    //reuse a free entry
    e = getEntry(entryIndexToUse);
    //we don't touch gen number, because it should have been 
    //incremented when the object was deleted
  }
  e->type = xrefEntryUncompressed;
  o->copy(&e->obj);
  e->setFlag(XRefEntry::Updated, gTrue);

  Ref r;
  r.num = entryIndexToUse;
  r.gen = e->gen;
  return r;
}

void XRef::removeIndirectObject(Ref r) {
  xrefLocker();
  if (r.num < 0 || r.num >= size) {
    error(errInternal, -1,"XRef::removeIndirectObject on unknown ref: {0:d}, {1:d}\n", r.num, r.gen);
    return;
  }
  XRefEntry *e = getEntry(r.num);
  if (e->type == xrefEntryFree) {
    return;
  }
  e->obj.free();
  e->type = xrefEntryFree;
  e->gen++;
  e->setFlag(XRefEntry::Updated, gTrue);
}

void XRef::writeXRef(XRef::XRefWriter *writer, GBool writeAllEntries) {
  //create free entries linked-list
  if (getEntry(0)->gen != 65535) {
    error(errInternal, -1, "XRef::writeXRef, entry 0 of the XRef is invalid (gen != 65535)\n");
  }
  int lastFreeEntry = 0;
  for (int i=0; i<size; i++) {
    if (getEntry(i)->type == xrefEntryFree) {
      getEntry(lastFreeEntry)->offset = i;
      lastFreeEntry = i;
    }
  }
  getEntry(lastFreeEntry)->offset = 0;

  if (writeAllEntries) {
    writer->startSection(0, size);
    for (int i=0; i<size; i++) {
      XRefEntry *e = getEntry(i);
      if(e->gen > 65535) e->gen = 65535; //cap generation number to 65535 (required by PDFReference)
      writer->writeEntry(e->offset, e->gen, e->type);
    }
  } else {
    int i = 0;
    while (i < size) {
      int j;
      for(j=i; j<size; j++) { //look for consecutive entries
        if ((getEntry(j)->type == xrefEntryFree) && (getEntry(j)->gen == 0))
          break;
      }
      if (j-i != 0)
      {
        writer->startSection(i, j-i);
        for (int k=i; k<j; k++) {
          XRefEntry *e = getEntry(k);
          if(e->gen > 65535) e->gen = 65535; //cap generation number to 65535 (required by PDFReference)
          writer->writeEntry(e->offset, e->gen, e->type);
        }
        i = j;
      }
      else ++i;
    }
  }
}

XRef::XRefTableWriter::XRefTableWriter(OutStream* outStrA) {
  outStr = outStrA;
}

void XRef::XRefTableWriter::startSection(int first, int count) {
  outStr->printf("%i %i\r\n", first, count);
}

void XRef::XRefTableWriter::writeEntry(Goffset offset, int gen, XRefEntryType type) {
  outStr->printf("%010lli %05i %c\r\n", (long long)offset, gen, (type==xrefEntryFree)?'f':'n');
}

void XRef::writeTableToFile(OutStream* outStr, GBool writeAllEntries) {
  XRefTableWriter writer(outStr);
  outStr->printf("xref\r\n");
  writeXRef(&writer, writeAllEntries);
}

XRef::XRefStreamWriter::XRefStreamWriter(Object *indexA, GooString *stmBufA, int offsetSizeA) {
  index = indexA;
  stmBuf = stmBufA;
  offsetSize = offsetSizeA;
}

void XRef::XRefStreamWriter::startSection(int first, int count) {
  Object obj;
  index->arrayAdd( obj.initInt(first) );
  index->arrayAdd( obj.initInt(count) );
}

void XRef::XRefStreamWriter::writeEntry(Goffset offset, int gen, XRefEntryType type) {
  const int entryTotalSize = 1 + offsetSize + 2; /* type + offset + gen */
  char data[16];
  data[0] = (type==xrefEntryFree) ? 0 : 1;
  for (int i = offsetSize; i > 0; i--) {
    data[i] = offset & 0xff;
    offset >>= 8;
  }
  data[offsetSize + 1] = (gen >> 8) & 0xff;
  data[offsetSize + 2] = gen & 0xff;
  stmBuf->append(data, entryTotalSize);
}

XRef::XRefPreScanWriter::XRefPreScanWriter() {
  hasOffsetsBeyond4GB = gFalse;
}

void XRef::XRefPreScanWriter::startSection(int first, int count) {
}

void XRef::XRefPreScanWriter::writeEntry(Goffset offset, int gen, XRefEntryType type) {
  if (offset >= 0x100000000ll)
    hasOffsetsBeyond4GB = gTrue;
}

void XRef::writeStreamToBuffer(GooString *stmBuf, Dict *xrefDict, XRef *xref) {
  Object index;
  index.initArray(xref);
  stmBuf->clear();

  // First pass: determine whether all offsets fit in 4 bytes or not
  XRefPreScanWriter prescan;
  writeXRef(&prescan, gFalse);
  const int offsetSize = prescan.hasOffsetsBeyond4GB ? sizeof(Goffset) : 4;

  // Second pass: actually write the xref stream
  XRefStreamWriter writer(&index, stmBuf, offsetSize);
  writeXRef(&writer, gFalse);

  Object obj1, obj2;
  xrefDict->set("Type", obj1.initName("XRef"));
  xrefDict->set("Index", &index);
  obj2.initArray(xref);
  obj2.arrayAdd( obj1.initInt(1) );
  obj2.arrayAdd( obj1.initInt(offsetSize) );
  obj2.arrayAdd( obj1.initInt(2) );
  xrefDict->set("W", &obj2);
}

GBool XRef::parseEntry(Goffset offset, XRefEntry *entry)
{
  GBool r;

  Object obj;
  obj.initNull();
  Parser parser = Parser(NULL, new Lexer(NULL,
     str->makeSubStream(offset, gFalse, 20, &obj)), gTrue);

  Object obj1, obj2, obj3;
  if (((parser.getObj(&obj1)->isInt()) ||
       parser.getObj(&obj1)->isInt64()) &&
      (parser.getObj(&obj2)->isInt()) &&
      (parser.getObj(&obj3)->isCmd("n") || obj3.isCmd("f"))) {
    if (obj1.isInt64())
      entry->offset = obj1.getInt64();
    else
      entry->offset = obj1.getInt();
    entry->gen = obj2.getInt();
    entry->type = obj3.isCmd("n") ? xrefEntryUncompressed : xrefEntryFree;
    entry->obj.initNull ();
    entry->flags = 0;
    r = gTrue;
  } else {
    r = gFalse;
  }
  obj1.free();
  obj2.free();
  obj3.free();

  return r;
}

/* Traverse all XRef tables and, if untilEntryNum != -1, stop as soon as
 * untilEntryNum is found, or try to reconstruct the xref table if it's not
 * present in any xref.
 * If xrefStreamObjsNum is not NULL, it is filled with the list of the object
 * numbers of the XRef streams that have been traversed */
void XRef::readXRefUntil(int untilEntryNum, std::vector<int> *xrefStreamObjsNum)
{
  std::vector<Goffset> followedPrev;
  while (prevXRefOffset && (untilEntryNum == -1 || entries[untilEntryNum].type == xrefEntryNone)) {
    bool followed = false;
    for (size_t j = 0; j < followedPrev.size(); j++) {
      if (followedPrev.at(j) == prevXRefOffset) {
        followed = true;
        break;
      }
    }
    if (followed) {
      error(errSyntaxError, -1, "Circular XRef");
      if (!(ok = constructXRef(NULL))) {
        errCode = errDamaged;
      }
      break;
    }

    followedPrev.push_back (prevXRefOffset);

    std::vector<Goffset> followedXRefStm;
    if (!readXRef(&prevXRefOffset, &followedXRefStm, xrefStreamObjsNum)) {
        prevXRefOffset = 0;
    }

    // if there was a problem with the xref table, or we haven't found the entry
    // we were looking for, try to reconstruct the xref
    if (!ok || (!prevXRefOffset && untilEntryNum != -1 && entries[untilEntryNum].type == xrefEntryNone)) {
        GBool wasReconstructed = false;
        if (!(ok = constructXRef(&wasReconstructed))) {
            errCode = errDamaged;
            break;
        }
        break;
    }
  }
}

XRefEntry *XRef::getEntry(int i, GBool complainIfMissing)
{
  if (entries[i].type == xrefEntryNone) {

    if ((!xRefStream) && mainXRefEntriesOffset) {
      if (!parseEntry(mainXRefEntriesOffset + 20*i, &entries[i])) {
        error(errSyntaxError, -1, "Failed to parse XRef entry [{0:d}].", i);
      }
    } else {
      // Read XRef tables until the entry we're looking for is found
      readXRefUntil(i);
      
      // We might have reconstructed the xref
      // Check again i is in bounds
      if (unlikely(i >= size)) {
        static XRefEntry dummy;
        dummy.offset = 0;
        dummy.gen = -1;
        dummy.type = xrefEntryNone;
        dummy.flags = 0;
        return &dummy;
      }

      if (entries[i].type == xrefEntryNone) {
        if (complainIfMissing) {
          error(errSyntaxError, -1, "Invalid XRef entry");
        }
        entries[i].type = xrefEntryFree;
      }
    }
  }

  return &entries[i];
}

// Recursively sets the Unencrypted flag in all referenced xref entries
void XRef::markUnencrypted(Object *obj) {
  Object obj1;

  switch (obj->getType()) {
    case objArray:
    {
      Array *array = obj->getArray();
      for (int i = 0; i < array->getLength(); i++) {
        markUnencrypted(array->getNF(i, &obj1));
        obj1.free();
      }
      break;
    }
    case objStream:
    case objDict:
    {
      Dict *dict;
      if (obj->getType() == objStream) {
        Stream *stream = obj->getStream();
        dict = stream->getDict();
      } else {
        dict = obj->getDict();
      }
      for (int i = 0; i < dict->getLength(); i++) {
        markUnencrypted(dict->getValNF(i, &obj1));
        obj1.free();
      }
      break;
    }
    case objRef:
    {
      Ref ref = obj->getRef();
      XRefEntry *e = getEntry(ref.num);
      if (e->getFlag(XRefEntry::Unencrypted))
        return; // We've already been here: prevent infinite recursion
      e->setFlag(XRefEntry::Unencrypted, gTrue);
      fetch(ref.num, ref.gen, &obj1);
      markUnencrypted(&obj1);
      obj1.free();
      break;
    }
    default:
      break;
  }
}

void XRef::scanSpecialFlags() {
  if (scannedSpecialFlags) {
    return;
  }
  scannedSpecialFlags = gTrue;

  // "Rewind" the XRef linked list, so that readXRefUntil re-reads all XRef
  // tables/streams, even those that had already been parsed
  prevXRefOffset = mainXRefOffset;

  std::vector<int> xrefStreamObjNums;
  if (!streamEndsLen) { // don't do it for already reconstructed xref
    readXRefUntil(-1 /* read all xref sections */, &xrefStreamObjNums);
  }

  // Mark object streams as DontRewrite, because we write each object
  // individually in full rewrite mode.
  for (int i = 0; i < size; ++i) {
    if (entries[i].type == xrefEntryCompressed) {
      const int objStmNum = entries[i].offset;
      if (unlikely(objStmNum < 0 || objStmNum >= size)) {
        error(errSyntaxError, -1, "Compressed object offset out of xref bounds");
      } else {
        getEntry(objStmNum)->setFlag(XRefEntry::DontRewrite, gTrue);
      }
    }
  }

  // Mark XRef streams objects as Unencrypted and DontRewrite
  for (size_t i = 0; i < xrefStreamObjNums.size(); ++i) {
    const int objNum = xrefStreamObjNums.at(i);
    getEntry(objNum)->setFlag(XRefEntry::Unencrypted, gTrue);
    getEntry(objNum)->setFlag(XRefEntry::DontRewrite, gTrue);
  }

  // Mark objects referred from the Encrypt dict as Unencrypted
  Object obj;
  markUnencrypted(trailerDict.dictLookupNF("Encrypt", &obj));
  obj.free();
}

void XRef::markUnencrypted() {
  // Mark objects referred from the Encrypt dict as Unencrypted
  Object obj;
  trailerDict.dictLookupNF("Encrypt", &obj);
  if (obj.isRef()) {
    XRefEntry *e = getEntry(obj.getRefNum());
    e->setFlag(XRefEntry::Unencrypted, gTrue);
  }
  obj.free();
}

