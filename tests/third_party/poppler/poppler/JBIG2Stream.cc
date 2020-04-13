//========================================================================
//
// JBIG2Stream.cc
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
// Copyright (C) 2006 Raj Kumar <rkumar@archive.org>
// Copyright (C) 2006 Paul Walmsley <paul@booyaka.com>
// Copyright (C) 2006-2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 David Benjamin <davidben@mit.edu>
// Copyright (C) 2011 Edward Jiang <ejiang@google.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <limits.h>
#include "goo/GooList.h"
#include "Error.h"
#include "JArithmeticDecoder.h"
#include "JBIG2Stream.h"

//~ share these tables
#include "Stream-CCITT.h"

//------------------------------------------------------------------------

static const int contextSize[4] = { 16, 13, 10, 10 };
static const int refContextSize[2] = { 13, 10 };

//------------------------------------------------------------------------
// JBIG2HuffmanTable
//------------------------------------------------------------------------

#define jbig2HuffmanLOW 0xfffffffd
#define jbig2HuffmanOOB 0xfffffffe
#define jbig2HuffmanEOT 0xffffffff

struct JBIG2HuffmanTable {
  int val;
  Guint prefixLen;
  Guint rangeLen;		// can also be LOW, OOB, or EOT
  Guint prefix;
};

static JBIG2HuffmanTable huffTableA[] = {
  {     0, 1,  4,              0x000 },
  {    16, 2,  8,              0x002 },
  {   272, 3, 16,              0x006 },
  { 65808, 3, 32,              0x007 },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableB[] = {
  {     0, 1,  0,              0x000 },
  {     1, 2,  0,              0x002 },
  {     2, 3,  0,              0x006 },
  {     3, 4,  3,              0x00e },
  {    11, 5,  6,              0x01e },
  {    75, 6, 32,              0x03e },
  {     0, 6, jbig2HuffmanOOB, 0x03f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableC[] = {
  {     0, 1,  0,              0x000 },
  {     1, 2,  0,              0x002 },
  {     2, 3,  0,              0x006 },
  {     3, 4,  3,              0x00e },
  {    11, 5,  6,              0x01e },
  {     0, 6, jbig2HuffmanOOB, 0x03e },
  {    75, 7, 32,              0x0fe },
  {  -256, 8,  8,              0x0fe },
  {  -257, 8, jbig2HuffmanLOW, 0x0ff },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableD[] = {
  {     1, 1,  0,              0x000 },
  {     2, 2,  0,              0x002 },
  {     3, 3,  0,              0x006 },
  {     4, 4,  3,              0x00e },
  {    12, 5,  6,              0x01e },
  {    76, 5, 32,              0x01f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableE[] = {
  {     1, 1,  0,              0x000 },
  {     2, 2,  0,              0x002 },
  {     3, 3,  0,              0x006 },
  {     4, 4,  3,              0x00e },
  {    12, 5,  6,              0x01e },
  {    76, 6, 32,              0x03e },
  {  -255, 7,  8,              0x07e },
  {  -256, 7, jbig2HuffmanLOW, 0x07f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableF[] = {
  {     0, 2,  7,              0x000 },
  {   128, 3,  7,              0x002 },
  {   256, 3,  8,              0x003 },
  { -1024, 4,  9,              0x008 },
  {  -512, 4,  8,              0x009 },
  {  -256, 4,  7,              0x00a },
  {   -32, 4,  5,              0x00b },
  {   512, 4,  9,              0x00c },
  {  1024, 4, 10,              0x00d },
  { -2048, 5, 10,              0x01c },
  {  -128, 5,  6,              0x01d },
  {   -64, 5,  5,              0x01e },
  { -2049, 6, jbig2HuffmanLOW, 0x03e },
  {  2048, 6, 32,              0x03f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableG[] = {
  {  -512, 3,  8,              0x000 },
  {   256, 3,  8,              0x001 },
  {   512, 3,  9,              0x002 },
  {  1024, 3, 10,              0x003 },
  { -1024, 4,  9,              0x008 },
  {  -256, 4,  7,              0x009 },
  {   -32, 4,  5,              0x00a },
  {     0, 4,  5,              0x00b },
  {   128, 4,  7,              0x00c },
  {  -128, 5,  6,              0x01a },
  {   -64, 5,  5,              0x01b },
  {    32, 5,  5,              0x01c },
  {    64, 5,  6,              0x01d },
  { -1025, 5, jbig2HuffmanLOW, 0x01e },
  {  2048, 5, 32,              0x01f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableH[] = {
  {     0, 2,  1,              0x000 },
  {     0, 2, jbig2HuffmanOOB, 0x001 },
  {     4, 3,  4,              0x004 },
  {    -1, 4,  0,              0x00a },
  {    22, 4,  4,              0x00b },
  {    38, 4,  5,              0x00c },
  {     2, 5,  0,              0x01a },
  {    70, 5,  6,              0x01b },
  {   134, 5,  7,              0x01c },
  {     3, 6,  0,              0x03a },
  {    20, 6,  1,              0x03b },
  {   262, 6,  7,              0x03c },
  {   646, 6, 10,              0x03d },
  {    -2, 7,  0,              0x07c },
  {   390, 7,  8,              0x07d },
  {   -15, 8,  3,              0x0fc },
  {    -5, 8,  1,              0x0fd },
  {    -7, 9,  1,              0x1fc },
  {    -3, 9,  0,              0x1fd },
  {   -16, 9, jbig2HuffmanLOW, 0x1fe },
  {  1670, 9, 32,              0x1ff },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableI[] = {
  {     0, 2, jbig2HuffmanOOB, 0x000 },
  {    -1, 3,  1,              0x002 },
  {     1, 3,  1,              0x003 },
  {     7, 3,  5,              0x004 },
  {    -3, 4,  1,              0x00a },
  {    43, 4,  5,              0x00b },
  {    75, 4,  6,              0x00c },
  {     3, 5,  1,              0x01a },
  {   139, 5,  7,              0x01b },
  {   267, 5,  8,              0x01c },
  {     5, 6,  1,              0x03a },
  {    39, 6,  2,              0x03b },
  {   523, 6,  8,              0x03c },
  {  1291, 6, 11,              0x03d },
  {    -5, 7,  1,              0x07c },
  {   779, 7,  9,              0x07d },
  {   -31, 8,  4,              0x0fc },
  {   -11, 8,  2,              0x0fd },
  {   -15, 9,  2,              0x1fc },
  {    -7, 9,  1,              0x1fd },
  {   -32, 9, jbig2HuffmanLOW, 0x1fe },
  {  3339, 9, 32,              0x1ff },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableJ[] = {
  {    -2, 2,  2,              0x000 },
  {     6, 2,  6,              0x001 },
  {     0, 2, jbig2HuffmanOOB, 0x002 },
  {    -3, 5,  0,              0x018 },
  {     2, 5,  0,              0x019 },
  {    70, 5,  5,              0x01a },
  {     3, 6,  0,              0x036 },
  {   102, 6,  5,              0x037 },
  {   134, 6,  6,              0x038 },
  {   198, 6,  7,              0x039 },
  {   326, 6,  8,              0x03a },
  {   582, 6,  9,              0x03b },
  {  1094, 6, 10,              0x03c },
  {   -21, 7,  4,              0x07a },
  {    -4, 7,  0,              0x07b },
  {     4, 7,  0,              0x07c },
  {  2118, 7, 11,              0x07d },
  {    -5, 8,  0,              0x0fc },
  {     5, 8,  0,              0x0fd },
  {   -22, 8, jbig2HuffmanLOW, 0x0fe },
  {  4166, 8, 32,              0x0ff },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableK[] = {
  {     1, 1,  0,              0x000 },
  {     2, 2,  1,              0x002 },
  {     4, 4,  0,              0x00c },
  {     5, 4,  1,              0x00d },
  {     7, 5,  1,              0x01c },
  {     9, 5,  2,              0x01d },
  {    13, 6,  2,              0x03c },
  {    17, 7,  2,              0x07a },
  {    21, 7,  3,              0x07b },
  {    29, 7,  4,              0x07c },
  {    45, 7,  5,              0x07d },
  {    77, 7,  6,              0x07e },
  {   141, 7, 32,              0x07f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableL[] = {
  {     1, 1,  0,              0x000 },
  {     2, 2,  0,              0x002 },
  {     3, 3,  1,              0x006 },
  {     5, 5,  0,              0x01c },
  {     6, 5,  1,              0x01d },
  {     8, 6,  1,              0x03c },
  {    10, 7,  0,              0x07a },
  {    11, 7,  1,              0x07b },
  {    13, 7,  2,              0x07c },
  {    17, 7,  3,              0x07d },
  {    25, 7,  4,              0x07e },
  {    41, 8,  5,              0x0fe },
  {    73, 8, 32,              0x0ff },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableM[] = {
  {     1, 1,  0,              0x000 },
  {     2, 3,  0,              0x004 },
  {     7, 3,  3,              0x005 },
  {     3, 4,  0,              0x00c },
  {     5, 4,  1,              0x00d },
  {     4, 5,  0,              0x01c },
  {    15, 6,  1,              0x03a },
  {    17, 6,  2,              0x03b },
  {    21, 6,  3,              0x03c },
  {    29, 6,  4,              0x03d },
  {    45, 6,  5,              0x03e },
  {    77, 7,  6,              0x07e },
  {   141, 7, 32,              0x07f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableN[] = {
  {     0, 1,  0,              0x000 },
  {    -2, 3,  0,              0x004 },
  {    -1, 3,  0,              0x005 },
  {     1, 3,  0,              0x006 },
  {     2, 3,  0,              0x007 },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

static JBIG2HuffmanTable huffTableO[] = {
  {     0, 1,  0,              0x000 },
  {    -1, 3,  0,              0x004 },
  {     1, 3,  0,              0x005 },
  {    -2, 4,  0,              0x00c },
  {     2, 4,  0,              0x00d },
  {    -4, 5,  1,              0x01c },
  {     3, 5,  1,              0x01d },
  {    -8, 6,  2,              0x03c },
  {     5, 6,  2,              0x03d },
  {   -24, 7,  4,              0x07c },
  {     9, 7,  4,              0x07d },
  {   -25, 7, jbig2HuffmanLOW, 0x07e },
  {    25, 7, 32,              0x07f },
  {     0, 0, jbig2HuffmanEOT, 0     }
};

//------------------------------------------------------------------------
// JBIG2HuffmanDecoder
//------------------------------------------------------------------------

class JBIG2HuffmanDecoder {
public:

  JBIG2HuffmanDecoder();
  ~JBIG2HuffmanDecoder();
  void setStream(Stream *strA) { str = strA; }

  void reset();

  // Returns false for OOB, otherwise sets *<x> and returns true.
  GBool decodeInt(int *x, JBIG2HuffmanTable *table);

  Guint readBits(Guint n);
  Guint readBit();

  // Sort the table by prefix length and assign prefix values.
  void buildTable(JBIG2HuffmanTable *table, Guint len);

private:

  Stream *str;
  Guint buf;
  Guint bufLen;
};

JBIG2HuffmanDecoder::JBIG2HuffmanDecoder() {
  str = NULL;
  reset();
}

JBIG2HuffmanDecoder::~JBIG2HuffmanDecoder() {
}

void JBIG2HuffmanDecoder::reset() {
  buf = 0;
  bufLen = 0;
}

//~ optimize this
GBool JBIG2HuffmanDecoder::decodeInt(int *x, JBIG2HuffmanTable *table) {
  Guint i, len, prefix;

  i = 0;
  len = 0;
  prefix = 0;
  while (table[i].rangeLen != jbig2HuffmanEOT) {
    while (len < table[i].prefixLen) {
      prefix = (prefix << 1) | readBit();
      ++len;
    }
    if (prefix == table[i].prefix) {
      if (table[i].rangeLen == jbig2HuffmanOOB) {
	return gFalse;
      }
      if (table[i].rangeLen == jbig2HuffmanLOW) {
	*x = table[i].val - readBits(32);
      } else if (table[i].rangeLen > 0) {
	*x = table[i].val + readBits(table[i].rangeLen);
      } else {
	*x = table[i].val;
      }
      return gTrue;
    }
    ++i;
  }
  return gFalse;
}

Guint JBIG2HuffmanDecoder::readBits(Guint n) {
  Guint x, mask, nLeft;

  mask = (n == 32) ? 0xffffffff : ((1 << n) - 1);
  if (bufLen >= n) {
    x = (buf >> (bufLen - n)) & mask;
    bufLen -= n;
  } else {
    x = buf & ((1 << bufLen) - 1);
    nLeft = n - bufLen;
    bufLen = 0;
    while (nLeft >= 8) {
      x = (x << 8) | (str->getChar() & 0xff);
      nLeft -= 8;
    }
    if (nLeft > 0) {
      buf = str->getChar();
      bufLen = 8 - nLeft;
      x = (x << nLeft) | ((buf >> bufLen) & ((1 << nLeft) - 1));
    }
  }
  return x;
}

Guint JBIG2HuffmanDecoder::readBit() {
  if (bufLen == 0) {
    buf = str->getChar();
    bufLen = 8;
  }
  --bufLen;
  return (buf >> bufLen) & 1;
}

void JBIG2HuffmanDecoder::buildTable(JBIG2HuffmanTable *table, Guint len) {
  Guint i, j, k, prefix;
  JBIG2HuffmanTable tab;

  // stable selection sort:
  // - entries with prefixLen > 0, in ascending prefixLen order
  // - entry with prefixLen = 0, rangeLen = EOT
  // - all other entries with prefixLen = 0
  // (on entry, table[len] has prefixLen = 0, rangeLen = EOT)
  for (i = 0; i < len; ++i) {
    for (j = i; j < len && table[j].prefixLen == 0; ++j) ;
    if (j == len) {
      break;
    }
    for (k = j + 1; k < len; ++k) {
      if (table[k].prefixLen > 0 &&
	  table[k].prefixLen < table[j].prefixLen) {
	j = k;
      }
    }
    if (j != i) {
      tab = table[j];
      for (k = j; k > i; --k) {
	table[k] = table[k - 1];
      }
      table[i] = tab;
    }
  }
  table[i] = table[len];

  // assign prefixes
  if (table[0].rangeLen != jbig2HuffmanEOT) {
    i = 0;
    prefix = 0;
    table[i++].prefix = prefix++;
    for (; table[i].rangeLen != jbig2HuffmanEOT; ++i) {
      prefix <<= table[i].prefixLen - table[i-1].prefixLen;
      table[i].prefix = prefix++;
    }
  }
}

//------------------------------------------------------------------------
// JBIG2MMRDecoder
//------------------------------------------------------------------------

class JBIG2MMRDecoder {
public:

  JBIG2MMRDecoder();
  ~JBIG2MMRDecoder();
  void setStream(Stream *strA) { str = strA; }
  void reset();
  int get2DCode();
  int getBlackCode();
  int getWhiteCode();
  Guint get24Bits();
  void skipTo(Guint length);

private:

  Stream *str;
  Guint buf;
  Guint bufLen;
  Guint nBytesRead;
};

JBIG2MMRDecoder::JBIG2MMRDecoder() {
  str = NULL;
  reset();
}

JBIG2MMRDecoder::~JBIG2MMRDecoder() {
}

void JBIG2MMRDecoder::reset() {
  buf = 0;
  bufLen = 0;
  nBytesRead = 0;
}

int JBIG2MMRDecoder::get2DCode() {
  const CCITTCode *p;

  if (bufLen == 0) {
    buf = str->getChar() & 0xff;
    bufLen = 8;
    ++nBytesRead;
    p = &twoDimTab1[(buf >> 1) & 0x7f];
  } else if (bufLen == 8) {
    p = &twoDimTab1[(buf >> 1) & 0x7f];
  } else {
    p = &twoDimTab1[(buf << (7 - bufLen)) & 0x7f];
    if (p->bits < 0 || p->bits > (int)bufLen) {
      buf = (buf << 8) | (str->getChar() & 0xff);
      bufLen += 8;
      ++nBytesRead;
      p = &twoDimTab1[(buf >> (bufLen - 7)) & 0x7f];
    }
  }
  if (p->bits < 0) {
    error(str->getPos(), "Bad two dim code in JBIG2 MMR stream");
    return EOF;
  }
  bufLen -= p->bits;
  return p->n;
}

int JBIG2MMRDecoder::getWhiteCode() {
  const CCITTCode *p;
  Guint code;

  if (bufLen == 0) {
    buf = str->getChar() & 0xff;
    bufLen = 8;
    ++nBytesRead;
  }
  while (1) {
    if (bufLen >= 11 && ((buf >> (bufLen - 7)) & 0x7f) == 0) {
      if (bufLen <= 12) {
	code = buf << (12 - bufLen);
      } else {
	code = buf >> (bufLen - 12);
      }
      p = &whiteTab1[code & 0x1f];
    } else {
      if (bufLen <= 9) {
	code = buf << (9 - bufLen);
      } else {
	code = buf >> (bufLen - 9);
      }
      p = &whiteTab2[code & 0x1ff];
    }
    if (p->bits > 0 && p->bits <= (int)bufLen) {
      bufLen -= p->bits;
      return p->n;
    }
    if (bufLen >= 12) {
      break;
    }
    buf = (buf << 8) | (str->getChar() & 0xff);
    bufLen += 8;
    ++nBytesRead;
  }
  error(str->getPos(), "Bad white code in JBIG2 MMR stream");
  // eat a bit and return a positive number so that the caller doesn't
  // go into an infinite loop
  --bufLen;
  return 1;
}

int JBIG2MMRDecoder::getBlackCode() {
  const CCITTCode *p;
  Guint code;

  if (bufLen == 0) {
    buf = str->getChar() & 0xff;
    bufLen = 8;
    ++nBytesRead;
  }
  while (1) {
    if (bufLen >= 10 && ((buf >> (bufLen - 6)) & 0x3f) == 0) {
      if (bufLen <= 13) {
	code = buf << (13 - bufLen);
      } else {
	code = buf >> (bufLen - 13);
      }
      p = &blackTab1[code & 0x7f];
    } else if (bufLen >= 7 && ((buf >> (bufLen - 4)) & 0x0f) == 0 &&
	       ((buf >> (bufLen - 6)) & 0x03) != 0) {
      if (bufLen <= 12) {
	code = buf << (12 - bufLen);
      } else {
	code = buf >> (bufLen - 12);
      }
      p = &blackTab2[(code & 0xff) - 64];
    } else {
      if (bufLen <= 6) {
	code = buf << (6 - bufLen);
      } else {
	code = buf >> (bufLen - 6);
      }
      p = &blackTab3[code & 0x3f];
    }
    if (p->bits > 0 && p->bits <= (int)bufLen) {
      bufLen -= p->bits;
      return p->n;
    }
    if (bufLen >= 13) {
      break;
    }
    buf = (buf << 8) | (str->getChar() & 0xff);
    bufLen += 8;
    ++nBytesRead;
  }
  error(str->getPos(), "Bad black code in JBIG2 MMR stream");
  // eat a bit and return a positive number so that the caller doesn't
  // go into an infinite loop
  --bufLen;
  return 1;
}

Guint JBIG2MMRDecoder::get24Bits() {
  while (bufLen < 24) {
    buf = (buf << 8) | (str->getChar() & 0xff);
    bufLen += 8;
    ++nBytesRead;
  }
  return (buf >> (bufLen - 24)) & 0xffffff;
}

void JBIG2MMRDecoder::skipTo(Guint length) {
  while (nBytesRead < length) {
    str->getChar();
    ++nBytesRead;
  }
}

//------------------------------------------------------------------------
// JBIG2Segment
//------------------------------------------------------------------------

enum JBIG2SegmentType {
  jbig2SegBitmap,
  jbig2SegSymbolDict,
  jbig2SegPatternDict,
  jbig2SegCodeTable
};

class JBIG2Segment {
public:

  JBIG2Segment(Guint segNumA) { segNum = segNumA; }
  virtual ~JBIG2Segment() {}
  void setSegNum(Guint segNumA) { segNum = segNumA; }
  Guint getSegNum() { return segNum; }
  virtual JBIG2SegmentType getType() = 0;

private:

  Guint segNum;
};

//------------------------------------------------------------------------
// JBIG2Bitmap
//------------------------------------------------------------------------

struct JBIG2BitmapPtr {
  Guchar *p;
  int shift;
  int x;
};

class JBIG2Bitmap: public JBIG2Segment {
public:

  JBIG2Bitmap(Guint segNumA, int wA, int hA);
  virtual ~JBIG2Bitmap();
  virtual JBIG2SegmentType getType() { return jbig2SegBitmap; }
  JBIG2Bitmap *copy() { return new JBIG2Bitmap(0, this); }
  JBIG2Bitmap *getSlice(Guint x, Guint y, Guint wA, Guint hA);
  void expand(int newH, Guint pixel);
  void clearToZero();
  void clearToOne();
  int getWidth() { return w; }
  int getHeight() { return h; }
  int getPixel(int x, int y)
    { return (x < 0 || x >= w || y < 0 || y >= h) ? 0 :
             (data[y * line + (x >> 3)] >> (7 - (x & 7))) & 1; }
  void setPixel(int x, int y)
    { data[y * line + (x >> 3)] |= 1 << (7 - (x & 7)); }
  void clearPixel(int x, int y)
    { data[y * line + (x >> 3)] &= 0x7f7f >> (x & 7); }
  void getPixelPtr(int x, int y, JBIG2BitmapPtr *ptr);
  int nextPixel(JBIG2BitmapPtr *ptr);
  void duplicateRow(int yDest, int ySrc);
  void combine(JBIG2Bitmap *bitmap, int x, int y, Guint combOp);
  Guchar *getDataPtr() { return data; }
  int getDataSize() { return h * line; }
  GBool isOk() { return data != NULL; }

private:

  JBIG2Bitmap(Guint segNumA, JBIG2Bitmap *bitmap);

  int w, h, line;
  Guchar *data;
};

JBIG2Bitmap::JBIG2Bitmap(Guint segNumA, int wA, int hA):
  JBIG2Segment(segNumA)
{
  w = wA;
  h = hA;
  line = (wA + 7) >> 3;

  if (w <= 0 || h <= 0 || line <= 0 || h >= (INT_MAX - 1) / line) {
    error(-1, "invalid width/height");
    data = NULL;
    return;
  }
  // need to allocate one extra guard byte for use in combine()
  data = (Guchar *)gmalloc(h * line + 1);
  data[h * line] = 0;
}

JBIG2Bitmap::JBIG2Bitmap(Guint segNumA, JBIG2Bitmap *bitmap):
  JBIG2Segment(segNumA)
{
  w = bitmap->w;
  h = bitmap->h;
  line = bitmap->line;

  if (w <= 0 || h <= 0 || line <= 0 || h >= (INT_MAX - 1) / line) {
    error(-1, "invalid width/height");
    data = NULL;
    return;
  }
  // need to allocate one extra guard byte for use in combine()
  data = (Guchar *)gmalloc(h * line + 1);
  memcpy(data, bitmap->data, h * line);
  data[h * line] = 0;
}

JBIG2Bitmap::~JBIG2Bitmap() {
  gfree(data);
}

//~ optimize this
JBIG2Bitmap *JBIG2Bitmap::getSlice(Guint x, Guint y, Guint wA, Guint hA) {
  JBIG2Bitmap *slice;
  Guint xx, yy;

  slice = new JBIG2Bitmap(0, wA, hA);
  if (slice->isOk()) {
    slice->clearToZero();
    for (yy = 0; yy < hA; ++yy) {
      for (xx = 0; xx < wA; ++xx) {
        if (getPixel(x + xx, y + yy)) {
	  slice->setPixel(xx, yy);
        }
      }
    }
  } else {
    delete slice;
    slice = NULL;
  }
  return slice;
}

void JBIG2Bitmap::expand(int newH, Guint pixel) {
  if (newH <= h || line <= 0 || newH >= (INT_MAX - 1) / line) {
    error(-1, "invalid width/height");
    gfree(data);
    data = NULL;
    return;
  }
  // need to allocate one extra guard byte for use in combine()
  data = (Guchar *)grealloc(data, newH * line + 1);
  if (pixel) {
    memset(data + h * line, 0xff, (newH - h) * line);
  } else {
    memset(data + h * line, 0x00, (newH - h) * line);
  }
  h = newH;
  data[h * line] = 0;
}

void JBIG2Bitmap::clearToZero() {
  memset(data, 0, h * line);
}

void JBIG2Bitmap::clearToOne() {
  memset(data, 0xff, h * line);
}

inline void JBIG2Bitmap::getPixelPtr(int x, int y, JBIG2BitmapPtr *ptr) {
  if (y < 0 || y >= h || x >= w) {
    ptr->p = NULL;
    ptr->shift = 0; // make gcc happy
    ptr->x = 0; // make gcc happy
  } else if (x < 0) {
    ptr->p = &data[y * line];
    ptr->shift = 7;
    ptr->x = x;
  } else {
    ptr->p = &data[y * line + (x >> 3)];
    ptr->shift = 7 - (x & 7);
    ptr->x = x;
  }
}

inline int JBIG2Bitmap::nextPixel(JBIG2BitmapPtr *ptr) {
  int pix;

  if (!ptr->p) {
    pix = 0;
  } else if (ptr->x < 0) {
    ++ptr->x;
    pix = 0;
  } else {
    pix = (*ptr->p >> ptr->shift) & 1;
    if (++ptr->x == w) {
      ptr->p = NULL;
    } else if (ptr->shift == 0) {
      ++ptr->p;
      ptr->shift = 7;
    } else {
      --ptr->shift;
    }
  }
  return pix;
}

void JBIG2Bitmap::duplicateRow(int yDest, int ySrc) {
  memcpy(data + yDest * line, data + ySrc * line, line);
}

void JBIG2Bitmap::combine(JBIG2Bitmap *bitmap, int x, int y,
			  Guint combOp) {
  int x0, x1, y0, y1, xx, yy;
  Guchar *srcPtr, *destPtr;
  Guint src0, src1, src, dest, s1, s2, m1, m2, m3;
  GBool oneByte;

  // check for the pathological case where y = -2^31
  if (y < -0x7fffffff) {
    return;
  }
  if (y < 0) {
    y0 = -y;
  } else {
    y0 = 0;
  }
  if (y + bitmap->h > h) {
    y1 = h - y;
  } else {
    y1 = bitmap->h;
  }
  if (y0 >= y1) {
    return;
  }

  if (x >= 0) {
    x0 = x & ~7;
  } else {
    x0 = 0;
  }
  x1 = x + bitmap->w;
  if (x1 > w) {
    x1 = w;
  }
  if (x0 >= x1) {
    return;
  }

  s1 = x & 7;
  s2 = 8 - s1;
  m1 = 0xff >> (x1 & 7);
  m2 = 0xff << (((x1 & 7) == 0) ? 0 : 8 - (x1 & 7));
  m3 = (0xff >> s1) & m2;

  oneByte = x0 == ((x1 - 1) & ~7);

  for (yy = y0; yy < y1; ++yy) {

    // one byte per line -- need to mask both left and right side
    if (oneByte) {
      if (x >= 0) {
	destPtr = data + (y + yy) * line + (x >> 3);
	srcPtr = bitmap->data + yy * bitmap->line;
	dest = *destPtr;
	src1 = *srcPtr;
	switch (combOp) {
	case 0: // or
	  dest |= (src1 >> s1) & m2;
	  break;
	case 1: // and
	  dest &= ((0xff00 | src1) >> s1) | m1;
	  break;
	case 2: // xor
	  dest ^= (src1 >> s1) & m2;
	  break;
	case 3: // xnor
	  dest ^= ((src1 ^ 0xff) >> s1) & m2;
	  break;
	case 4: // replace
	  dest = (dest & ~m3) | ((src1 >> s1) & m3);
	  break;
	}
	*destPtr = dest;
      } else {
	destPtr = data + (y + yy) * line;
	srcPtr = bitmap->data + yy * bitmap->line + (-x >> 3);
	dest = *destPtr;
	src1 = *srcPtr;
	switch (combOp) {
	case 0: // or
	  dest |= src1 & m2;
	  break;
	case 1: // and
	  dest &= src1 | m1;
	  break;
	case 2: // xor
	  dest ^= src1 & m2;
	  break;
	case 3: // xnor
	  dest ^= (src1 ^ 0xff) & m2;
	  break;
	case 4: // replace
	  dest = (src1 & m2) | (dest & m1);
	  break;
	}
	*destPtr = dest;
      }

    // multiple bytes per line -- need to mask left side of left-most
    // byte and right side of right-most byte
    } else {

      // left-most byte
      if (x >= 0) {
	destPtr = data + (y + yy) * line + (x >> 3);
	srcPtr = bitmap->data + yy * bitmap->line;
	src1 = *srcPtr++;
	dest = *destPtr;
	switch (combOp) {
	case 0: // or
	  dest |= src1 >> s1;
	  break;
	case 1: // and
	  dest &= (0xff00 | src1) >> s1;
	  break;
	case 2: // xor
	  dest ^= src1 >> s1;
	  break;
	case 3: // xnor
	  dest ^= (src1 ^ 0xff) >> s1;
	  break;
	case 4: // replace
	  dest = (dest & (0xff << s2)) | (src1 >> s1);
	  break;
	}
	*destPtr++ = dest;
	xx = x0 + 8;
      } else {
	destPtr = data + (y + yy) * line;
	srcPtr = bitmap->data + yy * bitmap->line + (-x >> 3);
	src1 = *srcPtr++;
	xx = x0;
      }

      // middle bytes
      for (; xx < x1 - 8; xx += 8) {
	dest = *destPtr;
	src0 = src1;
	src1 = *srcPtr++;
	src = (((src0 << 8) | src1) >> s1) & 0xff;
	switch (combOp) {
	case 0: // or
	  dest |= src;
	  break;
	case 1: // and
	  dest &= src;
	  break;
	case 2: // xor
	  dest ^= src;
	  break;
	case 3: // xnor
	  dest ^= src ^ 0xff;
	  break;
	case 4: // replace
	  dest = src;
	  break;
	}
	*destPtr++ = dest;
      }

      // right-most byte
      // note: this last byte (src1) may not actually be used, depending
      // on the values of s1, m1, and m2 - and in fact, it may be off
      // the edge of the source bitmap, which means we need to allocate
      // one extra guard byte at the end of each bitmap
      dest = *destPtr;
      src0 = src1;
      src1 = *srcPtr++;
      src = (((src0 << 8) | src1) >> s1) & 0xff;
      switch (combOp) {
      case 0: // or
	dest |= src & m2;
	break;
      case 1: // and
	dest &= src | m1;
	break;
      case 2: // xor
	dest ^= src & m2;
	break;
      case 3: // xnor
	dest ^= (src ^ 0xff) & m2;
	break;
      case 4: // replace
	dest = (src & m2) | (dest & m1);
	break;
      }
      *destPtr = dest;
    }
  }
}

//------------------------------------------------------------------------
// JBIG2SymbolDict
//------------------------------------------------------------------------

class JBIG2SymbolDict: public JBIG2Segment {
public:

  JBIG2SymbolDict(Guint segNumA, Guint sizeA);
  virtual ~JBIG2SymbolDict();
  virtual JBIG2SegmentType getType() { return jbig2SegSymbolDict; }
  Guint getSize() { return size; }
  void setBitmap(Guint idx, JBIG2Bitmap *bitmap) { bitmaps[idx] = bitmap; }
  JBIG2Bitmap *getBitmap(Guint idx) { return bitmaps[idx]; }
  GBool isOk() { return bitmaps != NULL; }
  void setGenericRegionStats(JArithmeticDecoderStats *stats)
    { genericRegionStats = stats; }
  void setRefinementRegionStats(JArithmeticDecoderStats *stats)
    { refinementRegionStats = stats; }
  JArithmeticDecoderStats *getGenericRegionStats()
    { return genericRegionStats; }
  JArithmeticDecoderStats *getRefinementRegionStats()
    { return refinementRegionStats; }

private:

  Guint size;
  JBIG2Bitmap **bitmaps;
  JArithmeticDecoderStats *genericRegionStats;
  JArithmeticDecoderStats *refinementRegionStats;
};

JBIG2SymbolDict::JBIG2SymbolDict(Guint segNumA, Guint sizeA):
  JBIG2Segment(segNumA)
{
  size = sizeA;
  bitmaps = (JBIG2Bitmap **)gmallocn_checkoverflow(size, sizeof(JBIG2Bitmap *));
  if (!bitmaps) size = 0;
  genericRegionStats = NULL;
  refinementRegionStats = NULL;
}

JBIG2SymbolDict::~JBIG2SymbolDict() {
  Guint i;

  for (i = 0; i < size; ++i) {
    delete bitmaps[i];
  }
  gfree(bitmaps);
  if (genericRegionStats) {
    delete genericRegionStats;
  }
  if (refinementRegionStats) {
    delete refinementRegionStats;
  }
}

//------------------------------------------------------------------------
// JBIG2PatternDict
//------------------------------------------------------------------------

class JBIG2PatternDict: public JBIG2Segment {
public:

  JBIG2PatternDict(Guint segNumA, Guint sizeA);
  virtual ~JBIG2PatternDict();
  virtual JBIG2SegmentType getType() { return jbig2SegPatternDict; }
  Guint getSize() { return size; }
  void setBitmap(Guint idx, JBIG2Bitmap *bitmap) { bitmaps[idx] = bitmap; }
  JBIG2Bitmap *getBitmap(Guint idx) { return bitmaps[idx]; }

private:

  Guint size;
  JBIG2Bitmap **bitmaps;
};

JBIG2PatternDict::JBIG2PatternDict(Guint segNumA, Guint sizeA):
  JBIG2Segment(segNumA)
{
  size = sizeA;
  bitmaps = (JBIG2Bitmap **)gmallocn(size, sizeof(JBIG2Bitmap *));
}

JBIG2PatternDict::~JBIG2PatternDict() {
  Guint i;

  for (i = 0; i < size; ++i) {
    delete bitmaps[i];
  }
  gfree(bitmaps);
}

//------------------------------------------------------------------------
// JBIG2CodeTable
//------------------------------------------------------------------------

class JBIG2CodeTable: public JBIG2Segment {
public:

  JBIG2CodeTable(Guint segNumA, JBIG2HuffmanTable *tableA);
  virtual ~JBIG2CodeTable();
  virtual JBIG2SegmentType getType() { return jbig2SegCodeTable; }
  JBIG2HuffmanTable *getHuffTable() { return table; }

private:

  JBIG2HuffmanTable *table;
};

JBIG2CodeTable::JBIG2CodeTable(Guint segNumA, JBIG2HuffmanTable *tableA):
  JBIG2Segment(segNumA)
{
  table = tableA;
}

JBIG2CodeTable::~JBIG2CodeTable() {
  gfree(table);
}

//------------------------------------------------------------------------
// JBIG2Stream
//------------------------------------------------------------------------

JBIG2Stream::JBIG2Stream(Stream *strA, Object *globalsStreamA):
  FilterStream(strA)
{
  pageBitmap = NULL;

  arithDecoder = new JArithmeticDecoder();
  genericRegionStats = new JArithmeticDecoderStats(1 << 1);
  refinementRegionStats = new JArithmeticDecoderStats(1 << 1);
  iadhStats = new JArithmeticDecoderStats(1 << 9);
  iadwStats = new JArithmeticDecoderStats(1 << 9);
  iaexStats = new JArithmeticDecoderStats(1 << 9);
  iaaiStats = new JArithmeticDecoderStats(1 << 9);
  iadtStats = new JArithmeticDecoderStats(1 << 9);
  iaitStats = new JArithmeticDecoderStats(1 << 9);
  iafsStats = new JArithmeticDecoderStats(1 << 9);
  iadsStats = new JArithmeticDecoderStats(1 << 9);
  iardxStats = new JArithmeticDecoderStats(1 << 9);
  iardyStats = new JArithmeticDecoderStats(1 << 9);
  iardwStats = new JArithmeticDecoderStats(1 << 9);
  iardhStats = new JArithmeticDecoderStats(1 << 9);
  iariStats = new JArithmeticDecoderStats(1 << 9);
  iaidStats = new JArithmeticDecoderStats(1 << 1);
  huffDecoder = new JBIG2HuffmanDecoder();
  mmrDecoder = new JBIG2MMRDecoder();

  globalsStreamA->copy(&globalsStream);
  segments = globalSegments = NULL;
  curStr = NULL;
  dataPtr = dataEnd = NULL;
}

JBIG2Stream::~JBIG2Stream() {
  close();
  globalsStream.free();
  delete arithDecoder;
  delete genericRegionStats;
  delete refinementRegionStats;
  delete iadhStats;
  delete iadwStats;
  delete iaexStats;
  delete iaaiStats;
  delete iadtStats;
  delete iaitStats;
  delete iafsStats;
  delete iadsStats;
  delete iardxStats;
  delete iardyStats;
  delete iardwStats;
  delete iardhStats;
  delete iariStats;
  delete iaidStats;
  delete huffDecoder;
  delete mmrDecoder;
  delete str;
}

void JBIG2Stream::reset() {
  // read the globals stream
  globalSegments = new GooList();
  if (globalsStream.isStream()) {
    segments = globalSegments;
    curStr = globalsStream.getStream();
    curStr->reset();
    arithDecoder->setStream(curStr);
    huffDecoder->setStream(curStr);
    mmrDecoder->setStream(curStr);
    readSegments();
    curStr->close();
  }

  // read the main stream
  segments = new GooList();
  curStr = str;
  curStr->reset();
  arithDecoder->setStream(curStr);
  huffDecoder->setStream(curStr);
  mmrDecoder->setStream(curStr);
  readSegments();

  if (pageBitmap) {
    dataPtr = pageBitmap->getDataPtr();
    dataEnd = dataPtr + pageBitmap->getDataSize();
  } else {
    dataPtr = dataEnd = NULL;
  }
}

void JBIG2Stream::close() {
  if (pageBitmap) {
    delete pageBitmap;
    pageBitmap = NULL;
  }
  if (segments) {
    deleteGooList(segments, JBIG2Segment);
    segments = NULL;
  }
  if (globalSegments) {
    deleteGooList(globalSegments, JBIG2Segment);
    globalSegments = NULL;
  }
  dataPtr = dataEnd = NULL;
  FilterStream::close();
}

int JBIG2Stream::getChar() {
  if (dataPtr && dataPtr < dataEnd) {
    return (*dataPtr++ ^ 0xff) & 0xff;
  }
  return EOF;
}

int JBIG2Stream::lookChar() {
  if (dataPtr && dataPtr < dataEnd) {
    return (*dataPtr ^ 0xff) & 0xff;
  }
  return EOF;
}

int JBIG2Stream::getPos() {
  if (pageBitmap == NULL) {
    return 0;
  }
  return dataPtr - pageBitmap->getDataPtr();
}

GooString *JBIG2Stream::getPSFilter(int psLevel, char *indent) {
  return NULL;
}

GBool JBIG2Stream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

void JBIG2Stream::readSegments() {
  Guint segNum, segFlags, segType, page, segLength;
  Guint refFlags, nRefSegs;
  Guint *refSegs;
  int segDataPos;
  int c1, c2, c3;
  Guint i;

  while (readULong(&segNum)) {

    // segment header flags
    if (!readUByte(&segFlags)) {
      goto eofError1;
    }
    segType = segFlags & 0x3f;

    // referred-to segment count and retention flags
    if (!readUByte(&refFlags)) {
      goto eofError1;
    }
    nRefSegs = refFlags >> 5;
    if (nRefSegs == 7) {
      if ((c1 = curStr->getChar()) == EOF ||
	  (c2 = curStr->getChar()) == EOF ||
	  (c3 = curStr->getChar()) == EOF) {
	goto eofError1;
      }
      refFlags = (refFlags << 24) | (c1 << 16) | (c2 << 8) | c3;
      nRefSegs = refFlags & 0x1fffffff;
      for (i = 0; i < (nRefSegs + 9) >> 3; ++i) {
	c1 = curStr->getChar();
      }
    }

    // referred-to segment numbers
    refSegs = (Guint *)gmallocn(nRefSegs, sizeof(Guint));
    if (segNum <= 256) {
      for (i = 0; i < nRefSegs; ++i) {
	if (!readUByte(&refSegs[i])) {
	  goto eofError2;
	}
      }
    } else if (segNum <= 65536) {
      for (i = 0; i < nRefSegs; ++i) {
	if (!readUWord(&refSegs[i])) {
	  goto eofError2;
	}
      }
    } else {
      for (i = 0; i < nRefSegs; ++i) {
	if (!readULong(&refSegs[i])) {
	  goto eofError2;
	}
      }
    }

    // segment page association
    if (segFlags & 0x40) {
      if (!readULong(&page)) {
	goto eofError2;
      }
    } else {
      if (!readUByte(&page)) {
	goto eofError2;
      }
    }

    // segment data length
    if (!readULong(&segLength)) {
      goto eofError2;
    }

    // keep track of the start of the segment data 
    segDataPos = curStr->getPos();

    // check for missing page information segment
    if (!pageBitmap && ((segType >= 4 && segType <= 7) ||
			(segType >= 20 && segType <= 43))) {
      error(curStr->getPos(), "First JBIG2 segment associated with a page must be a page information segment");
      goto syntaxError;
    }

    // read the segment data
    switch (segType) {
    case 0:
      if (!readSymbolDictSeg(segNum, segLength, refSegs, nRefSegs)) {
	goto syntaxError;
      }
      break;
    case 4:
      readTextRegionSeg(segNum, gFalse, gFalse, segLength, refSegs, nRefSegs);
      break;
    case 6:
      readTextRegionSeg(segNum, gTrue, gFalse, segLength, refSegs, nRefSegs);
      break;
    case 7:
      readTextRegionSeg(segNum, gTrue, gTrue, segLength, refSegs, nRefSegs);
      break;
    case 16:
      readPatternDictSeg(segNum, segLength);
      break;
    case 20:
      readHalftoneRegionSeg(segNum, gFalse, gFalse, segLength,
			    refSegs, nRefSegs);
      break;
    case 22:
      readHalftoneRegionSeg(segNum, gTrue, gFalse, segLength,
			    refSegs, nRefSegs);
      break;
    case 23:
      readHalftoneRegionSeg(segNum, gTrue, gTrue, segLength,
			    refSegs, nRefSegs);
      break;
    case 36:
      readGenericRegionSeg(segNum, gFalse, gFalse, segLength);
      break;
    case 38:
      readGenericRegionSeg(segNum, gTrue, gFalse, segLength);
      break;
    case 39:
      readGenericRegionSeg(segNum, gTrue, gTrue, segLength);
      break;
    case 40:
      readGenericRefinementRegionSeg(segNum, gFalse, gFalse, segLength,
				     refSegs, nRefSegs);
      break;
    case 42:
      readGenericRefinementRegionSeg(segNum, gTrue, gFalse, segLength,
				     refSegs, nRefSegs);
      break;
    case 43:
      readGenericRefinementRegionSeg(segNum, gTrue, gTrue, segLength,
				     refSegs, nRefSegs);
      break;
    case 48:
      readPageInfoSeg(segLength);
      break;
    case 50:
      readEndOfStripeSeg(segLength);
      break;
    case 52:
      readProfilesSeg(segLength);
      break;
    case 53:
      readCodeTableSeg(segNum, segLength);
      break;
    case 62:
      readExtensionSeg(segLength);
      break;
    default:
      error(curStr->getPos(), "Unknown segment type in JBIG2 stream");
      for (i = 0; i < segLength; ++i) {
	if ((c1 = curStr->getChar()) == EOF) {
	  goto eofError2;
	}
      }
      break;
    }

    // Make sure the segment handler read all of the bytes in the 
    // segment data, unless this segment is marked as having an
    // unknown length (section 7.2.7 of the JBIG2 Final Committee Draft)

    if (segLength != 0xffffffff) {

      int segExtraBytes = segDataPos + segLength - curStr->getPos();
      if (segExtraBytes > 0) {

	// If we didn't read all of the bytes in the segment data,
	// indicate an error, and throw away the rest of the data.
	
	// v.3.1.01.13 of the LuraTech PDF Compressor Server will
	// sometimes generate an extraneous NULL byte at the end of
	// arithmetic-coded symbol dictionary segments when numNewSyms
	// == 0.  Segments like this often occur for blank pages.
	
	error(curStr->getPos(), "%d extraneous byte%s after segment",
	      segExtraBytes, (segExtraBytes > 1) ? "s" : "");
	
	// Burn through the remaining bytes -- inefficient, but
	// hopefully we're not doing this much
	
	int trash;
	for (int i = segExtraBytes; i > 0; i--) {
	  readByte(&trash);
	}
	
      } else if (segExtraBytes < 0) {
	
	// If we read more bytes than we should have, according to the 
	// segment length field, note an error.
	
	error(curStr->getPos(), "Previous segment handler read too many bytes");
	
      }

    }
    
    gfree(refSegs);
  }

  return;

 syntaxError:
  gfree(refSegs);
  return;

 eofError2:
  gfree(refSegs);
 eofError1:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
}

GBool JBIG2Stream::readSymbolDictSeg(Guint segNum, Guint length,
				     Guint *refSegs, Guint nRefSegs) {
  JBIG2SymbolDict *symbolDict;
  JBIG2HuffmanTable *huffDHTable, *huffDWTable;
  JBIG2HuffmanTable *huffBMSizeTable, *huffAggInstTable;
  JBIG2Segment *seg;
  GooList *codeTables;
  JBIG2SymbolDict *inputSymbolDict;
  Guint flags, sdTemplate, sdrTemplate, huff, refAgg;
  Guint huffDH, huffDW, huffBMSize, huffAggInst;
  Guint contextUsed, contextRetained;
  int sdATX[4], sdATY[4], sdrATX[2], sdrATY[2];
  Guint numExSyms, numNewSyms, numInputSyms, symCodeLen;
  JBIG2Bitmap **bitmaps;
  JBIG2Bitmap *collBitmap, *refBitmap;
  Guint *symWidths;
  Guint symHeight, symWidth, totalWidth, x, symID;
  int dh, dw, refAggNum, refDX, refDY, bmSize;
  GBool ex;
  int run, cnt;
  Guint i, j, k;
  Guchar *p;

  symWidths = NULL;

  // symbol dictionary flags
  if (!readUWord(&flags)) {
    goto eofError;
  }
  sdTemplate = (flags >> 10) & 3;
  sdrTemplate = (flags >> 12) & 1;
  huff = flags & 1;
  refAgg = (flags >> 1) & 1;
  huffDH = (flags >> 2) & 3;
  huffDW = (flags >> 4) & 3;
  huffBMSize = (flags >> 6) & 1;
  huffAggInst = (flags >> 7) & 1;
  contextUsed = (flags >> 8) & 1;
  contextRetained = (flags >> 9) & 1;

  // symbol dictionary AT flags
  if (!huff) {
    if (sdTemplate == 0) {
      if (!readByte(&sdATX[0]) ||
	  !readByte(&sdATY[0]) ||
	  !readByte(&sdATX[1]) ||
	  !readByte(&sdATY[1]) ||
	  !readByte(&sdATX[2]) ||
	  !readByte(&sdATY[2]) ||
	  !readByte(&sdATX[3]) ||
	  !readByte(&sdATY[3])) {
	goto eofError;
      }
    } else {
      if (!readByte(&sdATX[0]) ||
	  !readByte(&sdATY[0])) {
	goto eofError;
      }
    }
  }

  // symbol dictionary refinement AT flags
  if (refAgg && !sdrTemplate) {
    if (!readByte(&sdrATX[0]) ||
	!readByte(&sdrATY[0]) ||
	!readByte(&sdrATX[1]) ||
	!readByte(&sdrATY[1])) {
      goto eofError;
    }
  }

  // SDNUMEXSYMS and SDNUMNEWSYMS
  if (!readULong(&numExSyms) || !readULong(&numNewSyms)) {
    goto eofError;
  }

  // get referenced segments: input symbol dictionaries and code tables
  codeTables = new GooList();
  numInputSyms = 0;
  for (i = 0; i < nRefSegs; ++i) {
    // This is need by bug 12014, returning gFalse makes it not crash
    // but we end up with a empty page while acroread is able to render
    // part of it
    if ((seg = findSegment(refSegs[i]))) {
      if (seg->getType() == jbig2SegSymbolDict) {
	j = ((JBIG2SymbolDict *)seg)->getSize();
	if (numInputSyms > UINT_MAX - j) {
	  error(curStr->getPos(), "Too many input symbols in JBIG2 symbol dictionary");
	  delete codeTables;
	  goto eofError;
	}
	numInputSyms += j;
      } else if (seg->getType() == jbig2SegCodeTable) {
        codeTables->append(seg);
      }
    } else {
      delete codeTables;
      return gFalse;
    }
  }
  if (numInputSyms > UINT_MAX - numNewSyms) {
    error(curStr->getPos(), "Too many input symbols in JBIG2 symbol dictionary");
    delete codeTables;
    goto eofError;
  }

  // compute symbol code length, per 6.5.8.2.3
  //  symCodeLen = ceil( log2( numInputSyms + numNewSyms ) )
  symCodeLen = 1;
  if (likely(numInputSyms + numNewSyms > 0)) { // don't fail too badly if the sum is 0
    i = (numInputSyms + numNewSyms - 1) >> 1;
    while (i) {
      ++symCodeLen;
      i >>= 1;
    }
  }

  // get the input symbol bitmaps
  bitmaps = (JBIG2Bitmap **)gmallocn(numInputSyms + numNewSyms,
				     sizeof(JBIG2Bitmap *));
  for (i = 0; i < numInputSyms + numNewSyms; ++i) {
    bitmaps[i] = NULL;
  }
  k = 0;
  inputSymbolDict = NULL;
  for (i = 0; i < nRefSegs; ++i) {
    seg = findSegment(refSegs[i]);
    if (seg != NULL && seg->getType() == jbig2SegSymbolDict) {
      inputSymbolDict = (JBIG2SymbolDict *)seg;
      for (j = 0; j < inputSymbolDict->getSize(); ++j) {
	bitmaps[k++] = inputSymbolDict->getBitmap(j);
      }
    }
  }

  // get the Huffman tables
  huffDHTable = huffDWTable = NULL; // make gcc happy
  huffBMSizeTable = huffAggInstTable = NULL; // make gcc happy
  i = 0;
  if (huff) {
    if (huffDH == 0) {
      huffDHTable = huffTableD;
    } else if (huffDH == 1) {
      huffDHTable = huffTableE;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffDHTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffDW == 0) {
      huffDWTable = huffTableB;
    } else if (huffDW == 1) {
      huffDWTable = huffTableC;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffDWTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffBMSize == 0) {
      huffBMSizeTable = huffTableA;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffBMSizeTable =
	  ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffAggInst == 0) {
      huffAggInstTable = huffTableA;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffAggInstTable =
	  ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
  }
  delete codeTables;

  // set up the Huffman decoder
  if (huff) {
    huffDecoder->reset();

  // set up the arithmetic decoder
  } else {
    if (contextUsed && inputSymbolDict) {
      resetGenericStats(sdTemplate, inputSymbolDict->getGenericRegionStats());
    } else {
      resetGenericStats(sdTemplate, NULL);
    }
    resetIntStats(symCodeLen);
    arithDecoder->start();
  }

  // set up the arithmetic decoder for refinement/aggregation
  if (refAgg) {
    if (contextUsed && inputSymbolDict) {
      resetRefinementStats(sdrTemplate,
			   inputSymbolDict->getRefinementRegionStats());
    } else {
      resetRefinementStats(sdrTemplate, NULL);
    }
  }

  // allocate symbol widths storage
  if (huff && !refAgg) {
    symWidths = (Guint *)gmallocn(numNewSyms, sizeof(Guint));
  }

  symHeight = 0;
  i = 0;
  while (i < numNewSyms) {

    // read the height class delta height
    if (huff) {
      huffDecoder->decodeInt(&dh, huffDHTable);
    } else {
      arithDecoder->decodeInt(&dh, iadhStats);
    }
    if (dh < 0 && (Guint)-dh >= symHeight) {
      error(curStr->getPos(), "Bad delta-height value in JBIG2 symbol dictionary");
      goto syntaxError;
    }
    symHeight += dh;
    symWidth = 0;
    totalWidth = 0;
    j = i;

    // read the symbols in this height class
    while (1) {

      // read the delta width
      if (huff) {
	if (!huffDecoder->decodeInt(&dw, huffDWTable)) {
	  break;
	}
      } else {
	if (!arithDecoder->decodeInt(&dw, iadwStats)) {
	  break;
	}
      }
      if (dw < 0 && (Guint)-dw >= symWidth) {
	error(curStr->getPos(), "Bad delta-height value in JBIG2 symbol dictionary");
	goto syntaxError;
      }
      symWidth += dw;
      if (i >= numNewSyms) {
	error(curStr->getPos(), "Too many symbols in JBIG2 symbol dictionary");
	goto syntaxError;
      }

      // using a collective bitmap, so don't read a bitmap here
      if (huff && !refAgg) {
	symWidths[i] = symWidth;
	totalWidth += symWidth;

      // refinement/aggregate coding
      } else if (refAgg) {
	if (huff) {
	  if (!huffDecoder->decodeInt(&refAggNum, huffAggInstTable)) {
	    break;
	  }
	} else {
	  if (!arithDecoder->decodeInt(&refAggNum, iaaiStats)) {
	    break;
	  }
	}
#if 0 //~ This special case was added about a year before the final draft
      //~ of the JBIG2 spec was released.  I have encountered some old
      //~ JBIG2 images that predate it.
	if (0) {
#else
	if (refAggNum == 1) {
#endif
	  if (huff) {
	    symID = huffDecoder->readBits(symCodeLen);
	    huffDecoder->decodeInt(&refDX, huffTableO);
	    huffDecoder->decodeInt(&refDY, huffTableO);
	    huffDecoder->decodeInt(&bmSize, huffTableA);
	    huffDecoder->reset();
	    arithDecoder->start();
	  } else {
	    symID = arithDecoder->decodeIAID(symCodeLen, iaidStats);
	    arithDecoder->decodeInt(&refDX, iardxStats);
	    arithDecoder->decodeInt(&refDY, iardyStats);
	  }
	  if (symID >= numInputSyms + i) {
	    error(curStr->getPos(), "Invalid symbol ID in JBIG2 symbol dictionary");
	    goto syntaxError;
	  }
	  refBitmap = bitmaps[symID];
	  bitmaps[numInputSyms + i] =
	      readGenericRefinementRegion(symWidth, symHeight,
					  sdrTemplate, gFalse,
					  refBitmap, refDX, refDY,
					  sdrATX, sdrATY);
	  //~ do we need to use the bmSize value here (in Huffman mode)?
	} else {
	  bitmaps[numInputSyms + i] =
	      readTextRegion(huff, gTrue, symWidth, symHeight,
			     refAggNum, 0, numInputSyms + i, NULL,
			     symCodeLen, bitmaps, 0, 0, 0, 1, 0,
			     huffTableF, huffTableH, huffTableK, huffTableO,
			     huffTableO, huffTableO, huffTableO, huffTableA,
			     sdrTemplate, sdrATX, sdrATY);
	}

      // non-ref/agg coding
      } else {
	bitmaps[numInputSyms + i] =
	    readGenericBitmap(gFalse, symWidth, symHeight,
			      sdTemplate, gFalse, gFalse, NULL,
			      sdATX, sdATY, 0);
      }

      ++i;
    }

    // read the collective bitmap
    if (huff && !refAgg) {
      huffDecoder->decodeInt(&bmSize, huffBMSizeTable);
      huffDecoder->reset();
      if (bmSize == 0) {
	collBitmap = new JBIG2Bitmap(0, totalWidth, symHeight);
	bmSize = symHeight * ((totalWidth + 7) >> 3);
	p = collBitmap->getDataPtr();
	for (k = 0; k < (Guint)bmSize; ++k) {
	  *p++ = curStr->getChar();
	}
      } else {
	collBitmap = readGenericBitmap(gTrue, totalWidth, symHeight,
				       0, gFalse, gFalse, NULL, NULL, NULL,
				       bmSize);
      }
      x = 0;
      for (; j < i; ++j) {
	bitmaps[numInputSyms + j] =
	    collBitmap->getSlice(x, 0, symWidths[j], symHeight);
	x += symWidths[j];
      }
      delete collBitmap;
    }
  }

  // create the symbol dict object
  symbolDict = new JBIG2SymbolDict(segNum, numExSyms);
  if (!symbolDict->isOk()) {
    delete symbolDict;
    goto syntaxError;
  }

  // exported symbol list
  i = j = 0;
  ex = gFalse;
  while (i < numInputSyms + numNewSyms) {
    if (huff) {
      huffDecoder->decodeInt(&run, huffTableA);
    } else {
      arithDecoder->decodeInt(&run, iaexStats);
    }
    if (i + run > numInputSyms + numNewSyms ||
	(ex && j + run > numExSyms)) {
      error(curStr->getPos(), "Too many exported symbols in JBIG2 symbol dictionary");
      for ( ; j < numExSyms; ++j) symbolDict->setBitmap(j, NULL);
      delete symbolDict;
      goto syntaxError;
    }
    if (ex) {
      for (cnt = 0; cnt < run; ++cnt) {
	symbolDict->setBitmap(j++, bitmaps[i++]->copy());
      }
    } else {
      i += run;
    }
    ex = !ex;
  }
  if (j != numExSyms) {
    error(curStr->getPos(), "Too few symbols in JBIG2 symbol dictionary");
    for ( ; j < numExSyms; ++j) symbolDict->setBitmap(j, NULL);
    delete symbolDict;
    goto syntaxError;
  }

  for (i = 0; i < numNewSyms; ++i) {
    delete bitmaps[numInputSyms + i];
  }
  gfree(bitmaps);
  if (symWidths) {
    gfree(symWidths);
  }

  // save the arithmetic decoder stats
  if (!huff && contextRetained) {
    symbolDict->setGenericRegionStats(genericRegionStats->copy());
    if (refAgg) {
      symbolDict->setRefinementRegionStats(refinementRegionStats->copy());
    }
  }

  // store the new symbol dict
  segments->append(symbolDict);

  return gTrue;

 codeTableError:
  error(curStr->getPos(), "Missing code table in JBIG2 symbol dictionary");
  delete codeTables;

 syntaxError:
  for (i = 0; i < numNewSyms; ++i) {
    if (bitmaps[numInputSyms + i]) {
      delete bitmaps[numInputSyms + i];
    }
  }
  gfree(bitmaps);
  if (symWidths) {
    gfree(symWidths);
  }
  return gFalse;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
  return gFalse;
}

void JBIG2Stream::readTextRegionSeg(Guint segNum, GBool imm,
				    GBool lossless, Guint length,
				    Guint *refSegs, Guint nRefSegs) {
  JBIG2Bitmap *bitmap;
  JBIG2HuffmanTable runLengthTab[36];
  JBIG2HuffmanTable *symCodeTab;
  JBIG2HuffmanTable *huffFSTable, *huffDSTable, *huffDTTable;
  JBIG2HuffmanTable *huffRDWTable, *huffRDHTable;
  JBIG2HuffmanTable *huffRDXTable, *huffRDYTable, *huffRSizeTable;
  JBIG2Segment *seg;
  GooList *codeTables;
  JBIG2SymbolDict *symbolDict;
  JBIG2Bitmap **syms;
  Guint w, h, x, y, segInfoFlags, extCombOp;
  Guint flags, huff, refine, logStrips, refCorner, transposed;
  Guint combOp, defPixel, templ;
  int sOffset;
  Guint huffFlags, huffFS, huffDS, huffDT;
  Guint huffRDW, huffRDH, huffRDX, huffRDY, huffRSize;
  Guint numInstances, numSyms, symCodeLen;
  int atx[2], aty[2];
  Guint i, k, kk;
  int j;

  // region segment info field
  if (!readULong(&w) || !readULong(&h) ||
      !readULong(&x) || !readULong(&y) ||
      !readUByte(&segInfoFlags)) {
    goto eofError;
  }
  extCombOp = segInfoFlags & 7;

  // rest of the text region header
  if (!readUWord(&flags)) {
    goto eofError;
  }
  huff = flags & 1;
  refine = (flags >> 1) & 1;
  logStrips = (flags >> 2) & 3;
  refCorner = (flags >> 4) & 3;
  transposed = (flags >> 6) & 1;
  combOp = (flags >> 7) & 3;
  defPixel = (flags >> 9) & 1;
  sOffset = (flags >> 10) & 0x1f;
  if (sOffset & 0x10) {
    sOffset |= -1 - 0x0f;
  }
  templ = (flags >> 15) & 1;
  huffFS = huffDS = huffDT = 0; // make gcc happy
  huffRDW = huffRDH = huffRDX = huffRDY = huffRSize = 0; // make gcc happy
  if (huff) {
    if (!readUWord(&huffFlags)) {
      goto eofError;
    }
    huffFS = huffFlags & 3;
    huffDS = (huffFlags >> 2) & 3;
    huffDT = (huffFlags >> 4) & 3;
    huffRDW = (huffFlags >> 6) & 3;
    huffRDH = (huffFlags >> 8) & 3;
    huffRDX = (huffFlags >> 10) & 3;
    huffRDY = (huffFlags >> 12) & 3;
    huffRSize = (huffFlags >> 14) & 1;
  }
  if (refine && templ == 0) {
    if (!readByte(&atx[0]) || !readByte(&aty[0]) ||
	!readByte(&atx[1]) || !readByte(&aty[1])) {
      goto eofError;
    }
  }
  if (!readULong(&numInstances)) {
    goto eofError;
  }

  // get symbol dictionaries and tables
  codeTables = new GooList();
  numSyms = 0;
  for (i = 0; i < nRefSegs; ++i) {
    if ((seg = findSegment(refSegs[i]))) {
      if (seg->getType() == jbig2SegSymbolDict) {
	numSyms += ((JBIG2SymbolDict *)seg)->getSize();
      } else if (seg->getType() == jbig2SegCodeTable) {
	codeTables->append(seg);
      }
    } else {
      error(curStr->getPos(), "Invalid segment reference in JBIG2 text region");
      delete codeTables;
      return;
    }
  }
  symCodeLen = 0;
  i = 1;
  while (i < numSyms) {
    ++symCodeLen;
    i <<= 1;
  }

  // get the symbol bitmaps
  syms = (JBIG2Bitmap **)gmallocn(numSyms, sizeof(JBIG2Bitmap *));
  kk = 0;
  for (i = 0; i < nRefSegs; ++i) {
    if ((seg = findSegment(refSegs[i]))) {
      if (seg->getType() == jbig2SegSymbolDict) {
	symbolDict = (JBIG2SymbolDict *)seg;
	for (k = 0; k < symbolDict->getSize(); ++k) {
	  syms[kk++] = symbolDict->getBitmap(k);
	}
      }
    }
  }

  // get the Huffman tables
  huffFSTable = huffDSTable = huffDTTable = NULL; // make gcc happy
  huffRDWTable = huffRDHTable = NULL; // make gcc happy
  huffRDXTable = huffRDYTable = huffRSizeTable = NULL; // make gcc happy
  i = 0;
  if (huff) {
    if (huffFS == 0) {
      huffFSTable = huffTableF;
    } else if (huffFS == 1) {
      huffFSTable = huffTableG;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffFSTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffDS == 0) {
      huffDSTable = huffTableH;
    } else if (huffDS == 1) {
      huffDSTable = huffTableI;
    } else if (huffDS == 2) {
      huffDSTable = huffTableJ;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffDSTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffDT == 0) {
      huffDTTable = huffTableK;
    } else if (huffDT == 1) {
      huffDTTable = huffTableL;
    } else if (huffDT == 2) {
      huffDTTable = huffTableM;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffDTTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffRDW == 0) {
      huffRDWTable = huffTableN;
    } else if (huffRDW == 1) {
      huffRDWTable = huffTableO;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffRDWTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffRDH == 0) {
      huffRDHTable = huffTableN;
    } else if (huffRDH == 1) {
      huffRDHTable = huffTableO;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffRDHTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffRDX == 0) {
      huffRDXTable = huffTableN;
    } else if (huffRDX == 1) {
      huffRDXTable = huffTableO;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffRDXTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffRDY == 0) {
      huffRDYTable = huffTableN;
    } else if (huffRDY == 1) {
      huffRDYTable = huffTableO;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffRDYTable = ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
    if (huffRSize == 0) {
      huffRSizeTable = huffTableA;
    } else {
      if (i >= (Guint)codeTables->getLength()) {
	goto codeTableError;
      }
      huffRSizeTable =
	  ((JBIG2CodeTable *)codeTables->get(i++))->getHuffTable();
    }
  }
  delete codeTables;

  // symbol ID Huffman decoding table
  if (huff) {
    huffDecoder->reset();
    for (i = 0; i < 32; ++i) {
      runLengthTab[i].val = i;
      runLengthTab[i].prefixLen = huffDecoder->readBits(4);
      runLengthTab[i].rangeLen = 0;
    }
    runLengthTab[32].val = 0x103;
    runLengthTab[32].prefixLen = huffDecoder->readBits(4);
    runLengthTab[32].rangeLen = 2;
    runLengthTab[33].val = 0x203;
    runLengthTab[33].prefixLen = huffDecoder->readBits(4);
    runLengthTab[33].rangeLen = 3;
    runLengthTab[34].val = 0x20b;
    runLengthTab[34].prefixLen = huffDecoder->readBits(4);
    runLengthTab[34].rangeLen = 7;
    runLengthTab[35].prefixLen = 0;
    runLengthTab[35].rangeLen = jbig2HuffmanEOT;
    huffDecoder->buildTable(runLengthTab, 35);
    symCodeTab = (JBIG2HuffmanTable *)gmallocn(numSyms + 1,
					       sizeof(JBIG2HuffmanTable));
    for (i = 0; i < numSyms; ++i) {
      symCodeTab[i].val = i;
      symCodeTab[i].rangeLen = 0;
    }
    i = 0;
    while (i < numSyms) {
      huffDecoder->decodeInt(&j, runLengthTab);
      if (j > 0x200) {
	for (j -= 0x200; j && i < numSyms; --j) {
	  symCodeTab[i++].prefixLen = 0;
	}
      } else if (j > 0x100) {
	for (j -= 0x100; j && i < numSyms; --j) {
	  symCodeTab[i].prefixLen = symCodeTab[i-1].prefixLen;
	  ++i;
	}
      } else {
	symCodeTab[i++].prefixLen = j;
      }
    }
    symCodeTab[numSyms].prefixLen = 0;
    symCodeTab[numSyms].rangeLen = jbig2HuffmanEOT;
    huffDecoder->buildTable(symCodeTab, numSyms);
    huffDecoder->reset();

  // set up the arithmetic decoder
  } else {
    symCodeTab = NULL;
    resetIntStats(symCodeLen);
    arithDecoder->start();
  }
  if (refine) {
    resetRefinementStats(templ, NULL);
  }

  bitmap = readTextRegion(huff, refine, w, h, numInstances,
			  logStrips, numSyms, symCodeTab, symCodeLen, syms,
			  defPixel, combOp, transposed, refCorner, sOffset,
			  huffFSTable, huffDSTable, huffDTTable,
			  huffRDWTable, huffRDHTable,
			  huffRDXTable, huffRDYTable, huffRSizeTable,
			  templ, atx, aty);

  gfree(syms);

  if (bitmap) {
    // combine the region bitmap into the page bitmap
    if (imm) {
      if (pageH == 0xffffffff && y + h > curPageH) {
        pageBitmap->expand(y + h, pageDefPixel);
      }
      pageBitmap->combine(bitmap, x, y, extCombOp);
      delete bitmap;

    // store the region bitmap
    } else {
      bitmap->setSegNum(segNum);
      segments->append(bitmap);
    }
  }

  // clean up the Huffman decoder
  if (huff) {
    gfree(symCodeTab);
  }

  return;

 codeTableError:
  error(curStr->getPos(), "Missing code table in JBIG2 text region");
  gfree(codeTables);
  delete syms;
  return;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
  return;
}

JBIG2Bitmap *JBIG2Stream::readTextRegion(GBool huff, GBool refine,
					 int w, int h,
					 Guint numInstances,
					 Guint logStrips,
					 int numSyms,
					 JBIG2HuffmanTable *symCodeTab,
					 Guint symCodeLen,
					 JBIG2Bitmap **syms,
					 Guint defPixel, Guint combOp,
					 Guint transposed, Guint refCorner,
					 int sOffset,
					 JBIG2HuffmanTable *huffFSTable,
					 JBIG2HuffmanTable *huffDSTable,
					 JBIG2HuffmanTable *huffDTTable,
					 JBIG2HuffmanTable *huffRDWTable,
					 JBIG2HuffmanTable *huffRDHTable,
					 JBIG2HuffmanTable *huffRDXTable,
					 JBIG2HuffmanTable *huffRDYTable,
					 JBIG2HuffmanTable *huffRSizeTable,
					 Guint templ,
					 int *atx, int *aty) {
  JBIG2Bitmap *bitmap;
  JBIG2Bitmap *symbolBitmap;
  Guint strips;
  int t, dt, tt, s, ds, sFirst, j;
  int rdw, rdh, rdx, rdy, ri, refDX, refDY, bmSize;
  Guint symID, inst, bw, bh;

  strips = 1 << logStrips;

  // allocate the bitmap
  bitmap = new JBIG2Bitmap(0, w, h);
  if (!bitmap->isOk()) {
    delete bitmap;
    return NULL;
  }
  if (defPixel) {
    bitmap->clearToOne();
  } else {
    bitmap->clearToZero();
  }

  // decode initial T value
  if (huff) {
    huffDecoder->decodeInt(&t, huffDTTable);
  } else {
    arithDecoder->decodeInt(&t, iadtStats);
  }
  t *= -(int)strips;

  inst = 0;
  sFirst = 0;
  while (inst < numInstances) {

    // decode delta-T
    if (huff) {
      huffDecoder->decodeInt(&dt, huffDTTable);
    } else {
      arithDecoder->decodeInt(&dt, iadtStats);
    }
    t += dt * strips;

    // first S value
    if (huff) {
      huffDecoder->decodeInt(&ds, huffFSTable);
    } else {
      arithDecoder->decodeInt(&ds, iafsStats);
    }
    sFirst += ds;
    s = sFirst;

    // read the instances
    while (inst < numInstances) {

      // T value
      if (strips == 1) {
	dt = 0;
      } else if (huff) {
	dt = huffDecoder->readBits(logStrips);
      } else {
	arithDecoder->decodeInt(&dt, iaitStats);
      }
      tt = t + dt;

      // symbol ID
      if (huff) {
	if (symCodeTab) {
	  huffDecoder->decodeInt(&j, symCodeTab);
	  symID = (Guint)j;
	} else {
	  symID = huffDecoder->readBits(symCodeLen);
	}
      } else {
	symID = arithDecoder->decodeIAID(symCodeLen, iaidStats);
      }

      if (symID >= (Guint)numSyms) {
	error(curStr->getPos(), "Invalid symbol number in JBIG2 text region");
      } else {

	// get the symbol bitmap
	symbolBitmap = NULL;
	if (refine) {
	  if (huff) {
	    ri = (int)huffDecoder->readBit();
	  } else {
	    arithDecoder->decodeInt(&ri, iariStats);
	  }
	} else {
	  ri = 0;
	}
	if (ri) {
	  GBool decodeSuccess;
	  if (huff) {
	    decodeSuccess = huffDecoder->decodeInt(&rdw, huffRDWTable);
	    decodeSuccess = decodeSuccess && huffDecoder->decodeInt(&rdh, huffRDHTable);
	    decodeSuccess = decodeSuccess && huffDecoder->decodeInt(&rdx, huffRDXTable);
	    decodeSuccess = decodeSuccess && huffDecoder->decodeInt(&rdy, huffRDYTable);
	    decodeSuccess = decodeSuccess && huffDecoder->decodeInt(&bmSize, huffRSizeTable);
	    huffDecoder->reset();
	    arithDecoder->start();
	  } else {
	    decodeSuccess = arithDecoder->decodeInt(&rdw, iardwStats);
	    decodeSuccess = decodeSuccess && arithDecoder->decodeInt(&rdh, iardhStats);
	    decodeSuccess = decodeSuccess && arithDecoder->decodeInt(&rdx, iardxStats);
	    decodeSuccess = decodeSuccess && arithDecoder->decodeInt(&rdy, iardyStats);
	  }
	  
	  if (decodeSuccess && syms[symID])
	  {
	    refDX = ((rdw >= 0) ? rdw : rdw - 1) / 2 + rdx;
	    refDY = ((rdh >= 0) ? rdh : rdh - 1) / 2 + rdy;

	    symbolBitmap =
	      readGenericRefinementRegion(rdw + syms[symID]->getWidth(),
					  rdh + syms[symID]->getHeight(),
					  templ, gFalse, syms[symID],
					  refDX, refDY, atx, aty);
	  }
	  //~ do we need to use the bmSize value here (in Huffman mode)?
	} else {
	  symbolBitmap = syms[symID];
	}

	if (symbolBitmap) {
	  // combine the symbol bitmap into the region bitmap
	  //~ something is wrong here - refCorner shouldn't degenerate into
	  //~   two cases
	  bw = symbolBitmap->getWidth() - 1;
	  bh = symbolBitmap->getHeight() - 1;
	  if (transposed) {
	    switch (refCorner) {
	    case 0: // bottom left
	      bitmap->combine(symbolBitmap, tt, s, combOp);
	      break;
	    case 1: // top left
	      bitmap->combine(symbolBitmap, tt, s, combOp);
	      break;
	    case 2: // bottom right
	      bitmap->combine(symbolBitmap, tt - bw, s, combOp);
	      break;
	    case 3: // top right
	      bitmap->combine(symbolBitmap, tt - bw, s, combOp);
	      break;
	    }
	    s += bh;
	  } else {
	    switch (refCorner) {
	    case 0: // bottom left
	      bitmap->combine(symbolBitmap, s, tt - bh, combOp);
	      break;
	    case 1: // top left
	      bitmap->combine(symbolBitmap, s, tt, combOp);
	      break;
	    case 2: // bottom right
	      bitmap->combine(symbolBitmap, s, tt - bh, combOp);
	      break;
	    case 3: // top right
	      bitmap->combine(symbolBitmap, s, tt, combOp);
	      break;
	    }
	    s += bw;
	  }
	  if (ri) {
	    delete symbolBitmap;
	  }
	} else {
	  // NULL symbolBitmap only happens on error
	  delete bitmap;
	  return NULL;
	}
      }

      // next instance
      ++inst;

      // next S value
      if (huff) {
	if (!huffDecoder->decodeInt(&ds, huffDSTable)) {
	  break;
	}
      } else {
	if (!arithDecoder->decodeInt(&ds, iadsStats)) {
	  break;
	}
      }
      s += sOffset + ds;
    }
  }

  return bitmap;
}

void JBIG2Stream::readPatternDictSeg(Guint segNum, Guint length) {
  JBIG2PatternDict *patternDict;
  JBIG2Bitmap *bitmap;
  Guint flags, patternW, patternH, grayMax, templ, mmr;
  int atx[4], aty[4];
  Guint i, x;

  // halftone dictionary flags, pattern width and height, max gray value
  if (!readUByte(&flags) ||
      !readUByte(&patternW) ||
      !readUByte(&patternH) ||
      !readULong(&grayMax)) {
    goto eofError;
  }
  templ = (flags >> 1) & 3;
  mmr = flags & 1;

  // set up the arithmetic decoder
  if (!mmr) {
    resetGenericStats(templ, NULL);
    arithDecoder->start();
  }

  // read the bitmap
  atx[0] = -(int)patternW; aty[0] =  0;
  atx[1] = -3;             aty[1] = -1;
  atx[2] =  2;             aty[2] = -2;
  atx[3] = -2;             aty[3] = -2;
  bitmap = readGenericBitmap(mmr, (grayMax + 1) * patternW, patternH,
			     templ, gFalse, gFalse, NULL,
			     atx, aty, length - 7);

  if (!bitmap)
    return;

  // create the pattern dict object
  patternDict = new JBIG2PatternDict(segNum, grayMax + 1);

  // split up the bitmap
  x = 0;
  for (i = 0; i <= grayMax; ++i) {
    patternDict->setBitmap(i, bitmap->getSlice(x, 0, patternW, patternH));
    x += patternW;
  }

  // free memory
  delete bitmap;

  // store the new pattern dict
  segments->append(patternDict);

  return;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
}

void JBIG2Stream::readHalftoneRegionSeg(Guint segNum, GBool imm,
					GBool lossless, Guint length,
					Guint *refSegs, Guint nRefSegs) {
  JBIG2Bitmap *bitmap;
  JBIG2Segment *seg;
  JBIG2PatternDict *patternDict;
  JBIG2Bitmap *skipBitmap;
  Guint *grayImg;
  JBIG2Bitmap *grayBitmap;
  JBIG2Bitmap *patternBitmap;
  Guint w, h, x, y, segInfoFlags, extCombOp;
  Guint flags, mmr, templ, enableSkip, combOp;
  Guint gridW, gridH, stepX, stepY, patW, patH;
  int atx[4], aty[4];
  int gridX, gridY, xx, yy, bit, j;
  Guint bpp, m, n, i;

  // region segment info field
  if (!readULong(&w) || !readULong(&h) ||
      !readULong(&x) || !readULong(&y) ||
      !readUByte(&segInfoFlags)) {
    goto eofError;
  }
  extCombOp = segInfoFlags & 7;

  // rest of the halftone region header
  if (!readUByte(&flags)) {
    goto eofError;
  }
  mmr = flags & 1;
  templ = (flags >> 1) & 3;
  enableSkip = (flags >> 3) & 1;
  combOp = (flags >> 4) & 7;
  if (!readULong(&gridW) || !readULong(&gridH) ||
      !readLong(&gridX) || !readLong(&gridY) ||
      !readUWord(&stepX) || !readUWord(&stepY)) {
    goto eofError;
  }
  if (w == 0 || h == 0 || w >= INT_MAX / h) {
    error(curStr->getPos(), "Bad bitmap size in JBIG2 halftone segment");
    return;
  }
  if (gridH == 0 || gridW >= INT_MAX / gridH) {
    error(curStr->getPos(), "Bad grid size in JBIG2 halftone segment");
    return;
  }

  // get pattern dictionary
  if (nRefSegs != 1) {
    error(curStr->getPos(), "Bad symbol dictionary reference in JBIG2 halftone segment");
    return;
  }
  seg = findSegment(refSegs[0]);
  if (seg == NULL || seg->getType() != jbig2SegPatternDict) {
    error(curStr->getPos(), "Bad symbol dictionary reference in JBIG2 halftone segment");
    return;
  }

  patternDict = (JBIG2PatternDict *)seg;
  bpp = 0;
  i = 1;
  while (i < patternDict->getSize()) {
    ++bpp;
    i <<= 1;
  }
  patW = patternDict->getBitmap(0)->getWidth();
  patH = patternDict->getBitmap(0)->getHeight();

  // set up the arithmetic decoder
  if (!mmr) {
    resetGenericStats(templ, NULL);
    arithDecoder->start();
  }

  // allocate the bitmap
  bitmap = new JBIG2Bitmap(segNum, w, h);
  if (flags & 0x80) { // HDEFPIXEL
    bitmap->clearToOne();
  } else {
    bitmap->clearToZero();
  }

  // compute the skip bitmap
  skipBitmap = NULL;
  if (enableSkip) {
    skipBitmap = new JBIG2Bitmap(0, gridW, gridH);
    skipBitmap->clearToZero();
    for (m = 0; m < gridH; ++m) {
      for (n = 0; n < gridW; ++n) {
	xx = gridX + m * stepY + n * stepX;
	yy = gridY + m * stepX - n * stepY;
	if (((xx + (int)patW) >> 8) <= 0 || (xx >> 8) >= (int)w ||
	    ((yy + (int)patH) >> 8) <= 0 || (yy >> 8) >= (int)h) {
	  skipBitmap->setPixel(n, m);
	}
      }
    }
  }

  // read the gray-scale image
  grayImg = (Guint *)gmallocn(gridW * gridH, sizeof(Guint));
  memset(grayImg, 0, gridW * gridH * sizeof(Guint));
  atx[0] = templ <= 1 ? 3 : 2;  aty[0] = -1;
  atx[1] = -3;                  aty[1] = -1;
  atx[2] =  2;                  aty[2] = -2;
  atx[3] = -2;                  aty[3] = -2;
  for (j = bpp - 1; j >= 0; --j) {
    grayBitmap = readGenericBitmap(mmr, gridW, gridH, templ, gFalse,
				   enableSkip, skipBitmap, atx, aty, -1);
    i = 0;
    for (m = 0; m < gridH; ++m) {
      for (n = 0; n < gridW; ++n) {
	bit = grayBitmap->getPixel(n, m) ^ (grayImg[i] & 1);
	grayImg[i] = (grayImg[i] << 1) | bit;
	++i;
      }
    }
    delete grayBitmap;
  }

  // decode the image
  i = 0;
  for (m = 0; m < gridH; ++m) {
    xx = gridX + m * stepY;
    yy = gridY + m * stepX;
    for (n = 0; n < gridW; ++n) {
      if (!(enableSkip && skipBitmap->getPixel(n, m))) {
	patternBitmap = patternDict->getBitmap(grayImg[i]);
	bitmap->combine(patternBitmap, xx >> 8, yy >> 8, combOp);
      }
      xx += stepX;
      yy -= stepY;
      ++i;
    }
  }

  gfree(grayImg);
  if (skipBitmap) {
    delete skipBitmap;
  }

  // combine the region bitmap into the page bitmap
  if (imm) {
    if (pageH == 0xffffffff && y + h > curPageH) {
      pageBitmap->expand(y + h, pageDefPixel);
    }
    pageBitmap->combine(bitmap, x, y, extCombOp);
    delete bitmap;

  // store the region bitmap
  } else {
    segments->append(bitmap);
  }

  return;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
}

void JBIG2Stream::readGenericRegionSeg(Guint segNum, GBool imm,
				       GBool lossless, Guint length) {
  JBIG2Bitmap *bitmap;
  Guint w, h, x, y, segInfoFlags, extCombOp;
  Guint flags, mmr, templ, tpgdOn;
  int atx[4], aty[4];

  // region segment info field
  if (!readULong(&w) || !readULong(&h) ||
      !readULong(&x) || !readULong(&y) ||
      !readUByte(&segInfoFlags)) {
    goto eofError;
  }
  extCombOp = segInfoFlags & 7;

  // rest of the generic region segment header
  if (!readUByte(&flags)) {
    goto eofError;
  }
  mmr = flags & 1;
  templ = (flags >> 1) & 3;
  tpgdOn = (flags >> 3) & 1;

  // AT flags
  if (!mmr) {
    if (templ == 0) {
      if (!readByte(&atx[0]) ||
	  !readByte(&aty[0]) ||
	  !readByte(&atx[1]) ||
	  !readByte(&aty[1]) ||
	  !readByte(&atx[2]) ||
	  !readByte(&aty[2]) ||
	  !readByte(&atx[3]) ||
	  !readByte(&aty[3])) {
	goto eofError;
      }
    } else {
      if (!readByte(&atx[0]) ||
	  !readByte(&aty[0])) {
	goto eofError;
      }
    }
  }

  // set up the arithmetic decoder
  if (!mmr) {
    resetGenericStats(templ, NULL);
    arithDecoder->start();
  }

  // read the bitmap
  bitmap = readGenericBitmap(mmr, w, h, templ, tpgdOn, gFalse,
			     NULL, atx, aty, mmr ? length - 18 : 0);
  if (!bitmap)
    return;

  // combine the region bitmap into the page bitmap
  if (imm) {
    if (pageH == 0xffffffff && y + h > curPageH) {
      pageBitmap->expand(y + h, pageDefPixel);
    }
    pageBitmap->combine(bitmap, x, y, extCombOp);
    delete bitmap;

  // store the region bitmap
  } else {
    bitmap->setSegNum(segNum);
    segments->append(bitmap);
  }

  return;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
}

inline void JBIG2Stream::mmrAddPixels(int a1, int blackPixels,
				      int *codingLine, int *a0i, int w) {
  if (a1 > codingLine[*a0i]) {
    if (a1 > w) {
      error(curStr->getPos(), "JBIG2 MMR row is wrong length (%d)", a1);
      a1 = w;
    }
    if ((*a0i & 1) ^ blackPixels) {
      ++*a0i;
    }
    codingLine[*a0i] = a1;
  }
}

inline void JBIG2Stream::mmrAddPixelsNeg(int a1, int blackPixels,
					 int *codingLine, int *a0i, int w) {
  if (a1 > codingLine[*a0i]) {
    if (a1 > w) {
      error(curStr->getPos(), "JBIG2 MMR row is wrong length (%d)", a1);
      a1 = w;
    }
    if ((*a0i & 1) ^ blackPixels) {
      ++*a0i;
    }
    codingLine[*a0i] = a1;
  } else if (a1 < codingLine[*a0i]) {
    if (a1 < 0) {
      error(curStr->getPos(), "Invalid JBIG2 MMR code");
      a1 = 0;
    }
    while (*a0i > 0 && a1 <= codingLine[*a0i - 1]) {
      --*a0i;
    }
    codingLine[*a0i] = a1;
  }
}

JBIG2Bitmap *JBIG2Stream::readGenericBitmap(GBool mmr, int w, int h,
					    int templ, GBool tpgdOn,
					    GBool useSkip, JBIG2Bitmap *skip,
					    int *atx, int *aty,
					    int mmrDataLength) {
  JBIG2Bitmap *bitmap;
  GBool ltp;
  Guint ltpCX, cx, cx0, cx1, cx2;
  JBIG2BitmapPtr cxPtr0 = {0}, cxPtr1 = {0};
  JBIG2BitmapPtr atPtr0 = {0}, atPtr1 = {0}, atPtr2 = {0}, atPtr3 = {0};
  int *refLine, *codingLine;
  int code1, code2, code3;
  int x, y, a0i, b1i, blackPixels, pix, i;

  bitmap = new JBIG2Bitmap(0, w, h);
  if (!bitmap->isOk()) {
    delete bitmap;
    return NULL;
  }
  bitmap->clearToZero();

  //----- MMR decode

  if (mmr) {

    mmrDecoder->reset();
    if (w > INT_MAX - 2) {
      error(curStr->getPos(), "Bad width in JBIG2 generic bitmap");
      // force a call to gmalloc(-1), which will throw an exception
      w = -3;
    }
    // 0 <= codingLine[0] < codingLine[1] < ... < codingLine[n] = w
    // ---> max codingLine size = w + 1
    // refLine has one extra guard entry at the end
    // ---> max refLine size = w + 2
    codingLine = (int *)gmallocn(w + 1, sizeof(int));
    refLine = (int *)gmallocn(w + 2, sizeof(int));
    for (i = 0; i < w + 1; ++i) codingLine[i] = w;

    for (y = 0; y < h; ++y) {

      // copy coding line to ref line
      for (i = 0; codingLine[i] < w; ++i) {
	refLine[i] = codingLine[i];
      }
      refLine[i++] = w;
      refLine[i] = w;

      // decode a line
      codingLine[0] = 0;
      a0i = 0;
      b1i = 0;
      blackPixels = 0;
      // invariant:
      // refLine[b1i-1] <= codingLine[a0i] < refLine[b1i] < refLine[b1i+1] <= w
      // exception at left edge:
      //   codingLine[a0i = 0] = refLine[b1i = 0] = 0 is possible
      // exception at right edge:
      //   refLine[b1i] = refLine[b1i+1] = w is possible
      while (codingLine[a0i] < w) {
	code1 = mmrDecoder->get2DCode();
	switch (code1) {
	case twoDimPass:
          mmrAddPixels(refLine[b1i + 1], blackPixels, codingLine, &a0i, w);
          if (refLine[b1i + 1] < w) {
            b1i += 2;
          }
          break;
	case twoDimHoriz:
          code1 = code2 = 0;
          if (blackPixels) {
            do {
              code1 += code3 = mmrDecoder->getBlackCode();
            } while (code3 >= 64);
            do {
              code2 += code3 = mmrDecoder->getWhiteCode();
            } while (code3 >= 64);
          } else {
            do {
              code1 += code3 = mmrDecoder->getWhiteCode();
            } while (code3 >= 64);
            do {
              code2 += code3 = mmrDecoder->getBlackCode();
            } while (code3 >= 64);
          }
          mmrAddPixels(codingLine[a0i] + code1, blackPixels,
		       codingLine, &a0i, w);
          if (codingLine[a0i] < w) {
            mmrAddPixels(codingLine[a0i] + code2, blackPixels ^ 1,
			 codingLine, &a0i, w);
          }
          while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
            b1i += 2;
          }
          break;
	case twoDimVertR3:
          mmrAddPixels(refLine[b1i] + 3, blackPixels, codingLine, &a0i, w);
          blackPixels ^= 1;
          if (codingLine[a0i] < w) {
            ++b1i;
            while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
              b1i += 2;
            }
          }
          break;
	case twoDimVertR2:
          mmrAddPixels(refLine[b1i] + 2, blackPixels, codingLine, &a0i, w);
          blackPixels ^= 1;
          if (codingLine[a0i] < w) {
            ++b1i;
            while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
              b1i += 2;
            }
          }
          break;
	case twoDimVertR1:
          mmrAddPixels(refLine[b1i] + 1, blackPixels, codingLine, &a0i, w);
          blackPixels ^= 1;
          if (codingLine[a0i] < w) {
            ++b1i;
            while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
              b1i += 2;
            }
          }
          break;
	case twoDimVert0:
          mmrAddPixels(refLine[b1i], blackPixels, codingLine, &a0i, w);
          blackPixels ^= 1;
          if (codingLine[a0i] < w) {
            ++b1i;
            while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
              b1i += 2;
            }
          }
          break;
	case twoDimVertL3:
          mmrAddPixelsNeg(refLine[b1i] - 3, blackPixels, codingLine, &a0i, w);
          blackPixels ^= 1;
          if (codingLine[a0i] < w) {
            if (b1i > 0) {
              --b1i;
            } else {
              ++b1i;
            }
            while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
              b1i += 2;
            }
          }
          break;
	case twoDimVertL2:
          mmrAddPixelsNeg(refLine[b1i] - 2, blackPixels, codingLine, &a0i, w);
          blackPixels ^= 1;
          if (codingLine[a0i] < w) {
            if (b1i > 0) {
              --b1i;
            } else {
              ++b1i;
            }
            while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
              b1i += 2;
            }
          }
          break;
	case twoDimVertL1:
          mmrAddPixelsNeg(refLine[b1i] - 1, blackPixels, codingLine, &a0i, w);
          blackPixels ^= 1;
          if (codingLine[a0i] < w) {
            if (b1i > 0) {
              --b1i;
            } else {
              ++b1i;
            }
            while (refLine[b1i] <= codingLine[a0i] && refLine[b1i] < w) {
              b1i += 2;
            }
          }
          break;
	case EOF:
          mmrAddPixels(w, 0, codingLine, &a0i, w);
          break;
	default:
	  error(curStr->getPos(), "Illegal code in JBIG2 MMR bitmap data");
          mmrAddPixels(w, 0, codingLine, &a0i, w);
	  break;
	}
      }

      // convert the run lengths to a bitmap line
      i = 0;
      while (1) {
	for (x = codingLine[i]; x < codingLine[i+1]; ++x) {
	  bitmap->setPixel(x, y);
	}
	if (codingLine[i+1] >= w || codingLine[i+2] >= w) {
	  break;
	}
	i += 2;
      }
    }

    if (mmrDataLength >= 0) {
      mmrDecoder->skipTo(mmrDataLength);
    } else {
      if (mmrDecoder->get24Bits() != 0x001001) {
	error(curStr->getPos(), "Missing EOFB in JBIG2 MMR bitmap data");
      }
    }

    gfree(refLine);
    gfree(codingLine);

  //----- arithmetic decode

  } else {
    // set up the typical row context
    ltpCX = 0; // make gcc happy
    if (tpgdOn) {
      switch (templ) {
      case 0:
	ltpCX = 0x3953; // 001 11001 0101 0011
	break;
      case 1:
	ltpCX = 0x079a; // 0011 11001 101 0
	break;
      case 2:
	ltpCX = 0x0e3; // 001 1100 01 1
	break;
      case 3:
	ltpCX = 0x18a; // 01100 0101 1
	break;
      }
    }

    ltp = 0;
    cx = cx0 = cx1 = cx2 = 0; // make gcc happy
    for (y = 0; y < h; ++y) {

      // check for a "typical" (duplicate) row
      if (tpgdOn) {
	if (arithDecoder->decodeBit(ltpCX, genericRegionStats)) {
	  ltp = !ltp;
	}
	if (ltp) {
	  if (y > 0) {
	    bitmap->duplicateRow(y, y-1);
	  }
	  continue;
	}
      }

      switch (templ) {
      case 0:

	// set up the context
	bitmap->getPixelPtr(0, y-2, &cxPtr0);
	cx0 = bitmap->nextPixel(&cxPtr0);
	cx0 = (cx0 << 1) | bitmap->nextPixel(&cxPtr0);
	bitmap->getPixelPtr(0, y-1, &cxPtr1);
	cx1 = bitmap->nextPixel(&cxPtr1);
	cx1 = (cx1 << 1) | bitmap->nextPixel(&cxPtr1);
	cx1 = (cx1 << 1) | bitmap->nextPixel(&cxPtr1);
	cx2 = 0;
	bitmap->getPixelPtr(atx[0], y + aty[0], &atPtr0);
	bitmap->getPixelPtr(atx[1], y + aty[1], &atPtr1);
	bitmap->getPixelPtr(atx[2], y + aty[2], &atPtr2);
	bitmap->getPixelPtr(atx[3], y + aty[3], &atPtr3);

	// decode the row
	for (x = 0; x < w; ++x) {

	  // build the context
	  cx = (cx0 << 13) | (cx1 << 8) | (cx2 << 4) |
	       (bitmap->nextPixel(&atPtr0) << 3) |
	       (bitmap->nextPixel(&atPtr1) << 2) |
	       (bitmap->nextPixel(&atPtr2) << 1) |
	       bitmap->nextPixel(&atPtr3);

	  // check for a skipped pixel
	  if (useSkip && skip->getPixel(x, y)) {
	    pix = 0;

	  // decode the pixel
	  } else if ((pix = arithDecoder->decodeBit(cx, genericRegionStats))) {
	    bitmap->setPixel(x, y);
	  }

	  // update the context
	  cx0 = ((cx0 << 1) | bitmap->nextPixel(&cxPtr0)) & 0x07;
	  cx1 = ((cx1 << 1) | bitmap->nextPixel(&cxPtr1)) & 0x1f;
	  cx2 = ((cx2 << 1) | pix) & 0x0f;
	}
	break;

      case 1:

	// set up the context
	bitmap->getPixelPtr(0, y-2, &cxPtr0);
	cx0 = bitmap->nextPixel(&cxPtr0);
	cx0 = (cx0 << 1) | bitmap->nextPixel(&cxPtr0);
	cx0 = (cx0 << 1) | bitmap->nextPixel(&cxPtr0);
	bitmap->getPixelPtr(0, y-1, &cxPtr1);
	cx1 = bitmap->nextPixel(&cxPtr1);
	cx1 = (cx1 << 1) | bitmap->nextPixel(&cxPtr1);
	cx1 = (cx1 << 1) | bitmap->nextPixel(&cxPtr1);
	cx2 = 0;
	bitmap->getPixelPtr(atx[0], y + aty[0], &atPtr0);

	// decode the row
	for (x = 0; x < w; ++x) {

	  // build the context
	  cx = (cx0 << 9) | (cx1 << 4) | (cx2 << 1) |
	       bitmap->nextPixel(&atPtr0);

	  // check for a skipped pixel
	  if (useSkip && skip->getPixel(x, y)) {
	    pix = 0;

	  // decode the pixel
	  } else if ((pix = arithDecoder->decodeBit(cx, genericRegionStats))) {
	    bitmap->setPixel(x, y);
	  }

	  // update the context
	  cx0 = ((cx0 << 1) | bitmap->nextPixel(&cxPtr0)) & 0x0f;
	  cx1 = ((cx1 << 1) | bitmap->nextPixel(&cxPtr1)) & 0x1f;
	  cx2 = ((cx2 << 1) | pix) & 0x07;
	}
	break;

      case 2:

	// set up the context
	bitmap->getPixelPtr(0, y-2, &cxPtr0);
	cx0 = bitmap->nextPixel(&cxPtr0);
	cx0 = (cx0 << 1) | bitmap->nextPixel(&cxPtr0);
	bitmap->getPixelPtr(0, y-1, &cxPtr1);
	cx1 = bitmap->nextPixel(&cxPtr1);
	cx1 = (cx1 << 1) | bitmap->nextPixel(&cxPtr1);
	cx2 = 0;
	bitmap->getPixelPtr(atx[0], y + aty[0], &atPtr0);

	// decode the row
	for (x = 0; x < w; ++x) {

	  // build the context
	  cx = (cx0 << 7) | (cx1 << 3) | (cx2 << 1) |
	       bitmap->nextPixel(&atPtr0);

	  // check for a skipped pixel
	  if (useSkip && skip->getPixel(x, y)) {
	    pix = 0;

	  // decode the pixel
	  } else if ((pix = arithDecoder->decodeBit(cx, genericRegionStats))) {
	    bitmap->setPixel(x, y);
	  }

	  // update the context
	  cx0 = ((cx0 << 1) | bitmap->nextPixel(&cxPtr0)) & 0x07;
	  cx1 = ((cx1 << 1) | bitmap->nextPixel(&cxPtr1)) & 0x0f;
	  cx2 = ((cx2 << 1) | pix) & 0x03;
	}
	break;

      case 3:

	// set up the context
	bitmap->getPixelPtr(0, y-1, &cxPtr1);
	cx1 = bitmap->nextPixel(&cxPtr1);
	cx1 = (cx1 << 1) | bitmap->nextPixel(&cxPtr1);
	cx2 = 0;
	bitmap->getPixelPtr(atx[0], y + aty[0], &atPtr0);

	// decode the row
	for (x = 0; x < w; ++x) {

	  // build the context
	  cx = (cx1 << 5) | (cx2 << 1) |
	       bitmap->nextPixel(&atPtr0);

	  // check for a skipped pixel
	  if (useSkip && skip->getPixel(x, y)) {
	    pix = 0;

	  // decode the pixel
	  } else if ((pix = arithDecoder->decodeBit(cx, genericRegionStats))) {
	    bitmap->setPixel(x, y);
	  }

	  // update the context
	  cx1 = ((cx1 << 1) | bitmap->nextPixel(&cxPtr1)) & 0x1f;
	  cx2 = ((cx2 << 1) | pix) & 0x0f;
	}
	break;
      }
    }
  }

  return bitmap;
}

void JBIG2Stream::readGenericRefinementRegionSeg(Guint segNum, GBool imm,
						 GBool lossless, Guint length,
						 Guint *refSegs,
						 Guint nRefSegs) {
  JBIG2Bitmap *bitmap, *refBitmap;
  Guint w, h, x, y, segInfoFlags, extCombOp;
  Guint flags, templ, tpgrOn;
  int atx[2], aty[2];
  JBIG2Segment *seg;

  // region segment info field
  if (!readULong(&w) || !readULong(&h) ||
      !readULong(&x) || !readULong(&y) ||
      !readUByte(&segInfoFlags)) {
    goto eofError;
  }
  extCombOp = segInfoFlags & 7;

  // rest of the generic refinement region segment header
  if (!readUByte(&flags)) {
    goto eofError;
  }
  templ = flags & 1;
  tpgrOn = (flags >> 1) & 1;

  // AT flags
  if (!templ) {
    if (!readByte(&atx[0]) || !readByte(&aty[0]) ||
	!readByte(&atx[1]) || !readByte(&aty[1])) {
      goto eofError;
    }
  }

  // resize the page bitmap if needed
  if (nRefSegs == 0 || imm) {
    if (pageH == 0xffffffff && y + h > curPageH) {
      pageBitmap->expand(y + h, pageDefPixel);
    }
  }

  // get referenced bitmap
  if (nRefSegs > 1) {
    error(curStr->getPos(), "Bad reference in JBIG2 generic refinement segment");
    return;
  }
  if (nRefSegs == 1) {
    seg = findSegment(refSegs[0]);
    if (seg == NULL || seg->getType() != jbig2SegBitmap) {
      error(curStr->getPos(), "Bad bitmap reference in JBIG2 generic refinement segment");
      return;
    }
    refBitmap = (JBIG2Bitmap *)seg;
  } else {
    refBitmap = pageBitmap->getSlice(x, y, w, h);
  }

  // set up the arithmetic decoder
  resetRefinementStats(templ, NULL);
  arithDecoder->start();

  // read
  bitmap = readGenericRefinementRegion(w, h, templ, tpgrOn,
				       refBitmap, 0, 0, atx, aty);

  // combine the region bitmap into the page bitmap
  if (imm) {
    pageBitmap->combine(bitmap, x, y, extCombOp);
    delete bitmap;

  // store the region bitmap
  } else {
    if (bitmap) {
      bitmap->setSegNum(segNum);
      segments->append(bitmap);
    } else {
      error(curStr->getPos(), "readGenericRefinementRegionSeg with null bitmap");
    }
  }

  // delete the referenced bitmap
  if (nRefSegs == 1) {
    discardSegment(refSegs[0]);
  } else {
    delete refBitmap;
  }

  return;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
}

JBIG2Bitmap *JBIG2Stream::readGenericRefinementRegion(int w, int h,
						      int templ, GBool tpgrOn,
						      JBIG2Bitmap *refBitmap,
						      int refDX, int refDY,
						      int *atx, int *aty) {
  JBIG2Bitmap *bitmap;
  GBool ltp;
  Guint ltpCX, cx, cx0, cx2, cx3, cx4, tpgrCX0, tpgrCX1, tpgrCX2;
  JBIG2BitmapPtr cxPtr0 = {0};
  JBIG2BitmapPtr cxPtr1 = {0};
  JBIG2BitmapPtr cxPtr2 = {0};
  JBIG2BitmapPtr cxPtr3 = {0};
  JBIG2BitmapPtr cxPtr4 = {0};
  JBIG2BitmapPtr cxPtr5 = {0};
  JBIG2BitmapPtr cxPtr6 = {0};
  JBIG2BitmapPtr tpgrCXPtr0 = {0};
  JBIG2BitmapPtr tpgrCXPtr1 = {0};
  JBIG2BitmapPtr tpgrCXPtr2 = {0};
  int x, y, pix;

  bitmap = new JBIG2Bitmap(0, w, h);
  if (!bitmap->isOk())
  {
    delete bitmap;
    return NULL;
  }
  bitmap->clearToZero();

  // set up the typical row context
  if (templ) {
    ltpCX = 0x008;
  } else {
    ltpCX = 0x0010;
  }

  ltp = 0;
  for (y = 0; y < h; ++y) {

    if (templ) {

      // set up the context
      bitmap->getPixelPtr(0, y-1, &cxPtr0);
      cx0 = bitmap->nextPixel(&cxPtr0);
      bitmap->getPixelPtr(-1, y, &cxPtr1);
      refBitmap->getPixelPtr(-refDX, y-1-refDY, &cxPtr2);
      refBitmap->getPixelPtr(-1-refDX, y-refDY, &cxPtr3);
      cx3 = refBitmap->nextPixel(&cxPtr3);
      cx3 = (cx3 << 1) | refBitmap->nextPixel(&cxPtr3);
      refBitmap->getPixelPtr(-refDX, y+1-refDY, &cxPtr4);
      cx4 = refBitmap->nextPixel(&cxPtr4);

      // set up the typical prediction context
      tpgrCX0 = tpgrCX1 = tpgrCX2 = 0; // make gcc happy
      if (tpgrOn) {
	refBitmap->getPixelPtr(-1-refDX, y-1-refDY, &tpgrCXPtr0);
	tpgrCX0 = refBitmap->nextPixel(&tpgrCXPtr0);
	tpgrCX0 = (tpgrCX0 << 1) | refBitmap->nextPixel(&tpgrCXPtr0);
	tpgrCX0 = (tpgrCX0 << 1) | refBitmap->nextPixel(&tpgrCXPtr0);
	refBitmap->getPixelPtr(-1-refDX, y-refDY, &tpgrCXPtr1);
	tpgrCX1 = refBitmap->nextPixel(&tpgrCXPtr1);
	tpgrCX1 = (tpgrCX1 << 1) | refBitmap->nextPixel(&tpgrCXPtr1);
	tpgrCX1 = (tpgrCX1 << 1) | refBitmap->nextPixel(&tpgrCXPtr1);
	refBitmap->getPixelPtr(-1-refDX, y+1-refDY, &tpgrCXPtr2);
	tpgrCX2 = refBitmap->nextPixel(&tpgrCXPtr2);
	tpgrCX2 = (tpgrCX2 << 1) | refBitmap->nextPixel(&tpgrCXPtr2);
	tpgrCX2 = (tpgrCX2 << 1) | refBitmap->nextPixel(&tpgrCXPtr2);
      } else {
	tpgrCXPtr0.p = tpgrCXPtr1.p = tpgrCXPtr2.p = NULL; // make gcc happy
	tpgrCXPtr0.shift = tpgrCXPtr1.shift = tpgrCXPtr2.shift = 0;
	tpgrCXPtr0.x = tpgrCXPtr1.x = tpgrCXPtr2.x = 0;
      }

      for (x = 0; x < w; ++x) {

	// update the context
	cx0 = ((cx0 << 1) | bitmap->nextPixel(&cxPtr0)) & 7;
	cx3 = ((cx3 << 1) | refBitmap->nextPixel(&cxPtr3)) & 7;
	cx4 = ((cx4 << 1) | refBitmap->nextPixel(&cxPtr4)) & 3;

	if (tpgrOn) {
	  // update the typical predictor context
	  tpgrCX0 = ((tpgrCX0 << 1) | refBitmap->nextPixel(&tpgrCXPtr0)) & 7;
	  tpgrCX1 = ((tpgrCX1 << 1) | refBitmap->nextPixel(&tpgrCXPtr1)) & 7;
	  tpgrCX2 = ((tpgrCX2 << 1) | refBitmap->nextPixel(&tpgrCXPtr2)) & 7;

	  // check for a "typical" pixel
	  if (arithDecoder->decodeBit(ltpCX, refinementRegionStats)) {
	    ltp = !ltp;
	  }
	  if (tpgrCX0 == 0 && tpgrCX1 == 0 && tpgrCX2 == 0) {
	    bitmap->clearPixel(x, y);
	    continue;
	  } else if (tpgrCX0 == 7 && tpgrCX1 == 7 && tpgrCX2 == 7) {
	    bitmap->setPixel(x, y);
	    continue;
	  }
	}

	// build the context
	cx = (cx0 << 7) | (bitmap->nextPixel(&cxPtr1) << 6) |
	     (refBitmap->nextPixel(&cxPtr2) << 5) |
	     (cx3 << 2) | cx4;

	// decode the pixel
	if ((pix = arithDecoder->decodeBit(cx, refinementRegionStats))) {
	  bitmap->setPixel(x, y);
	}
      }

    } else {

      // set up the context
      bitmap->getPixelPtr(0, y-1, &cxPtr0);
      cx0 = bitmap->nextPixel(&cxPtr0);
      bitmap->getPixelPtr(-1, y, &cxPtr1);
      refBitmap->getPixelPtr(-refDX, y-1-refDY, &cxPtr2);
      cx2 = refBitmap->nextPixel(&cxPtr2);
      refBitmap->getPixelPtr(-1-refDX, y-refDY, &cxPtr3);
      cx3 = refBitmap->nextPixel(&cxPtr3);
      cx3 = (cx3 << 1) | refBitmap->nextPixel(&cxPtr3);
      refBitmap->getPixelPtr(-1-refDX, y+1-refDY, &cxPtr4);
      cx4 = refBitmap->nextPixel(&cxPtr4);
      cx4 = (cx4 << 1) | refBitmap->nextPixel(&cxPtr4);
      bitmap->getPixelPtr(atx[0], y+aty[0], &cxPtr5);
      refBitmap->getPixelPtr(atx[1]-refDX, y+aty[1]-refDY, &cxPtr6);

      // set up the typical prediction context
      tpgrCX0 = tpgrCX1 = tpgrCX2 = 0; // make gcc happy
      if (tpgrOn) {
	refBitmap->getPixelPtr(-1-refDX, y-1-refDY, &tpgrCXPtr0);
	tpgrCX0 = refBitmap->nextPixel(&tpgrCXPtr0);
	tpgrCX0 = (tpgrCX0 << 1) | refBitmap->nextPixel(&tpgrCXPtr0);
	tpgrCX0 = (tpgrCX0 << 1) | refBitmap->nextPixel(&tpgrCXPtr0);
	refBitmap->getPixelPtr(-1-refDX, y-refDY, &tpgrCXPtr1);
	tpgrCX1 = refBitmap->nextPixel(&tpgrCXPtr1);
	tpgrCX1 = (tpgrCX1 << 1) | refBitmap->nextPixel(&tpgrCXPtr1);
	tpgrCX1 = (tpgrCX1 << 1) | refBitmap->nextPixel(&tpgrCXPtr1);
	refBitmap->getPixelPtr(-1-refDX, y+1-refDY, &tpgrCXPtr2);
	tpgrCX2 = refBitmap->nextPixel(&tpgrCXPtr2);
	tpgrCX2 = (tpgrCX2 << 1) | refBitmap->nextPixel(&tpgrCXPtr2);
	tpgrCX2 = (tpgrCX2 << 1) | refBitmap->nextPixel(&tpgrCXPtr2);
      } else {
	tpgrCXPtr0.p = tpgrCXPtr1.p = tpgrCXPtr2.p = NULL; // make gcc happy
	tpgrCXPtr0.shift = tpgrCXPtr1.shift = tpgrCXPtr2.shift = 0;
	tpgrCXPtr0.x = tpgrCXPtr1.x = tpgrCXPtr2.x = 0;
      }

      for (x = 0; x < w; ++x) {

	// update the context
	cx0 = ((cx0 << 1) | bitmap->nextPixel(&cxPtr0)) & 3;
	cx2 = ((cx2 << 1) | refBitmap->nextPixel(&cxPtr2)) & 3;
	cx3 = ((cx3 << 1) | refBitmap->nextPixel(&cxPtr3)) & 7;
	cx4 = ((cx4 << 1) | refBitmap->nextPixel(&cxPtr4)) & 7;

	if (tpgrOn) {
	  // update the typical predictor context
	  tpgrCX0 = ((tpgrCX0 << 1) | refBitmap->nextPixel(&tpgrCXPtr0)) & 7;
	  tpgrCX1 = ((tpgrCX1 << 1) | refBitmap->nextPixel(&tpgrCXPtr1)) & 7;
	  tpgrCX2 = ((tpgrCX2 << 1) | refBitmap->nextPixel(&tpgrCXPtr2)) & 7;

	  // check for a "typical" pixel
	  if (arithDecoder->decodeBit(ltpCX, refinementRegionStats)) {
	    ltp = !ltp;
	  }
	  if (tpgrCX0 == 0 && tpgrCX1 == 0 && tpgrCX2 == 0) {
	    bitmap->clearPixel(x, y);
	    continue;
	  } else if (tpgrCX0 == 7 && tpgrCX1 == 7 && tpgrCX2 == 7) {
	    bitmap->setPixel(x, y);
	    continue;
	  }
	}

	// build the context
	cx = (cx0 << 11) | (bitmap->nextPixel(&cxPtr1) << 10) |
	     (cx2 << 8) | (cx3 << 5) | (cx4 << 2) |
	     (bitmap->nextPixel(&cxPtr5) << 1) |
	     refBitmap->nextPixel(&cxPtr6);

	// decode the pixel
	if ((pix = arithDecoder->decodeBit(cx, refinementRegionStats))) {
	  bitmap->setPixel(x, y);
	}
      }
    }
  }

  return bitmap;
}

void JBIG2Stream::readPageInfoSeg(Guint length) {
  Guint xRes, yRes, flags, striping;

  if (!readULong(&pageW) || !readULong(&pageH) ||
      !readULong(&xRes) || !readULong(&yRes) ||
      !readUByte(&flags) || !readUWord(&striping)) {
    goto eofError;
  }
  pageDefPixel = (flags >> 2) & 1;
  defCombOp = (flags >> 3) & 3;

  // allocate the page bitmap
  if (pageH == 0xffffffff) {
    curPageH = striping & 0x7fff;
  } else {
    curPageH = pageH;
  }
  pageBitmap = new JBIG2Bitmap(0, pageW, curPageH);

  if (!pageBitmap->isOk()) {
    delete pageBitmap;
    pageBitmap = NULL;
    return;
  }
  
  // default pixel value
  if (pageDefPixel) {
    pageBitmap->clearToOne();
  } else {
    pageBitmap->clearToZero();
  }

  return;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
}

void JBIG2Stream::readEndOfStripeSeg(Guint length) {
  Guint i;

  // skip the segment
  for (i = 0; i < length; ++i) {
    curStr->getChar();
  }
}

void JBIG2Stream::readProfilesSeg(Guint length) {
  Guint i;

  // skip the segment
  for (i = 0; i < length; ++i) {
    curStr->getChar();
  }
}

void JBIG2Stream::readCodeTableSeg(Guint segNum, Guint length) {
  JBIG2HuffmanTable *huffTab;
  Guint flags, oob, prefixBits, rangeBits;
  int lowVal, highVal, val;
  Guint huffTabSize, i;

  if (!readUByte(&flags) || !readLong(&lowVal) || !readLong(&highVal)) {
    goto eofError;
  }
  oob = flags & 1;
  prefixBits = ((flags >> 1) & 7) + 1;
  rangeBits = ((flags >> 4) & 7) + 1;

  huffDecoder->reset();
  huffTabSize = 8;
  huffTab = (JBIG2HuffmanTable *)
                gmallocn(huffTabSize, sizeof(JBIG2HuffmanTable));
  i = 0;
  val = lowVal;
  while (val < highVal) {
    if (i == huffTabSize) {
      huffTabSize *= 2;
      huffTab = (JBIG2HuffmanTable *)
	            greallocn(huffTab, huffTabSize, sizeof(JBIG2HuffmanTable));
    }
    huffTab[i].val = val;
    huffTab[i].prefixLen = huffDecoder->readBits(prefixBits);
    huffTab[i].rangeLen = huffDecoder->readBits(rangeBits);
    val += 1 << huffTab[i].rangeLen;
    ++i;
  }
  if (i + oob + 3 > huffTabSize) {
    huffTabSize = i + oob + 3;
    huffTab = (JBIG2HuffmanTable *)
                  greallocn(huffTab, huffTabSize, sizeof(JBIG2HuffmanTable));
  }
  huffTab[i].val = lowVal - 1;
  huffTab[i].prefixLen = huffDecoder->readBits(prefixBits);
  huffTab[i].rangeLen = jbig2HuffmanLOW;
  ++i;
  huffTab[i].val = highVal;
  huffTab[i].prefixLen = huffDecoder->readBits(prefixBits);
  huffTab[i].rangeLen = 32;
  ++i;
  if (oob) {
    huffTab[i].val = 0;
    huffTab[i].prefixLen = huffDecoder->readBits(prefixBits);
    huffTab[i].rangeLen = jbig2HuffmanOOB;
    ++i;
  }
  huffTab[i].val = 0;
  huffTab[i].prefixLen = 0;
  huffTab[i].rangeLen = jbig2HuffmanEOT;
  huffDecoder->buildTable(huffTab, i);

  // create and store the new table segment
  segments->append(new JBIG2CodeTable(segNum, huffTab));

  return;

 eofError:
  error(curStr->getPos(), "Unexpected EOF in JBIG2 stream");
}

void JBIG2Stream::readExtensionSeg(Guint length) {
  Guint i;

  // skip the segment
  for (i = 0; i < length; ++i) {
    curStr->getChar();
  }
}

JBIG2Segment *JBIG2Stream::findSegment(Guint segNum) {
  JBIG2Segment *seg;
  int i;

  for (i = 0; i < globalSegments->getLength(); ++i) {
    seg = (JBIG2Segment *)globalSegments->get(i);
    if (seg->getSegNum() == segNum) {
      return seg;
    }
  }
  for (i = 0; i < segments->getLength(); ++i) {
    seg = (JBIG2Segment *)segments->get(i);
    if (seg->getSegNum() == segNum) {
      return seg;
    }
  }
  return NULL;
}

void JBIG2Stream::discardSegment(Guint segNum) {
  JBIG2Segment *seg;
  int i;

  for (i = 0; i < globalSegments->getLength(); ++i) {
    seg = (JBIG2Segment *)globalSegments->get(i);
    if (seg->getSegNum() == segNum) {
      globalSegments->del(i);
      return;
    }
  }
  for (i = 0; i < segments->getLength(); ++i) {
    seg = (JBIG2Segment *)segments->get(i);
    if (seg->getSegNum() == segNum) {
      segments->del(i);
      return;
    }
  }
}

void JBIG2Stream::resetGenericStats(Guint templ,
				    JArithmeticDecoderStats *prevStats) {
  int size;

  size = contextSize[templ];
  if (prevStats && prevStats->getContextSize() == size) {
    if (genericRegionStats->getContextSize() == size) {
      genericRegionStats->copyFrom(prevStats);
    } else {
      delete genericRegionStats;
      genericRegionStats = prevStats->copy();
    }
  } else {
    if (genericRegionStats->getContextSize() == size) {
      genericRegionStats->reset();
    } else {
      delete genericRegionStats;
      genericRegionStats = new JArithmeticDecoderStats(1 << size);
    }
  }
}

void JBIG2Stream::resetRefinementStats(Guint templ,
				       JArithmeticDecoderStats *prevStats) {
  int size;

  size = refContextSize[templ];
  if (prevStats && prevStats->getContextSize() == size) {
    if (refinementRegionStats->getContextSize() == size) {
      refinementRegionStats->copyFrom(prevStats);
    } else {
      delete refinementRegionStats;
      refinementRegionStats = prevStats->copy();
    }
  } else {
    if (refinementRegionStats->getContextSize() == size) {
      refinementRegionStats->reset();
    } else {
      delete refinementRegionStats;
      refinementRegionStats = new JArithmeticDecoderStats(1 << size);
    }
  }
}

void JBIG2Stream::resetIntStats(int symCodeLen) {
  iadhStats->reset();
  iadwStats->reset();
  iaexStats->reset();
  iaaiStats->reset();
  iadtStats->reset();
  iaitStats->reset();
  iafsStats->reset();
  iadsStats->reset();
  iardxStats->reset();
  iardyStats->reset();
  iardwStats->reset();
  iardhStats->reset();
  iariStats->reset();
  if (iaidStats->getContextSize() == 1 << (symCodeLen + 1)) {
    iaidStats->reset();
  } else {
    delete iaidStats;
    iaidStats = new JArithmeticDecoderStats(1 << (symCodeLen + 1));
  }
}

GBool JBIG2Stream::readUByte(Guint *x) {
  int c0;

  if ((c0 = curStr->getChar()) == EOF) {
    return gFalse;
  }
  *x = (Guint)c0;
  return gTrue;
}

GBool JBIG2Stream::readByte(int *x) {
 int c0;

  if ((c0 = curStr->getChar()) == EOF) {
    return gFalse;
  }
  *x = c0;
  if (c0 & 0x80) {
    *x |= -1 - 0xff;
  }
  return gTrue;
}

GBool JBIG2Stream::readUWord(Guint *x) {
  int c0, c1;

  if ((c0 = curStr->getChar()) == EOF ||
      (c1 = curStr->getChar()) == EOF) {
    return gFalse;
  }
  *x = (Guint)((c0 << 8) | c1);
  return gTrue;
}

GBool JBIG2Stream::readULong(Guint *x) {
  int c0, c1, c2, c3;

  if ((c0 = curStr->getChar()) == EOF ||
      (c1 = curStr->getChar()) == EOF ||
      (c2 = curStr->getChar()) == EOF ||
      (c3 = curStr->getChar()) == EOF) {
    return gFalse;
  }
  *x = (Guint)((c0 << 24) | (c1 << 16) | (c2 << 8) | c3);
  return gTrue;
}

GBool JBIG2Stream::readLong(int *x) {
  int c0, c1, c2, c3;

  if ((c0 = curStr->getChar()) == EOF ||
      (c1 = curStr->getChar()) == EOF ||
      (c2 = curStr->getChar()) == EOF ||
      (c3 = curStr->getChar()) == EOF) {
    return gFalse;
  }
  *x = ((c0 << 24) | (c1 << 16) | (c2 << 8) | c3);
  if (c0 & 0x80) {
    *x |= -1 - (int)0xffffffff;
  }
  return gTrue;
}
