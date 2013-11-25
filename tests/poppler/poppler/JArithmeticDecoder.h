//========================================================================
//
// JArithmeticDecoder.h
//
// Arithmetic decoder used by the JBIG2 and JPEG2000 decoders.
//
// Copyright 2002-2004 Glyph & Cog, LLC
//
//========================================================================

#ifndef JARITHMETICDECODER_H
#define JARITHMETICDECODER_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"

class Stream;

//------------------------------------------------------------------------
// JArithmeticDecoderStats
//------------------------------------------------------------------------

class JArithmeticDecoderStats {
public:

  JArithmeticDecoderStats(int contextSizeA);
  ~JArithmeticDecoderStats();
  JArithmeticDecoderStats *copy();
  void reset();
  int getContextSize() { return contextSize; }
  void copyFrom(JArithmeticDecoderStats *stats);
  void setEntry(Guint cx, int i, int mps);

private:

  Guchar *cxTab;		// cxTab[cx] = (i[cx] << 1) + mps[cx]
  int contextSize;

  friend class JArithmeticDecoder;
};

//------------------------------------------------------------------------
// JArithmeticDecoder
//------------------------------------------------------------------------

class JArithmeticDecoder {
public:

  JArithmeticDecoder();
  ~JArithmeticDecoder();

  void setStream(Stream *strA)
    { str = strA; dataLen = 0; limitStream = gFalse; }
  void setStream(Stream *strA, int dataLenA)
    { str = strA; dataLen = dataLenA; limitStream = gTrue; }

  // Start decoding on a new stream.  This fills the byte buffers and
  // runs INITDEC.
  void start();

  // Restart decoding on an interrupted stream.  This refills the
  // buffers if needed, but does not run INITDEC.  (This is used in
  // JPEG 2000 streams when codeblock data is split across multiple
  // packets/layers.)
  void restart(int dataLenA);

  // Read any leftover data in the stream.
  void cleanup();

  // Decode one bit.
  int decodeBit(Guint context, JArithmeticDecoderStats *stats);

  // Decode eight bits.
  int decodeByte(Guint context, JArithmeticDecoderStats *stats);

  // Returns false for OOB, otherwise sets *<x> and returns true.
  GBool decodeInt(int *x, JArithmeticDecoderStats *stats);

  Guint decodeIAID(Guint codeLen,
		   JArithmeticDecoderStats *stats);

  void resetByteCounter() { nBytesRead = 0; }
  Guint getByteCounter() { return nBytesRead; }

private:

  Guint readByte();
  int decodeIntBit(JArithmeticDecoderStats *stats);
  void byteIn();

  static Guint qeTab[47];
  static int nmpsTab[47];
  static int nlpsTab[47];
  static int switchTab[47];

  Guint buf0, buf1;
  Guint c, a;
  int ct;

  Guint prev;			// for the integer decoder

  Stream *str;
  Guint nBytesRead;
  int dataLen;
  GBool limitStream;
};

#endif
