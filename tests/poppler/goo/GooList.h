//========================================================================
//
// GooList.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
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

  //----- ordered list support

  // Return the <i>th element.
  // Assumes 0 <= i < length.
  void *get(int i) { return data[i]; }

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

  //----- control

  // Set allocation increment to <inc>.  If inc > 0, that many
  // elements will be allocated every time the list is expanded.
  // If inc <= 0, the list will be doubled in size.
  void setAllocIncr(int incA) { inc = incA; }

private:

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
