//========================================================================
//
// JBIG2Stream.h
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
// Copyright (C) 2009 David Benjamin <davidben@mit.edu>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef JBIG2STREAM_H
#define JBIG2STREAM_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "Object.h"
#include "Stream.h"

class GooList;
class JBIG2Segment;
class JBIG2Bitmap;
class JArithmeticDecoder;
class JArithmeticDecoderStats;
class JBIG2HuffmanDecoder;
struct JBIG2HuffmanTable;
class JBIG2MMRDecoder;

//------------------------------------------------------------------------

class JBIG2Stream: public FilterStream {
public:

  JBIG2Stream(Stream *strA, Object *globalsStreamA);
  virtual ~JBIG2Stream();
  virtual StreamKind getKind() { return strJBIG2; }
  virtual void reset();
  virtual void close();
  virtual int getPos();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, char *indent);
  virtual GBool isBinary(GBool last = gTrue);

private:

  void readSegments();
  GBool readSymbolDictSeg(Guint segNum, Guint length,
			  Guint *refSegs, Guint nRefSegs);
  void readTextRegionSeg(Guint segNum, GBool imm,
			 GBool lossless, Guint length,
			 Guint *refSegs, Guint nRefSegs);
  JBIG2Bitmap *readTextRegion(GBool huff, GBool refine,
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
			      int *atx, int *aty);
  void readPatternDictSeg(Guint segNum, Guint length);
  void readHalftoneRegionSeg(Guint segNum, GBool imm,
			     GBool lossless, Guint length,
			     Guint *refSegs, Guint nRefSegs);
  void readGenericRegionSeg(Guint segNum, GBool imm,
			    GBool lossless, Guint length);
  void mmrAddPixels(int a1, int blackPixels,
		    int *codingLine, int *a0i, int w);
  void mmrAddPixelsNeg(int a1, int blackPixels,
		       int *codingLine, int *a0i, int w);
  JBIG2Bitmap *readGenericBitmap(GBool mmr, int w, int h,
				 int templ, GBool tpgdOn,
				 GBool useSkip, JBIG2Bitmap *skip,
				 int *atx, int *aty,
				 int mmrDataLength);
  void readGenericRefinementRegionSeg(Guint segNum, GBool imm,
				      GBool lossless, Guint length,
				      Guint *refSegs,
				      Guint nRefSegs);
  JBIG2Bitmap *readGenericRefinementRegion(int w, int h,
					   int templ, GBool tpgrOn,
					   JBIG2Bitmap *refBitmap,
					   int refDX, int refDY,
					   int *atx, int *aty);
  void readPageInfoSeg(Guint length);
  void readEndOfStripeSeg(Guint length);
  void readProfilesSeg(Guint length);
  void readCodeTableSeg(Guint segNum, Guint length);
  void readExtensionSeg(Guint length);
  JBIG2Segment *findSegment(Guint segNum);
  void discardSegment(Guint segNum);
  void resetGenericStats(Guint templ,
			 JArithmeticDecoderStats *prevStats);
  void resetRefinementStats(Guint templ,
			    JArithmeticDecoderStats *prevStats);
  void resetIntStats(int symCodeLen);
  GBool readUByte(Guint *x);
  GBool readByte(int *x);
  GBool readUWord(Guint *x);
  GBool readULong(Guint *x);
  GBool readLong(int *x);

  Object globalsStream;
  Guint pageW, pageH, curPageH;
  Guint pageDefPixel;
  JBIG2Bitmap *pageBitmap;
  Guint defCombOp;
  GooList *segments;		// [JBIG2Segment]
  GooList *globalSegments;	// [JBIG2Segment]
  Stream *curStr;
  Guchar *dataPtr;
  Guchar *dataEnd;

  JArithmeticDecoder *arithDecoder;
  JArithmeticDecoderStats *genericRegionStats;
  JArithmeticDecoderStats *refinementRegionStats;
  JArithmeticDecoderStats *iadhStats;
  JArithmeticDecoderStats *iadwStats;
  JArithmeticDecoderStats *iaexStats;
  JArithmeticDecoderStats *iaaiStats;
  JArithmeticDecoderStats *iadtStats;
  JArithmeticDecoderStats *iaitStats;
  JArithmeticDecoderStats *iafsStats;
  JArithmeticDecoderStats *iadsStats;
  JArithmeticDecoderStats *iardxStats;
  JArithmeticDecoderStats *iardyStats;
  JArithmeticDecoderStats *iardwStats;
  JArithmeticDecoderStats *iardhStats;
  JArithmeticDecoderStats *iariStats;
  JArithmeticDecoderStats *iaidStats;
  JBIG2HuffmanDecoder *huffDecoder;
  JBIG2MMRDecoder *mmrDecoder;
};

#endif
