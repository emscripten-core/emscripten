//========================================================================
//
// Linearization.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
//
//========================================================================

#ifndef LINEARIZATION_H
#define LINEARIZATION_H

#include "goo/gtypes.h"
#include "Object.h"
class BaseStream;

//------------------------------------------------------------------------
// Linearization
//------------------------------------------------------------------------

class Linearization {
public:

  Linearization(BaseStream *str);
  ~Linearization();

  Guint getLength();
  Guint getHintsOffset();
  Guint getHintsLength();
  Guint getHintsOffset2();
  Guint getHintsLength2();
  int getObjectNumberFirst();
  Guint getEndFirst();
  int getNumPages();
  Guint getMainXRefEntriesOffset();
  int getPageFirst();

private:

  Object linDict;

};

#endif
