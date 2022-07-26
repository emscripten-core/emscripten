//========================================================================
//
// Parser.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2010 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef PARSER_H
#define PARSER_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Lexer.h"

//------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------

class Parser {
public:

  // Constructor.
  Parser(XRef *xrefA, Lexer *lexerA, GBool allowStreamsA);

  // Destructor.
  ~Parser();

  // Get the next object from the input stream.
  Object *getObj(Object *obj, Guchar *fileKey = NULL,
		 CryptAlgorithm encAlgorithm = cryptRC4, int keyLength = 0,
		 int objNum = 0, int objGen = 0);
  
  Object *getObj(Object *obj, Guchar *fileKey,
     CryptAlgorithm encAlgorithm, int keyLength,
     int objNum, int objGen, std::set<int> *fetchOriginatorNums);

  Object *getObj(Object *obj, std::set<int> *fetchOriginatorNums);

  // Get stream.
  Stream *getStream() { return lexer->getStream(); }

  // Get current position in file.
  int getPos() { return lexer->getPos(); }

private:

  XRef *xref;			// the xref table for this PDF file
  Lexer *lexer;			// input stream
  GBool allowStreams;		// parse stream objects?
  Object buf1, buf2;		// next two tokens
  int inlineImg;		// set when inline image data is encountered

  Stream *makeStream(Object *dict, Guchar *fileKey,
		     CryptAlgorithm encAlgorithm, int keyLength,
		     int objNum, int objGen, std::set<int> *fetchOriginatorNums);
  void shift(int objNum = -1);
};

#endif

