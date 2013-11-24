//========================================================================
//
// StructTreeRoot.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2013 Igalia S.L.
//
//========================================================================

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/GooString.h"
#include "StructTreeRoot.h"
#include "StructElement.h"
#include "PDFDoc.h"
#include "Object.h"
#include "Dict.h"
#include <set>
#include <assert.h>


StructTreeRoot::StructTreeRoot(PDFDoc *docA, Dict *structTreeRootDict):
  doc(docA)
{
  assert(doc);
  assert(structTreeRootDict);
  parse(structTreeRootDict);
}

StructTreeRoot::~StructTreeRoot()
{
  for (ElemPtrArray::iterator i = elements.begin(); i != elements.end(); ++i)
    delete *i;
  classMap.free();
  roleMap.free();
}

void StructTreeRoot::parse(Dict *root)
{
  // The RoleMap/ClassMap dictionaries are needed by all the parsing
  // functions, which will resolve the custom names to canonical
  // standard names.
  root->lookup("RoleMap", &roleMap);
  root->lookup("ClassMap", &classMap);

  // ParentTree (optional). If present, it must be a number tree,
  // otherwise it is not possible to map stream objects to their
  // corresponsing structure element. Here only the references are
  // loaded into the array, the pointers to the StructElements will
  // be filled-in later when parsing them.
  Object obj;
  if (root->lookup("ParentTree", &obj)->isDict()) {
    Object nums;
    if (obj.dictLookup("Nums", &nums)->isArray()) {
      if (nums.arrayGetLength() % 2 == 0) {
        parentTree.resize(nums.arrayGetLength() / 2);
        // Index numbers in even positions, references in odd ones
        for (int i = 0; i < nums.arrayGetLength(); i += 2) {
          Object index, value;

          if (!nums.arrayGet(i, &index)->isInt()) {
            error(errSyntaxError, -1, "Nums item at position {0:d} is wrong type ({1:s})", i, index.getTypeName());
            index.free();
            continue;
          }
          if (index.getInt() < 0) {
            error(errSyntaxError, -1, "Nums item at position {0:d} is invalid value ({1:d})", i, index.getInt());
            index.free();
            continue;
          }

          const unsigned idx = index.getInt();
          if (nums.arrayGetNF(i + 1, &value)->isRef()) {
            parentTree[idx].resize(1);
            parentTree[idx][0].ref = value.getRef();
          } else if (nums.arrayGet(i + 1, &value)->isArray()) {
            parentTree[idx].resize(value.arrayGetLength());
            for (int j = 0; j < value.arrayGetLength(); j++) {
              Object itemvalue;
              if (value.arrayGetNF(j, &itemvalue)->isRef())
                parentTree[idx][j].ref = itemvalue.getRef();
              else
                error(errSyntaxError, -1, "Nums array item at position {0:d}/{1:d} is invalid type ({2:s})", i, j, itemvalue.getTypeName());
              itemvalue.free();
            }
          } else {
            error(errSyntaxError, -1, "Nums item at position {0:d} is wrong type ({1:s})", i + 1, value.getTypeName());
          }

          value.free();
          index.free();
        }
      } else {
        error(errSyntaxError, -1, "Nums array length is not a even ({0:i})", nums.arrayGetLength());
      }
    } else {
      error(errSyntaxError, -1, "Nums object is wrong type ({0:s})", nums.getTypeName());
    }
    nums.free();
  }
  obj.free();

  std::set<int> seenElements;

  // Parse the children StructElements
  const GBool marked = doc->getCatalog()->getMarkInfo() & Catalog::markInfoMarked;
  Object kids;
  if (root->lookup("K", &kids)->isArray()) {
    if (marked && kids.arrayGetLength() > 1) {
      error(errSyntaxWarning, -1, "K in StructTreeRoot has more than one children in a tagged PDF");
    }
    for (int i = 0; i < kids.arrayGetLength(); i++) {
      Object obj, ref;
      kids.arrayGetNF(i, &ref);
      if (ref.isRef()) {
        seenElements.insert(ref.getRefNum());
      }
      if (kids.arrayGet(i, &obj)->isDict()) {
        StructElement *child = new StructElement(obj.getDict(), this, NULL, seenElements);
        if (child->isOk()) {
          if (marked && !(child->getType() == StructElement::Document ||
                          child->getType() == StructElement::Part ||
                          child->getType() == StructElement::Art ||
                          child->getType() == StructElement::Div)) {
            error(errSyntaxWarning, -1, "StructTreeRoot element of tagged PDF is wrong type ({0:s})", child->getTypeName());
          }
          appendElement(child);
          if (ref.isRef()) {
            parentTreeAdd(ref.getRef(), child);
          }
        } else {
          error(errSyntaxWarning, -1, "StructTreeRoot element could not be parsed");
          delete child;
        }
      } else {
        error(errSyntaxWarning, -1, "K has a child of wrong type ({0:s})", obj.getTypeName());
      }
      obj.free();
      ref.free();
    }
  } else if (kids.isDict()) {
    if (marked) {
      error(errSyntaxWarning, -1, "K has a child of wrong type for a tagged PDF ({0:s})", kids.getTypeName());
    }
    StructElement *child = new StructElement(kids.getDict(), this, NULL, seenElements);
    if (child->isOk()) {
      appendElement(child);
      Object ref;
      if (root->lookupNF("K", &ref)->isRef())
        parentTreeAdd(ref.getRef(), child);
      ref.free();
    } else {
      error(errSyntaxWarning, -1, "StructTreeRoot element could not be parsed");
      delete child;
    }
  } else if (!kids.isNull()) {
    error(errSyntaxWarning, -1, "K in StructTreeRoot is wrong type ({0:s})", kids.getTypeName());
  }

  kids.free();
}

void StructTreeRoot::parentTreeAdd(const Ref &objectRef, StructElement *element)
{
  for (std::vector< std::vector<Parent> >::iterator i = parentTree.begin(); i != parentTree.end(); ++i) {
    for (std::vector<Parent>::iterator j = i->begin(); j != i->end(); ++j) {
      if (j->ref.num == objectRef.num && j->ref.gen == objectRef.gen)
        j->element = element;
    }
  }
}
