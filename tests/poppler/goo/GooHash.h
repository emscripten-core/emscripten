//========================================================================
//
// GooHash.h
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
// Copyright (C) 2012 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef GHASH_H
#define GHASH_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"

class GooString;
struct GooHashBucket;
struct GooHashIter;

//------------------------------------------------------------------------

class GooHash {
public:

  GooHash(GBool deleteKeysA = gFalse);
  ~GooHash();
  void add(GooString *key, void *val);
  void add(GooString *key, int val);
  void replace(GooString *key, void *val);
  void replace(GooString *key, int val);
  void *lookup(GooString *key);
  int lookupInt(GooString *key);
  void *lookup(const char *key);
  int lookupInt(const char *key);
  void *remove(GooString *key);
  int removeInt(GooString *key);
  void *remove(const char *key);
  int removeInt(const char *key);
  int getLength() { return len; }
  void startIter(GooHashIter **iter);
  GBool getNext(GooHashIter **iter, GooString **key, void **val);
  GBool getNext(GooHashIter **iter, GooString **key, int *val);
  void killIter(GooHashIter **iter);

private:
  GooHash(const GooHash &other);
  GooHash& operator=(const GooHash &other);

  void expand();
  GooHashBucket *find(GooString *key, int *h);
  GooHashBucket *find(const char *key, int *h);
  int hash(GooString *key);
  int hash(const char *key);

  GBool deleteKeys;		// set if key strings should be deleted
  int size;			// number of buckets
  int len;			// number of entries
  GooHashBucket **tab;
};

#define deleteGooHash(hash, T)                       \
  do {                                             \
    GooHash *_hash = (hash);                         \
    {                                              \
      GooHashIter *_iter;                            \
      GooString *_key;                               \
      void *_p;                                    \
      _hash->startIter(&_iter);                    \
      while (_hash->getNext(&_iter, &_key, &_p)) { \
        delete (T*)_p;                             \
      }                                            \
      delete _hash;                                \
    }                                              \
  } while(0)

#endif
