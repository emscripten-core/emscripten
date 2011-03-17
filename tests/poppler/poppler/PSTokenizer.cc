//========================================================================
//
// PSTokenizer.cc
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stdlib.h>
#include "PSTokenizer.h"

//------------------------------------------------------------------------

// A '1' in this array means the character is white space.  A '1' or
// '2' means the character ends a name or command.
static const char specialChars[256] = {
  1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,   // 0x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 1x
  1, 0, 0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 2,   // 2x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0,   // 3x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 4x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0,   // 5x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 6x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0,   // 7x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 8x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 9x
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // ax
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // bx
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // cx
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // dx
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // ex
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0    // fx
};

//------------------------------------------------------------------------

PSTokenizer::PSTokenizer(int (*getCharFuncA)(void *), void *dataA) {
  getCharFunc = getCharFuncA;
  data = dataA;
  charBuf = -1;
}

PSTokenizer::~PSTokenizer() {
}

GBool PSTokenizer::getToken(char *buf, int size, int *length) {
  GBool comment, backslash;
  int c;
  int i;

  // skip leading whitespace and comments
  comment = gFalse;
  while (1) {
    if ((c = getChar()) == EOF) {
      buf[0] = '\0';
      *length = 0;
      return gFalse;
    }
    if (comment) {
      if (c == '\x0a' || c == '\x0d') {
	comment = gFalse;
      }
    } else if (c == '%') {
      comment = gTrue;
    } else if (specialChars[c] != 1) {
      break;
    }
  }

  // Reserve room for terminating '\0'
  size--;

  // read a token
  i = 0;
  buf[i++] = c;
  if (c == '(') {
    backslash = gFalse;
    while ((c = lookChar()) != EOF) {
      consumeChar();
      if (i < size) {
	buf[i++] = c;
      }
      if (c == '\\') {
	backslash = gTrue;
      } else if (!backslash && c == ')') {
	break;
      } else {
	backslash = gFalse;
      }
    }
  } else if (c == '<') {
    while ((c = lookChar()) != EOF) {
      consumeChar();
      if (i < size && specialChars[c] != 1) {
	buf[i++] = c;
      }
      if (c == '>') {
	break;
      }
    }
  } else if (c != '[' && c != ']') {
    while ((c = lookChar()) != EOF && !specialChars[c]) {
      consumeChar();
      if (i < size) {
	buf[i++] = c;
      }
    }
  }
  // Zero terminate token string
  buf[i] = '\0';
  // Return length of token
  *length = i;

  return gTrue;
}

int PSTokenizer::lookChar() {
  if (charBuf < 0) {
    charBuf = (*getCharFunc)(data);
  }
  return charBuf;
}

void PSTokenizer::consumeChar() {
  charBuf = -1;
}

int PSTokenizer::getChar() {
  int c = charBuf;

  if (c < 0) {
    c = (*getCharFunc)(data);
  } else {
    charBuf = -1;
  }
  return c;
}
