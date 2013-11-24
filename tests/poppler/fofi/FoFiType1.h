//========================================================================
//
// FoFiType1.h
//
// Copyright 1999-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef FOFITYPE1_H
#define FOFITYPE1_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "FoFiBase.h"

//------------------------------------------------------------------------
// FoFiType1
//------------------------------------------------------------------------

class FoFiType1: public FoFiBase {
public:

  // Create a FoFiType1 object from a memory buffer.
  static FoFiType1 *make(char *fileA, int lenA);

  // Create a FoFiType1 object from a file on disk.
  static FoFiType1 *load(char *fileName);

  virtual ~FoFiType1();

  // Return the font name.
  char *getName();

  // Return the encoding, as an array of 256 names (any of which may
  // be NULL).
  char **getEncoding();

  // Return the font matrix as an array of six numbers.
  void getFontMatrix(double *mat);

  // Write a version of the Type 1 font file with a new encoding.
  void writeEncoded(const char **newEncoding,
		    FoFiOutputFunc outputFunc, void *outputStream);

private:

  FoFiType1(char *fileA, int lenA, GBool freeFileDataA);

  char *getNextLine(char *line);
  void parse();
  void undoPFB();

  char *name;
  char **encoding;
  double fontMatrix[6];
  GBool parsed;
};

#endif
