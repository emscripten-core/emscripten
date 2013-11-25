//========================================================================
//
// FoFiIdentifier.cc
//
// Copyright 2009 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2013 Christoph Duelli <duelli@melosgmbh.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "goo/gtypes.h"
#include "FoFiIdentifier.h"

//------------------------------------------------------------------------

namespace { // do not pollute global namespace

class Reader {
public:

  virtual ~Reader() {}

  // Read one byte.  Returns -1 if past EOF.
  virtual int getByte(int pos) = 0;

  // Read a big-endian unsigned 16-bit integer.  Fills in *val and
  // returns true if successful.
  virtual GBool getU16BE(int pos, int *val) = 0;

  // Read a big-endian unsigned 32-bit integer.  Fills in *val and
  // returns true if successful.
  virtual GBool getU32BE(int pos, Guint *val) = 0;

  // Read a little-endian unsigned 32-bit integer.  Fills in *val and
  // returns true if successful.
  virtual GBool getU32LE(int pos, Guint *val) = 0;

  // Read a big-endian unsigned <size>-byte integer, where 1 <= size
  // <= 4.  Fills in *val and returns true if successful.
  virtual GBool getUVarBE(int pos, int size, Guint *val) = 0;

  // Compare against a string.  Returns true if equal.
  virtual GBool cmp(int pos, const char *s) = 0;

};

//------------------------------------------------------------------------

class MemReader: public Reader {
public:

  static MemReader *make(char *bufA, int lenA);
  virtual ~MemReader();
  virtual int getByte(int pos);
  virtual GBool getU16BE(int pos, int *val);
  virtual GBool getU32BE(int pos, Guint *val);
  virtual GBool getU32LE(int pos, Guint *val);
  virtual GBool getUVarBE(int pos, int size, Guint *val);
  virtual GBool cmp(int pos, const char *s);

private:

  MemReader(char *bufA, int lenA);

  char *buf;
  int len;
};

MemReader *MemReader::make(char *bufA, int lenA) {
  return new MemReader(bufA, lenA);
}

MemReader::MemReader(char *bufA, int lenA) {
  buf = bufA;
  len = lenA;
}

MemReader::~MemReader() {
}

int MemReader::getByte(int pos) {
  if (pos < 0 || pos >= len) {
    return -1;
  }
  return buf[pos] & 0xff;
}

GBool MemReader::getU16BE(int pos, int *val) {
  if (pos < 0 || pos > len - 2) {
    return gFalse;
  }
  *val = ((buf[pos] & 0xff) << 8) +
         (buf[pos+1] & 0xff);
  return gTrue;
}

GBool MemReader::getU32BE(int pos, Guint *val) {
  if (pos < 0 || pos > len - 4) {
    return gFalse;
  }
  *val = ((buf[pos] & 0xff) << 24) +
         ((buf[pos+1] & 0xff) << 16) +
         ((buf[pos+2] & 0xff) << 8) +
         (buf[pos+3] & 0xff);
  return gTrue;
}

GBool MemReader::getU32LE(int pos, Guint *val) {
  if (pos < 0 || pos > len - 4) {
    return gFalse;
  }
  *val = (buf[pos] & 0xff) +
         ((buf[pos+1] & 0xff) << 8) +
         ((buf[pos+2] & 0xff) << 16) +
         ((buf[pos+3] & 0xff) << 24);
  return gTrue;
}

GBool MemReader::getUVarBE(int pos, int size, Guint *val) {
  int i;

  if (size < 1 || size > 4 || pos < 0 || pos > len - size) {
    return gFalse;
  }
  *val = 0;
  for (i = 0; i < size; ++i) {
    *val = (*val << 8) + (buf[pos + i] & 0xff);
  }
  return gTrue;
}

GBool MemReader::cmp(int pos, const char *s) {
  int n;

  n = (int)strlen(s);
  if (pos < 0 || len < n || pos > len - n) {
    return gFalse;
  }
  return !memcmp(buf + pos, s, n);
}

//------------------------------------------------------------------------

class FileReader: public Reader {
public:

  static FileReader *make(char *fileName);
  virtual ~FileReader();
  virtual int getByte(int pos);
  virtual GBool getU16BE(int pos, int *val);
  virtual GBool getU32BE(int pos, Guint *val);
  virtual GBool getU32LE(int pos, Guint *val);
  virtual GBool getUVarBE(int pos, int size, Guint *val);
  virtual GBool cmp(int pos, const char *s);

private:

  FileReader(FILE *fA);
  GBool fillBuf(int pos, int len);

  FILE *f;
  char buf[1024];
  int bufPos, bufLen;
};

FileReader *FileReader::make(char *fileName) {
  FILE *fA;

  if (!(fA = fopen(fileName, "rb"))) {
    return NULL;
  }
  return new FileReader(fA);
}

FileReader::FileReader(FILE *fA) {
  f = fA;
  bufPos = 0;
  bufLen = 0;
}

FileReader::~FileReader() {
  fclose(f);
}

int FileReader::getByte(int pos) {
  if (!fillBuf(pos, 1)) {
    return -1;
  }
  return buf[pos - bufPos] & 0xff;
}

GBool FileReader::getU16BE(int pos, int *val) {
  if (!fillBuf(pos, 2)) {
    return gFalse;
  }
  *val = ((buf[pos - bufPos] & 0xff) << 8) +
         (buf[pos - bufPos + 1] & 0xff);
  return gTrue;
}

GBool FileReader::getU32BE(int pos, Guint *val) {
  if (!fillBuf(pos, 4)) {
    return gFalse;
  }
  *val = ((buf[pos - bufPos] & 0xff) << 24) +
         ((buf[pos - bufPos + 1] & 0xff) << 16) +
         ((buf[pos - bufPos + 2] & 0xff) << 8) +
         (buf[pos - bufPos + 3] & 0xff);
  return gTrue;
}

GBool FileReader::getU32LE(int pos, Guint *val) {
  if (!fillBuf(pos, 4)) {
    return gFalse;
  }
  *val = (buf[pos - bufPos] & 0xff) +
         ((buf[pos - bufPos + 1] & 0xff) << 8) +
         ((buf[pos - bufPos + 2] & 0xff) << 16) +
         ((buf[pos - bufPos + 3] & 0xff) << 24);
  return gTrue;
}

GBool FileReader::getUVarBE(int pos, int size, Guint *val) {
  int i;

  if (size < 1 || size > 4 || !fillBuf(pos, size)) {
    return gFalse;
  }
  *val = 0;
  for (i = 0; i < size; ++i) {
    *val = (*val << 8) + (buf[pos - bufPos + i] & 0xff);
  }
  return gTrue;
}

GBool FileReader::cmp(int pos, const char *s) {
  int n;

  n = (int)strlen(s);
  if (!fillBuf(pos, n)) {
    return gFalse;
  }
  return !memcmp(buf - bufPos + pos, s, n);
}

GBool FileReader::fillBuf(int pos, int len) {
  if (pos < 0 || len < 0 || len > (int)sizeof(buf) ||
      pos > INT_MAX - (int)sizeof(buf)) {
    return gFalse;
  }
  if (pos >= bufPos && pos + len <= bufPos + bufLen) {
    return gTrue;
  }
  if (fseek(f, pos, SEEK_SET)) {
    return gFalse;
  }
  bufPos = pos;
  bufLen = (int)fread(buf, 1, sizeof(buf), f);
  if (bufLen < len) {
    return gFalse;
  }
  return gTrue;
}

//------------------------------------------------------------------------

class StreamReader: public Reader {
public:

  static StreamReader *make(int (*getCharA)(void *data), void *dataA);
  virtual ~StreamReader();
  virtual int getByte(int pos);
  virtual GBool getU16BE(int pos, int *val);
  virtual GBool getU32BE(int pos, Guint *val);
  virtual GBool getU32LE(int pos, Guint *val);
  virtual GBool getUVarBE(int pos, int size, Guint *val);
  virtual GBool cmp(int pos, const char *s);

private:

  StreamReader(int (*getCharA)(void *data), void *dataA);
  GBool fillBuf(int pos, int len);

  int (*getChar)(void *data);
  void *data;
  int streamPos;
  char buf[1024];
  int bufPos, bufLen;
};

StreamReader *StreamReader::make(int (*getCharA)(void *data), void *dataA) {
  return new StreamReader(getCharA, dataA);
}

StreamReader::StreamReader(int (*getCharA)(void *data), void *dataA) {
  getChar = getCharA;
  data = dataA;
  streamPos = 0;
  bufPos = 0;
  bufLen = 0;
}

StreamReader::~StreamReader() {
}

int StreamReader::getByte(int pos) {
  if (!fillBuf(pos, 1)) {
    return -1;
  }
  return buf[pos - bufPos] & 0xff;
}

GBool StreamReader::getU16BE(int pos, int *val) {
  if (!fillBuf(pos, 2)) {
    return gFalse;
  }
  *val = ((buf[pos - bufPos] & 0xff) << 8) +
         (buf[pos - bufPos + 1] & 0xff);
  return gTrue;
}

GBool StreamReader::getU32BE(int pos, Guint *val) {
  if (!fillBuf(pos, 4)) {
    return gFalse;
  }
  *val = ((buf[pos - bufPos] & 0xff) << 24) +
         ((buf[pos - bufPos + 1] & 0xff) << 16) +
         ((buf[pos - bufPos + 2] & 0xff) << 8) +
         (buf[pos - bufPos + 3] & 0xff);
  return gTrue;
}

GBool StreamReader::getU32LE(int pos, Guint *val) {
  if (!fillBuf(pos, 4)) {
    return gFalse;
  }
  *val = (buf[pos - bufPos] & 0xff) +
         ((buf[pos - bufPos + 1] & 0xff) << 8) +
         ((buf[pos - bufPos + 2] & 0xff) << 16) +
         ((buf[pos - bufPos + 3] & 0xff) << 24);
  return gTrue;
}

GBool StreamReader::getUVarBE(int pos, int size, Guint *val) {
  int i;

  if (size < 1 || size > 4 || !fillBuf(pos, size)) {
    return gFalse;
  }
  *val = 0;
  for (i = 0; i < size; ++i) {
    *val = (*val << 8) + (buf[pos - bufPos + i] & 0xff);
  }
  return gTrue;
}

GBool StreamReader::cmp(int pos, const char *s) {
  int n;

  n = (int)strlen(s);
  if (!fillBuf(pos, n)) {
    return gFalse;
  }
  return !memcmp(buf - bufPos + pos, s, n);
}

GBool StreamReader::fillBuf(int pos, int len) {
  int c;

  if (pos < 0 || len < 0 || len > (int)sizeof(buf) ||
      pos > INT_MAX - (int)sizeof(buf)) {
    return gFalse;
  }
  if (pos < bufPos) {
    return gFalse;
  }

  // if requested region will not fit in the current buffer...
  if (pos + len > bufPos + (int)sizeof(buf)) {

    // if the start of the requested data is already in the buffer, move
    // it to the start of the buffer
    if (pos < bufPos + bufLen) {
      bufLen -= pos - bufPos;
      memmove(buf, buf + (pos - bufPos), bufLen);
      bufPos = pos;

    // otherwise discard data from the
    // stream until we get to the requested position
    } else {
      bufPos += bufLen;
      bufLen = 0;
      while (bufPos < pos) {
	if ((c = (*getChar)(data)) < 0) {
	  return gFalse;
	}
	++bufPos;
      }
    }
  }

  // read the rest of the requested data
  while (bufPos + bufLen < pos + len) {
    if ((c = (*getChar)(data)) < 0) {
      return gFalse;
    }
    buf[bufLen++] = (char)c;
  }

  return gTrue;
}

}

//------------------------------------------------------------------------

static FoFiIdentifierType identify(Reader *reader);
static FoFiIdentifierType identifyOpenType(Reader *reader);
static FoFiIdentifierType identifyCFF(Reader *reader, int start);

FoFiIdentifierType FoFiIdentifier::identifyMem(char *file, int len) {
  MemReader *reader;
  FoFiIdentifierType type;

  if (!(reader = MemReader::make(file, len))) {
    return fofiIdError;
  }
  type = identify(reader);
  delete reader;
  return type;
}

FoFiIdentifierType FoFiIdentifier::identifyFile(char *fileName) {
  FileReader *reader;
  FoFiIdentifierType type;

  if (!(reader = FileReader::make(fileName))) {
    return fofiIdError;
  }
  type = identify(reader);
  delete reader;
  return type;
}

FoFiIdentifierType FoFiIdentifier::identifyStream(int (*getChar)(void *data),
						  void *data) {
  StreamReader *reader;
  FoFiIdentifierType type;

  if (!(reader = StreamReader::make(getChar, data))) {
    return fofiIdError;
  }
  type = identify(reader);
  delete reader;
  return type;
}

static FoFiIdentifierType identify(Reader *reader) {
  Guint n;

  //----- PFA
  if (reader->cmp(0, "%!PS-AdobeFont-1") ||
      reader->cmp(0, "%!FontType1")) {
    return fofiIdType1PFA;
  }

  //----- PFB
  if (reader->getByte(0) == 0x80 &&
      reader->getByte(1) == 0x01 &&
      reader->getU32LE(2, &n)) {
    if ((n >= 16 && reader->cmp(6, "%!PS-AdobeFont-1")) ||
	(n >= 11 && reader->cmp(6, "%!FontType1"))) {
      return fofiIdType1PFB;
    }
  }

  //----- TrueType
  if ((reader->getByte(0) == 0x00 &&
       reader->getByte(1) == 0x01 &&
       reader->getByte(2) == 0x00 &&
       reader->getByte(3) == 0x00) ||
      (reader->getByte(0) == 0x74 &&	// 'true'
       reader->getByte(1) == 0x72 &&
       reader->getByte(2) == 0x75 &&
       reader->getByte(3) == 0x65)) {
    return fofiIdTrueType;
  }
  if (reader->getByte(0) == 0x74 &&	// 'ttcf'
      reader->getByte(1) == 0x74 &&
      reader->getByte(2) == 0x63 &&
      reader->getByte(3) == 0x66) {
    return fofiIdTrueTypeCollection;
  }

  //----- OpenType
  if (reader->getByte(0) == 0x4f &&	// 'OTTO
      reader->getByte(1) == 0x54 &&
      reader->getByte(2) == 0x54 &&
      reader->getByte(3) == 0x4f) {
    return identifyOpenType(reader);
  }

  //----- CFF
  if (reader->getByte(0) == 0x01 &&
      reader->getByte(1) == 0x00) {
    return identifyCFF(reader, 0);
  }
  // some tools embed CFF fonts with an extra whitespace char at the
  // beginning
  if (reader->getByte(1) == 0x01 &&
      reader->getByte(2) == 0x00) {
    return identifyCFF(reader, 1);
  }

  return fofiIdUnknown;
}

static FoFiIdentifierType identifyOpenType(Reader *reader) {
  FoFiIdentifierType type;
  Guint offset;
  int nTables, i;

  if (!reader->getU16BE(4, &nTables)) {
    return fofiIdUnknown;
  }
  for (i = 0; i < nTables; ++i) {
    if (reader->cmp(12 + i*16, "CFF ")) {
      if (reader->getU32BE(12 + i*16 + 8, &offset) &&
	  offset < (Guint)INT_MAX) {
	type = identifyCFF(reader, (int)offset);
	if (type == fofiIdCFF8Bit) {
	  type = fofiIdOpenTypeCFF8Bit;
	} else if (type == fofiIdCFFCID) {
	  type = fofiIdOpenTypeCFFCID;
	}
	return type;
      }
      return fofiIdUnknown;
    }
  }
  return fofiIdUnknown;
}

static FoFiIdentifierType identifyCFF(Reader *reader, int start) {
  Guint offset0, offset1;
  int hdrSize, offSize0, offSize1, pos, endPos, b0, n, i;

  //----- read the header
  if (reader->getByte(start) != 0x01 ||
      reader->getByte(start + 1) != 0x00) {
    return fofiIdUnknown;
  }
  if ((hdrSize = reader->getByte(start + 2)) < 0) {
    return fofiIdUnknown;
  }
  if ((offSize0 = reader->getByte(start + 3)) < 1 || offSize0 > 4) {
    return fofiIdUnknown;
  }
  pos = start + hdrSize;
  if (pos < 0) {
    return fofiIdUnknown;
  }

  //----- skip the name index
  if (!reader->getU16BE(pos, &n)) {
    return fofiIdUnknown;
  }
  if (n == 0) {
    pos += 2;
  } else {
    if ((offSize1 = reader->getByte(pos + 2)) < 1 || offSize1 > 4) {
      return fofiIdUnknown;
    }
    if (!reader->getUVarBE(pos + 3 + n * offSize1, offSize1, &offset1) ||
	offset1 > (Guint)INT_MAX) {
      return fofiIdUnknown;
    }
    pos += 3 + (n + 1) * offSize1 + (int)offset1 - 1;
  }
  if (pos < 0) {
    return fofiIdUnknown;
  }

  //----- parse the top dict index
  if (!reader->getU16BE(pos, &n) || n < 1) {
    return fofiIdUnknown;
  }
  if ((offSize1 = reader->getByte(pos + 2)) < 1 || offSize1 > 4) {
    return fofiIdUnknown;
  }
  if (!reader->getUVarBE(pos + 3, offSize1, &offset0) ||
      offset0 > (Guint)INT_MAX ||
      !reader->getUVarBE(pos + 3 + offSize1, offSize1, &offset1) ||
      offset1 > (Guint)INT_MAX ||
      offset0 > offset1) {
    return fofiIdUnknown;
  }
  pos = pos + 3 + (n + 1) * offSize1 + (int)offset0 - 1;
  endPos = pos + 3 + (n + 1) * offSize1 + (int)offset1 - 1;
  if (pos < 0 || endPos < 0 || pos > endPos) {
    return fofiIdUnknown;
  }
  
  //----- parse the top dict, look for ROS as first entry
  // for a CID font, the top dict starts with:
  //     <int> <int> <int> ROS
  for (i = 0; i < 3; ++i) {
    b0 = reader->getByte(pos++);
    if (b0 == 0x1c) {
      pos += 2;
    } else if (b0 == 0x1d) {
      pos += 4;
    } else if (b0 >= 0xf7 && b0 <= 0xfe) {
      pos += 1;
    } else if (b0 < 0x20 || b0 > 0xf6) {
      return fofiIdCFF8Bit;
    }
    if (pos >= endPos || pos < 0) {
      return fofiIdCFF8Bit;
    }
  }
  if (pos + 1 < endPos &&
      reader->getByte(pos) == 12 &&
      reader->getByte(pos + 1) == 30) {
    return fofiIdCFFCID;
  } else {
    return fofiIdCFF8Bit;
  }
}
