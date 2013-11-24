//========================================================================
//
// FoFiIdentifier.h
//
// Copyright 2009 Glyph & Cog, LLC
//
//========================================================================

#ifndef FOFIIDENTIFIER_H
#define FOFIIDENTIFIER_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

//------------------------------------------------------------------------
// FoFiIdentifier
//------------------------------------------------------------------------

enum FoFiIdentifierType {
  fofiIdType1PFA,		// Type 1 font in PFA format
  fofiIdType1PFB,		// Type 1 font in PFB format
  fofiIdCFF8Bit,		// 8-bit CFF font
  fofiIdCFFCID,			// CID CFF font
  fofiIdTrueType,		// TrueType font
  fofiIdTrueTypeCollection,	// TrueType collection
  fofiIdOpenTypeCFF8Bit,	// OpenType wrapper with 8-bit CFF font
  fofiIdOpenTypeCFFCID,		// OpenType wrapper with CID CFF font
  fofiIdUnknown,		// unknown type
  fofiIdError			// error in reading the file
};

class FoFiIdentifier {
public:

  static FoFiIdentifierType identifyMem(char *file, int len);
  static FoFiIdentifierType identifyFile(char *fileName);
  static FoFiIdentifierType identifyStream(int (*getChar)(void *data),
					   void *data);
};

#endif
