//========================================================================
//
// Decrypt.h
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
// Copyright (C) 2008 Julien Rebetez <julien@fhtagn.net>
// Copyright (C) 2009 David Benjamin <davidben@mit.edu>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef DECRYPT_H
#define DECRYPT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "goo/GooString.h"
#include "Object.h"
#include "Stream.h"

//------------------------------------------------------------------------
// Decrypt
//------------------------------------------------------------------------

class Decrypt {
public:
  static void md5(Guchar *msg, int msgLen, Guchar *digest);

  // Generate a file key.  The <fileKey> buffer must have space for at
  // least 16 bytes.  Checks <ownerPassword> and then <userPassword>
  // and returns true if either is correct.  Sets <ownerPasswordOk> if
  // the owner password was correct.  Either or both of the passwords
  // may be NULL, which is treated as an empty string.
  static GBool makeFileKey(int encVersion, int encRevision, int keyLength,
			   GooString *ownerKey, GooString *userKey,
			   int permissions, GooString *fileID,
			   GooString *ownerPassword, GooString *userPassword,
			   Guchar *fileKey, GBool encryptMetadata,
			   GBool *ownerPasswordOk);

private:

  static GBool makeFileKey2(int encVersion, int encRevision, int keyLength,
			    GooString *ownerKey, GooString *userKey,
			    int permissions, GooString *fileID,
			    GooString *userPassword, Guchar *fileKey,
			    GBool encryptMetadata);
};

//------------------------------------------------------------------------
// DecryptStream
//------------------------------------------------------------------------

struct DecryptRC4State {
  Guchar state[256];
  Guchar x, y;
  int buf;
};

struct DecryptAESState {
  Guint w[44];
  Guchar state[16];
  Guchar cbc[16];
  Guchar buf[16];
  int bufIdx;
};

class DecryptStream: public FilterStream {
public:

  DecryptStream(Stream *strA, Guchar *fileKey,
		CryptAlgorithm algoA, int keyLength,
		int objNum, int objGen);
  virtual ~DecryptStream();
  virtual StreamKind getKind() { return strWeird; }
  virtual void reset();
  virtual int getChar();
  virtual int lookChar();
  virtual int getPos();
  virtual GBool isBinary(GBool last);
  virtual Stream *getUndecodedStream() { return this; }

private:

  CryptAlgorithm algo;
  int objKeyLength;
  Guchar objKey[16 + 9];
  int charactersRead; // so that getPos() can be correct

  union {
    DecryptRC4State rc4;
    DecryptAESState aes;
  } state;
};

#endif
