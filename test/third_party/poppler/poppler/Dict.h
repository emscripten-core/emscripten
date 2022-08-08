//========================================================================
//
// Dict.h
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
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2007-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef DICT_H
#define DICT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"

//------------------------------------------------------------------------
// Dict
//------------------------------------------------------------------------

struct DictEntry {
  char *key;
  Object val;
};

class Dict {
public:

  // Constructor.
  Dict(XRef *xrefA);
  Dict(Dict* dictA);

  // Destructor.
  ~Dict();

  // Reference counting.
  int incRef() { return ++ref; }
  int decRef() { return --ref; }

  // Get number of entries.
  int getLength() { return length; }

  // Add an entry.  NB: does not copy key.
  void add(char *key, Object *val);

  // Update the value of an existing entry, otherwise create it
  void set(char *key, Object *val);
  // Remove an entry. This invalidate indexes
  void remove(char *key);

  // Check if dictionary is of specified type.
  GBool is(char *type);

  // Look up an entry and return the value.  Returns a null object
  // if <key> is not in the dictionary.
  Object *lookup(char *key, Object *obj, std::set<int> *fetchOriginatorNums = NULL);
  Object *lookupNF(char *key, Object *obj);
  GBool lookupInt(const char *key, const char *alt_key, int *value);

  // Iterative accessors.
  char *getKey(int i);
  Object *getVal(int i, Object *obj);
  Object *getValNF(int i, Object *obj);

  // Set the xref pointer.  This is only used in one special case: the
  // trailer dictionary, which is read before the xref table is
  // parsed.
  void setXRef(XRef *xrefA) { xref = xrefA; }
  
  XRef *getXRef() { return xref; }
  
  GBool hasKey(char *key);

private:

  GBool sorted;
  XRef *xref;			// the xref table for this PDF file
  DictEntry *entries;		// array of entries
  int size;			// size of <entries> array
  int length;			// number of entries in dictionary
  int ref;			// reference count

  DictEntry *find(char *key);
};

#endif
