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
// Copyright (C) 2006, 2010, 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2012 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
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

  // Get the next object from the input stream.  If <simpleOnly> is
  // true, do not parse compound objects (arrays, dictionaries, or
  // streams).
  Object *getObj(Object *obj, GBool simpleOnly = gFalse, 
     Guchar *fileKey = NULL,
		 CryptAlgorithm encAlgorithm = cryptRC4, int keyLength = 0,
		 int objNum = 0, int objGen = 0, int recursion = 0,
		 GBool strict = gFalse);
  
  Object *getObj(Object *obj, int recursion);

  // Get stream.
  Stream *getStream() { return lexer->getStream(); }

  // Get current position in file.
  Goffset getPos() { return lexer->getPos(); }

private:

  XRef *xref;			// the xref table for this PDF file
  Lexer *lexer;			// input stream
  GBool allowStreams;		// parse stream objects?
  Object buf1, buf2;		// next two tokens
  int inlineImg;		// set when inline image data is encountered

  Stream *makeStream(Object *dict, Guchar *fileKey,
		     CryptAlgorithm encAlgorithm, int keyLength,
		     int objNum, int objGen, int recursion,
		     GBool strict);
  void shift(int objNum = -1);
  void shift(const char *cmdA, int objNum);
};

#endif

