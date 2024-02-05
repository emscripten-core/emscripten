//========================================================================
//
// Array.cc
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
#include "goo/gmem.h"
#include "Object.h"
#include "Array.h"

//------------------------------------------------------------------------
// Array
//------------------------------------------------------------------------

Array::Array(XRef *xrefA) {
  xref = xrefA;
  elems = NULL;
  size = length = 0;
  ref = 1;
}

Array::~Array() {
  int i;

  for (i = 0; i < length; ++i)
    elems[i].free();
  gfree(elems);
}

void Array::add(Object *elem) {
  if (length == size) {
    if (length == 0) {
      size = 8;
    } else {
      size *= 2;
    }
    elems = (Object *)greallocn(elems, size, sizeof(Object));
    // XXX Emscripten: Initialize the entries, to prevent undefined values
    for (int i=length; i<size; i++) {
      elems[i].zeroUnion();
    }
  }
  elems[length] = *elem;
  ++length;
}

Object *Array::get(int i, Object *obj) {
  if (i < 0 || i >= length) {
#ifdef DEBUG_MEM
    abort();
#else
    return obj->initNull();
#endif
  }
  return elems[i].fetch(xref, obj);
}

Object *Array::getNF(int i, Object *obj) {
  if (i < 0 || i >= length) {
#ifdef DEBUG_MEM
    abort();
#else
    return obj->initNull();
#endif
  }
  return elems[i].copy(obj);
}

GBool Array::getString(int i, GooString *string)
{
  Object obj;

  if (getNF(i, &obj)->isString()) {
    string->clear();
    string->append(obj.getString());
    obj.free();
    return gTrue;
  } else {
    obj.free();
    return gFalse;
  }
}
