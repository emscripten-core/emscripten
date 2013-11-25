//========================================================================
//
// XRef.h
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
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2006, 2008, 2010-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007-2008 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Ilya Gorenbein <igorenbein@finjan.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@kabelmail.de>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef XREF_H
#define XREF_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "poppler-config.h"
#include "goo/gtypes.h"
#include "goo/GooMutex.h"
#include "Object.h"
#include "Stream.h"

#include <vector>

class Dict;
class Stream;
class Parser;
class PopplerCache;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

enum XRefEntryType {
  xrefEntryFree,
  xrefEntryUncompressed,
  xrefEntryCompressed,
  xrefEntryNone
};

struct XRefEntry {
  Goffset offset;
  int gen;
  XRefEntryType type;
  int flags;
  Object obj; //if this entry was updated, obj will contains the updated object

  enum Flag {
    // Regular flags
    Updated,     // Entry was modified

    // Special flags -- available only after xref->scanSpecialFlags() is run
    Unencrypted, // Entry is stored in unencrypted form (meaningless in unencrypted documents)
    DontRewrite  // Entry must not be written back in case of full rewrite
  };

  inline GBool getFlag(Flag flag) {
    const int mask = (1 << (int)flag);
    return (flags & mask) != 0;
  }

  inline void setFlag(Flag flag, GBool value) {
    const int mask = (1 << (int)flag);
    if (value) {
      flags |= mask;
    } else {
      flags &= ~mask;
    }
  }
};

class XRef {
public:

  // Constructor, create an empty XRef, used for PDF writing
  XRef();
  // Constructor, create an empty XRef but with info dict, used for PDF writing
  XRef(Object *trailerDictA);
  // Constructor.  Read xref table from stream.
  XRef(BaseStream *strA, Goffset pos, Goffset mainXRefEntriesOffsetA = 0, GBool *wasReconstructed = NULL, GBool reconstruct = false);

  // Destructor.
  ~XRef();

  // Copy xref but with new base stream!
  XRef *copy();

  // Is xref table valid?
  GBool isOk() { return ok; }

  // Is the last XRef section a stream or a table?
  GBool isXRefStream() { return xRefStream; }

  // Get the error code (if isOk() returns false).
  int getErrorCode() { return errCode; }

  // Set the encryption parameters.
  void setEncryption(int permFlagsA, GBool ownerPasswordOkA,
		     Guchar *fileKeyA, int keyLengthA,
		     int encVersionA, int encRevisionA,
		     CryptAlgorithm encAlgorithmA);
  // Mark Encrypt entry as Unencrypted
  void markUnencrypted();

  void getEncryptionParameters(Guchar **fileKeyA, CryptAlgorithm *encAlgorithmA, int *keyLengthA);

  // Is the file encrypted?
  GBool isEncrypted() { return encrypted; }

  // Check various permissions.
  GBool okToPrint(GBool ignoreOwnerPW = gFalse);
  GBool okToPrintHighRes(GBool ignoreOwnerPW = gFalse);
  GBool okToChange(GBool ignoreOwnerPW = gFalse);
  GBool okToCopy(GBool ignoreOwnerPW = gFalse);
  GBool okToAddNotes(GBool ignoreOwnerPW = gFalse);
  GBool okToFillForm(GBool ignoreOwnerPW = gFalse);
  GBool okToAccessibility(GBool ignoreOwnerPW = gFalse);
  GBool okToAssemble(GBool ignoreOwnerPW = gFalse);
  int getPermFlags() { return permFlags; }

  // Get catalog object.
  Object *getCatalog(Object *obj);

  // Fetch an indirect reference.
  Object *fetch(int num, int gen, Object *obj, int recursion = 0);

  // Return the document's Info dictionary (if any).
  Object *getDocInfo(Object *obj);
  Object *getDocInfoNF(Object *obj);

  // Return the number of objects in the xref table.
  int getNumObjects() { return size; }

  // Return the catalog object reference.
  int getRootNum() { return rootNum; }
  int getRootGen() { return rootGen; }

  // Get end position for a stream in a damaged file.
  // Returns false if unknown or file is not damaged.
  GBool getStreamEnd(Goffset streamStart, Goffset *streamEnd);

  // Retuns the entry that belongs to the offset
  int getNumEntry(Goffset offset);

  // Scans the document and sets special flags in all xref entries. One of those
  // flags is Unencrypted, which affects how the object is fetched. Therefore,
  // this function must be called before fetching unencrypted objects (e.g.
  // Encrypt dictionary, XRef streams). Note that the code that initializes
  // decryption doesn't need to call this function, because it runs before
  // decryption is enabled, and therefore the Unencrypted flag is ignored.
  void scanSpecialFlags();

  // Direct access.
  XRefEntry *getEntry(int i, GBool complainIfMissing = gTrue);
  Object *getTrailerDict() { return &trailerDict; }

  // Write access
  void setModifiedObject(Object* o, Ref r);
  Ref addIndirectObject (Object* o);
  void removeIndirectObject(Ref r);
  void add(int num, int gen,  Goffset offs, GBool used);

  // Output XRef table to stream
  void writeTableToFile(OutStream* outStr, GBool writeAllEntries);
  // Output XRef stream contents to GooString and fill trailerDict fields accordingly
  void writeStreamToBuffer(GooString *stmBuf, Dict *xrefDict, XRef *xref);

  // to be thread safe during write where changes are not allowed
  void lock();
  void unlock();

private:

  BaseStream *str;		// input stream
  Goffset start;		// offset in file (to allow for garbage
				//   at beginning of file)
  XRefEntry *entries;		// xref entries
  int capacity;			// size of <entries> array
  int size;			// number of entries
  int rootNum, rootGen;		// catalog dict
  GBool ok;			// true if xref table is valid
  int errCode;			// error code (if <ok> is false)
  Object trailerDict;		// trailer dictionary
  Goffset *streamEnds;		// 'endstream' positions - only used in
				//   damaged files
  int streamEndsLen;		// number of valid entries in streamEnds
  PopplerCache *objStrs;	// cached object streams
  GBool encrypted;		// true if file is encrypted
  int encRevision;		
  int encVersion;		// encryption algorithm
  CryptAlgorithm encAlgorithm;	// encryption algorithm
  int keyLength;		// length of key, in bytes
  int permFlags;		// permission bits
  Guchar fileKey[32];		// file decryption key
  GBool ownerPasswordOk;	// true if owner password is correct
  Goffset prevXRefOffset;		// position of prev XRef section (= next to read)
  Goffset mainXRefEntriesOffset; // offset of entries in main XRef table
  GBool xRefStream;		// true if last XRef section is a stream
  Goffset mainXRefOffset;	// position of the main XRef table/stream
  GBool scannedSpecialFlags;	// true if scanSpecialFlags has been called
  GBool strOwner;     // true if str is owned by the instance
#if MULTITHREADED
  GooMutex mutex;
#endif

  void init();
  int reserve(int newSize);
  int resize(int newSize);
  GBool readXRef(Goffset *pos, std::vector<Goffset> *followedXRefStm, std::vector<int> *xrefStreamObjsNum);
  GBool readXRefTable(Parser *parser, Goffset *pos, std::vector<Goffset> *followedXRefStm, std::vector<int> *xrefStreamObjsNum);
  GBool readXRefStreamSection(Stream *xrefStr, int *w, int first, int n);
  GBool readXRefStream(Stream *xrefStr, Goffset *pos);
  GBool constructXRef(GBool *wasReconstructed, GBool needCatalogDict = gFalse);
  GBool parseEntry(Goffset offset, XRefEntry *entry);
  void readXRefUntil(int untilEntryNum, std::vector<int> *xrefStreamObjsNum = NULL);
  void markUnencrypted(Object *obj);

  class XRefWriter {
  public:
    virtual void startSection(int first, int count) = 0;
    virtual void writeEntry(Goffset offset, int gen, XRefEntryType type) = 0;
    virtual ~XRefWriter() {};
  };

  // XRefWriter subclass that writes a XRef table
  class XRefTableWriter: public XRefWriter {
  public:
    XRefTableWriter(OutStream* outStrA);
    void startSection(int first, int count);
    void writeEntry(Goffset offset, int gen, XRefEntryType type);
  private:
    OutStream* outStr;
  };

  // XRefWriter subclass that writes a XRef stream
  class XRefStreamWriter: public XRefWriter {
  public:
    XRefStreamWriter(Object *index, GooString *stmBuf, int offsetSize);
    void startSection(int first, int count);
    void writeEntry(Goffset offset, int gen, XRefEntryType type);
  private:
    Object *index;
    GooString *stmBuf;
    int offsetSize;
  };

  // Dummy XRefWriter subclass that only checks if all offsets fit in 4 bytes
  class XRefPreScanWriter: public XRefWriter {
  public:
    XRefPreScanWriter();
    void startSection(int first, int count);
    void writeEntry(Goffset offset, int gen, XRefEntryType type);

    GBool hasOffsetsBeyond4GB;
  };

  void writeXRef(XRefWriter *writer, GBool writeAllEntries);
};

#endif
