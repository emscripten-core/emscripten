//========================================================================
//
// CMap.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008 Koji Otani <sho@bbr.jp>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef CMAP_H
#define CMAP_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include "goo/gtypes.h"
#include "CharTypes.h"

#if MULTITHREADED
#include "goo/GooMutex.h"
#endif

class GooString;
class Object;
struct CMapVectorEntry;
class CMapCache;
class Stream;

//------------------------------------------------------------------------

class CMap {
public:

  // Parse a CMap from <obj>, which can be a name or a stream.  Sets
  // the initial reference count to 1.  Returns NULL on failure.
  static CMap *parse(CMapCache *cache, GooString *collectionA, Object *obj);

  // Create the CMap specified by <collection> and <cMapName>.  Sets
  // the initial reference count to 1.  Returns NULL on failure.
  static CMap *parse(CMapCache *cache, GooString *collectionA,
		     GooString *cMapNameA);

  // Parse a CMap from <str>.  Sets the initial reference count to 1.
  // Returns NULL on failure.
  static CMap *parse(CMapCache *cache, GooString *collectionA, Stream *str);

  // Create the CMap specified by <collection> and <cMapName>.  Sets
  // the initial reference count to 1.
  // Stream is a stream containing the CMap, can be NULL and 
  // this means the CMap will be searched in the CMap files
  // Returns NULL on failure.
  static CMap *parse(CMapCache *cache, GooString *collectionA,
		     GooString *cMapNameA, Stream *stream);

  ~CMap();

  void incRefCnt();
  void decRefCnt();

  // Return collection name (<registry>-<ordering>).
  GooString *getCollection() { return collection; }

  GooString *getCMapName() { return cMapName; }

  // Return true if this CMap matches the specified <collectionA>, and
  // <cMapNameA>.
  GBool match(GooString *collectionA, GooString *cMapNameA);

  // Return the CID corresponding to the character code starting at
  // <s>, which contains <len> bytes.  Sets *<c> to the char code, and
  // *<nUsed> to the number of bytes used by the char code.
  CID getCID(char *s, int len, CharCode *c, int *nUsed);

  // Return the writing mode (0=horizontal, 1=vertical).
  int getWMode() { return wMode; }

  void setReverseMap(Guint *rmap, Guint rmapSize, Guint ncand);

private:

  void parse2(CMapCache *cache, int (*getCharFunc)(void *), void *data);
  CMap(GooString *collectionA, GooString *cMapNameA);
  CMap(GooString *collectionA, GooString *cMapNameA, int wModeA);
  void useCMap(CMapCache *cache, char *useName);
  void useCMap(CMapCache *cache, Object *obj);
  void copyVector(CMapVectorEntry *dest, CMapVectorEntry *src);
  void addCIDs(Guint start, Guint end, Guint nBytes, CID firstCID);
  void freeCMapVector(CMapVectorEntry *vec);
  void setReverseMapVector(Guint startCode, CMapVectorEntry *vec,
          Guint *rmap, Guint rmapSize, Guint ncand);

  GooString *collection;
  GooString *cMapName;
  GBool isIdent;		// true if this CMap is an identity mapping,
				//   or is based on one (via usecmap)
  int wMode;			// writing mode (0=horizontal, 1=vertical)
  CMapVectorEntry *vector;	// vector for first byte (NULL for
				//   identity CMap)
  int refCnt;
#if MULTITHREADED
  GooMutex mutex;
#endif
};

//------------------------------------------------------------------------

#define cMapCacheSize 4

class CMapCache {
public:

  CMapCache();
  ~CMapCache();

  // Get the <cMapName> CMap for the specified character collection.
  // Increments its reference count; there will be one reference for
  // the cache plus one for the caller of this function.
  // Stream is a stream containing the CMap, can be NULL and 
  // this means the CMap will be searched in the CMap files
  // Returns NULL on failure.
  CMap *getCMap(GooString *collection, GooString *cMapName, Stream *stream);

private:

  CMap *cache[cMapCacheSize];
};

#endif
