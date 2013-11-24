//========================================================================
//
// StructTreeRoot.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2013 Igalia S.L.
//
//========================================================================

#ifndef STRUCTTREEROOT_H
#define STRUCTTREEROOT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "Object.h"
#include "StructElement.h"
#include <vector>

class Dict;
class PDFDoc;


class StructTreeRoot
{
public:
  StructTreeRoot(PDFDoc *docA, Dict *rootDict);
  ~StructTreeRoot();

  PDFDoc *getDoc() { return doc; }
  Dict *getRoleMap() { return roleMap.isDict() ? roleMap.getDict() : NULL; }
  Dict *getClassMap() { return classMap.isDict() ? classMap.getDict() : NULL; }
  unsigned getNumElements() const { return elements.size(); }
  const StructElement *getElement(int i) const { return elements.at(i); }
  StructElement *getElement(int i) { return elements.at(i); }

  void appendElement(StructElement *element) {
    if (element && element->isOk()) {
      elements.push_back(element);
    }
  }

  const StructElement *findParentElement(unsigned index) const {
    if (index < parentTree.size() && parentTree[index].size() == 1) {
      return parentTree[index][0].element;
    }
    return NULL;
  }

private:
  typedef std::vector<StructElement*> ElemPtrArray;

  // Structure for items in /ParentTree, it keeps a mapping of
  // object references and pointers to StructElement objects.
  struct Parent {
    Ref            ref;
    StructElement *element;

    Parent(): element(NULL) { ref.num = ref.gen = -1; }
    Parent(const Parent &p): element(p.element) {
      ref.num = p.ref.num;
      ref.gen = p.ref.gen;
    }
    ~Parent() {}
  };

  PDFDoc *doc;
  Object roleMap;
  Object classMap;
  ElemPtrArray elements;
  std::vector< std::vector<Parent> > parentTree;

  void parse(Dict *rootDict);
  void parentTreeAdd(const Ref &objectRef, StructElement *element);

  friend class StructElement;
};

#endif

