//========================================================================
//
// Hints.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
// Copyright 2010 Pino Toscano <pino@kde.org>
//
//========================================================================

#include <config.h>

#include "Hints.h"

#include "Linearization.h"
#include "Object.h"
#include "Stream.h"
#include "XRef.h"
#include "Parser.h"
#include "Lexer.h"
#include "SecurityHandler.h"

#include <limits.h>

//------------------------------------------------------------------------
// Hints
//------------------------------------------------------------------------

Hints::Hints(BaseStream *str, Linearization *linearization, XRef *xref, SecurityHandler *secHdlr)
{
  mainXRefEntriesOffset = linearization->getMainXRefEntriesOffset();
  nPages = linearization->getNumPages();
  pageFirst = linearization->getPageFirst();
  pageEndFirst = linearization->getEndFirst();
  pageObjectFirst = linearization->getObjectNumberFirst();
  if (pageObjectFirst < 0 || pageObjectFirst >= xref->getNumObjects()) {
    error(-1,
      "Invalid reference for first page object (%d) in linearization table ",
      pageObjectFirst);
    pageObjectFirst = 0;
  }
  pageOffsetFirst = xref->getEntry(pageObjectFirst)->offset;

  if (nPages >= INT_MAX / (int)sizeof(Guint)) {
     error(-1, "Invalid number of pages (%d) for hints table", nPages);
     nPages = 0;
  }
  nObjects = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  pageObjectNum = (int *) gmallocn_checkoverflow(nPages, sizeof(int));
  xRefOffset = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  pageLength = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  pageOffset = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  numSharedObject = (Guint *) gmallocn_checkoverflow(nPages, sizeof(Guint));
  sharedObjectId = (Guint **) gmallocn_checkoverflow(nPages, sizeof(Guint*));
  if (!nObjects || !pageObjectNum || !xRefOffset || !pageLength || !pageOffset ||
      !numSharedObject || !sharedObjectId) {
    error(-1, "Failed to allocate memory for hints tabel");
    nPages = 0;
  }

  memset(numSharedObject, 0, nPages * sizeof(Guint));

  nSharedGroups = 0;
  groupLength = NULL;
  groupOffset = NULL;
  groupHasSignature = NULL;
  groupNumObjects = NULL;
  groupXRefOffset = NULL;

  readTables(str, linearization, xref, secHdlr);
}

Hints::~Hints()
{
  gfree(nObjects);
  gfree(pageObjectNum);
  gfree(xRefOffset);
  gfree(pageLength);
  gfree(pageOffset);
  for (int i=0; i< nPages; i++) {
    if (numSharedObject[i]) {
       gfree(sharedObjectId[i]);
    }
  }
  gfree(sharedObjectId);
  gfree(numSharedObject);

  gfree(groupLength);
  gfree(groupOffset);
  gfree(groupHasSignature);
  gfree(groupNumObjects);
  gfree(groupXRefOffset);
}

void Hints::readTables(BaseStream *str, Linearization *linearization, XRef *xref, SecurityHandler *secHdlr)
{
  hintsOffset = linearization->getHintsOffset();
  hintsLength = linearization->getHintsLength();
  hintsOffset2 = linearization->getHintsOffset2();
  hintsLength2 = linearization->getHintsLength2();

  Parser *parser;
  Object obj;

  int bufLength = hintsLength + hintsLength2;

  std::vector<char> buf(bufLength);
  char *p = &buf[0];

  obj.initNull();
  Stream *s = str->makeSubStream(hintsOffset, gFalse, hintsLength, &obj);
  s->reset();
  for (Guint i=0; i < hintsLength; i++) { *p++ = s->getChar(); }
  delete s;

  if (hintsOffset2 && hintsLength2) {
    obj.initNull();
    s = str->makeSubStream(hintsOffset2, gFalse, hintsLength2, &obj);
    s->reset();
    for (Guint i=0; i < hintsLength2; i++) { *p++ = s->getChar(); }
    delete s;
  }

  obj.initNull();
  MemStream *memStream = new MemStream (&buf[0], 0, bufLength, &obj);

  obj.initNull();
  parser = new Parser(xref, new Lexer(xref, memStream), gTrue);

  int num, gen;
  if (parser->getObj(&obj)->isInt() &&
     (num = obj.getInt(), obj.free(), parser->getObj(&obj)->isInt()) &&
     (gen = obj.getInt(), obj.free(), parser->getObj(&obj)->isCmd("obj")) &&
     (obj.free(), parser->getObj(&obj,
         secHdlr ? secHdlr->getFileKey() : (Guchar *)NULL,
         secHdlr ? secHdlr->getEncAlgorithm() : cryptRC4,
         secHdlr ? secHdlr->getFileKeyLength() : 0,
         num, gen)->isStream())) {
    Stream *hintsStream = obj.getStream();
    Dict *hintsDict = obj.streamGetDict();

    int sharedStreamOffset = 0;
    if (hintsDict->lookupInt("S", NULL, &sharedStreamOffset) &&
        sharedStreamOffset > 0) {

        hintsStream->reset();
        readPageOffsetTable(hintsStream);

        hintsStream->reset();
        for (int i=0; i<sharedStreamOffset; i++) hintsStream->getChar();
        readSharedObjectsTable(hintsStream);
    } else {
      error(-1, "Invalid shared object hint table offset");
    }
  } else {
    error(-1, "Failed parsing hints table object");
  }
  obj.free();

  delete parser;
}

void Hints::readPageOffsetTable(Stream *str)
{
  if (nPages < 1) {
    error(-1, "Invalid number of pages reading page offset hints table");
    return;
  }

  inputBits = 0; // reset on byte boundary.

  nObjectLeast = readBits(32, str);

  objectOffsetFirst = readBits(32, str);
  if (objectOffsetFirst >= hintsOffset) objectOffsetFirst += hintsLength;

  nBitsDiffObjects = readBits(16, str);

  pageLengthLeast = readBits(32, str);

  nBitsDiffPageLength = readBits(16, str);

  OffsetStreamLeast = readBits(32, str);

  nBitsOffsetStream = readBits(16, str);

  lengthStreamLeast = readBits(32, str);

  nBitsLengthStream = readBits(16, str);

  nBitsNumShared = readBits(16, str);

  nBitsShared = readBits(16, str);

  nBitsNumerator = readBits(16, str);

  denominator = readBits(16, str);

  for (int i=0; i<nPages; i++) {
    nObjects[i] = nObjectLeast + readBits(nBitsDiffObjects, str);
  }

  nObjects[0] = 0;
  xRefOffset[0] = mainXRefEntriesOffset + 20;
  for (int i=1; i<nPages; i++) {
    xRefOffset[i] = xRefOffset[i-1] + 20*nObjects[i-1];
  }

  pageObjectNum[0] = 1;
  for (int i=1; i<nPages; i++) {
    pageObjectNum[i] = pageObjectNum[i-1] + nObjects[i-1];
  }
  pageObjectNum[0] = pageObjectFirst;

  inputBits = 0; // reset on byte boundary. Not in specs!
  for (int i=0; i<nPages; i++) {
    pageLength[i] = pageLengthLeast + readBits(nBitsDiffPageLength, str);
  }

  inputBits = 0; // reset on byte boundary. Not in specs!
  numSharedObject[0] = readBits(nBitsNumShared, str);
  numSharedObject[0] = 0; // Do not trust the read value to be 0.
  sharedObjectId[0] = NULL;
  for (int i=1; i<nPages; i++) {
    numSharedObject[i] = readBits(nBitsNumShared, str);
    if (numSharedObject[i] >= INT_MAX / (int)sizeof(Guint)) {
       error(-1, "Invalid number of shared objects");
       numSharedObject[i] = 0;
       return;
    }
    sharedObjectId[i] = (Guint *) gmallocn_checkoverflow(numSharedObject[i], sizeof(Guint));
    if (numSharedObject[i] && !sharedObjectId[i]) {
       error(-1, "Failed to allocate memory for shared object IDs");
       numSharedObject[i] = 0;
       return;
    }
  }

  inputBits = 0; // reset on byte boundary. Not in specs!
  for (int i=1; i<nPages; i++) {
    for (Guint j=0; j < numSharedObject[i]; j++) {
      sharedObjectId[i][j] = readBits(nBitsShared, str);
    }
  }

  pageOffset[0] = pageOffsetFirst;
  // find pageOffsets.
  for (int i=1; i<nPages; i++) {
    pageOffset[i] = pageOffset[i-1] + pageLength[i-1];
  }

}

void Hints::readSharedObjectsTable(Stream *str)
{
  inputBits = 0; // reset on byte boundary.

  Guint firstSharedObjectNumber = readBits(32, str);

  Guint firstSharedObjectOffset = readBits(32, str);
  firstSharedObjectOffset += hintsLength;

  Guint nSharedGroupsFirst = readBits(32, str);

  Guint nSharedGroups = readBits(32, str);

  Guint nBitsNumObjects = readBits(16, str);

  Guint groupLengthLeast = readBits(32, str);

  Guint nBitsDiffGroupLength = readBits(16, str);

  if ((!nSharedGroups) || (nSharedGroups >= INT_MAX / (int)sizeof(Guint))) {
     error(-1, "Invalid number of shared object groups");
     nSharedGroups = 0;
     return;
  }
  if ((!nSharedGroupsFirst) || (nSharedGroupsFirst > nSharedGroups)) {
     error(-1, "Invalid number of first page shared object groups");
     nSharedGroupsFirst = nSharedGroups;
  }

  groupLength = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupOffset = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupHasSignature = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupNumObjects = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  groupXRefOffset = (Guint *) gmallocn_checkoverflow(nSharedGroups, sizeof(Guint));
  if (!groupLength || !groupOffset || !groupHasSignature ||
      !groupNumObjects || !groupXRefOffset) {
     error(-1, "Failed to allocate memory for shared object groups");
     nSharedGroups = 0;
     return;
  }

  inputBits = 0; // reset on byte boundary. Not in specs!
  for (Guint i=0; i<nSharedGroups; i++) {
    groupLength[i] = groupLengthLeast + readBits(nBitsDiffGroupLength, str);
  }

  groupOffset[0] = objectOffsetFirst;
  for (Guint i=1; i<nSharedGroupsFirst; i++) {
    groupOffset[i] = groupOffset[i-1] + groupLength[i-1];
  }
  if (nSharedGroups > nSharedGroupsFirst ) {
    groupOffset[nSharedGroupsFirst] = firstSharedObjectOffset;
    for (Guint i=nSharedGroupsFirst+1; i<nSharedGroups; i++) {
      groupOffset[i] = groupOffset[i-1] + groupLength[i-1];
    }
  }

  inputBits = 0; // reset on byte boundary. Not in specs!
  for (Guint i=0; i<nSharedGroups; i++) {
    groupHasSignature[i] = readBits(1, str);
  }

  inputBits = 0; // reset on byte boundary. Not in specs!
  for (Guint i=0; i<nSharedGroups; i++) {
    if (groupHasSignature[i]) {
       readBits(128, str);
    }
  }

  inputBits = 0; // reset on byte boundary. Not in specs!
  for (Guint i=0; i<nSharedGroups; i++) {
    groupNumObjects[i] =
       nBitsNumObjects ? 1 + readBits(nBitsNumObjects, str) : 1;
  }

  for (Guint i=0; i<nSharedGroupsFirst; i++) {
    groupNumObjects[i] = 0;
    groupXRefOffset[i] = 0;
  }
  if (nSharedGroups > nSharedGroupsFirst ) {
    groupXRefOffset[nSharedGroupsFirst] =
        mainXRefEntriesOffset + 20*firstSharedObjectNumber;
    for (Guint i=nSharedGroupsFirst+1; i<nSharedGroups; i++) {
      groupXRefOffset[i] = groupXRefOffset[i-1] + 20*groupNumObjects[i-1];
    }
  }
}

Guint Hints::getPageOffset(int page)
{
  if ((page < 1) || (page > nPages)) return 0;

  if (page-1 > pageFirst)
    return pageOffset[page-1];
  else if (page-1 < pageFirst)
    return pageOffset[page];
  else
    return pageOffset[0];
}

std::vector<ByteRange>* Hints::getPageRanges(int page)
{
  if ((page < 1) || (page > nPages)) return NULL;

  int idx;
  if (page-1 > pageFirst)
     idx = page-1;
  else if (page-1 < pageFirst)
     idx = page;
  else
     idx = 0;

  ByteRange pageRange;
  std::vector<ByteRange> *v = new std::vector<ByteRange>;

  pageRange.offset = pageOffset[idx];
  pageRange.length = pageLength[idx];
  v->push_back(pageRange);

  pageRange.offset = xRefOffset[idx];
  pageRange.length = 20*nObjects[idx];
  v->push_back(pageRange);

  for (Guint j=0; j<numSharedObject[idx]; j++) {
     Guint k = sharedObjectId[idx][j];

     pageRange.offset = groupOffset[k];
     pageRange.length = groupLength[k];
     v->push_back(pageRange);

     pageRange.offset = groupXRefOffset[k];
     pageRange.length = 20*groupNumObjects[k];
     v->push_back(pageRange);
  }

  return v;
}

Guint Hints::readBit(Stream *str)
{
  Guint bit;
  int c;

  if (inputBits == 0) {
    if ((c = str->getChar()) == EOF) {
      return (Guint) -1;
    }
    bitsBuffer = c;
    inputBits = 8;
  }
  bit = (bitsBuffer >> (inputBits - 1)) & 1;
  --inputBits;
  return bit;
}

Guint Hints::readBits(int n, Stream *str)
{
  Guint bit, bits;

  if (n < 0) return -1;
  if (n == 0) return 0;

  if (n == 1)
    return readBit(str);

  bit = (readBit(str) << (n-1));
  if (bit == (Guint) -1)
    return -1;

  bits = readBits(n-1, str);
  if (bits == (Guint) -1)
    return -1;

  return bit | bits;
}

int Hints::getPageObjectNum(int page) {
  if ((page < 1) || (page > nPages)) return 0;

  if (page-1 > pageFirst)
    return pageObjectNum[page-1];
  else if (page-1 < pageFirst)
    return pageObjectNum[page];
  else
    return pageObjectNum[0];
}
