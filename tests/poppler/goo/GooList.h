//========================================================================
//
// GooList.h
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

#ifndef GLIST_H
#define GLIST_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"

//------------------------------------------------------------------------
// GooList
//------------------------------------------------------------------------

class GooList {
public:

  // Create an empty list.
  GooList();

  // Create an empty list with space for <size1> elements.
  GooList(int sizeA);

  // Destructor - does not free pointed-to objects.
  ~GooList();

  //----- general

  // Get the number of elements.
  int getLength() { return length; }

  // Returns a (shallow) copy of this list.
  GooList *copy();

  //----- ordered list support

  // Return the <i>th element.
  // Assumes 0 <= i < length.
  void *get(int i) { return data[i]; }

  // Replace the <i>th element.
  // Assumes 0 <= i < length.
  void put(int i, void *p) { data[i] = p; }

  // Append an element to the end of the list.
  void append(void *p);

  // Append another list to the end of this one.
  void append(GooList *list);

  // Insert an element at index <i>.
  // Assumes 0 <= i <= length.
  void insert(int i, void *p);

  // Deletes and returns the element at index <i>.
  // Assumes 0 <= i < length.
  void *del(int i);

  // Sort the list accoring to the given comparison function.
  // NB: this sorts an array of pointers, so the pointer args need to
  // be double-dereferenced.
  void sort(int (*cmp)(const void *ptr1, const void *ptr2));

  // Reverse the list.
  void reverse();

  //----- control

  // Set allocation increment to <inc>.  If inc > 0, that many
  // elements will be allocated every time the list is expanded.
  // If inc <= 0, the list will be doubled in size.
  void setAllocIncr(int incA) { inc = incA; }

private:
  GooList(const GooList &other);
  GooList& operator=(const GooList &other);

  void expand();
  void shrink();

  void **data;			// the list elements
  int size;			// size of data array
  int length;			// number of elements on list
  int inc;			// allocation increment
};

#define deleteGooList(list, T)                        \
  do {                                              \
    GooList *_list = (list);                          \
    {                                               \
      int _i;                                       \
      for (_i = 0; _i < _list->getLength(); ++_i) { \
        delete (T*)_list->get(_i);                  \
      }                                             \
      delete _list;                                 \
    }                                               \
  } while (0)

#endif
