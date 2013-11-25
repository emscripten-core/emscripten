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
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
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

  // Generate a file key.  The <fileKey> buffer must have space for at
  // least 16 bytes.  Checks <ownerPassword> and then <userPassword>
  // and returns true if either is correct.  Sets <ownerPasswordOk> if
  // the owner password was correct.  Either or both of the passwords
  // may be NULL, which is treated as an empty string.
  static GBool makeFileKey(int encVersion, int encRevision, int keyLength,
			   GooString *ownerKey, GooString *userKey,
			   GooString *ownerEnc, GooString *userEnc,
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
// Helper classes
//------------------------------------------------------------------------

/* DecryptRC4State, DecryptAESState, DecryptAES256State are named like this for
 * historical reasons, but they're used for encryption too.
 * In case of decryption, the cbc field in AES and AES-256 contains the previous
 * input block or the CBC initialization vector (IV) if the stream has just been
 * reset). In case of encryption, it always contains the IV, whereas the
 * previous output is kept in buf. The paddingReached field is only used in
 * case of encryption. */
struct DecryptRC4State {
  Guchar state[256];
  Guchar x, y;
};

struct DecryptAESState {
  Guint w[44];
  Guchar state[16];
  Guchar cbc[16];
  Guchar buf[16];
  GBool paddingReached; // encryption only
  int bufIdx;
};

struct DecryptAES256State {
  Guint w[60];
  Guchar state[16];
  Guchar cbc[16];
  Guchar buf[16];
  GBool paddingReached; // encryption only
  int bufIdx;
};

class BaseCryptStream : public FilterStream {
public:

  BaseCryptStream(Stream *strA, Guchar *fileKey, CryptAlgorithm algoA,
                  int keyLength, int objNum, int objGen);
  virtual ~BaseCryptStream();
  virtual StreamKind getKind() { return strCrypt; }
  virtual void reset();
  virtual int getChar();
  virtual int lookChar() = 0;
  virtual Goffset getPos();
  virtual GBool isBinary(GBool last);
  virtual Stream *getUndecodedStream() { return this; }
  void setAutoDelete(GBool val);

protected:
  CryptAlgorithm algo;
  int objKeyLength;
  Guchar objKey[32];
  Goffset charactersRead; // so that getPos() can be correct
  int nextCharBuff;   // EOF means not read yet
  GBool autoDelete;

  union {
    DecryptRC4State rc4;
    DecryptAESState aes;
    DecryptAES256State aes256;
  } state;
};

//------------------------------------------------------------------------
// EncryptStream / DecryptStream
//------------------------------------------------------------------------

class EncryptStream : public BaseCryptStream {
public:

  EncryptStream(Stream *strA, Guchar *fileKey, CryptAlgorithm algoA,
                int keyLength, int objNum, int objGen);
  ~EncryptStream();
  virtual void reset();
  virtual int lookChar();
};

class DecryptStream : public BaseCryptStream {
public:

  DecryptStream(Stream *strA, Guchar *fileKey, CryptAlgorithm algoA,
                int keyLength, int objNum, int objGen);
  ~DecryptStream();
  virtual void reset();
  virtual int lookChar();
};
 
//------------------------------------------------------------------------

extern void md5(Guchar *msg, int msgLen, Guchar *digest);

#endif
