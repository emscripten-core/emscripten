//========================================================================
//
// Outline.cc
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Marco Pesenti Gritti <mpg@redhat.com>
// Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Nick Jones <nick.jones@network-box.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "XRef.h"
#include "Link.h"
#include "PDFDocEncoding.h"
#include "Outline.h"

//------------------------------------------------------------------------

Outline::Outline(Object *outlineObj, XRef *xref) {
  Object first, last;

  items = NULL;
  if (!outlineObj->isDict()) {
    return;
  }
  items = OutlineItem::readItemList(outlineObj->dictLookupNF("First", &first),
				    outlineObj->dictLookupNF("Last", &last),
				    xref);
  first.free();
  last.free();
}

Outline::~Outline() {
  if (items) {
    deleteGooList(items, OutlineItem);
  }
}

//------------------------------------------------------------------------

OutlineItem::OutlineItem(Dict *dict, XRef *xrefA) {
  Object obj1;
  GooString *s;
  int i;

  xref = xrefA;
  title = NULL;
  action = NULL;
  kids = NULL;

  if (dict->lookup("Title", &obj1)->isString()) {
    s = obj1.getString();
    if ((s->getChar(0) & 0xff) == 0xfe &&
	(s->getChar(1) & 0xff) == 0xff) {
      titleLen = (s->getLength() - 2) / 2;
      title = (Unicode *)gmallocn(titleLen, sizeof(Unicode));
      for (i = 0; i < titleLen; ++i) {
	title[i] = ((s->getChar(2 + 2*i) & 0xff) << 8) |
	           (s->getChar(3 + 2*i) & 0xff);
      }
    } else {
      titleLen = s->getLength();
      title = (Unicode *)gmallocn(titleLen, sizeof(Unicode));
      for (i = 0; i < titleLen; ++i) {
	title[i] = pdfDocEncoding[s->getChar(i) & 0xff];
      }
    }
  } else {
    titleLen = 0;
  }
  obj1.free();

  if (!dict->lookup("Dest", &obj1)->isNull()) {
    action = LinkAction::parseDest(&obj1);
  } else {
      obj1.free();
    if (!dict->lookup("A", &obj1)->isNull()) {
        action = LinkAction::parseAction(&obj1);
  }
  }
  obj1.free();

  dict->lookupNF("First", &firstRef);
  dict->lookupNF("Last", &lastRef);
  dict->lookupNF("Next", &nextRef);

  startsOpen = gFalse;
  if (dict->lookup("Count", &obj1)->isInt()) {
    if (obj1.getInt() > 0) {
      startsOpen = gTrue;
    }
  }
  obj1.free();
}

OutlineItem::~OutlineItem() {
  close();
  if (title) {
    gfree(title);
  }
  if (action) {
    delete action;
  }
  firstRef.free();
  lastRef.free();
  nextRef.free();
}

GooList *OutlineItem::readItemList(Object *firstItemRef, Object *lastItemRef,
				 XRef *xrefA) {
  GooList *items;
  char* alreadyRead;
  OutlineItem *item;
  Object obj;
  Object *p;

  if (!lastItemRef->isRef())
    return NULL;

  items = new GooList();

  alreadyRead = (char *)gmalloc(xrefA->getNumObjects());
  memset(alreadyRead, 0, xrefA->getNumObjects());

  p = firstItemRef;
  while (p->isRef() && 
	 (p->getRefNum() >= 0) && 
         (p->getRefNum() < xrefA->getNumObjects()) && 
         !alreadyRead[p->getRefNum()]) {
    if (!p->fetch(xrefA, &obj)->isDict()) {
      obj.free();
      break;
    }
    alreadyRead[p->getRefNum()] = 1;
    item = new OutlineItem(obj.getDict(), xrefA);
    obj.free();
    items->append(item);
    if (p->getRef().num == lastItemRef->getRef().num &&
	p->getRef().gen == lastItemRef->getRef().gen) {
      break;
    }
    p = &item->nextRef;
  }

  gfree(alreadyRead);

  if (!items->getLength()) {
    delete items;
    items = NULL;
  }

  return items;
}

void OutlineItem::open() {
  if (!kids) {
    kids = readItemList(&firstRef, &lastRef, xref);
  }
}

void OutlineItem::close() {
  if (kids) {
    deleteGooList(kids, OutlineItem);
    kids = NULL;
  }
}
