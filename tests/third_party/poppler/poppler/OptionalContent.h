//========================================================================
//
// OptionalContent.h
//
// Copyright 2007 Brad Hards <bradh@kde.org>
// Copyright 2008 Carlos Garcia Campos <carlosgc@gnome.org>
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

  Array* getOrderArray() 
    { return (order.isArray() && order.arrayGetLength() > 0) ? order.getArray() : NULL; }
  Array* getRBGroupsArray() 
    { return (rbgroups.isArray() && rbgroups.arrayGetLength()) ? rbgroups.getArray() : NULL; }

  bool optContentIsVisible( Object *dictRef );

private:
  GBool ok;
  
  bool allOn( Array *ocgArray );
  bool allOff( Array *ocgArray );
  bool anyOn( Array *ocgArray );
  bool anyOff( Array *ocgArray );

  GooList *optionalContentGroups;

  Object order;
  Object rbgroups;
  XRef *m_xref;
};

//------------------------------------------------------------------------

class OptionalContentGroup {
public:
  enum State { On, Off };

  OptionalContentGroup(Dict *dict);

  OptionalContentGroup(GooString *label);

  ~OptionalContentGroup();

  GooString* getName() const;

  Ref getRef() const;
  void setRef(const Ref ref);

  State getState() { return m_state; };
  void setState(State state) { m_state = state; };

private:
  XRef *xref;
  GooString *m_name;
  Ref m_ref;
  State m_state;  
};

#endif
