//========================================================================
//
// OptionalContent.h
//
// Copyright 2007 Brad Hards <bradh@kde.org>
// Copyright 2008 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2013 Albert Astals Cid <aacid@kde.org>
//
// Released under the GPL (version 2, or later, at your option)
//
//========================================================================

#ifndef OPTIONALCONTENT_H
#define OPTIONALCONTENT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"
#include "CharTypes.h"

class GooString;
class GooList;
class XRef;

class OptionalContentGroup;
class OCDisplayNode;

//------------------------------------------------------------------------

class OCGs {
public:

  OCGs(Object *ocgObject, XRef *xref);
  ~OCGs();

  // Is OCGS valid?
  GBool isOk() { return ok; }
  
  bool hasOCGs();
  GooList *getOCGs() const { return optionalContentGroups; }

  OptionalContentGroup* findOcgByRef( const Ref &ref);

  // Get the root node of the optional content group display tree
  // (which does not necessarily include all of the OCGs).
  OCDisplayNode *getDisplayRoot();

  Array* getOrderArray() 
    { return (order.isArray() && order.arrayGetLength() > 0) ? order.getArray() : NULL; }
  Array* getRBGroupsArray() 
    { return (rbgroups.isArray() && rbgroups.arrayGetLength()) ? rbgroups.getArray() : NULL; }

  bool optContentIsVisible( Object *dictRef );

private:
  GBool ok;

  GBool evalOCVisibilityExpr(Object *expr, int recursion);
  bool allOn( Array *ocgArray );
  bool allOff( Array *ocgArray );
  bool anyOn( Array *ocgArray );
  bool anyOff( Array *ocgArray );

  GooList *optionalContentGroups;

  Object order;
  Object rbgroups;
  XRef *m_xref;
  OCDisplayNode *display; // root node of display tree
};

//------------------------------------------------------------------------

class OptionalContentGroup {
public:
  enum State { On, Off };

  // Values from the optional content usage dictionary.
  enum UsageState {
    ocUsageOn,
    ocUsageOff,
    ocUsageUnset
  };

  OptionalContentGroup(Dict *dict);

  OptionalContentGroup(GooString *label);

  ~OptionalContentGroup();

  GooString* getName() const;

  Ref getRef() const;
  void setRef(const Ref ref);

  State getState() { return m_state; };
  void setState(State state) { m_state = state; };

  UsageState getViewState() { return viewState; }
  UsageState getPrintState() { return printState; }

private:
  GooString *m_name;
  Ref m_ref;
  State m_state;
  UsageState viewState;	 // suggested state when viewing
  UsageState printState; // suggested state when printing
};

//------------------------------------------------------------------------

class OCDisplayNode {
public:

  static OCDisplayNode *parse(Object *obj, OCGs *oc, XRef *xref, int recursion = 0);
  OCDisplayNode();
  ~OCDisplayNode();

  GooString *getName() { return name; }
  OptionalContentGroup *getOCG() { return ocg; }
  int getNumChildren();
  OCDisplayNode *getChild(int idx);

private:

  OCDisplayNode(GooString *nameA);
  OCDisplayNode(OptionalContentGroup *ocgA);
  void addChild(OCDisplayNode *child);
  void addChildren(GooList *childrenA);
  GooList *takeChildren();

  GooString *name;		// display name (may be NULL)
  OptionalContentGroup *ocg;	// NULL for display labels
  GooList *children;		// NULL if there are no children
				//   [OCDisplayNode]
};

#endif
