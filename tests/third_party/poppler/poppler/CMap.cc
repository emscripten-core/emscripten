//========================================================================
//
// CMap.cc
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
// Copyright (C) 2008, 2009 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "goo/GooString.h"
#include "Error.h"
#include "GlobalParams.h"
#include "PSTokenizer.h"
#include "CMap.h"
#include "Object.h"

//------------------------------------------------------------------------

struct CMapVectorEntry {
  GBool isVector;
  union {
    CMapVectorEntry *vector;
    CID cid;
  };
};

//------------------------------------------------------------------------

static int getCharFromFile(void *data) {
  return fgetc((FILE *)data);
}

static int getCharFromStream(void *data) {
  return ((Stream *)data)->getChar();
}

//------------------------------------------------------------------------

CMap *CMap::parse(CMapCache *cache, GooString *collectionA,
		  GooString *cMapNameA, Stream *stream) {
  FILE *f = NULL;
  CMap *cmap;
  PSTokenizer *pst;
  char tok1[256], tok2[256], tok3[256];
  int n1, n2, n3;
  Guint start, end, code;

  if (stream) {
    stream->reset();
    pst = new PSTokenizer(&getCharFromStream, stream);
  } else {
    if (!(f = globalParams->findCMapFile(collectionA, cMapNameA))) {

      // Check for an identity CMap.
      if (!cMapNameA->cmp("Identity") || !cMapNameA->cmp("Identity-H")) {
        return new CMap(collectionA->copy(), cMapNameA->copy(), 0);
      }
      if (!cMapNameA->cmp("Identity-V")) {
        return new CMap(collectionA->copy(), cMapNameA->copy(), 1);
      }

      error(-1, "Couldn't find '%s' CMap file for '%s' collection",
	    cMapNameA->getCString(), collectionA->getCString());
      return NULL;
    }
    pst = new PSTokenizer(&getCharFromFile, f);
  }

  cmap = new CMap(collectionA->copy(), cMapNameA->copy());

  pst->getToken(tok1, sizeof(tok1), &n1);
  while (pst->getToken(tok2, sizeof(tok2), &n2)) {
    if (!strcmp(tok2, "usecmap")) {
      if (tok1[0] == '/') {
	cmap->useCMap(cache, tok1 + 1);
      }
      pst->getToken(tok1, sizeof(tok1), &n1);
    } else if (!strcmp(tok1, "/WMode")) {
      cmap->wMode = atoi(tok2);
      pst->getToken(tok1, sizeof(tok1), &n1);
    } else if (!strcmp(tok2, "begincodespacerange")) {
      while (pst->getToken(tok1, sizeof(tok1), &n1)) {
	if (!strcmp(tok1, "endcodespacerange")) {
	  break;
	}
	if (!pst->getToken(tok2, sizeof(tok2), &n2) ||
	    !strcmp(tok2, "endcodespacerange")) {
	  error(-1, "Illegal entry in codespacerange block in CMap");
	  break;
	}
	if (tok1[0] == '<' && tok2[0] == '<' &&
	    n1 == n2 && n1 >= 4 && (n1 & 1) == 0) {
	  tok1[n1 - 1] = tok2[n1 - 1] = '\0';
	  sscanf(tok1 + 1, "%x", &start);
	  sscanf(tok2 + 1, "%x", &end);
	  n1 = (n1 - 2) / 2;
	  cmap->addCodeSpace(cmap->vector, start, end, n1);
	}
      }
      pst->getToken(tok1, sizeof(tok1), &n1);
    } else if (!strcmp(tok2, "begincidchar")) {
      while (pst->getToken(tok1, sizeof(tok1), &n1)) {
	if (!strcmp(tok1, "endcidchar")) {
	  break;
	}
	if (!pst->getToken(tok2, sizeof(tok2), &n2) ||
	    !strcmp(tok2, "endcidchar")) {
	  error(-1, "Illegal entry in cidchar block in CMap");
	  break;
	}
	if (!(tok1[0] == '<' && tok1[n1 - 1] == '>' &&
	      n1 >= 4 && (n1 & 1) == 0)) {
	  error(-1, "Illegal entry in cidchar block in CMap");
	  continue;
	}
	tok1[n1 - 1] = '\0';
	if (sscanf(tok1 + 1, "%x", &code) != 1) {
	  error(-1, "Illegal entry in cidchar block in CMap");
	  continue;
	}
	n1 = (n1 - 2) / 2;
	cmap->addCIDs(code, code, n1, (CID)atoi(tok2));
      }
      pst->getToken(tok1, sizeof(tok1), &n1);
    } else if (!strcmp(tok2, "begincidrange")) {
      while (pst->getToken(tok1, sizeof(tok1), &n1)) {
	if (!strcmp(tok1, "endcidrange")) {
	  break;
	}
	if (!pst->getToken(tok2, sizeof(tok2), &n2) ||
	    !strcmp(tok2, "endcidrange") ||
	    !pst->getToken(tok3, sizeof(tok3), &n3) ||
	    !strcmp(tok3, "endcidrange")) {
	  error(-1, "Illegal entry in cidrange block in CMap");
	  break;
	}
	if (tok1[0] == '<' && tok2[0] == '<' &&
	    n1 == n2 && n1 >= 4 && (n1 & 1) == 0) {
	  tok1[n1 - 1] = tok2[n1 - 1] = '\0';
	  sscanf(tok1 + 1, "%x", &start);
	  sscanf(tok2 + 1, "%x", &end);
	  n1 = (n1 - 2) / 2;
	  cmap->addCIDs(start, end, n1, (CID)atoi(tok3));
	}
      }
      pst->getToken(tok1, sizeof(tok1), &n1);
    } else {
      strcpy(tok1, tok2);
    }
  }
  delete pst;

  if (f) {
    fclose(f);
  }

  return cmap;
}

CMap::CMap(GooString *collectionA, GooString *cMapNameA) {
  int i;

  collection = collectionA;
  cMapName = cMapNameA;
  wMode = 0;
  vector = (CMapVectorEntry *)gmallocn(256, sizeof(CMapVectorEntry));
  for (i = 0; i < 256; ++i) {
    vector[i].isVector = gFalse;
    vector[i].cid = 0;
  }
  refCnt = 1;
#if MULTITHREADED
  gInitMutex(&mutex);
#endif
}

CMap::CMap(GooString *collectionA, GooString *cMapNameA, int wModeA) {
  collection = collectionA;
  cMapName = cMapNameA;
  wMode = wModeA;
  vector = NULL;
  refCnt = 1;
#if MULTITHREADED
  gInitMutex(&mutex);
#endif
}

void CMap::useCMap(CMapCache *cache, char *useName) {
  GooString *useNameStr;
  CMap *subCMap;

  useNameStr = new GooString(useName);
  subCMap = cache->getCMap(collection, useNameStr, NULL);
  delete useNameStr;
  if (!subCMap) {
    return;
  }
  copyVector(vector, subCMap->vector);
  subCMap->decRefCnt();
}

void CMap::copyVector(CMapVectorEntry *dest, CMapVectorEntry *src) {
  int i, j;

  for (i = 0; i < 256; ++i) {
    if (src[i].isVector) {
      if (!dest[i].isVector) {
	dest[i].isVector = gTrue;
	dest[i].vector =
	  (CMapVectorEntry *)gmallocn(256, sizeof(CMapVectorEntry));
	for (j = 0; j < 256; ++j) {
	  dest[i].vector[j].isVector = gFalse;
	  dest[i].vector[j].cid = 0;
	}
      }
      copyVector(dest[i].vector, src[i].vector);
    } else {
      if (dest[i].isVector) {
	error(-1, "Collision in usecmap");
      } else {
	dest[i].cid = src[i].cid;
      }
    }
  }
}

void CMap::addCodeSpace(CMapVectorEntry *vec, Guint start, Guint end,
			Guint nBytes) {
  Guint start2, end2;
  int startByte, endByte, i, j;

  if (nBytes > 1) {
    startByte = (start >> (8 * (nBytes - 1))) & 0xff;
    endByte = (end >> (8 * (nBytes - 1))) & 0xff;
    start2 = start & ((1 << (8 * (nBytes - 1))) - 1);
    end2 = end & ((1 << (8 * (nBytes - 1))) - 1);
    for (i = startByte; i <= endByte; ++i) {
      if (!vec[i].isVector) {
	vec[i].isVector = gTrue;
	vec[i].vector =
	  (CMapVectorEntry *)gmallocn(256, sizeof(CMapVectorEntry));
	for (j = 0; j < 256; ++j) {
	  vec[i].vector[j].isVector = gFalse;
	  vec[i].vector[j].cid = 0;
	}
      }
      addCodeSpace(vec[i].vector, start2, end2, nBytes - 1);
    }
  }
}

void CMap::addCIDs(Guint start, Guint end, Guint nBytes, CID firstCID) {
  CMapVectorEntry *vec;
  CID cid;
  int byte;
  Guint i;

  vec = vector;
  for (i = nBytes - 1; i >= 1; --i) {
    byte = (start >> (8 * i)) & 0xff;
    if (!vec[byte].isVector) {
      error(-1, "Invalid CID (%0*x - %0*x) in CMap",
	    2*nBytes, start, 2*nBytes, end);
      return;
    }
    vec = vec[byte].vector;
  }
  cid = firstCID;
  for (byte = (int)(start & 0xff); byte <= (int)(end & 0xff); ++byte) {
    if (vec[byte].isVector) {
      error(-1, "Invalid CID (%0*x - %0*x) in CMap",
	    2*nBytes, start, 2*nBytes, end);
    } else {
      vec[byte].cid = cid;
    }
    ++cid;
  }
}

CMap::~CMap() {
  delete collection;
  delete cMapName;
  if (vector) {
    freeCMapVector(vector);
  }
#if MULTITHREADED
  gDestroyMutex(&mutex);
#endif
}

void CMap::freeCMapVector(CMapVectorEntry *vec) {
  int i;

  for (i = 0; i < 256; ++i) {
    if (vec[i].isVector) {
      freeCMapVector(vec[i].vector);
    }
  }
  gfree(vec);
}

void CMap::incRefCnt() {
#if MULTITHREADED
  gLockMutex(&mutex);
#endif
  ++refCnt;
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
}

void CMap::decRefCnt() {
  GBool done;

#if MULTITHREADED
  gLockMutex(&mutex);
#endif
  done = --refCnt == 0;
#if MULTITHREADED
  gUnlockMutex(&mutex);
#endif
  if (done) {
    delete this;
  }
}

GBool CMap::match(GooString *collectionA, GooString *cMapNameA) {
  return !collection->cmp(collectionA) && !cMapName->cmp(cMapNameA);
}

CID CMap::getCID(char *s, int len, int *nUsed) {
  CMapVectorEntry *vec;
  int n, i;

  if (!(vec = vector)) {
    // identity CMap
    *nUsed = 2;
    if (len < 2) {
      return 0;
    }
    return ((s[0] & 0xff) << 8) + (s[1] & 0xff);
  }
  n = 0;
  while (1) {
    if (n >= len) {
      *nUsed = n;
      return 0;
    }
    i = s[n++] & 0xff;
    if (!vec[i].isVector) {
      *nUsed = n;
      return vec[i].cid;
    }
    vec = vec[i].vector;
  }
}

void CMap::setReverseMapVector(Guint startCode, CMapVectorEntry *vec,
 Guint *rmap, Guint rmapSize, Guint ncand) {
  int i;

  if (vec == 0) return;
  for (i = 0;i < 256;i++) {
    if (vec[i].isVector) {
      setReverseMapVector((startCode+i) << 8,
	  vec[i].vector,rmap,rmapSize,ncand);
    } else {
      Guint cid = vec[i].cid;

      if (cid < rmapSize) {
	Guint cand;

	for (cand = 0;cand < ncand;cand++) {
	  Guint code = startCode+i;
	  Guint idx = cid*ncand+cand;
	  if (rmap[idx] == 0) {
	    rmap[idx] = code;
	    break;
	  } else if (rmap[idx] == code) {
	    break;
	  }
	}
      }
    }
  }
}

void CMap::setReverseMap(Guint *rmap, Guint rmapSize, Guint ncand) {
  setReverseMapVector(0,vector,rmap,rmapSize,ncand);
}

//------------------------------------------------------------------------

CMapCache::CMapCache() {
  int i;

  for (i = 0; i < cMapCacheSize; ++i) {
    cache[i] = NULL;
  }
}

CMapCache::~CMapCache() {
  int i;

  for (i = 0; i < cMapCacheSize; ++i) {
    if (cache[i]) {
      cache[i]->decRefCnt();
    }
  }
}

CMap *CMapCache::getCMap(GooString *collection, GooString *cMapName, Stream *stream) {
  CMap *cmap;
  int i, j;

  if (cache[0] && cache[0]->match(collection, cMapName)) {
    cache[0]->incRefCnt();
    return cache[0];
  }
  for (i = 1; i < cMapCacheSize; ++i) {
    if (cache[i] && cache[i]->match(collection, cMapName)) {
      cmap = cache[i];
      for (j = i; j >= 1; --j) {
	cache[j] = cache[j - 1];
      }
      cache[0] = cmap;
      cmap->incRefCnt();
      return cmap;
    }
  }
  if ((cmap = CMap::parse(this, collection, cMapName, stream))) {
    if (cache[cMapCacheSize - 1]) {
      cache[cMapCacheSize - 1]->decRefCnt();
    }
    for (j = cMapCacheSize - 1; j >= 1; --j) {
      cache[j] = cache[j - 1];
    }
    cache[0] = cmap;
    cmap->incRefCnt();
    return cmap;
  }
  return NULL;
}
