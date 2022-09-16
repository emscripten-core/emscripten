//========================================================================
//
// GooHash.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "gmem.h"
#include "GooString.h"
#include "GooHash.h"

//------------------------------------------------------------------------

struct GooHashBucket {
  GooString *key;
  union {
    void *p;
    int i;
  } val;
  GooHashBucket *next;
};

struct GooHashIter {
  int h;
  GooHashBucket *p;
};

//------------------------------------------------------------------------

GooHash::GooHash(GBool deleteKeysA) {
  int h;

  deleteKeys = deleteKeysA;
  size = 7;
  tab = (GooHashBucket **)gmallocn(size, sizeof(GooHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = NULL;
  }
  len = 0;
}

GooHash::~GooHash() {
  GooHashBucket *p;
  int h;

  for (h = 0; h < size; ++h) {
    while (tab[h]) {
      p = tab[h];
      tab[h] = p->next;
      if (deleteKeys) {
	delete p->key;
      }
      delete p;
    }
  }
  gfree(tab);
}

void GooHash::add(GooString *key, void *val) {
  GooHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GooHashBucket;
  p->key = key;
  p->val.p = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GooHash::add(GooString *key, int val) {
  GooHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GooHashBucket;
  p->key = key;
  p->val.i = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GooHash::replace(GooString *key, void *val) {
  GooHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.p = val;
    delete key;
  } else {
    add(key, val);
  }
}

void GooHash::replace(GooString *key, int val) {
  GooHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.i = val;
    delete key;
  } else {
    add(key, val);
  }
}

void *GooHash::lookup(GooString *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  return p->val.p;
}

int GooHash::lookupInt(GooString *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GooHash::lookup(char *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  return p->val.p;
}

int GooHash::lookupInt(char *key) {
  GooHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GooHash::remove(GooString *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GooHash::removeInt(GooString *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void *GooHash::remove(char *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GooHash::removeInt(char *key) {
  GooHashBucket *p;
  GooHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void GooHash::startIter(GooHashIter **iter) {
  *iter = new GooHashIter;
  (*iter)->h = -1;
  (*iter)->p = NULL;
}

GBool GooHash::getNext(GooHashIter **iter, GooString **key, void **val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = NULL;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.p;
  return gTrue;
}

GBool GooHash::getNext(GooHashIter **iter, GooString **key, int *val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = NULL;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.i;
  return gTrue;
}

void GooHash::killIter(GooHashIter **iter) {
  delete *iter;
  *iter = NULL;
}

void GooHash::expand() {
  GooHashBucket **oldTab;
  GooHashBucket *p;
  int oldSize, h, i;

  oldSize = size;
  oldTab = tab;
  size = 2*size + 1;
  tab = (GooHashBucket **)gmallocn(size, sizeof(GooHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = NULL;
  }
  for (i = 0; i < oldSize; ++i) {
    while (oldTab[i]) {
      p = oldTab[i];
      oldTab[i] = oldTab[i]->next;
      h = hash(p->key);
      p->next = tab[h];
      tab[h] = p;
    }
  }
  gfree(oldTab);
}

GooHashBucket *GooHash::find(GooString *key, int *h) {
  GooHashBucket *p;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return NULL;
}

GooHashBucket *GooHash::find(char *key, int *h) {
  GooHashBucket *p;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return NULL;
}

int GooHash::hash(GooString *key) {
  char *p;
  unsigned int h;
  int i;

  h = 0;
  for (p = key->getCString(), i = 0; i < key->getLength(); ++p, ++i) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}

int GooHash::hash(char *key) {
  char *p;
  unsigned int h;

  h = 0;
  for (p = key; *p; ++p) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}
