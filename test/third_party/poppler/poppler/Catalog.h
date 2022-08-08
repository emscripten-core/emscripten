//========================================================================
//
// Catalog.h
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
// Copyright (C) 2005, 2007, 2009, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2005, 2006, 2008 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2008 Pino Toscano <pino@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef CATALOG_H
#define CATALOG_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <vector>

class XRef;
class Object;
class Page;
class PageAttrs;
struct Ref;
class LinkDest;
class PageLabelInfo;
class Form;
class OCGs;

//------------------------------------------------------------------------
// NameTree
//------------------------------------------------------------------------

class NameTree {
public:
  NameTree();
  ~NameTree();
  void init(XRef *xref, Object *tree);
  void parse(Object *tree);
  GBool lookup(GooString *name, Object *obj);
  int numEntries() { return length; };
  // iterator accessor
  Object getValue(int i);
  GooString *getName(int i);

private:
  struct Entry {
    Entry(Array *array, int index);
    ~Entry();
    GooString name;
    Object value;
    void free();
    static int cmp(const void *key, const void *entry);
  };

  void addEntry(Entry *entry);

  XRef *xref;
  Object *root;
  Entry **entries;
  int size, length; // size is the number of entries in
                    // the array of Entry*
                    // length is the number of real Entry
};

class EmbFile {
public:
  EmbFile(GooString *name, GooString *description, 
	  int size,
	  GooString *createDate,
	  GooString *modDate, GooString *checksum,
	  GooString *mimetype,
	  Object objStr) :
    m_name(name),
    m_description(description),
    m_size(size),
    m_createDate(createDate),
    m_modDate(modDate),
    m_checksum(checksum),
    m_mimetype(mimetype)
  {
    objStr.copy(&m_objStr);
  }
  EmbFile(Object *efDict, GooString *description = 0);

  ~EmbFile()
  {
    delete m_name;
    delete m_description;
    delete m_modDate;
    delete m_createDate;
    delete m_checksum;
    delete m_mimetype;
    m_objStr.free();
  }

  GooString *name() { return m_name; }
  GooString *description() { return m_description; }
  int size() { return m_size; }
  GooString *modDate() { return m_modDate; }
  GooString *createDate() { return m_createDate; }
  GooString *checksum() { return m_checksum; }
  GooString *mimeType() { return m_mimetype; }
  Object &streamObject() { return m_objStr; }
  bool isOk() { return m_objStr.isStream(); }

private:
  GooString *m_name;
  GooString *m_description;
  int m_size;
  GooString *m_createDate;
  GooString *m_modDate;
  GooString *m_checksum;
  GooString *m_mimetype;
  Object m_objStr;
};

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

class Catalog {
public:

  // Constructor.
  Catalog(XRef *xrefA);

  // Destructor.
  ~Catalog();

  // Is catalog valid?
  GBool isOk() { return ok; }

  // Get number of pages.
  int getNumPages();

  // Get a page.
  Page *getPage(int i);

  // Get the reference for a page object.
  Ref *getPageRef(int i);

  // Return base URI, or NULL if none.
  GooString *getBaseURI() { return baseURI; }

  // Return the contents of the metadata stream, or NULL if there is
  // no metadata.
  GooString *readMetadata();

  // Return the structure tree root object.
  Object *getStructTreeRoot();

  // Find a page, given its object ID.  Returns page number, or 0 if
  // not found.
  int findPage(int num, int gen);

  // Find a named destination.  Returns the link destination, or
  // NULL if <name> is not a destination.
  LinkDest *findDest(GooString *name);

  Object *getDests();

  // Get the number of embedded files
  int numEmbeddedFiles() { return getEmbeddedFileNameTree()->numEntries(); }

  // Get the i'th file embedded (at the Document level) in the document
  EmbFile *embeddedFile(int i);

  // Get the number of javascript scripts
  int numJS() { return getJSNameTree()->numEntries(); }

  // Get the i'th JavaScript script (at the Document level) in the document
  GooString *getJS(int i);

  // Convert between page indices and page labels.
  GBool labelToIndex(GooString *label, int *index);
  GBool indexToLabel(int index, GooString *label);

  Object *getOutline();

  Object *getAcroForm() { return &acroForm; }

  OCGs *getOptContentConfig() { return optContent; }

  Form* getForm();

  enum PageMode {
    pageModeNone,
    pageModeOutlines,
    pageModeThumbs,
    pageModeFullScreen,
    pageModeOC,
    pageModeAttach,
    pageModeNull
  };
  enum PageLayout {
    pageLayoutNone,
    pageLayoutSinglePage,
    pageLayoutOneColumn,
    pageLayoutTwoColumnLeft,
    pageLayoutTwoColumnRight,
    pageLayoutTwoPageLeft,
    pageLayoutTwoPageRight,
    pageLayoutNull
  };

  // Returns the page mode.
  PageMode getPageMode();
  PageLayout getPageLayout();

private:

  // Get page label info.
  PageLabelInfo *getPageLabelInfo();

  XRef *xref;			// the xref table for this PDF file
  Page **pages;			// array of pages
  Ref *pageRefs;		// object ID for each page
  int lastCachedPage;
  std::vector<Dict *> *pagesList;
  std::vector<Ref> *pagesRefList;
  std::vector<PageAttrs *> *attrsList;
  std::vector<int> *kidsIdxList;
  Form *form;
  int numPages;			// number of pages
  int pagesSize;		// size of pages array
  Object dests;			// named destination dictionary
  Object names;			// named names dictionary
  NameTree *destNameTree;	// named destination name-tree
  NameTree *embeddedFileNameTree;  // embedded file name-tree
  NameTree *jsNameTree;		// Java Script name-tree
  GooString *baseURI;		// base URI for URI-type links
  Object metadata;		// metadata stream
  Object structTreeRoot;	// structure tree root dictionary
  Object outline;		// outline dictionary
  Object acroForm;		// AcroForm dictionary
  OCGs *optContent;		// Optional Content groups
  GBool ok;			// true if catalog is valid
  PageLabelInfo *pageLabelInfo; // info about page labels
  PageMode pageMode;		// page mode
  PageLayout pageLayout;	// page layout

  GBool cachePageTree(int page); // Cache first <page> pages.
  Object *findDestInTree(Object *tree, GooString *name, Object *obj);

  Object *getNames();
  NameTree *getDestNameTree();
  NameTree *getEmbeddedFileNameTree();
  NameTree *getJSNameTree();

};

#endif
