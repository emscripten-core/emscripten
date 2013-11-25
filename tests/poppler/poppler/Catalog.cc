//========================================================================
//
// Catalog.cc
//
// Copyright 1996-2007 Glyph & Cog, LLC
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
// Copyright (C) 2005-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Jeff Muizelaar <jrmuizel@nit.ca>
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2005 Marco Pesenti Gritti <mpg@redhat.com>
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006, 2008, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008, 2011 Pino Toscano <pino@kde.org>
// Copyright (C) 2009 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Julien Nabet <serval2412@yahoo.fr>
// Copyright (C) 2013 Adrian Perez de Castro <aperez@igalia.com>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 José Aliste <jaliste@src.gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <stdlib.h>
#include "goo/gmem.h"
#include "Object.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "Array.h"
#include "Dict.h"
#include "Page.h"
#include "Error.h"
#include "Link.h"
#include "PageLabelInfo.h"
#include "Catalog.h"
#include "Form.h"
#include "OptionalContent.h"
#include "ViewerPreferences.h"
#include "FileSpec.h"
#include "StructTreeRoot.h"

#if MULTITHREADED
#  define catalogLocker()   MutexLocker locker(&mutex)
#else
#  define catalogLocker()
#endif
//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

Catalog::Catalog(PDFDoc *docA) {
  Object catDict, pagesDict, pagesDictRef;
  Object obj, obj2;
  Object optContentProps;

#if MULTITHREADED
  gInitMutex(&mutex);
#endif
  ok = gTrue;
  doc = docA;
  xref = doc->getXRef();
  pages = NULL;
  pageRefs = NULL;
  numPages = -1;
  pagesSize = 0;
  baseURI = NULL;
  pageLabelInfo = NULL;
  form = NULL;
  optContent = NULL;
  pageMode = pageModeNull;
  pageLayout = pageLayoutNull;
  destNameTree = NULL;
  embeddedFileNameTree = NULL;
  jsNameTree = NULL;
  viewerPrefs = NULL;
  structTreeRoot = NULL;

  pagesList = NULL;
  pagesRefList = NULL;
  attrsList = NULL;
  kidsIdxList = NULL;
  lastCachedPage = 0;
  markInfo = markInfoNull;

  xref->getCatalog(&catDict);
  if (!catDict.isDict()) {
    error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
    goto err1;
  }
  // get the AcroForm dictionary
  catDict.dictLookup("AcroForm", &acroForm);

  // read base URI
  if (catDict.dictLookup("URI", &obj)->isDict()) {
    if (obj.dictLookup("Base", &obj2)->isString()) {
      baseURI = obj2.getString()->copy();
    }
    obj2.free();
  }
  obj.free();

  // get the Optional Content dictionary
  if (catDict.dictLookup("OCProperties", &optContentProps)->isDict()) {
    optContent = new OCGs(&optContentProps, xref);
    if (!optContent->isOk ()) {
      delete optContent;
      optContent = NULL;
    }
  }
  optContentProps.free();

  // actions
  catDict.dictLookupNF("AA", &additionalActions);

  // get the ViewerPreferences dictionary
  catDict.dictLookup("ViewerPreferences", &viewerPreferences);
  catDict.free();
  return;

 err1:
  catDict.free();
  ok = gFalse;
}

Catalog::~Catalog() {
  delete kidsIdxList;
  if (attrsList) {
    std::vector<PageAttrs *>::iterator it;
    for (it = attrsList->begin() ; it != attrsList->end(); ++it ) {
      delete *it;
    }
    delete attrsList;
  }
  delete pagesRefList;
  if (pagesList) {
    std::vector<Dict *>::iterator it;
    for (it = pagesList->begin() ; it != pagesList->end(); ++it ) {
      if (!(*it)->decRef()) {
         delete *it;
      }
    }
    delete pagesList;
  }
  if (pages) {
    for (int i = 0; i < pagesSize; ++i) {
      if (pages[i]) {
	delete pages[i];
      }
    }
    gfree(pages);
    gfree(pageRefs);
  }
  names.free();
  dests.free();
  delete destNameTree;
  delete embeddedFileNameTree;
  delete jsNameTree;
  if (baseURI) {
    delete baseURI;
  }
  delete pageLabelInfo;
  delete form;
  delete optContent;
  delete viewerPrefs;
  delete structTreeRoot;
  metadata.free();
  outline.free();
  acroForm.free();
  viewerPreferences.free();
  additionalActions.free();
#if MULTITHREADED
  gDestroyMutex(&mutex);
#endif
}

GooString *Catalog::readMetadata() {
  GooString *s;
  Dict *dict;
  Object obj;

  catalogLocker();
  if (metadata.isNone()) {
    Object catDict;

    xref->getCatalog(&catDict);
    if (catDict.isDict()) {
      catDict.dictLookup("Metadata", &metadata);
    } else {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      metadata.initNull();
    }
    catDict.free();
  }

  if (!metadata.isStream()) {
    return NULL;
  }
  dict = metadata.streamGetDict();
  if (!dict->lookup("Subtype", &obj)->isName("XML")) {
    error(errSyntaxWarning, -1, "Unknown Metadata type: '{0:s}'",
	  obj.isName() ? obj.getName() : "???");
  }
  obj.free();
  s = new GooString();
  metadata.getStream()->fillGooString(s);
  metadata.streamClose();
  return s;
}

Page *Catalog::getPage(int i)
{
  if (i < 1) return NULL;

  catalogLocker();
  if (i > lastCachedPage) {
     GBool cached = cachePageTree(i);
     if ( cached == gFalse) {
       return NULL;
     }
  }
  return pages[i-1];
}

Ref *Catalog::getPageRef(int i)
{
  if (i < 1) return NULL;

  catalogLocker();
  if (i > lastCachedPage) {
     GBool cached = cachePageTree(i);
     if ( cached == gFalse) {
       return NULL;
     }
  }
  return &pageRefs[i-1];
}

GBool Catalog::cachePageTree(int page)
{
  Dict *pagesDict;

  if (pagesList == NULL) {

    Object catDict;
    Ref pagesRef;

    xref->getCatalog(&catDict);

    if (catDict.isDict()) {
      Object pagesDictRef;
      if (catDict.dictLookupNF("Pages", &pagesDictRef)->isRef() &&
          pagesDictRef.getRefNum() >= 0 &&
          pagesDictRef.getRefNum() < xref->getNumObjects()) {
        pagesRef = pagesDictRef.getRef();
        pagesDictRef.free();
      } else {
        error(errSyntaxError, -1, "Catalog dictionary does not contain a valid \"Pages\" entry");
        pagesDictRef.free();
        catDict.free();
        return gFalse;
      }
    } else {
      error(errSyntaxError, -1, "Could not find catalog dictionary");
      catDict.free();
      return gFalse;
    }

    Object obj;
    catDict.dictLookup("Pages", &obj);
    catDict.free();
    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    if (obj.isDict()) {
      obj.getDict()->incRef();
      pagesDict = obj.getDict();
      obj.free();
    }
    else {
      error(errSyntaxError, -1, "Top-level pages object is wrong type ({0:s})", obj.getTypeName());
      obj.free();
      return gFalse;
    }

    pagesSize = getNumPages();
    pages = (Page **)gmallocn(pagesSize, sizeof(Page *));
    pageRefs = (Ref *)gmallocn(pagesSize, sizeof(Ref));
    for (int i = 0; i < pagesSize; ++i) {
      pages[i] = NULL;
      pageRefs[i].num = -1;
      pageRefs[i].gen = -1;
    }

    pagesList = new std::vector<Dict *>();
    pagesList->push_back(pagesDict);
    pagesRefList = new std::vector<Ref>();
    pagesRefList->push_back(pagesRef);
    attrsList = new std::vector<PageAttrs *>();
    attrsList->push_back(new PageAttrs(NULL, pagesDict));
    kidsIdxList = new std::vector<int>();
    kidsIdxList->push_back(0);
    lastCachedPage = 0;

  }

  while(1) {

    if (page <= lastCachedPage) return gTrue;

    if (pagesList->empty()) return gFalse;

    pagesDict = pagesList->back();
    Object kids;
    pagesDict->lookup("Kids", &kids);
    if (!kids.isArray()) {
      error(errSyntaxError, -1, "Kids object (page {0:d}) is wrong type ({1:s})",
            lastCachedPage+1, kids.getTypeName());
      kids.free();
      return gFalse;
    }

    int kidsIdx = kidsIdxList->back();
    if (kidsIdx >= kids.arrayGetLength()) {
       if (!pagesList->back()->decRef()) {
         delete pagesList->back();
       }
       pagesList->pop_back();
       pagesRefList->pop_back();
       delete attrsList->back();
       attrsList->pop_back();
       kidsIdxList->pop_back();
       if (!kidsIdxList->empty()) kidsIdxList->back()++;
       kids.free();
       continue;
    }

    Object kidRef;
    kids.arrayGetNF(kidsIdx, &kidRef);
    if (!kidRef.isRef()) {
      error(errSyntaxError, -1, "Kid object (page {0:d}) is not an indirect reference ({1:s})",
            lastCachedPage+1, kidRef.getTypeName());
      kidRef.free();
      kids.free();
      return gFalse;
    }

    GBool loop = gFalse;;
    for (size_t i = 0; i < pagesRefList->size(); i++) {
      if (((*pagesRefList)[i]).num == kidRef.getRefNum()) {
         loop = gTrue;
         break;
      }
    }
    if (loop) {
      error(errSyntaxError, -1, "Loop in Pages tree");
      kidRef.free();
      kids.free();
      kidsIdxList->back()++;
      continue;
    }

    Object kid;
    kids.arrayGet(kidsIdx, &kid);
    kids.free();
    if (kid.isDict("Page") || (kid.isDict() && !kid.getDict()->hasKey("Kids"))) {
      PageAttrs *attrs = new PageAttrs(attrsList->back(), kid.getDict());
      Page *p = new Page(doc, lastCachedPage+1, kid.getDict(),
                     kidRef.getRef(), attrs, form);
      if (!p->isOk()) {
        error(errSyntaxError, -1, "Failed to create page (page {0:d})", lastCachedPage+1);
        delete p;
        kidRef.free();
        kid.free();
        return gFalse;
      }

      if (lastCachedPage >= numPages) {
        error(errSyntaxError, -1, "Page count in top-level pages object is incorrect");
        kidRef.free();
        kid.free();
        return gFalse;
      }

      pages[lastCachedPage] = p;
      pageRefs[lastCachedPage].num = kidRef.getRefNum();
      pageRefs[lastCachedPage].gen = kidRef.getRefGen();

      lastCachedPage++;
      kidsIdxList->back()++;

    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    } else if (kid.isDict()) {
      attrsList->push_back(new PageAttrs(attrsList->back(), kid.getDict()));
      pagesRefList->push_back(kidRef.getRef());
      kid.getDict()->incRef();
      pagesList->push_back(kid.getDict());
      kidsIdxList->push_back(0);
    } else {
      error(errSyntaxError, -1, "Kid object (page {0:d}) is wrong type ({1:s})",
            lastCachedPage+1, kid.getTypeName());
      kidsIdxList->back()++;
    }
    kidRef.free();
    kid.free();

  }

  return gFalse;
}

int Catalog::findPage(int num, int gen) {
  int i;

  for (i = 0; i < getNumPages(); ++i) {
    Ref *ref = getPageRef(i+1);
    if (ref != NULL && ref->num == num && ref->gen == gen)
      return i + 1;
  }
  return 0;
}

LinkDest *Catalog::findDest(GooString *name) {
  LinkDest *dest;
  Object obj1, obj2;
  GBool found;

  // try named destination dictionary then name tree
  found = gFalse;
  if (getDests()->isDict()) {
    if (!getDests()->dictLookup(name->getCString(), &obj1)->isNull())
      found = gTrue;
    else
      obj1.free();
  }
  if (!found) {
    catalogLocker();
    if (getDestNameTree()->lookup(name, &obj1))
      found = gTrue;
    else
      obj1.free();
  }
  if (!found)
    return NULL;

  // construct LinkDest
  dest = NULL;
  if (obj1.isArray()) {
    dest = new LinkDest(obj1.getArray());
  } else if (obj1.isDict()) {
    if (obj1.dictLookup("D", &obj2)->isArray())
      dest = new LinkDest(obj2.getArray());
    else
      error(errSyntaxWarning, -1, "Bad named destination value");
    obj2.free();
  } else {
    error(errSyntaxWarning, -1, "Bad named destination value");
  }
  obj1.free();
  if (dest && !dest->isOk()) {
    delete dest;
    dest = NULL;
  }

  return dest;
}

FileSpec *Catalog::embeddedFile(int i)
{
    Object efDict;
    Object obj;
    catalogLocker();
    obj = getEmbeddedFileNameTree()->getValue(i);
    FileSpec *embeddedFile = 0;
    if (obj.isRef()) {
      Object fsDict;
      embeddedFile = new FileSpec(obj.fetch(xref, &fsDict));
      fsDict.free();
    } else if (obj.isDict()) {
      embeddedFile = new FileSpec(&obj);
    } else {
      Object null;
      embeddedFile = new FileSpec(&null);
    }
    return embeddedFile;
}

GooString *Catalog::getJS(int i)
{
  Object obj;
  // getJSNameTree()->getValue(i) returns a shallow copy of the object so we
  // do not need to free it
  catalogLocker();
  getJSNameTree()->getValue(i).fetch(xref, &obj);

  if (!obj.isDict()) {
    obj.free();
    return 0;
  }
  Object obj2;
  if (!obj.dictLookup("S", &obj2)->isName()) {
    obj2.free();
    obj.free();
    return 0;
  }
  if (strcmp(obj2.getName(), "JavaScript")) {
    obj2.free();
    obj.free();
    return 0;
  }
  obj2.free();
  obj.dictLookup("JS", &obj2);
  GooString *js = 0;
  if (obj2.isString()) {
    js = new GooString(obj2.getString());
  }
  else if (obj2.isStream()) {
    Stream *stream = obj2.getStream();
    js = new GooString();
    stream->fillGooString(js);
  }
  obj2.free();
  obj.free();
  return js;
}

Catalog::PageMode Catalog::getPageMode() {

  catalogLocker();
  if (pageMode == pageModeNull) {

    Object catDict, obj;

    pageMode = pageModeNone;

    xref->getCatalog(&catDict);
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      catDict.free();
      return pageMode;
    }

    if (catDict.dictLookup("PageMode", &obj)->isName()) {
      if (obj.isName("UseNone"))
        pageMode = pageModeNone;
      else if (obj.isName("UseOutlines"))
        pageMode = pageModeOutlines;
      else if (obj.isName("UseThumbs"))
        pageMode = pageModeThumbs;
      else if (obj.isName("FullScreen"))
        pageMode = pageModeFullScreen;
      else if (obj.isName("UseOC"))
        pageMode = pageModeOC;
      else if (obj.isName("UseAttachments"))
        pageMode = pageModeAttach;
    }
    obj.free();
    catDict.free();
  }
  return pageMode;
}

Catalog::PageLayout Catalog::getPageLayout() {

  catalogLocker();
  if (pageLayout == pageLayoutNull) {

    Object catDict, obj;

    pageLayout = pageLayoutNone;

    xref->getCatalog(&catDict);
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      catDict.free();
      return pageLayout;
    }

    pageLayout = pageLayoutNone;
    if (catDict.dictLookup("PageLayout", &obj)->isName()) {
      if (obj.isName("SinglePage"))
        pageLayout = pageLayoutSinglePage;
      if (obj.isName("OneColumn"))
        pageLayout = pageLayoutOneColumn;
      if (obj.isName("TwoColumnLeft"))
        pageLayout = pageLayoutTwoColumnLeft;
      if (obj.isName("TwoColumnRight"))
        pageLayout = pageLayoutTwoColumnRight;
      if (obj.isName("TwoPageLeft"))
        pageLayout = pageLayoutTwoPageLeft;
      if (obj.isName("TwoPageRight"))
        pageLayout = pageLayoutTwoPageRight;
    }
    obj.free();
    catDict.free();
  }
  return pageLayout;
}

NameTree::NameTree()
{
  size = 0;
  length = 0;
  entries = NULL;
}

NameTree::~NameTree()
{
  int i;

  for (i = 0; i < length; i++)
    delete entries[i];

  gfree(entries);
}

NameTree::Entry::Entry(Array *array, int index) {
    if (!array->getString(index, &name) || !array->getNF(index + 1, &value)) {
      Object aux;
      array->get(index, &aux);
      if (aux.isString() && array->getNF(index + 1, &value) )
      {
        name.append(aux.getString());
      }
      else
        error(errSyntaxError, -1, "Invalid page tree");
    }
}

NameTree::Entry::~Entry() {
  value.free();
}

void NameTree::addEntry(Entry *entry)
{
  if (length == size) {
    if (length == 0) {
      size = 8;
    } else {
      size *= 2;
    }
    entries = (Entry **) grealloc (entries, sizeof (Entry *) * size);
  }

  entries[length] = entry;
  ++length;
}

int NameTree::Entry::cmpEntry(const void *voidEntry, const void *voidOtherEntry)
{
  Entry *entry = *(NameTree::Entry **) voidEntry;
  Entry *otherEntry = *(NameTree::Entry **) voidOtherEntry;

  return entry->name.cmp(&otherEntry->name);
}

void NameTree::init(XRef *xrefA, Object *tree) {
  xref = xrefA;
  parse(tree);
  if (entries && length > 0) {
    qsort(entries, length, sizeof(Entry *), Entry::cmpEntry);
  }
}

void NameTree::parse(Object *tree) {
  Object names;
  Object kids, kid;
  int i;

  if (!tree->isDict())
    return;

  // leaf node
  if (tree->dictLookup("Names", &names)->isArray()) {
    for (i = 0; i < names.arrayGetLength(); i += 2) {
      NameTree::Entry *entry;

      entry = new Entry(names.getArray(), i);
      addEntry(entry);
    }
  }
  names.free();

  // root or intermediate node
  if (tree->dictLookup("Kids", &kids)->isArray()) {
    for (i = 0; i < kids.arrayGetLength(); ++i) {
      if (kids.arrayGet(i, &kid)->isDict())
	parse(&kid);
      kid.free();
    }
  }
  kids.free();
}

int NameTree::Entry::cmp(const void *voidKey, const void *voidEntry)
{
  GooString *key = (GooString *) voidKey;
  Entry *entry = *(NameTree::Entry **) voidEntry;

  return key->cmp(&entry->name);
}

GBool NameTree::lookup(GooString *name, Object *obj)
{
  Entry **entry;

  entry = (Entry **) bsearch(name, entries,
			     length, sizeof(Entry *), Entry::cmp);
  if (entry != NULL) {
    (*entry)->value.fetch(xref, obj);
    return gTrue;
  } else {
    printf("failed to look up %s\n", name->getCString());
    obj->initNull();
    return gFalse;
  }
}

Object NameTree::getValue(int index)
{
  if (index < length) {
    return entries[index]->value;
  } else {
    return Object();
  }
}

GooString *NameTree::getName(int index)
{
    if (index < length) {
	return &entries[index]->name;
    } else {
	return NULL;
    }
}

GBool Catalog::labelToIndex(GooString *label, int *index)
{
  char *end;

  PageLabelInfo *pli = getPageLabelInfo();
  if (pli != NULL) {
    if (!pli->labelToIndex(label, index))
      return gFalse;
  } else {
    *index = strtol(label->getCString(), &end, 10) - 1;
    if (*end != '\0')
      return gFalse;
  }

  if (*index < 0 || *index >= getNumPages())
    return gFalse;

  return gTrue;
}

GBool Catalog::indexToLabel(int index, GooString *label)
{
  char buffer[32];

  if (index < 0 || index >= getNumPages())
    return gFalse;

  PageLabelInfo *pli = getPageLabelInfo();
  if (pli != NULL) {
    return pli->indexToLabel(index, label);
  } else {
    snprintf(buffer, sizeof (buffer), "%d", index + 1);
    label->append(buffer);	      
    return gTrue;
  }
}

int Catalog::getNumPages()
{
  catalogLocker();
  if (numPages == -1)
  {
    Object catDict, pagesDict, obj;

    xref->getCatalog(&catDict);
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      catDict.free();
      return 0;
    }
    catDict.dictLookup("Pages", &pagesDict);
    catDict.free();

    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    if (!pagesDict.isDict()) {
      error(errSyntaxError, -1, "Top-level pages object is wrong type ({0:s})",
          pagesDict.getTypeName());
      pagesDict.free();
      return 0;
    }

    pagesDict.dictLookup("Count", &obj);
    // some PDF files actually use real numbers here ("/Count 9.0")
    if (!obj.isNum()) {
      error(errSyntaxError, -1, "Page count in top-level pages object is wrong type ({0:s})",
         obj.getTypeName());
      numPages = 0;
    } else {
      numPages = (int)obj.getNum();
    }

    obj.free();
    pagesDict.free();
  }

  return numPages;
}

PageLabelInfo *Catalog::getPageLabelInfo()
{
  catalogLocker();
  if (!pageLabelInfo) {
    Object catDict;
    Object obj;

    xref->getCatalog(&catDict);
    if (!catDict.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
      catDict.free();
      return NULL;
    }

    if (catDict.dictLookup("PageLabels", &obj)->isDict()) {
      pageLabelInfo = new PageLabelInfo(&obj, getNumPages());
    }
    obj.free();
    catDict.free();
  }

  return pageLabelInfo;
}

StructTreeRoot *Catalog::getStructTreeRoot()
{
  catalogLocker();
  if (!structTreeRoot) {
    Object catalog;
    Object root;

    xref->getCatalog(&catalog);
    if (!catalog.isDict()) {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catalog.getTypeName());
      catalog.free();
      return NULL;
    }

    if (catalog.dictLookup("StructTreeRoot", &root)->isDict("StructTreeRoot")) {
      structTreeRoot = new StructTreeRoot(doc, root.getDict());
    }

    root.free();
    catalog.free();
  }
  return structTreeRoot;
}

Guint Catalog::getMarkInfo()
{
  if (markInfo == markInfoNull) {
    markInfo = 0;

    Object catDict;
    catalogLocker();
    xref->getCatalog(&catDict);

    if (catDict.isDict()) {
      Object markInfoDict;
      catDict.dictLookup("MarkInfo", &markInfoDict);
      if (markInfoDict.isDict()) {
        Object value;

        if (markInfoDict.dictLookup("Marked", &value)->isBool() && value.getBool())
          markInfo |= markInfoMarked;
        else if (!value.isNull())
          error(errSyntaxError, -1, "Marked object is wrong type ({0:s})", value.getTypeName());
        value.free();

        if (markInfoDict.dictLookup("Suspects", &value)->isBool() && value.getBool())
          markInfo |= markInfoSuspects;
        else if (!value.isNull())
          error(errSyntaxError, -1, "Suspects object is wrong type ({0:s})", value.getTypeName());
        value.free();

        if (markInfoDict.dictLookup("UserProperties", &value)->isBool() && value.getBool())
          markInfo |= markInfoUserProperties;
        else if (!value.isNull())
          error(errSyntaxError, -1, "UserProperties object is wrong type ({0:s})", value.getTypeName());
        value.free();
      } else if (!markInfoDict.isNull()) {
        error(errSyntaxError, -1, "MarkInfo object is wrong type ({0:s})", markInfoDict.getTypeName());
      }
      markInfoDict.free();
    } else {
      error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
    }
    catDict.free();
  }
  return markInfo;
}

Object *Catalog::getOutline()
{
  catalogLocker();
  if (outline.isNone())
  {
     Object catDict;

     xref->getCatalog(&catDict);
     if (catDict.isDict()) {
       catDict.dictLookup("Outlines", &outline);
     } else {
       error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
       outline.initNull();
     }
     catDict.free();
  }

  return &outline;
}

Object *Catalog::getDests()
{
  catalogLocker();
  if (dests.isNone())
  {
     Object catDict;

     xref->getCatalog(&catDict);
     if (catDict.isDict()) {
       catDict.dictLookup("Dests", &dests);
     } else {
       error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
       dests.initNull();
     }
     catDict.free();
  }

  return &dests;
}

Catalog::FormType Catalog::getFormType()
{
  Object xfa;
  FormType res = NoForm;

  if (acroForm.isDict()) {
    acroForm.dictLookup("XFA", &xfa);
    if (xfa.isStream() || xfa.isArray()) {
      res = XfaForm;
    } else {
      res = AcroForm;
    }
    xfa.free();
  }

  return res;
}

Form *Catalog::getForm()
{
  catalogLocker();
  if (!form) {
    if (acroForm.isDict()) {
      form = new Form(doc, &acroForm);
      // perform form-related loading after all widgets have been loaded
      form->postWidgetsLoad();
    }
  }

  return form;
}

ViewerPreferences *Catalog::getViewerPreferences()
{
  catalogLocker();
  if (!viewerPrefs) {
    if (viewerPreferences.isDict()) {
      viewerPrefs = new ViewerPreferences(viewerPreferences.getDict());
    }
  }

  return viewerPrefs;
}

Object *Catalog::getNames()
{
  if (names.isNone())
  {
     Object catDict;

     xref->getCatalog(&catDict);
     if (catDict.isDict()) {
       catDict.dictLookup("Names", &names);
     } else {
       error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})", catDict.getTypeName());
       names.initNull();
     }
     catDict.free();
  }

  return &names;
}

NameTree *Catalog::getDestNameTree()
{
  if (!destNameTree) {

    destNameTree = new NameTree();

    if (getNames()->isDict()) {
       Object obj;

       getNames()->dictLookup("Dests", &obj);
       destNameTree->init(xref, &obj);
       obj.free();
    }

  }

  return destNameTree;
}

NameTree *Catalog::getEmbeddedFileNameTree()
{
  if (!embeddedFileNameTree) {

    embeddedFileNameTree = new NameTree();

    if (getNames()->isDict()) {
       Object obj;

       getNames()->dictLookup("EmbeddedFiles", &obj);
       embeddedFileNameTree->init(xref, &obj);
       obj.free();
    }

  }

  return embeddedFileNameTree;
}

NameTree *Catalog::getJSNameTree()
{
  if (!jsNameTree) {

    jsNameTree = new NameTree();

    if (getNames()->isDict()) {
       Object obj;

       getNames()->dictLookup("JavaScript", &obj);
       jsNameTree->init(xref, &obj);
       obj.free();
    }

  }

  return jsNameTree;
}

LinkAction* Catalog::getAdditionalAction(DocumentAdditionalActionsType type) {
  Object additionalActionsObject;
  LinkAction *linkAction = NULL;

  if (additionalActions.fetch(doc->getXRef(), &additionalActionsObject)->isDict()) {
    const char *key = (type == actionCloseDocument ?       "WC" :
                       type == actionSaveDocumentStart ?   "WS" :
                       type == actionSaveDocumentFinish ?  "DS" :
                       type == actionPrintDocumentStart ?  "WP" :
                       type == actionPrintDocumentFinish ? "DP" : NULL);

    Object actionObject;

    if (additionalActionsObject.dictLookup(key, &actionObject)->isDict())
      linkAction = LinkAction::parseAction(&actionObject, doc->getCatalog()->getBaseURI());
    actionObject.free();
  }

  additionalActionsObject.free();

  return linkAction;
}
