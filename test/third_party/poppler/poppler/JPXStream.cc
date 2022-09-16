//========================================================================
//
// JPXStream.cc
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
// Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>
#include <limits.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "goo/gmem.h"
#include "Error.h"
#include "JArithmeticDecoder.h"
#include "JPXStream.h"

//~ to do:
//  - precincts
//  - ROI
//  - progression order changes
//  - packed packet headers
//  - support for palettes, channel maps, etc.
//  - make sure all needed JP2/JPX subboxes are parsed (readBoxes)
//  - can we assume that QCC segments must come after the QCD segment?
//  - skip EPH markers (readTilePartData)
//  - handle tilePartToEOC in readTilePartData
//  - deal with multiple codeword segments (readTilePartData,
//    readCodeBlockData)
//  - progression orders 2, 3, and 4
//  - in coefficient decoding (readCodeBlockData):
//    - termination pattern: terminate after every coding pass
//    - error resilience segmentation symbol
//    - selective arithmetic coding bypass
//    - vertically causal context formation
//    - coeffs longer than 31 bits (should just ignore the extra bits?)
//  - handle boxes larger than 2^32 bytes
//  - the fixed-point arithmetic won't handle 16-bit pixels

//------------------------------------------------------------------------

// number of contexts for the arithmetic decoder
#define jpxNContexts        19

#define jpxContextSigProp    0	// 0 - 8: significance prop and cleanup
#define jpxContextSign       9	// 9 - 13: sign
#define jpxContextMagRef    14	// 14 -16: magnitude refinement
#define jpxContextRunLength 17	// cleanup: run length
#define jpxContextUniform   18	// cleanup: first signif coeff

//------------------------------------------------------------------------

#define jpxPassSigProp       0
#define jpxPassMagRef        1
#define jpxPassCleanup       2

//------------------------------------------------------------------------

// arithmetic decoder context for the significance propagation and
// cleanup passes:
//     [horiz][vert][diag][subband]
// where subband = 0 for HL
//               = 1 for LH and LL
//               = 2 for HH
static const Guint sigPropContext[3][3][5][3] = {
  {{{ 0, 0, 0 },   // horiz=0, vert=0, diag=0
    { 1, 1, 3 },   // horiz=0, vert=0, diag=1
    { 2, 2, 6 },   // horiz=0, vert=0, diag=2
    { 2, 2, 8 },   // horiz=0, vert=0, diag=3
    { 2, 2, 8 }},  // horiz=0, vert=0, diag=4
   {{ 5, 3, 1 },   // horiz=0, vert=1, diag=0
    { 6, 3, 4 },   // horiz=0, vert=1, diag=1
    { 6, 3, 7 },   // horiz=0, vert=1, diag=2
    { 6, 3, 8 },   // horiz=0, vert=1, diag=3
    { 6, 3, 8 }},  // horiz=0, vert=1, diag=4
   {{ 8, 4, 2 },   // horiz=0, vert=2, diag=0
    { 8, 4, 5 },   // horiz=0, vert=2, diag=1
    { 8, 4, 7 },   // horiz=0, vert=2, diag=2
    { 8, 4, 8 },   // horiz=0, vert=2, diag=3
    { 8, 4, 8 }}}, // horiz=0, vert=2, diag=4
  {{{ 3, 5, 1 },   // horiz=1, vert=0, diag=0
    { 3, 6, 4 },   // horiz=1, vert=0, diag=1
    { 3, 6, 7 },   // horiz=1, vert=0, diag=2
    { 3, 6, 8 },   // horiz=1, vert=0, diag=3
    { 3, 6, 8 }},  // horiz=1, vert=0, diag=4
   {{ 7, 7, 2 },   // horiz=1, vert=1, diag=0
    { 7, 7, 5 },   // horiz=1, vert=1, diag=1
    { 7, 7, 7 },   // horiz=1, vert=1, diag=2
    { 7, 7, 8 },   // horiz=1, vert=1, diag=3
    { 7, 7, 8 }},  // horiz=1, vert=1, diag=4
   {{ 8, 7, 2 },   // horiz=1, vert=2, diag=0
    { 8, 7, 5 },   // horiz=1, vert=2, diag=1
    { 8, 7, 7 },   // horiz=1, vert=2, diag=2
    { 8, 7, 8 },   // horiz=1, vert=2, diag=3
    { 8, 7, 8 }}}, // horiz=1, vert=2, diag=4
  {{{ 4, 8, 2 },   // horiz=2, vert=0, diag=0
    { 4, 8, 5 },   // horiz=2, vert=0, diag=1
    { 4, 8, 7 },   // horiz=2, vert=0, diag=2
    { 4, 8, 8 },   // horiz=2, vert=0, diag=3
    { 4, 8, 8 }},  // horiz=2, vert=0, diag=4
   {{ 7, 8, 2 },   // horiz=2, vert=1, diag=0
    { 7, 8, 5 },   // horiz=2, vert=1, diag=1
    { 7, 8, 7 },   // horiz=2, vert=1, diag=2
    { 7, 8, 8 },   // horiz=2, vert=1, diag=3
    { 7, 8, 8 }},  // horiz=2, vert=1, diag=4
   {{ 8, 8, 2 },   // horiz=2, vert=2, diag=0
    { 8, 8, 5 },   // horiz=2, vert=2, diag=1
    { 8, 8, 7 },   // horiz=2, vert=2, diag=2
    { 8, 8, 8 },   // horiz=2, vert=2, diag=3
    { 8, 8, 8 }}}  // horiz=2, vert=2, diag=4
};

// arithmetic decoder context and xor bit for the sign bit in the
// significance propagation pass:
//     [horiz][vert][k]
// where horiz/vert are offset by 2 (i.e., range is -2 .. 2)
// and k = 0 for the context
//       = 1 for the xor bit
static const Guint signContext[5][5][2] = {
  {{ 13, 1 },  // horiz=-2, vert=-2
   { 13, 1 },  // horiz=-2, vert=-1
   { 12, 1 },  // horiz=-2, vert= 0
   { 11, 1 },  // horiz=-2, vert=+1
   { 11, 1 }}, // horiz=-2, vert=+2
  {{ 13, 1 },  // horiz=-1, vert=-2
   { 13, 1 },  // horiz=-1, vert=-1
   { 12, 1 },  // horiz=-1, vert= 0
   { 11, 1 },  // horiz=-1, vert=+1
   { 11, 1 }}, // horiz=-1, vert=+2
  {{ 10, 1 },  // horiz= 0, vert=-2
   { 10, 1 },  // horiz= 0, vert=-1
   {  9, 0 },  // horiz= 0, vert= 0
   { 10, 0 },  // horiz= 0, vert=+1
   { 10, 0 }}, // horiz= 0, vert=+2
  {{ 11, 0 },  // horiz=+1, vert=-2
   { 11, 0 },  // horiz=+1, vert=-1
   { 12, 0 },  // horiz=+1, vert= 0
   { 13, 0 },  // horiz=+1, vert=+1
   { 13, 0 }}, // horiz=+1, vert=+2
  {{ 11, 0 },  // horiz=+2, vert=-2
   { 11, 0 },  // horiz=+2, vert=-1
   { 12, 0 },  // horiz=+2, vert= 0
   { 13, 0 },  // horiz=+2, vert=+1
   { 13, 0 }}, // horiz=+2, vert=+2
};

//------------------------------------------------------------------------

// constants used in the IDWT
#define idwtAlpha  -1.586134342059924
#define idwtBeta   -0.052980118572961
#define idwtGamma   0.882911075530934
#define idwtDelta   0.443506852043971
#define idwtKappa   1.230174104914001
#define idwtIKappa  (1.0 / idwtKappa)

// number of bits to the right of the decimal point for the fixed
// point arithmetic used in the IDWT
#define fracBits 16

//------------------------------------------------------------------------

// floor(x / y)
#define jpxFloorDiv(x, y) ((x) / (y))

// floor(x / 2^y)
#define jpxFloorDivPow2(x, y) ((x) >> (y))

// ceil(x / y)
#define jpxCeilDiv(x, y) (((x) + (y) - 1) / (y))

// ceil(x / 2^y)
#define jpxCeilDivPow2(x, y) (((x) + (1 << (y)) - 1) >> (y))

//------------------------------------------------------------------------

#if 1 //----- disable coverage tracking

#define cover(idx)

#else //----- enable coverage tracking

class JPXCover {
public:

  JPXCover(int sizeA);
  ~JPXCover();
  void incr(int idx);

private:

  int size, used;
  int *data;
};

JPXCover::JPXCover(int sizeA) {
  size = sizeA;
  used = -1;
  data = (int *)gmallocn(size, sizeof(int));
  memset(data, 0, size * sizeof(int));
}

JPXCover::~JPXCover() {
  int i;

  printf("JPX coverage:\n");
  for (i = 0; i <= used; ++i) {
    printf("  %4d: %8d\n", i, data[i]);
  }
  gfree(data);
}

void JPXCover::incr(int idx) {
  if (idx < size) {
    ++data[idx];
    if (idx > used) {
      used = idx;
    }
  }
}

JPXCover jpxCover(150);

#define cover(idx) jpxCover.incr(idx)

#endif //----- coverage tracking

//------------------------------------------------------------------------

JPXStream::JPXStream(Stream *strA):
  FilterStream(strA)
{
  nComps = 0;
  bpc = NULL;
  width = height = 0;
  haveCS = gFalse;
  havePalette = gFalse;
  haveCompMap = gFalse;
  haveChannelDefn = gFalse;

  img.tiles = NULL;
  bitBuf = 0;
  bitBufLen = 0;
  bitBufSkip = gFalse;
  byteCount = 0;
}

JPXStream::~JPXStream() {
  close();
  delete str;
}

void JPXStream::reset() {
  str->reset();
  if (readBoxes()) {
    curY = img.yOffset;
  } else {
    // readBoxes reported an error, so we go immediately to EOF
    curY = img.ySize;
  }
  curX = img.xOffset;
  curComp = 0;
  readBufLen = 0;
}

void JPXStream::close() {
  JPXTile *tile;
  JPXTileComp *tileComp;
  JPXResLevel *resLevel;
  JPXPrecinct *precinct;
  JPXSubband *subband;
  JPXCodeBlock *cb;
  Guint comp, i, k, r, pre, sb;

  gfree(bpc);
  bpc = NULL;
  if (havePalette) {
    gfree(palette.bpc);
    gfree(palette.c);
    havePalette = gFalse;
  }
  if (haveCompMap) {
    gfree(compMap.comp);
    gfree(compMap.type);
    gfree(compMap.pComp);
    haveCompMap = gFalse;
  }
  if (haveChannelDefn) {
    gfree(channelDefn.idx);
    gfree(channelDefn.type);
    gfree(channelDefn.assoc);
    haveChannelDefn = gFalse;
  }

  if (img.tiles) {
    for (i = 0; i < img.nXTiles * img.nYTiles; ++i) {
      tile = &img.tiles[i];
      if (tile->tileComps) {
	for (comp = 0; comp < img.nComps; ++comp) {
	  tileComp = &tile->tileComps[comp];
	  gfree(tileComp->quantSteps);
	  gfree(tileComp->data);
	  gfree(tileComp->buf);
	  if (tileComp->resLevels) {
	    for (r = 0; r <= tileComp->nDecompLevels; ++r) {
	      resLevel = &tileComp->resLevels[r];
	      if (resLevel->precincts) {
		for (pre = 0; pre < 1; ++pre) {
		  precinct = &resLevel->precincts[pre];
		  if (precinct->subbands) {
		    for (sb = 0; sb < (Guint)(r == 0 ? 1 : 3); ++sb) {
		      subband = &precinct->subbands[sb];
		      gfree(subband->inclusion);
		      gfree(subband->zeroBitPlane);
		      if (subband->cbs) {
			for (k = 0; k < subband->nXCBs * subband->nYCBs; ++k) {
			  cb = &subband->cbs[k];
			  gfree(cb->coeffs);
			  if (cb->arithDecoder) {
			    delete cb->arithDecoder;
			  }
			  if (cb->stats) {
			    delete cb->stats;
			  }
			}
			gfree(subband->cbs);
		      }
		    }
		    gfree(precinct->subbands);
		  }
		}
		gfree(img.tiles[i].tileComps[comp].resLevels[r].precincts);
	      }
	    }
	    gfree(img.tiles[i].tileComps[comp].resLevels);
	  }
	}
	gfree(img.tiles[i].tileComps);
      }
    }
    gfree(img.tiles);
    img.tiles = NULL;
  }
  FilterStream::close();
}

int JPXStream::getChar() {
  int c;

  if (readBufLen < 8) {
    fillReadBuf();
  }
  if (readBufLen == 8) {
    c = readBuf & 0xff;
    readBufLen = 0;
  } else if (readBufLen > 8) {
    c = (readBuf >> (readBufLen - 8)) & 0xff;
    readBufLen -= 8;
  } else if (readBufLen == 0) {
    c = EOF;
  } else {
    c = (readBuf << (8 - readBufLen)) & 0xff;
    readBufLen = 0;
  }
  return c;
}

int JPXStream::lookChar() {
  int c;

  if (readBufLen < 8) {
    fillReadBuf();
  }
  if (readBufLen == 8) {
    c = readBuf & 0xff;
  } else if (readBufLen > 8) {
    c = (readBuf >> (readBufLen - 8)) & 0xff;
  } else if (readBufLen == 0) {
    c = EOF;
  } else {
    c = (readBuf << (8 - readBufLen)) & 0xff;
  }
  return c;
}

void JPXStream::fillReadBuf() {
  JPXTileComp *tileComp;
  Guint tileIdx, tx, ty;
  int pix, pixBits;

  do {
    if (curY >= img.ySize) {
      return;
    }
    tileIdx = ((curY - img.yTileOffset) / img.yTileSize) * img.nXTiles
              + (curX - img.xTileOffset) / img.xTileSize;
#if 1 //~ ignore the palette, assume the PDF ColorSpace object is valid
    tileComp = &img.tiles[tileIdx].tileComps[curComp];
#else
    tileComp = &img.tiles[tileIdx].tileComps[havePalette ? 0 : curComp];
#endif
    tx = jpxCeilDiv((curX - img.xTileOffset) % img.xTileSize, tileComp->hSep);
    ty = jpxCeilDiv((curY - img.yTileOffset) % img.yTileSize, tileComp->vSep);
    pix = (int)tileComp->data[ty * (tileComp->x1 - tileComp->x0) + tx];
    pixBits = tileComp->prec;
#if 1 //~ ignore the palette, assume the PDF ColorSpace object is valid
    if (++curComp == img.nComps) {
#else
    if (havePalette) {
      if (pix >= 0 && pix < palette.nEntries) {
	pix = palette.c[pix * palette.nComps + curComp];
      } else {
	pix = 
      pixBits = palette.bpc[curComp];
    }
    if (++curComp == (Guint)(havePalette ? palette.nComps : img.nComps)) {
#endif
      curComp = 0;
      if (++curX == img.xSize) {
	curX = img.xOffset;
	++curY;
      }
    }
    if (pixBits == 8) {
      readBuf = (readBuf << 8) | (pix & 0xff);
    } else {
      readBuf = (readBuf << pixBits) | (pix & ((1 << pixBits) - 1));
    }
    readBufLen += pixBits;
  } while (readBufLen < 8);
}

GooString *JPXStream::getPSFilter(int psLevel, char *indent) {
  return NULL;
}

GBool JPXStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

void JPXStream::getImageParams(int *bitsPerComponent,
			       StreamColorSpaceMode *csMode) {
  Guint boxType, boxLen, dataLen, csEnum;
  Guint bpc1, dummy, i;
  int csMeth, csPrec, csPrec1, dummy2;
  StreamColorSpaceMode csMode1;
  GBool haveBPC, haveCSMode;

  csPrec = 0; // make gcc happy
  haveBPC = haveCSMode = gFalse;
  str->reset();
  if (str->lookChar() == 0xff) {
    getImageParams2(bitsPerComponent, csMode);
  } else {
    while (readBoxHdr(&boxType, &boxLen, &dataLen)) {
      if (boxType == 0x6a703268) { // JP2 header
	cover(0);
	// skip the superbox
      } else if (boxType == 0x69686472) { // image header
	cover(1);
	if (readULong(&dummy) &&
	    readULong(&dummy) &&
	    readUWord(&dummy) &&
	    readUByte(&bpc1) &&
	    readUByte(&dummy) &&
	    readUByte(&dummy) &&
	    readUByte(&dummy)) {
	  *bitsPerComponent = bpc1 + 1;
	  haveBPC = gTrue;
	}
      } else if (boxType == 0x636F6C72) { // color specification
	cover(2);
	if (readByte(&csMeth) &&
	    readByte(&csPrec1) &&
	    readByte(&dummy2)) {
	  if (csMeth == 1) {
	    if (readULong(&csEnum)) {
	      csMode1 = streamCSNone;
	      if (csEnum == jpxCSBiLevel ||
		  csEnum == jpxCSGrayscale) {
		csMode1 = streamCSDeviceGray;
	      } else if (csEnum == jpxCSCMYK) {
		csMode1 = streamCSDeviceCMYK;
	      } else if (csEnum == jpxCSsRGB ||
			 csEnum == jpxCSCISesRGB ||
			 csEnum == jpxCSROMMRGB) {
		csMode1 = streamCSDeviceRGB;
	      }
	      if (csMode1 != streamCSNone &&
		  (!haveCSMode || csPrec1 > csPrec)) {
		*csMode = csMode1;
		csPrec = csPrec1;
		haveCSMode = gTrue;
	      }
	      for (i = 0; i < dataLen - 7; ++i) {
		str->getChar();
	      }
	    }
	  } else {
	    for (i = 0; i < dataLen - 3; ++i) {
	      str->getChar();
	    }
	  }
	}
      } else if (boxType == 0x6A703263) { // codestream
	cover(3);
	if (!(haveBPC && haveCSMode)) {
	  getImageParams2(bitsPerComponent, csMode);
	}
	break;
      } else {
	cover(4);
	for (i = 0; i < dataLen; ++i) {
	  str->getChar();
	}
      }
    }
  }
  str->close();
}

// Get image parameters from the codestream.
void JPXStream::getImageParams2(int *bitsPerComponent,
				StreamColorSpaceMode *csMode) {
  int segType;
  Guint segLen, nComps1, bpc1, dummy, i;

  while (readMarkerHdr(&segType, &segLen)) {
    if (segType == 0x51) { // SIZ - image and tile size
      cover(5);
      if (readUWord(&dummy) &&
	  readULong(&dummy) &&
	  readULong(&dummy) &&
	  readULong(&dummy) &&
	  readULong(&dummy) &&
	  readULong(&dummy) &&
	  readULong(&dummy) &&
	  readULong(&dummy) &&
	  readULong(&dummy) &&
	  readUWord(&nComps1) &&
	  readUByte(&bpc1)) {
	*bitsPerComponent = (bpc1 & 0x7f) + 1;
	// if there's no color space info, take a guess
	if (nComps1 == 1) {
	  *csMode = streamCSDeviceGray;
	} else if (nComps1 == 3) {
	  *csMode = streamCSDeviceRGB;
	} else if (nComps1 == 4) {
	  *csMode = streamCSDeviceCMYK;
	}
      }
      break;
    } else {
      cover(6);
      if (segLen > 2) {
	for (i = 0; i < segLen - 2; ++i) {
	  str->getChar();
	}
      }
    }
  }
}

GBool JPXStream::readBoxes() {
  Guint boxType, boxLen, dataLen;
  Guint bpc1, compression, unknownColorspace, ipr;
  Guint i, j;

  haveImgHdr = gFalse;

  // check for a naked JPEG 2000 codestream (without the JP2/JPX
  // wrapper) -- this appears to be a violation of the PDF spec, but
  // Acrobat allows it
  if (str->lookChar() == 0xff) {
    cover(7);
    error(getPos(), "Naked JPEG 2000 codestream, missing JP2/JPX wrapper");
    readCodestream(0);
    nComps = img.nComps;
    bpc = (Guint *)gmallocn(nComps, sizeof(Guint));
    for (i = 0; i < nComps; ++i) {
      bpc[i] = img.tiles[0].tileComps[i].prec;
    }
    width = img.xSize - img.xOffset;
    height = img.ySize - img.yOffset;
    return gTrue;
  }

  while (readBoxHdr(&boxType, &boxLen, &dataLen)) {
    switch (boxType) {
    case 0x6a703268:		// JP2 header
      // this is a grouping box ('superbox') which has no real
      // contents and doesn't appear to be used consistently, i.e.,
      // some things which should be subboxes of the JP2 header box
      // show up outside of it - so we simply ignore the JP2 header
      // box
      cover(8);
      break;
    case 0x69686472:		// image header
      cover(9);
      if (!readULong(&height) ||
	  !readULong(&width) ||
	  !readUWord(&nComps) ||
	  !readUByte(&bpc1) ||
	  !readUByte(&compression) ||
	  !readUByte(&unknownColorspace) ||
	  !readUByte(&ipr)) {
	error(getPos(), "Unexpected EOF in JPX stream");
	return gFalse;
      }
      if (compression != 7) {
	error(getPos(), "Unknown compression type in JPX stream");
	return gFalse;
      }
      bpc = (Guint *)gmallocn(nComps, sizeof(Guint));
      for (i = 0; i < nComps; ++i) {
	bpc[i] = bpc1;
      }
      haveImgHdr = gTrue;
      break;
    case 0x62706363:		// bits per component
      cover(10);
      if (!haveImgHdr) {
	error(getPos(), "Found bits per component box before image header box in JPX stream");
	return gFalse;
      }
      if (dataLen != nComps) {
	error(getPos(), "Invalid bits per component box in JPX stream");
	return gFalse;
      }
      for (i = 0; i < nComps; ++i) {
	if (!readUByte(&bpc[i])) {
	  error(getPos(), "Unexpected EOF in JPX stream");
	  return gFalse;
	}
      }
      break;
    case 0x636F6C72:		// color specification
      cover(11);
      if (!readColorSpecBox(dataLen)) {
	return gFalse;
      }
      break;
    case 0x70636c72:		// palette
      cover(12);
      if (!readUWord(&palette.nEntries) ||
	  !readUByte(&palette.nComps)) {
	error(getPos(), "Unexpected EOF in JPX stream");
	return gFalse;
      }
      palette.bpc = (Guint *)gmallocn(palette.nComps, sizeof(Guint));
      palette.c =
          (int *)gmallocn(palette.nEntries * palette.nComps, sizeof(int));
      for (i = 0; i < palette.nComps; ++i) {
	if (!readUByte(&palette.bpc[i])) {
	  error(getPos(), "Unexpected EOF in JPX stream");
	  return gFalse;
	}
	++palette.bpc[i];
      }
      for (i = 0; i < palette.nEntries; ++i) {
	for (j = 0; j < palette.nComps; ++j) {
	  if (!readNBytes(((palette.bpc[j] & 0x7f) + 7) >> 3,
			  (palette.bpc[j] & 0x80) ? gTrue : gFalse,
			  &palette.c[i * palette.nComps + j])) {
	    error(getPos(), "Unexpected EOF in JPX stream");
	    return gFalse;
	  }
	}
      }
      havePalette = gTrue;
      break;
    case 0x636d6170:		// component mapping
      cover(13);
      compMap.nChannels = dataLen / 4;
      compMap.comp = (Guint *)gmallocn(compMap.nChannels, sizeof(Guint));
      compMap.type = (Guint *)gmallocn(compMap.nChannels, sizeof(Guint));
      compMap.pComp = (Guint *)gmallocn(compMap.nChannels, sizeof(Guint));
      for (i = 0; i < compMap.nChannels; ++i) {
	if (!readUWord(&compMap.comp[i]) ||
	    !readUByte(&compMap.type[i]) ||
	    !readUByte(&compMap.pComp[i])) {
	  error(getPos(), "Unexpected EOF in JPX stream");
	  return gFalse;
	}
      }
      haveCompMap = gTrue;
      break;
    case 0x63646566:		// channel definition
      cover(14);
      if (!readUWord(&channelDefn.nChannels)) {
	error(getPos(), "Unexpected EOF in JPX stream");
	return gFalse;
      }
      channelDefn.idx =
	  (Guint *)gmallocn(channelDefn.nChannels, sizeof(Guint));
      channelDefn.type =
	  (Guint *)gmallocn(channelDefn.nChannels, sizeof(Guint));
      channelDefn.assoc =
	  (Guint *)gmallocn(channelDefn.nChannels, sizeof(Guint));
      for (i = 0; i < channelDefn.nChannels; ++i) {
	if (!readUWord(&channelDefn.idx[i]) ||
	    !readUWord(&channelDefn.type[i]) ||
	    !readUWord(&channelDefn.assoc[i])) {
	  error(getPos(), "Unexpected EOF in JPX stream");
	  return gFalse;
	}
      }
      haveChannelDefn = gTrue;
      break;
    case 0x6A703263:		// contiguous codestream
      cover(15);
      if (!bpc) {
	error(getPos(), "JPX stream is missing the image header box");
      }
      if (!haveCS) {
	error(getPos(), "JPX stream has no supported color spec");
      }
      if (!readCodestream(dataLen)) {
	return gFalse;
      }
      break;
    default:
      cover(16);
      for (i = 0; i < dataLen; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Unexpected EOF in JPX stream");
	  return gFalse;
	}
      }
      break;
    }
  }
  return gTrue;
}

GBool JPXStream::readColorSpecBox(Guint dataLen) {
  JPXColorSpec newCS;
  Guint csApprox, csEnum;
  Guint i;
  GBool ok;

  ok = gFalse;
  if (!readUByte(&newCS.meth) ||
      !readByte(&newCS.prec) ||
      !readUByte(&csApprox)) {
    goto err;
  }
  switch (newCS.meth) {
  case 1:			// enumerated colorspace
    cover(17);
    if (!readULong(&csEnum)) {
      goto err;
    }
    newCS.enumerated.type = (JPXColorSpaceType)csEnum;
    switch (newCS.enumerated.type) {
    case jpxCSBiLevel:
      ok = gTrue;
      break;
    case jpxCSYCbCr1:
      ok = gTrue;
      break;
    case jpxCSYCbCr2:
      ok = gTrue;
      break;
    case jpxCSYCBCr3:
      ok = gTrue;
      break;
    case jpxCSPhotoYCC:
      ok = gTrue;
      break;
    case jpxCSCMY:
      ok = gTrue;
      break;
    case jpxCSCMYK:
      ok = gTrue;
      break;
    case jpxCSYCCK:
      ok = gTrue;
      break;
    case jpxCSCIELab:
      if (dataLen == 7 + 7*4) {
	if (!readULong(&newCS.enumerated.cieLab.rl) ||
	    !readULong(&newCS.enumerated.cieLab.ol) ||
	    !readULong(&newCS.enumerated.cieLab.ra) ||
	    !readULong(&newCS.enumerated.cieLab.oa) ||
	    !readULong(&newCS.enumerated.cieLab.rb) ||
	    !readULong(&newCS.enumerated.cieLab.ob) ||
	    !readULong(&newCS.enumerated.cieLab.il)) {
	  goto err;
	}
      } else if (dataLen == 7) {
	//~ this assumes the 8-bit case
	cover(92);
	newCS.enumerated.cieLab.rl = 100;
	newCS.enumerated.cieLab.ol = 0;
	newCS.enumerated.cieLab.ra = 255;
	newCS.enumerated.cieLab.oa = 128;
	newCS.enumerated.cieLab.rb = 255;
	newCS.enumerated.cieLab.ob = 96;
	newCS.enumerated.cieLab.il = 0x00443530;
      } else {
	goto err;
      }
      ok = gTrue;
      break;
    case jpxCSsRGB:
      ok = gTrue;
      break;
    case jpxCSGrayscale:
      ok = gTrue;
      break;
    case jpxCSBiLevel2:
      ok = gTrue;
      break;
    case jpxCSCIEJab:
      // not allowed in PDF
      goto err;
    case jpxCSCISesRGB:
      ok = gTrue;
      break;
    case jpxCSROMMRGB:
      ok = gTrue;
      break;
    case jpxCSsRGBYCbCr:
      ok = gTrue;
      break;
    case jpxCSYPbPr1125:
      ok = gTrue;
      break;
    case jpxCSYPbPr1250:
      ok = gTrue;
      break;
    default:
      goto err;
    }
    break;
  case 2:			// restricted ICC profile
  case 3: 			// any ICC profile (JPX)
  case 4:			// vendor color (JPX)
    cover(18);
    for (i = 0; i < dataLen - 3; ++i) {
      if (str->getChar() == EOF) {
	goto err;
      }
    }
    break;
  }

  if (ok && (!haveCS || newCS.prec > cs.prec)) {
    cs = newCS;
    haveCS = gTrue;
  }

  return gTrue;

 err:
  error(getPos(), "Error in JPX color spec");
  return gFalse;
}

GBool JPXStream::readCodestream(Guint len) {
  JPXTile *tile;
  JPXTileComp *tileComp;
  int segType;
  GBool haveSIZ, haveCOD, haveQCD, haveSOT;
  Guint precinctSize, style;
  Guint segLen, capabilities, comp, i, j, r;

  //----- main header
  haveSIZ = haveCOD = haveQCD = haveSOT = gFalse;
  do {
    if (!readMarkerHdr(&segType, &segLen)) {
      error(getPos(), "Error in JPX codestream");
      return gFalse;
    }
    switch (segType) {
    case 0x4f:			// SOC - start of codestream
      // marker only
      cover(19);
      break;
    case 0x51:			// SIZ - image and tile size
      cover(20);
      if (!readUWord(&capabilities) ||
	  !readULong(&img.xSize) ||
	  !readULong(&img.ySize) ||
	  !readULong(&img.xOffset) ||
	  !readULong(&img.yOffset) ||
	  !readULong(&img.xTileSize) ||
	  !readULong(&img.yTileSize) ||
	  !readULong(&img.xTileOffset) ||
	  !readULong(&img.yTileOffset) ||
	  !readUWord(&img.nComps)) {
	error(getPos(), "Error in JPX SIZ marker segment");
	return gFalse;
      }
      if (haveImgHdr && img.nComps != nComps) {
	error(getPos(), "Different number of components in JPX SIZ marker segment");
	return gFalse;
      }
      img.nXTiles = (img.xSize - img.xTileOffset + img.xTileSize - 1)
	            / img.xTileSize;
      img.nYTiles = (img.ySize - img.yTileOffset + img.yTileSize - 1)
	            / img.yTileSize;
      // check for overflow before allocating memory
      if (img.nXTiles <= 0 || img.nYTiles <= 0 ||
	  img.nXTiles >= INT_MAX / img.nYTiles) {
	error(getPos(), "Bad tile count in JPX SIZ marker segment");
	return gFalse;
      }
      img.tiles = (JPXTile *)gmallocn(img.nXTiles * img.nYTiles,
				      sizeof(JPXTile));
      for (i = 0; i < img.nXTiles * img.nYTiles; ++i) {
	img.tiles[i].tileComps = (JPXTileComp *)gmallocn(img.nComps,
							 sizeof(JPXTileComp));
	for (comp = 0; comp < img.nComps; ++comp) {
	  img.tiles[i].tileComps[comp].quantSteps = NULL;
	  img.tiles[i].tileComps[comp].data = NULL;
	  img.tiles[i].tileComps[comp].buf = NULL;
	  img.tiles[i].tileComps[comp].resLevels = NULL;
	}
      }
      for (comp = 0; comp < img.nComps; ++comp) {
	if (!readUByte(&img.tiles[0].tileComps[comp].prec) ||
	    !readUByte(&img.tiles[0].tileComps[comp].hSep) ||
	    !readUByte(&img.tiles[0].tileComps[comp].vSep)) {
	  error(getPos(), "Error in JPX SIZ marker segment");
	  return gFalse;
	}
	img.tiles[0].tileComps[comp].sgned =
	    (img.tiles[0].tileComps[comp].prec & 0x80) ? gTrue : gFalse;
	img.tiles[0].tileComps[comp].prec =
	    (img.tiles[0].tileComps[comp].prec & 0x7f) + 1;
	for (i = 1; i < img.nXTiles * img.nYTiles; ++i) {
	  img.tiles[i].tileComps[comp] = img.tiles[0].tileComps[comp];
	}
      }
      haveSIZ = gTrue;
      break;
    case 0x52:			// COD - coding style default
      cover(21);
      if (!readUByte(&img.tiles[0].tileComps[0].style) ||
	  !readUByte(&img.tiles[0].progOrder) ||
	  !readUWord(&img.tiles[0].nLayers) ||
	  !readUByte(&img.tiles[0].multiComp) ||
	  !readUByte(&img.tiles[0].tileComps[0].nDecompLevels) ||
	  !readUByte(&img.tiles[0].tileComps[0].codeBlockW) ||
	  !readUByte(&img.tiles[0].tileComps[0].codeBlockH) ||
	  !readUByte(&img.tiles[0].tileComps[0].codeBlockStyle) ||
	  !readUByte(&img.tiles[0].tileComps[0].transform)) {
	error(getPos(), "Error in JPX COD marker segment");
	return gFalse;
      }
      img.tiles[0].tileComps[0].codeBlockW += 2;
      img.tiles[0].tileComps[0].codeBlockH += 2;
      for (i = 0; i < img.nXTiles * img.nYTiles; ++i) {
	if (i != 0) {
	  img.tiles[i].progOrder = img.tiles[0].progOrder;
	  img.tiles[i].nLayers = img.tiles[0].nLayers;
	  img.tiles[i].multiComp = img.tiles[0].multiComp;
	}
	for (comp = 0; comp < img.nComps; ++comp) {
	  if (!(i == 0 && comp == 0)) {
	    img.tiles[i].tileComps[comp].style =
	        img.tiles[0].tileComps[0].style;
	    img.tiles[i].tileComps[comp].nDecompLevels =
	        img.tiles[0].tileComps[0].nDecompLevels;
	    img.tiles[i].tileComps[comp].codeBlockW =
	        img.tiles[0].tileComps[0].codeBlockW;
	    img.tiles[i].tileComps[comp].codeBlockH =
	        img.tiles[0].tileComps[0].codeBlockH;
	    img.tiles[i].tileComps[comp].codeBlockStyle =
	        img.tiles[0].tileComps[0].codeBlockStyle;
	    img.tiles[i].tileComps[comp].transform =
	        img.tiles[0].tileComps[0].transform;
	  }
	  img.tiles[i].tileComps[comp].resLevels =
	      (JPXResLevel *)gmallocn(
		     (img.tiles[i].tileComps[comp].nDecompLevels + 1),
		     sizeof(JPXResLevel));
	  for (r = 0; r <= img.tiles[i].tileComps[comp].nDecompLevels; ++r) {
	    img.tiles[i].tileComps[comp].resLevels[r].precincts = NULL;
	  }
	}
      }
      for (r = 0; r <= img.tiles[0].tileComps[0].nDecompLevels; ++r) {
	if (img.tiles[0].tileComps[0].style & 0x01) {
	  cover(91);
	  if (!readUByte(&precinctSize)) {
	    error(getPos(), "Error in JPX COD marker segment");
	    return gFalse;
	  }
	  img.tiles[0].tileComps[0].resLevels[r].precinctWidth =
	      precinctSize & 0x0f;
	  img.tiles[0].tileComps[0].resLevels[r].precinctHeight =
	      (precinctSize >> 4) & 0x0f;
	} else {
	  img.tiles[0].tileComps[0].resLevels[r].precinctWidth = 15;
	  img.tiles[0].tileComps[0].resLevels[r].precinctHeight = 15;
	}
      }
      for (i = 0; i < img.nXTiles * img.nYTiles; ++i) {
	for (comp = 0; comp < img.nComps; ++comp) {
	  if (!(i == 0 && comp == 0)) {
	    for (r = 0; r <= img.tiles[i].tileComps[comp].nDecompLevels; ++r) {
	      img.tiles[i].tileComps[comp].resLevels[r].precinctWidth =
		  img.tiles[0].tileComps[0].resLevels[r].precinctWidth;
	      img.tiles[i].tileComps[comp].resLevels[r].precinctHeight =
		  img.tiles[0].tileComps[0].resLevels[r].precinctHeight;
	    }
	  }
	}
      }
      haveCOD = gTrue;
      break;
    case 0x53:			// COC - coding style component
      cover(22);
      if (!haveCOD) {
	error(getPos(), "JPX COC marker segment before COD segment");
	return gFalse;
      }
      if ((img.nComps > 256 && !readUWord(&comp)) ||
	  (img.nComps <= 256 && !readUByte(&comp)) ||
	  comp >= img.nComps ||
	  !readUByte(&style) ||
	  !readUByte(&img.tiles[0].tileComps[comp].nDecompLevels) ||
	  !readUByte(&img.tiles[0].tileComps[comp].codeBlockW) ||
	  !readUByte(&img.tiles[0].tileComps[comp].codeBlockH) ||
	  !readUByte(&img.tiles[0].tileComps[comp].codeBlockStyle) ||
	  !readUByte(&img.tiles[0].tileComps[comp].transform)) {
	error(getPos(), "Error in JPX COC marker segment");
	return gFalse;
      }
      img.tiles[0].tileComps[comp].style =
	  (img.tiles[0].tileComps[comp].style & ~1) | (style & 1);
      img.tiles[0].tileComps[comp].codeBlockW += 2;
      img.tiles[0].tileComps[comp].codeBlockH += 2;
      for (i = 0; i < img.nXTiles * img.nYTiles; ++i) {
	if (i != 0) {
	  img.tiles[i].tileComps[comp].style =
	      img.tiles[0].tileComps[comp].style;
	  img.tiles[i].tileComps[comp].nDecompLevels =
	      img.tiles[0].tileComps[comp].nDecompLevels;
	  img.tiles[i].tileComps[comp].codeBlockW =
	      img.tiles[0].tileComps[comp].codeBlockW;
	  img.tiles[i].tileComps[comp].codeBlockH =
	      img.tiles[0].tileComps[comp].codeBlockH;
	  img.tiles[i].tileComps[comp].codeBlockStyle =
	      img.tiles[0].tileComps[comp].codeBlockStyle;
	  img.tiles[i].tileComps[comp].transform =
	      img.tiles[0].tileComps[comp].transform;
	}
	img.tiles[i].tileComps[comp].resLevels =
	    (JPXResLevel *)greallocn(
		     img.tiles[i].tileComps[comp].resLevels,
		     (img.tiles[i].tileComps[comp].nDecompLevels + 1),
		     sizeof(JPXResLevel));
	for (r = 0; r <= img.tiles[i].tileComps[comp].nDecompLevels; ++r) {
	  img.tiles[i].tileComps[comp].resLevels[r].precincts = NULL;
	}
      }
      for (r = 0; r <= img.tiles[0].tileComps[comp].nDecompLevels; ++r) {
	if (img.tiles[0].tileComps[comp].style & 0x01) {
	  if (!readUByte(&precinctSize)) {
	    error(getPos(), "Error in JPX COD marker segment");
	    return gFalse;
	  }
	  img.tiles[0].tileComps[comp].resLevels[r].precinctWidth =
	      precinctSize & 0x0f;
	  img.tiles[0].tileComps[comp].resLevels[r].precinctHeight =
	      (precinctSize >> 4) & 0x0f;
	} else {
	  img.tiles[0].tileComps[comp].resLevels[r].precinctWidth = 15;
	  img.tiles[0].tileComps[comp].resLevels[r].precinctHeight = 15;
	}
      }
      for (i = 1; i < img.nXTiles * img.nYTiles; ++i) {
	for (r = 0; r <= img.tiles[i].tileComps[comp].nDecompLevels; ++r) {
	  img.tiles[i].tileComps[comp].resLevels[r].precinctWidth =
	      img.tiles[0].tileComps[comp].resLevels[r].precinctWidth;
	  img.tiles[i].tileComps[comp].resLevels[r].precinctHeight =
	      img.tiles[0].tileComps[comp].resLevels[r].precinctHeight;
	}
      }
      break;
    case 0x5c:			// QCD - quantization default
      cover(23);
      if (!readUByte(&img.tiles[0].tileComps[0].quantStyle)) {
	error(getPos(), "Error in JPX QCD marker segment");
	return gFalse;
      }
      if ((img.tiles[0].tileComps[0].quantStyle & 0x1f) == 0x00) {
	img.tiles[0].tileComps[0].nQuantSteps = segLen - 3;
	img.tiles[0].tileComps[0].quantSteps =
	    (Guint *)greallocn(img.tiles[0].tileComps[0].quantSteps,
			       img.tiles[0].tileComps[0].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[0].tileComps[0].nQuantSteps; ++i) {
	  if (!readUByte(&img.tiles[0].tileComps[0].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCD marker segment");
	    return gFalse;
	  }
	}
      } else if ((img.tiles[0].tileComps[0].quantStyle & 0x1f) == 0x01) {
	img.tiles[0].tileComps[0].nQuantSteps = 1;
	img.tiles[0].tileComps[0].quantSteps =
	    (Guint *)greallocn(img.tiles[0].tileComps[0].quantSteps,
			       img.tiles[0].tileComps[0].nQuantSteps,
			       sizeof(Guint));
	if (!readUWord(&img.tiles[0].tileComps[0].quantSteps[0])) {
	  error(getPos(), "Error in JPX QCD marker segment");
	  return gFalse;
	}
      } else if ((img.tiles[0].tileComps[0].quantStyle & 0x1f) == 0x02) {
	img.tiles[0].tileComps[0].nQuantSteps = (segLen - 3) / 2;
	img.tiles[0].tileComps[0].quantSteps =
	    (Guint *)greallocn(img.tiles[0].tileComps[0].quantSteps,
			       img.tiles[0].tileComps[0].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[0].tileComps[0].nQuantSteps; ++i) {
	  if (!readUWord(&img.tiles[0].tileComps[0].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCD marker segment");
	    return gFalse;
	  }
	}
      } else {
	error(getPos(), "Error in JPX QCD marker segment");
	return gFalse;
      }
      for (i = 0; i < img.nXTiles * img.nYTiles; ++i) {
	for (comp = 0; comp < img.nComps; ++comp) {
	  if (!(i == 0 && comp == 0)) {
	    img.tiles[i].tileComps[comp].quantStyle =
	        img.tiles[0].tileComps[0].quantStyle;
	    img.tiles[i].tileComps[comp].nQuantSteps =
	        img.tiles[0].tileComps[0].nQuantSteps;
	    img.tiles[i].tileComps[comp].quantSteps = 
	        (Guint *)greallocn(img.tiles[i].tileComps[comp].quantSteps,
				   img.tiles[0].tileComps[0].nQuantSteps,
				   sizeof(Guint));
	    for (j = 0; j < img.tiles[0].tileComps[0].nQuantSteps; ++j) {
	      img.tiles[i].tileComps[comp].quantSteps[j] =
		  img.tiles[0].tileComps[0].quantSteps[j];
	    }
	  }
	}
      }
      haveQCD = gTrue;
      break;
    case 0x5d:			// QCC - quantization component
      cover(24);
      if (!haveQCD) {
	error(getPos(), "JPX QCC marker segment before QCD segment");
	return gFalse;
      }
      if ((img.nComps > 256 && !readUWord(&comp)) ||
	  (img.nComps <= 256 && !readUByte(&comp)) ||
	  comp >= img.nComps ||
	  !readUByte(&img.tiles[0].tileComps[comp].quantStyle)) {
	error(getPos(), "Error in JPX QCC marker segment");
	return gFalse;
      }
      if ((img.tiles[0].tileComps[comp].quantStyle & 0x1f) == 0x00) {
	img.tiles[0].tileComps[comp].nQuantSteps =
	    segLen - (img.nComps > 256 ? 5 : 4);
	img.tiles[0].tileComps[comp].quantSteps =
	    (Guint *)greallocn(img.tiles[0].tileComps[comp].quantSteps,
			       img.tiles[0].tileComps[comp].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[0].tileComps[comp].nQuantSteps; ++i) {
	  if (!readUByte(&img.tiles[0].tileComps[comp].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCC marker segment");
	    return gFalse;
	  }
	}
      } else if ((img.tiles[0].tileComps[comp].quantStyle & 0x1f) == 0x01) {
	img.tiles[0].tileComps[comp].nQuantSteps = 1;
	img.tiles[0].tileComps[comp].quantSteps =
	    (Guint *)greallocn(img.tiles[0].tileComps[comp].quantSteps,
			       img.tiles[0].tileComps[comp].nQuantSteps,
			       sizeof(Guint));
	if (!readUWord(&img.tiles[0].tileComps[comp].quantSteps[0])) {
	  error(getPos(), "Error in JPX QCC marker segment");
	  return gFalse;
	}
      } else if ((img.tiles[0].tileComps[comp].quantStyle & 0x1f) == 0x02) {
	img.tiles[0].tileComps[comp].nQuantSteps =
	    (segLen - (img.nComps > 256 ? 5 : 4)) / 2;
	img.tiles[0].tileComps[comp].quantSteps =
	    (Guint *)greallocn(img.tiles[0].tileComps[comp].quantSteps,
			       img.tiles[0].tileComps[comp].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[0].tileComps[comp].nQuantSteps; ++i) {
	  if (!readUWord(&img.tiles[0].tileComps[comp].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCD marker segment");
	    return gFalse;
	  }
	}
      } else {
	error(getPos(), "Error in JPX QCC marker segment");
	return gFalse;
      }
      for (i = 1; i < img.nXTiles * img.nYTiles; ++i) {
	img.tiles[i].tileComps[comp].quantStyle =
	    img.tiles[0].tileComps[comp].quantStyle;
	img.tiles[i].tileComps[comp].nQuantSteps =
	    img.tiles[0].tileComps[comp].nQuantSteps;
	img.tiles[i].tileComps[comp].quantSteps = 
	    (Guint *)greallocn(img.tiles[i].tileComps[comp].quantSteps,
			       img.tiles[0].tileComps[comp].nQuantSteps,
			       sizeof(Guint));
	for (j = 0; j < img.tiles[0].tileComps[comp].nQuantSteps; ++j) {
	  img.tiles[i].tileComps[comp].quantSteps[j] =
	      img.tiles[0].tileComps[comp].quantSteps[j];
	}
      }
      break;
    case 0x5e:			// RGN - region of interest
      cover(25);
#if 1 //~ ROI is unimplemented
      fprintf(stderr, "RGN\n");
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PPM marker segment");
	  return gFalse;
	}
      }
#else
      if ((img.nComps > 256 && !readUWord(&comp)) ||
	  (img.nComps <= 256 && !readUByte(&comp)) ||
	  comp >= img.nComps ||
	  !readUByte(&compInfo[comp].defROI.style) ||
	  !readUByte(&compInfo[comp].defROI.shift)) {
	error(getPos(), "Error in JPX RGN marker segment");
	return gFalse;
      }
#endif
      break;
    case 0x5f:			// POC - progression order change
      cover(26);
#if 1 //~ progression order changes are unimplemented
      fprintf(stderr, "POC\n");
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PPM marker segment");
	  return gFalse;
	}
      }
#else
      nProgs = (segLen - 2) / (img.nComps > 256 ? 9 : 7);
      progs = (JPXProgOrder *)gmallocn(nProgs, sizeof(JPXProgOrder));
      for (i = 0; i < nProgs; ++i) {
	if (!readUByte(&progs[i].startRes) ||
	    !(img.nComps > 256 && readUWord(&progs[i].startComp)) ||
	    !(img.nComps <= 256 && readUByte(&progs[i].startComp)) ||
	    !readUWord(&progs[i].endLayer) ||
	    !readUByte(&progs[i].endRes) ||
	    !(img.nComps > 256 && readUWord(&progs[i].endComp)) ||
	    !(img.nComps <= 256 && readUByte(&progs[i].endComp)) ||
	    !readUByte(&progs[i].progOrder)) {
	  error(getPos(), "Error in JPX POC marker segment");
	  return gFalse;
	}
      }
#endif
      break;
    case 0x60:			// PPM - packed packet headers, main header
      cover(27);
#if 1 //~ packed packet headers are unimplemented
      fprintf(stderr, "PPM\n");
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PPM marker segment");
	  return gFalse;
	}
      }
#endif
      break;
    case 0x55:			// TLM - tile-part lengths
      // skipped
      cover(28);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX TLM marker segment");
	  return gFalse;
	}
      }
      break;
    case 0x57:			// PLM - packet length, main header
      // skipped
      cover(29);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PLM marker segment");
	  return gFalse;
	}
      }
      break;
    case 0x63:			// CRG - component registration
      // skipped
      cover(30);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX CRG marker segment");
	  return gFalse;
	}
      }
      break;
    case 0x64:			// COM - comment
      // skipped
      cover(31);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX COM marker segment");
	  return gFalse;
	}
      }
      break;
    case 0x90:			// SOT - start of tile
      cover(32);
      haveSOT = gTrue;
      break;
    default:
      cover(33);
      error(getPos(), "Unknown marker segment %02x in JPX stream", segType);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  break;
	}
      }
      break;
    }
  } while (!haveSOT);

  if (!haveSIZ) {
    error(getPos(), "Missing SIZ marker segment in JPX stream");
    return gFalse;
  }
  if (!haveCOD) {
    error(getPos(), "Missing COD marker segment in JPX stream");
    return gFalse;
  }
  if (!haveQCD) {
    error(getPos(), "Missing QCD marker segment in JPX stream");
    return gFalse;
  }

  //----- read the tile-parts
  while (1) {
    if (!readTilePart()) {
      return gFalse;
    }
    if (!readMarkerHdr(&segType, &segLen)) {
      error(getPos(), "Error in JPX codestream");
      return gFalse;
    }
    if (segType != 0x90) {	// SOT - start of tile
      break;
    }
  }

  if (segType != 0xd9) {	// EOC - end of codestream
    error(getPos(), "Missing EOC marker in JPX codestream");
    return gFalse;
  }

  //----- finish decoding the image
  for (i = 0; i < img.nXTiles * img.nYTiles; ++i) {
    tile = &img.tiles[i];
    for (comp = 0; comp < img.nComps; ++comp) {
      tileComp = &tile->tileComps[comp];
      inverseTransform(tileComp);
    }
    if (!inverseMultiCompAndDC(tile)) {
      return gFalse;
    }
  }

  //~ can free memory below tileComps here, and also tileComp.buf

  return gTrue;
}

GBool JPXStream::readTilePart() {
  JPXTile *tile;
  JPXTileComp *tileComp;
  JPXResLevel *resLevel;
  JPXPrecinct *precinct;
  JPXSubband *subband;
  JPXCodeBlock *cb;
  GBool haveSOD;
  Guint tileIdx, tilePartLen, tilePartIdx, nTileParts;
  GBool tilePartToEOC;
  Guint precinctSize, style;
  Guint n, nSBs, nx, ny, sbx0, sby0, comp, segLen;
  Guint i, j, k, cbX, cbY, r, pre, sb, cbi;
  int segType, level;

  // process the SOT marker segment
  if (!readUWord(&tileIdx) ||
      !readULong(&tilePartLen) ||
      !readUByte(&tilePartIdx) ||
      !readUByte(&nTileParts)) {
    error(getPos(), "Error in JPX SOT marker segment");
    return gFalse;
  }

  if (tileIdx >= img.nXTiles * img.nYTiles) {
    error(getPos(), "Weird tile index in JPX stream");
    return gFalse;
  }

  tilePartToEOC = tilePartLen == 0;
  tilePartLen -= 12; // subtract size of SOT segment

  haveSOD = gFalse;
  do {
    if (!readMarkerHdr(&segType, &segLen)) {
      error(getPos(), "Error in JPX tile-part codestream");
      return gFalse;
    }
    tilePartLen -= 2 + segLen;
    switch (segType) {
    case 0x52:			// COD - coding style default
      cover(34);
      if (!readUByte(&img.tiles[tileIdx].tileComps[0].style) ||
	  !readUByte(&img.tiles[tileIdx].progOrder) ||
	  !readUWord(&img.tiles[tileIdx].nLayers) ||
	  !readUByte(&img.tiles[tileIdx].multiComp) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[0].nDecompLevels) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[0].codeBlockW) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[0].codeBlockH) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[0].codeBlockStyle) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[0].transform)) {
	error(getPos(), "Error in JPX COD marker segment");
	return gFalse;
      }
      img.tiles[tileIdx].tileComps[0].codeBlockW += 2;
      img.tiles[tileIdx].tileComps[0].codeBlockH += 2;
      for (comp = 0; comp < img.nComps; ++comp) {
	if (comp != 0) {
	  img.tiles[tileIdx].tileComps[comp].style =
	      img.tiles[tileIdx].tileComps[0].style;
	  img.tiles[tileIdx].tileComps[comp].nDecompLevels =
	      img.tiles[tileIdx].tileComps[0].nDecompLevels;
	  img.tiles[tileIdx].tileComps[comp].codeBlockW =
	      img.tiles[tileIdx].tileComps[0].codeBlockW;
	  img.tiles[tileIdx].tileComps[comp].codeBlockH =
	      img.tiles[tileIdx].tileComps[0].codeBlockH;
	  img.tiles[tileIdx].tileComps[comp].codeBlockStyle =
	      img.tiles[tileIdx].tileComps[0].codeBlockStyle;
	  img.tiles[tileIdx].tileComps[comp].transform =
	      img.tiles[tileIdx].tileComps[0].transform;
	}
	img.tiles[tileIdx].tileComps[comp].resLevels =
	    (JPXResLevel *)greallocn(
		     img.tiles[tileIdx].tileComps[comp].resLevels,
		     (img.tiles[tileIdx].tileComps[comp].nDecompLevels + 1),
		     sizeof(JPXResLevel));
	for (r = 0;
	     r <= img.tiles[tileIdx].tileComps[comp].nDecompLevels;
	     ++r) {
	  img.tiles[tileIdx].tileComps[comp].resLevels[r].precincts = NULL;
	}
      }
      for (r = 0; r <= img.tiles[tileIdx].tileComps[0].nDecompLevels; ++r) {
	if (img.tiles[tileIdx].tileComps[0].style & 0x01) {
	  if (!readUByte(&precinctSize)) {
	    error(getPos(), "Error in JPX COD marker segment");
	    return gFalse;
	  }
	  img.tiles[tileIdx].tileComps[0].resLevels[r].precinctWidth =
	      precinctSize & 0x0f;
	  img.tiles[tileIdx].tileComps[0].resLevels[r].precinctHeight =
	      (precinctSize >> 4) & 0x0f;
	} else {
	  img.tiles[tileIdx].tileComps[0].resLevels[r].precinctWidth = 15;
	  img.tiles[tileIdx].tileComps[0].resLevels[r].precinctHeight = 15;
	}
      }
      for (comp = 1; comp < img.nComps; ++comp) {
	for (r = 0;
	     r <= img.tiles[tileIdx].tileComps[comp].nDecompLevels;
	     ++r) {
	  img.tiles[tileIdx].tileComps[comp].resLevels[r].precinctWidth =
	      img.tiles[tileIdx].tileComps[0].resLevels[r].precinctWidth;
	  img.tiles[tileIdx].tileComps[comp].resLevels[r].precinctHeight =
	      img.tiles[tileIdx].tileComps[0].resLevels[r].precinctHeight;
	}
      }
      break;
    case 0x53:			// COC - coding style component
      cover(35);
      if ((img.nComps > 256 && !readUWord(&comp)) ||
	  (img.nComps <= 256 && !readUByte(&comp)) ||
	  comp >= img.nComps ||
	  !readUByte(&style) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[comp].nDecompLevels) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[comp].codeBlockW) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[comp].codeBlockH) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[comp].codeBlockStyle) ||
	  !readUByte(&img.tiles[tileIdx].tileComps[comp].transform)) {
	error(getPos(), "Error in JPX COC marker segment");
	return gFalse;
      }
      img.tiles[tileIdx].tileComps[comp].style =
	  (img.tiles[tileIdx].tileComps[comp].style & ~1) | (style & 1);
      img.tiles[tileIdx].tileComps[comp].codeBlockW += 2;
      img.tiles[tileIdx].tileComps[comp].codeBlockH += 2;
      img.tiles[tileIdx].tileComps[comp].resLevels =
	  (JPXResLevel *)greallocn(
		     img.tiles[tileIdx].tileComps[comp].resLevels,
		     (img.tiles[tileIdx].tileComps[comp].nDecompLevels + 1),
		     sizeof(JPXResLevel));
      for (r = 0; r <= img.tiles[tileIdx].tileComps[comp].nDecompLevels; ++r) {
	img.tiles[tileIdx].tileComps[comp].resLevels[r].precincts = NULL;
      }
      for (r = 0; r <= img.tiles[tileIdx].tileComps[comp].nDecompLevels; ++r) {
	if (img.tiles[tileIdx].tileComps[comp].style & 0x01) {
	  if (!readUByte(&precinctSize)) {
	    error(getPos(), "Error in JPX COD marker segment");
	    return gFalse;
	  }
	  img.tiles[tileIdx].tileComps[comp].resLevels[r].precinctWidth =
	      precinctSize & 0x0f;
	  img.tiles[tileIdx].tileComps[comp].resLevels[r].precinctHeight =
	      (precinctSize >> 4) & 0x0f;
	} else {
	  img.tiles[tileIdx].tileComps[comp].resLevels[r].precinctWidth = 15;
	  img.tiles[tileIdx].tileComps[comp].resLevels[r].precinctHeight = 15;
	}
      }
      break;
    case 0x5c:			// QCD - quantization default
      cover(36);
      if (!readUByte(&img.tiles[tileIdx].tileComps[0].quantStyle)) {
	error(getPos(), "Error in JPX QCD marker segment");
	return gFalse;
      }
      if ((img.tiles[tileIdx].tileComps[0].quantStyle & 0x1f) == 0x00) {
	img.tiles[tileIdx].tileComps[0].nQuantSteps =
	    segLen - 3;
	img.tiles[tileIdx].tileComps[0].quantSteps =
	    (Guint *)greallocn(img.tiles[tileIdx].tileComps[0].quantSteps,
			       img.tiles[tileIdx].tileComps[0].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[tileIdx].tileComps[0].nQuantSteps; ++i) {
	  if (!readUByte(&img.tiles[tileIdx].tileComps[0].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCD marker segment");
	    return gFalse;
	  }
	}
      } else if ((img.tiles[tileIdx].tileComps[0].quantStyle & 0x1f) == 0x01) {
	img.tiles[tileIdx].tileComps[0].nQuantSteps = 1;
	img.tiles[tileIdx].tileComps[0].quantSteps =
	    (Guint *)greallocn(img.tiles[tileIdx].tileComps[0].quantSteps,
			       img.tiles[tileIdx].tileComps[0].nQuantSteps,
			       sizeof(Guint));
	if (!readUWord(&img.tiles[tileIdx].tileComps[0].quantSteps[0])) {
	  error(getPos(), "Error in JPX QCD marker segment");
	  return gFalse;
	}
      } else if ((img.tiles[tileIdx].tileComps[0].quantStyle & 0x1f) == 0x02) {
	img.tiles[tileIdx].tileComps[0].nQuantSteps = (segLen - 3) / 2;
	img.tiles[tileIdx].tileComps[0].quantSteps =
	    (Guint *)greallocn(img.tiles[tileIdx].tileComps[0].quantSteps,
			       img.tiles[tileIdx].tileComps[0].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[tileIdx].tileComps[0].nQuantSteps; ++i) {
	  if (!readUWord(&img.tiles[tileIdx].tileComps[0].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCD marker segment");
	    return gFalse;
	  }
	}
      } else {
	error(getPos(), "Error in JPX QCD marker segment");
	return gFalse;
      }
      for (comp = 1; comp < img.nComps; ++comp) {
	img.tiles[tileIdx].tileComps[comp].quantStyle =
	    img.tiles[tileIdx].tileComps[0].quantStyle;
	img.tiles[tileIdx].tileComps[comp].nQuantSteps =
	    img.tiles[tileIdx].tileComps[0].nQuantSteps;
	img.tiles[tileIdx].tileComps[comp].quantSteps = 
	    (Guint *)greallocn(img.tiles[tileIdx].tileComps[comp].quantSteps,
			       img.tiles[tileIdx].tileComps[0].nQuantSteps,
			       sizeof(Guint));
	for (j = 0; j < img.tiles[tileIdx].tileComps[0].nQuantSteps; ++j) {
	  img.tiles[tileIdx].tileComps[comp].quantSteps[j] =
	      img.tiles[tileIdx].tileComps[0].quantSteps[j];
	}
      }
      break;
    case 0x5d:			// QCC - quantization component
      cover(37);
      if ((img.nComps > 256 && !readUWord(&comp)) ||
	  (img.nComps <= 256 && !readUByte(&comp)) ||
	  comp >= img.nComps ||
	  !readUByte(&img.tiles[tileIdx].tileComps[comp].quantStyle)) {
	error(getPos(), "Error in JPX QCC marker segment");
	return gFalse;
      }
      if ((img.tiles[tileIdx].tileComps[comp].quantStyle & 0x1f) == 0x00) {
	img.tiles[tileIdx].tileComps[comp].nQuantSteps =
	    segLen - (img.nComps > 256 ? 5 : 4);
	img.tiles[tileIdx].tileComps[comp].quantSteps =
	    (Guint *)greallocn(img.tiles[tileIdx].tileComps[comp].quantSteps,
			       img.tiles[tileIdx].tileComps[comp].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[tileIdx].tileComps[comp].nQuantSteps; ++i) {
	  if (!readUByte(&img.tiles[tileIdx].tileComps[comp].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCC marker segment");
	    return gFalse;
	  }
	}
      } else if ((img.tiles[tileIdx].tileComps[comp].quantStyle & 0x1f)
		 == 0x01) {
	img.tiles[tileIdx].tileComps[comp].nQuantSteps = 1;
	img.tiles[tileIdx].tileComps[comp].quantSteps =
	    (Guint *)greallocn(img.tiles[tileIdx].tileComps[comp].quantSteps,
			       img.tiles[tileIdx].tileComps[comp].nQuantSteps,
			       sizeof(Guint));
	if (!readUWord(&img.tiles[tileIdx].tileComps[comp].quantSteps[0])) {
	  error(getPos(), "Error in JPX QCC marker segment");
	  return gFalse;
	}
      } else if ((img.tiles[tileIdx].tileComps[comp].quantStyle & 0x1f)
		 == 0x02) {
	img.tiles[tileIdx].tileComps[comp].nQuantSteps =
	    (segLen - (img.nComps > 256 ? 5 : 4)) / 2;
	img.tiles[tileIdx].tileComps[comp].quantSteps =
	    (Guint *)greallocn(img.tiles[tileIdx].tileComps[comp].quantSteps,
			       img.tiles[tileIdx].tileComps[comp].nQuantSteps,
			       sizeof(Guint));
	for (i = 0; i < img.tiles[tileIdx].tileComps[comp].nQuantSteps; ++i) {
	  if (!readUWord(&img.tiles[tileIdx].tileComps[comp].quantSteps[i])) {
	    error(getPos(), "Error in JPX QCD marker segment");
	    return gFalse;
	  }
	}
      } else {
	error(getPos(), "Error in JPX QCC marker segment");
	return gFalse;
      }
      break;
    case 0x5e:			// RGN - region of interest
      cover(38);
#if 1 //~ ROI is unimplemented
      fprintf(stderr, "RGN\n");
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PPM marker segment");
	  return gFalse;
	}
      }
#else
      if ((img.nComps > 256 && !readUWord(&comp)) ||
	  (img.nComps <= 256 && !readUByte(&comp)) ||
	  comp >= img.nComps ||
	  !readUByte(&compInfo[comp].roi.style) ||
	  !readUByte(&compInfo[comp].roi.shift)) {
	error(getPos(), "Error in JPX RGN marker segment");
	return gFalse;
      }
#endif
      break;
    case 0x5f:			// POC - progression order change
      cover(39);
#if 1 //~ progression order changes are unimplemented
      fprintf(stderr, "POC\n");
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PPM marker segment");
	  return gFalse;
	}
      }
#else
      nTileProgs = (segLen - 2) / (img.nComps > 256 ? 9 : 7);
      tileProgs = (JPXProgOrder *)gmallocn(nTileProgs, sizeof(JPXProgOrder));
      for (i = 0; i < nTileProgs; ++i) {
	if (!readUByte(&tileProgs[i].startRes) ||
	    !(img.nComps > 256 && readUWord(&tileProgs[i].startComp)) ||
	    !(img.nComps <= 256 && readUByte(&tileProgs[i].startComp)) ||
	    !readUWord(&tileProgs[i].endLayer) ||
	    !readUByte(&tileProgs[i].endRes) ||
	    !(img.nComps > 256 && readUWord(&tileProgs[i].endComp)) ||
	    !(img.nComps <= 256 && readUByte(&tileProgs[i].endComp)) ||
	    !readUByte(&tileProgs[i].progOrder)) {
	  error(getPos(), "Error in JPX POC marker segment");
	  return gFalse;
	}
      }
#endif
      break;
    case 0x61:			// PPT - packed packet headers, tile-part hdr
      cover(40);
#if 1 //~ packed packet headers are unimplemented
      fprintf(stderr, "PPT\n");
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PPT marker segment");
	  return gFalse;
	}
      }
#endif
    case 0x58:			// PLT - packet length, tile-part header
      // skipped
      cover(41);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX PLT marker segment");
	  return gFalse;
	}
      }
      break;
    case 0x64:			// COM - comment
      // skipped
      cover(42);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  error(getPos(), "Error in JPX COM marker segment");
	  return gFalse;
	}
      }
      break;
    case 0x93:			// SOD - start of data
      cover(43);
      haveSOD = gTrue;
      break;
    default:
      cover(44);
      error(getPos(), "Unknown marker segment %02x in JPX tile-part stream",
	    segType);
      for (i = 0; i < segLen - 2; ++i) {
	if (str->getChar() == EOF) {
	  break;
	}
      }
      break;
    }
  } while (!haveSOD);

  //----- initialize the tile, precincts, and code-blocks
  if (tilePartIdx == 0) {
    tile = &img.tiles[tileIdx];
    i = tileIdx / img.nXTiles;
    j = tileIdx % img.nXTiles;
    if ((tile->x0 = img.xTileOffset + j * img.xTileSize) < img.xOffset) {
      tile->x0 = img.xOffset;
    }
    if ((tile->y0 = img.yTileOffset + i * img.yTileSize) < img.yOffset) {
      tile->y0 = img.yOffset;
    }
    if ((tile->x1 = img.xTileOffset + (j + 1) * img.xTileSize) > img.xSize) {
      tile->x1 = img.xSize;
    }
    if ((tile->y1 = img.yTileOffset + (i + 1) * img.yTileSize) > img.ySize) {
      tile->y1 = img.ySize;
    }
    tile->comp = 0;
    tile->res = 0;
    tile->precinct = 0;
    tile->layer = 0;
    tile->maxNDecompLevels = 0;
    for (comp = 0; comp < img.nComps; ++comp) {
      tileComp = &tile->tileComps[comp];
      if (tileComp->nDecompLevels > tile->maxNDecompLevels) {
	tile->maxNDecompLevels = tileComp->nDecompLevels;
      }
      tileComp->x0 = jpxCeilDiv(tile->x0, tileComp->hSep);
      tileComp->y0 = jpxCeilDiv(tile->y0, tileComp->hSep);
      tileComp->x1 = jpxCeilDiv(tile->x1, tileComp->hSep);
      tileComp->y1 = jpxCeilDiv(tile->y1, tileComp->hSep);
      tileComp->cbW = 1 << tileComp->codeBlockW;
      tileComp->cbH = 1 << tileComp->codeBlockH;
      tileComp->data = (int *)gmallocn((tileComp->x1 - tileComp->x0) *
				       (tileComp->y1 - tileComp->y0),
				       sizeof(int));
      if (tileComp->x1 - tileComp->x0 > tileComp->y1 - tileComp->y0) {
	n = tileComp->x1 - tileComp->x0;
      } else {
	n = tileComp->y1 - tileComp->y0;
      }
      tileComp->buf = (int *)gmallocn(n + 8, sizeof(int));
      for (r = 0; r <= tileComp->nDecompLevels; ++r) {
	resLevel = &tileComp->resLevels[r];
	k = r == 0 ? tileComp->nDecompLevels
	           : tileComp->nDecompLevels - r + 1;
	resLevel->x0 = jpxCeilDivPow2(tileComp->x0, k);
	resLevel->y0 = jpxCeilDivPow2(tileComp->y0, k);
	resLevel->x1 = jpxCeilDivPow2(tileComp->x1, k);
	resLevel->y1 = jpxCeilDivPow2(tileComp->y1, k);
	if (r == 0) {
	  resLevel->bx0[0] = resLevel->x0;
	  resLevel->by0[0] = resLevel->y0;
	  resLevel->bx1[0] = resLevel->x1;
	  resLevel->by1[0] = resLevel->y1;
	} else {
	  resLevel->bx0[0] = jpxCeilDivPow2(tileComp->x0 - (1 << (k-1)), k);
	  resLevel->by0[0] = resLevel->y0;
	  resLevel->bx1[0] = jpxCeilDivPow2(tileComp->x1 - (1 << (k-1)), k);
	  resLevel->by1[0] = resLevel->y1;
	  resLevel->bx0[1] = resLevel->x0;
	  resLevel->by0[1] = jpxCeilDivPow2(tileComp->y0 - (1 << (k-1)), k);
	  resLevel->bx1[1] = resLevel->x1;
	  resLevel->by1[1] = jpxCeilDivPow2(tileComp->y1 - (1 << (k-1)), k);
	  resLevel->bx0[2] = jpxCeilDivPow2(tileComp->x0 - (1 << (k-1)), k);
	  resLevel->by0[2] = jpxCeilDivPow2(tileComp->y0 - (1 << (k-1)), k);
	  resLevel->bx1[2] = jpxCeilDivPow2(tileComp->x1 - (1 << (k-1)), k);
	  resLevel->by1[2] = jpxCeilDivPow2(tileComp->y1 - (1 << (k-1)), k);
	}
	resLevel->precincts = (JPXPrecinct *)gmallocn(1, sizeof(JPXPrecinct));
	for (pre = 0; pre < 1; ++pre) {
	  precinct = &resLevel->precincts[pre];
	  precinct->x0 = resLevel->x0;
	  precinct->y0 = resLevel->y0;
	  precinct->x1 = resLevel->x1;
	  precinct->y1 = resLevel->y1;
	  nSBs = r == 0 ? 1 : 3;
	  precinct->subbands =
	      (JPXSubband *)gmallocn(nSBs, sizeof(JPXSubband));
	  for (sb = 0; sb < nSBs; ++sb) {
	    subband = &precinct->subbands[sb];
	    subband->x0 = resLevel->bx0[sb];
	    subband->y0 = resLevel->by0[sb];
	    subband->x1 = resLevel->bx1[sb];
	    subband->y1 = resLevel->by1[sb];
	    subband->nXCBs = jpxCeilDivPow2(subband->x1,
					    tileComp->codeBlockW)
	                     - jpxFloorDivPow2(subband->x0,
					       tileComp->codeBlockW);
	    subband->nYCBs = jpxCeilDivPow2(subband->y1,
					    tileComp->codeBlockH)
	                     - jpxFloorDivPow2(subband->y0,
					       tileComp->codeBlockH);
	    n = subband->nXCBs > subband->nYCBs ? subband->nXCBs
	                                        : subband->nYCBs;
	    for (subband->maxTTLevel = 0, --n;
		 n;
		 ++subband->maxTTLevel, n >>= 1) ;
	    n = 0;
	    for (level = subband->maxTTLevel; level >= 0; --level) {
	      nx = jpxCeilDivPow2(subband->nXCBs, level);
	      ny = jpxCeilDivPow2(subband->nYCBs, level);
	      n += nx * ny;
	    }
	    subband->inclusion =
	        (JPXTagTreeNode *)gmallocn(n, sizeof(JPXTagTreeNode));
	    subband->zeroBitPlane =
	        (JPXTagTreeNode *)gmallocn(n, sizeof(JPXTagTreeNode));
	    for (k = 0; k < n; ++k) {
	      subband->inclusion[k].finished = gFalse;
	      subband->inclusion[k].val = 0;
	      subband->zeroBitPlane[k].finished = gFalse;
	      subband->zeroBitPlane[k].val = 0;
	    }
	    subband->cbs = (JPXCodeBlock *)gmallocn(subband->nXCBs *
						      subband->nYCBs,
						    sizeof(JPXCodeBlock));
	    sbx0 = jpxFloorDivPow2(subband->x0, tileComp->codeBlockW);
	    sby0 = jpxFloorDivPow2(subband->y0, tileComp->codeBlockH);
	    cb = subband->cbs;
	    for (cbY = 0; cbY < subband->nYCBs; ++cbY) {
	      for (cbX = 0; cbX < subband->nXCBs; ++cbX) {
		cb->x0 = (sbx0 + cbX) << tileComp->codeBlockW;
		cb->x1 = cb->x0 + tileComp->cbW;
		if (subband->x0 > cb->x0) {
		  cb->x0 = subband->x0;
		}
		if (subband->x1 < cb->x1) {
		  cb->x1 = subband->x1;
		}
		cb->y0 = (sby0 + cbY) << tileComp->codeBlockH;
		cb->y1 = cb->y0 + tileComp->cbH;
		if (subband->y0 > cb->y0) {
		  cb->y0 = subband->y0;
		}
		if (subband->y1 < cb->y1) {
		  cb->y1 = subband->y1;
		}
		cb->seen = gFalse;
		cb->lBlock = 3;
		cb->nextPass = jpxPassCleanup;
		cb->nZeroBitPlanes = 0;
		cb->coeffs =
		    (JPXCoeff *)gmallocn((1 << (tileComp->codeBlockW
						+ tileComp->codeBlockH)),
					 sizeof(JPXCoeff));
		for (cbi = 0;
		     cbi < (Guint)(1 << (tileComp->codeBlockW
					 + tileComp->codeBlockH));
		     ++cbi) {
		  cb->coeffs[cbi].flags = 0;
		  cb->coeffs[cbi].len = 0;
		  cb->coeffs[cbi].mag = 0;
		}
		cb->arithDecoder = NULL;
		cb->stats = NULL;
		++cb;
	      }
	    }
	  }
	}
      }
    }
  }

  return readTilePartData(tileIdx, tilePartLen, tilePartToEOC);
}

GBool JPXStream::readTilePartData(Guint tileIdx,
				  Guint tilePartLen, GBool tilePartToEOC) {
  JPXTile *tile;
  JPXTileComp *tileComp;
  JPXResLevel *resLevel;
  JPXPrecinct *precinct;
  JPXSubband *subband;
  JPXCodeBlock *cb;
  Guint ttVal;
  Guint bits, cbX, cbY, nx, ny, i, j, n, sb;
  int level;

  tile = &img.tiles[tileIdx];

  // read all packets from this tile-part
  while (1) {
    if (tilePartToEOC) {
      //~ peek for an EOC marker
      cover(93);
    } else if (tilePartLen == 0) {
      break;
    }

    tileComp = &tile->tileComps[tile->comp];
    resLevel = &tileComp->resLevels[tile->res];
    precinct = &resLevel->precincts[tile->precinct];

    //----- packet header

    // setup
    startBitBuf(tilePartLen);

    // zero-length flag
    if (!readBits(1, &bits)) {
      goto err;
    }
    if (!bits) {
      // packet is empty -- clear all code-block inclusion flags
      cover(45);
      for (sb = 0; sb < (Guint)(tile->res == 0 ? 1 : 3); ++sb) {
	subband = &precinct->subbands[sb];
	for (cbY = 0; cbY < subband->nYCBs; ++cbY) {
	  for (cbX = 0; cbX < subband->nXCBs; ++cbX) {
	    cb = &subband->cbs[cbY * subband->nXCBs + cbX];
	    cb->included = gFalse;
	  }
	}
      }
    } else {

      for (sb = 0; sb < (Guint)(tile->res == 0 ? 1 : 3); ++sb) {
	subband = &precinct->subbands[sb];
	for (cbY = 0; cbY < subband->nYCBs; ++cbY) {
	  for (cbX = 0; cbX < subband->nXCBs; ++cbX) {
	    cb = &subband->cbs[cbY * subband->nXCBs + cbX];

	    // skip code-blocks with no coefficients
	    if (cb->x0 >= cb->x1 || cb->y0 >= cb->y1) {
	      cover(46);
	      cb->included = gFalse;
	      continue;
	    }

	    // code-block inclusion
	    if (cb->seen) {
	      cover(47);
	      if (!readBits(1, &cb->included)) {
		goto err;
	      }
	    } else {
	      cover(48);
	      ttVal = 0;
	      i = 0;
	      for (level = subband->maxTTLevel; level >= 0; --level) {
		nx = jpxCeilDivPow2(subband->nXCBs, level);
		ny = jpxCeilDivPow2(subband->nYCBs, level);
		j = i + (cbY >> level) * nx + (cbX >> level);
		if (!subband->inclusion[j].finished &&
		    !subband->inclusion[j].val) {
		  subband->inclusion[j].val = ttVal;
		} else {
		  ttVal = subband->inclusion[j].val;
		}
		while (!subband->inclusion[j].finished &&
		       ttVal <= tile->layer) {
		  if (!readBits(1, &bits)) {
		    goto err;
		  }
		  if (bits == 1) {
		    subband->inclusion[j].finished = gTrue;
		  } else {
		    ++ttVal;
		  }
		}
		subband->inclusion[j].val = ttVal;
		if (ttVal > tile->layer) {
		  break;
		}
		i += nx * ny;
	      }
	      cb->included = level < 0;
	    }

	    if (cb->included) {
	      cover(49);

	      // zero bit-plane count
	      if (!cb->seen) {
		cover(50);
		ttVal = 0;
		i = 0;
		for (level = subband->maxTTLevel; level >= 0; --level) {
		  nx = jpxCeilDivPow2(subband->nXCBs, level);
		  ny = jpxCeilDivPow2(subband->nYCBs, level);
		  j = i + (cbY >> level) * nx + (cbX >> level);
		  if (!subband->zeroBitPlane[j].finished &&
		      !subband->zeroBitPlane[j].val) {
		    subband->zeroBitPlane[j].val = ttVal;
		  } else {
		    ttVal = subband->zeroBitPlane[j].val;
		  }
		  while (!subband->zeroBitPlane[j].finished) {
		    if (!readBits(1, &bits)) {
		      goto err;
		    }
		    if (bits == 1) {
		      subband->zeroBitPlane[j].finished = gTrue;
		    } else {
		      ++ttVal;
		    }
		  }
		  subband->zeroBitPlane[j].val = ttVal;
		  i += nx * ny;
		}
		cb->nZeroBitPlanes = ttVal;
	      }

	      // number of coding passes
	      if (!readBits(1, &bits)) {
		goto err;
	      }
	      if (bits == 0) {
		cover(51);
		cb->nCodingPasses = 1;
	      } else {
		if (!readBits(1, &bits)) {
		  goto err;
		}
		if (bits == 0) {
		  cover(52);
		  cb->nCodingPasses = 2;
		} else {
		  cover(53);
		  if (!readBits(2, &bits)) {
		    goto err;
		  }
		  if (bits < 3) {
		    cover(54);
		    cb->nCodingPasses = 3 + bits;
		  } else {
		    cover(55);
		    if (!readBits(5, &bits)) {
		      goto err;
		    }
		    if (bits < 31) {
		      cover(56);
		      cb->nCodingPasses = 6 + bits;
		    } else {
		      cover(57);
		      if (!readBits(7, &bits)) {
			goto err;
		      }
		      cb->nCodingPasses = 37 + bits;
		    }
		  }
		}
	      }

	      // update Lblock
	      while (1) {
		if (!readBits(1, &bits)) {
		  goto err;
		}
		if (!bits) {
		  break;
		}
		++cb->lBlock;
	      }

	      // length of compressed data
	      //~ deal with multiple codeword segments
	      for (n = cb->lBlock, i = cb->nCodingPasses >> 1;
		   i;
		   ++n, i >>= 1) ;
	      if (!readBits(n, &cb->dataLen)) {
		goto err;
	      }
	    }
	  }
	}
      }
    }
    tilePartLen = finishBitBuf();

    //----- packet data

    for (sb = 0; sb < (Guint)(tile->res == 0 ? 1 : 3); ++sb) {
      subband = &precinct->subbands[sb];
      for (cbY = 0; cbY < subband->nYCBs; ++cbY) {
	for (cbX = 0; cbX < subband->nXCBs; ++cbX) {
	  cb = &subband->cbs[cbY * subband->nXCBs + cbX];
	  if (cb->included) {
	    if (!readCodeBlockData(tileComp, resLevel, precinct, subband,
				   tile->res, sb, cb)) {
	      return gFalse;
	    }
	    tilePartLen -= cb->dataLen;
	    cb->seen = gTrue;
	  }
	}
      }
    }

    //----- next packet

    switch (tile->progOrder) {
    case 0: // layer, resolution level, component, precinct
      cover(58);
      if (++tile->comp == img.nComps) {
	tile->comp = 0;
	if (++tile->res == tile->maxNDecompLevels + 1) {
	  tile->res = 0;
	  if (++tile->layer == tile->nLayers) {
	    tile->layer = 0;
	  }
	}
      }
      break;
    case 1: // resolution level, layer, component, precinct
      cover(59);
      if (++tile->comp == img.nComps) {
	tile->comp = 0;
	if (++tile->layer == tile->nLayers) {
	  tile->layer = 0;
	  if (++tile->res == tile->maxNDecompLevels + 1) {
	    tile->res = 0;
	  }
	}
      }
      break;
    case 2: // resolution level, precinct, component, layer
      //~ this isn't correct -- see B.12.1.3
      cover(60);
      if (++tile->layer == tile->nLayers) {
	tile->layer = 0;
	if (++tile->comp == img.nComps) {
	  tile->comp = 0;
	  if (++tile->res == tile->maxNDecompLevels + 1) {
	    tile->res = 0;
	  }
	}
      }
      break;
    case 3: // precinct, component, resolution level, layer
      //~ this isn't correct -- see B.12.1.4
      cover(61);
      if (++tile->layer == tile->nLayers) {
	tile->layer = 0;
	if (++tile->res == tile->maxNDecompLevels + 1) {
	  tile->res = 0;
	  if (++tile->comp == img.nComps) {
	    tile->comp = 0;
	  }
	}
      }
      break;
    case 4: // component, precinct, resolution level, layer
      //~ this isn't correct -- see B.12.1.5
      cover(62);
      if (++tile->layer == tile->nLayers) {
	tile->layer = 0;
	if (++tile->res == tile->maxNDecompLevels + 1) {
	  tile->res = 0;
	  if (++tile->comp == img.nComps) {
	    tile->comp = 0;
	  }
	}
      }
      break;
    }
  }

  return gTrue;

 err:
  error(getPos(), "Error in JPX stream");
  return gFalse;
}

GBool JPXStream::readCodeBlockData(JPXTileComp *tileComp,
				   JPXResLevel *resLevel,
				   JPXPrecinct *precinct,
				   JPXSubband *subband,
				   Guint res, Guint sb,
				   JPXCodeBlock *cb) {
  JPXCoeff *coeff0, *coeff1, *coeff;
  Guint horiz, vert, diag, all, cx, xorBit;
  int horizSign, vertSign;
  Guint i, x, y0, y1, y2;

  if (cb->arithDecoder) {
    cover(63);
    cb->arithDecoder->restart(cb->dataLen);
  } else {
    cover(64);
    cb->arithDecoder = new JArithmeticDecoder();
    cb->arithDecoder->setStream(str, cb->dataLen);
    cb->arithDecoder->start();
    cb->stats = new JArithmeticDecoderStats(jpxNContexts);
    cb->stats->setEntry(jpxContextSigProp, 4, 0);
    cb->stats->setEntry(jpxContextRunLength, 3, 0);
    cb->stats->setEntry(jpxContextUniform, 46, 0);
  }

  for (i = 0; i < cb->nCodingPasses; ++i) {
    switch (cb->nextPass) {

    //----- significance propagation pass
    case jpxPassSigProp:
      cover(65);
      for (y0 = cb->y0, coeff0 = cb->coeffs;
	   y0 < cb->y1;
	   y0 += 4, coeff0 += 4 << tileComp->codeBlockW) {
	for (x = cb->x0, coeff1 = coeff0;
	     x < cb->x1;
	     ++x, ++coeff1) {
	  for (y1 = 0, coeff = coeff1;
	       y1 < 4 && y0+y1 < cb->y1;
	       ++y1, coeff += tileComp->cbW) {
	    if (!(coeff->flags & jpxCoeffSignificant)) {
	      horiz = vert = diag = 0;
	      horizSign = vertSign = 2;
	      if (x > cb->x0) {
		if (coeff[-1].flags & jpxCoeffSignificant) {
		  ++horiz;
		  horizSign += (coeff[-1].flags & jpxCoeffSign) ? -1 : 1;
		}
		if (y0+y1 > cb->y0) {
		  diag += (coeff[-(int)tileComp->cbW - 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
		if (y0+y1 < cb->y1 - 1) {
		  diag += (coeff[tileComp->cbW - 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
	      }
	      if (x < cb->x1 - 1) {
		if (coeff[1].flags & jpxCoeffSignificant) {
		  ++horiz;
		  horizSign += (coeff[1].flags & jpxCoeffSign) ? -1 : 1;
		}
		if (y0+y1 > cb->y0) {
		  diag += (coeff[-(int)tileComp->cbW + 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
		if (y0+y1 < cb->y1 - 1) {
		  diag += (coeff[tileComp->cbW + 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
	      }
	      if (y0+y1 > cb->y0) {
		if (coeff[-(int)tileComp->cbW].flags & jpxCoeffSignificant) {
		  ++vert;
		  vertSign += (coeff[-(int)tileComp->cbW].flags & jpxCoeffSign)
		              ? -1 : 1;
		}
	      }
	      if (y0+y1 < cb->y1 - 1) {
		if (coeff[tileComp->cbW].flags & jpxCoeffSignificant) {
		  ++vert;
		  vertSign += (coeff[tileComp->cbW].flags & jpxCoeffSign)
		              ? -1 : 1;
		}
	      }
	      cx = sigPropContext[horiz][vert][diag][res == 0 ? 1 : sb];
	      if (cx != 0) {
		if (cb->arithDecoder->decodeBit(cx, cb->stats)) {
		  coeff->flags |= jpxCoeffSignificant | jpxCoeffFirstMagRef;
		  coeff->mag = (coeff->mag << 1) | 1;
		  cx = signContext[horizSign][vertSign][0];
		  xorBit = signContext[horizSign][vertSign][1];
		  if (cb->arithDecoder->decodeBit(cx, cb->stats) ^ xorBit) {
		    coeff->flags |= jpxCoeffSign;
		  }
		}
		++coeff->len;
		coeff->flags |= jpxCoeffTouched;
	      }
	    }
	  }
	}
      }
      ++cb->nextPass;
      break;

    //----- magnitude refinement pass
    case jpxPassMagRef:
      cover(66);
      for (y0 = cb->y0, coeff0 = cb->coeffs;
	   y0 < cb->y1;
	   y0 += 4, coeff0 += 4 << tileComp->codeBlockW) {
	for (x = cb->x0, coeff1 = coeff0;
	     x < cb->x1;
	     ++x, ++coeff1) {
	  for (y1 = 0, coeff = coeff1;
	       y1 < 4 && y0+y1 < cb->y1;
	       ++y1, coeff += tileComp->cbW) {
	    if ((coeff->flags & jpxCoeffSignificant) &&
		!(coeff->flags & jpxCoeffTouched)) {
	      if (coeff->flags & jpxCoeffFirstMagRef) {
		all = 0;
		if (x > cb->x0) {
		  all += (coeff[-1].flags >> jpxCoeffSignificantB) & 1;
		  if (y0+y1 > cb->y0) {
		    all += (coeff[-(int)tileComp->cbW - 1].flags
			    >> jpxCoeffSignificantB) & 1;
		  }
		  if (y0+y1 < cb->y1 - 1) {
		    all += (coeff[tileComp->cbW - 1].flags
			    >> jpxCoeffSignificantB) & 1;
		  }
		}
		if (x < cb->x1 - 1) {
		  all += (coeff[1].flags >> jpxCoeffSignificantB) & 1;
		  if (y0+y1 > cb->y0) {
		    all += (coeff[-(int)tileComp->cbW + 1].flags
			    >> jpxCoeffSignificantB) & 1;
		  }
		  if (y0+y1 < cb->y1 - 1) {
		    all += (coeff[tileComp->cbW + 1].flags
			    >> jpxCoeffSignificantB) & 1;
		  }
		}
		if (y0+y1 > cb->y0) {
		  all += (coeff[-(int)tileComp->cbW].flags
			  >> jpxCoeffSignificantB) & 1;
		}
		if (y0+y1 < cb->y1 - 1) {
		  all += (coeff[tileComp->cbW].flags
			  >> jpxCoeffSignificantB) & 1;
		}
		cx = all ? 15 : 14;
	      } else {
		cx = 16;
	      }
	      coeff->mag = (coeff->mag << 1) |
		           cb->arithDecoder->decodeBit(cx, cb->stats);
	      ++coeff->len;
	      coeff->flags |= jpxCoeffTouched;
	      coeff->flags &= ~jpxCoeffFirstMagRef;
	    }
	  }
	}
      }
      ++cb->nextPass;
      break;

    //----- cleanup pass
    case jpxPassCleanup:
      cover(67);
      for (y0 = cb->y0, coeff0 = cb->coeffs;
	   y0 < cb->y1;
	   y0 += 4, coeff0 += 4 << tileComp->codeBlockW) {
	for (x = cb->x0, coeff1 = coeff0;
	     x < cb->x1;
	     ++x, ++coeff1) {
	  y1 = 0;
	  if (y0 + 3 < cb->y1 &&
	      !(coeff1->flags & jpxCoeffTouched) &&
	      !(coeff1[tileComp->cbW].flags & jpxCoeffTouched) &&
	      !(coeff1[2 * tileComp->cbW].flags & jpxCoeffTouched) &&
	      !(coeff1[3 * tileComp->cbW].flags & jpxCoeffTouched) &&
	      (x == cb->x0 || y0 == cb->y0 ||
	       !(coeff1[-(int)tileComp->cbW - 1].flags
		 & jpxCoeffSignificant)) &&
	      (y0 == cb->y0 ||
	       !(coeff1[-(int)tileComp->cbW].flags
		 & jpxCoeffSignificant)) &&
	      (x == cb->x1 - 1 || y0 == cb->y0 ||
	       !(coeff1[-(int)tileComp->cbW + 1].flags
		 & jpxCoeffSignificant)) &&
	      (x == cb->x0 ||
	       (!(coeff1[-1].flags & jpxCoeffSignificant) &&
		!(coeff1[tileComp->cbW - 1].flags
		  & jpxCoeffSignificant) &&
		!(coeff1[2 * tileComp->cbW - 1].flags
		  & jpxCoeffSignificant) && 
		!(coeff1[3 * tileComp->cbW - 1].flags
		  & jpxCoeffSignificant))) &&
	      (x == cb->x1 - 1 ||
	       (!(coeff1[1].flags & jpxCoeffSignificant) &&
		!(coeff1[tileComp->cbW + 1].flags
		  & jpxCoeffSignificant) &&
		!(coeff1[2 * tileComp->cbW + 1].flags
		  & jpxCoeffSignificant) &&
		!(coeff1[3 * tileComp->cbW + 1].flags
		  & jpxCoeffSignificant))) &&
	      (x == cb->x0 || y0+4 == cb->y1 ||
	       !(coeff1[4 * tileComp->cbW - 1].flags & jpxCoeffSignificant)) &&
	      (y0+4 == cb->y1 ||
	       !(coeff1[4 * tileComp->cbW].flags & jpxCoeffSignificant)) &&
	      (x == cb->x1 - 1 || y0+4 == cb->y1 ||
	       !(coeff1[4 * tileComp->cbW + 1].flags
		 & jpxCoeffSignificant))) {
	    if (cb->arithDecoder->decodeBit(jpxContextRunLength, cb->stats)) {
	      y1 = cb->arithDecoder->decodeBit(jpxContextUniform, cb->stats);
	      y1 = (y1 << 1) |
		   cb->arithDecoder->decodeBit(jpxContextUniform, cb->stats);
	      for (y2 = 0, coeff = coeff1;
		   y2 < y1;
		   ++y2, coeff += tileComp->cbW) {
		++coeff->len;
	      }
	      coeff->flags |= jpxCoeffSignificant | jpxCoeffFirstMagRef;
	      coeff->mag = (coeff->mag << 1) | 1;
	      ++coeff->len;
	      cx = signContext[2][2][0];
	      xorBit = signContext[2][2][1];
	      if (cb->arithDecoder->decodeBit(cx, cb->stats) ^ xorBit) {
		coeff->flags |= jpxCoeffSign;
	      }
	      ++y1;
	    } else {
	      for (y1 = 0, coeff = coeff1;
		   y1 < 4;
		   ++y1, coeff += tileComp->cbW) {
		++coeff->len;
	      }
	      y1 = 4;
	    }
	  }
	  for (coeff = &coeff1[y1 << tileComp->codeBlockW];
	       y1 < 4 && y0 + y1 < cb->y1;
	       ++y1, coeff += tileComp->cbW) {
	    if (!(coeff->flags & jpxCoeffTouched)) {
	      horiz = vert = diag = 0;
	      horizSign = vertSign = 2;
	      if (x > cb->x0) {
		if (coeff[-1].flags & jpxCoeffSignificant) {
		  ++horiz;
		  horizSign += (coeff[-1].flags & jpxCoeffSign) ? -1 : 1;
		}
		if (y0+y1 > cb->y0) {
		  diag += (coeff[-(int)tileComp->cbW - 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
		if (y0+y1 < cb->y1 - 1) {
		  diag += (coeff[tileComp->cbW - 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
	      }
	      if (x < cb->x1 - 1) {
		if (coeff[1].flags & jpxCoeffSignificant) {
		  ++horiz;
		  horizSign += (coeff[1].flags & jpxCoeffSign) ? -1 : 1;
		}
		if (y0+y1 > cb->y0) {
		  diag += (coeff[-(int)tileComp->cbW + 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
		if (y0+y1 < cb->y1 - 1) {
		  diag += (coeff[tileComp->cbW + 1].flags
			   >> jpxCoeffSignificantB) & 1;
		}
	      }
	      if (y0+y1 > cb->y0) {
		if (coeff[-(int)tileComp->cbW].flags & jpxCoeffSignificant) {
		  ++vert;
		  vertSign += (coeff[-(int)tileComp->cbW].flags & jpxCoeffSign)
		              ? -1 : 1;
		}
	      }
	      if (y0+y1 < cb->y1 - 1) {
		if (coeff[tileComp->cbW].flags & jpxCoeffSignificant) {
		  ++vert;
		  vertSign += (coeff[tileComp->cbW].flags & jpxCoeffSign)
		              ? -1 : 1;
		}
	      }
	      cx = sigPropContext[horiz][vert][diag][res == 0 ? 1 : sb];
	      if (cb->arithDecoder->decodeBit(cx, cb->stats)) {
		coeff->flags |= jpxCoeffSignificant | jpxCoeffFirstMagRef;
		coeff->mag = (coeff->mag << 1) | 1;
		cx = signContext[horizSign][vertSign][0];
		xorBit = signContext[horizSign][vertSign][1];
		if (cb->arithDecoder->decodeBit(cx, cb->stats) ^ xorBit) {
		  coeff->flags |= jpxCoeffSign;
		}
	      }
	      ++coeff->len;
	    } else {
	      coeff->flags &= ~jpxCoeffTouched;
	    }
	  }
	}
      }
      cb->nextPass = jpxPassSigProp;
      break;
    }
  }

  cb->arithDecoder->cleanup();
  return gTrue;
}

// Inverse quantization, and wavelet transform (IDWT).  This also does
// the initial shift to convert to fixed point format.
void JPXStream::inverseTransform(JPXTileComp *tileComp) {
  JPXResLevel *resLevel;
  JPXPrecinct *precinct;
  JPXSubband *subband;
  JPXCodeBlock *cb;
  JPXCoeff *coeff0, *coeff;
  Guint qStyle, guard, eps, shift;
  int shift2;
  double mu;
  int val;
  int *dataPtr;
  Guint nx0, ny0, nx1, ny1;
  Guint r, cbX, cbY, x, y;

  cover(68);

  //----- (NL)LL subband (resolution level 0)

  resLevel = &tileComp->resLevels[0];
  precinct = &resLevel->precincts[0];
  subband = &precinct->subbands[0];

  // i-quant parameters
  qStyle = tileComp->quantStyle & 0x1f;
  guard = (tileComp->quantStyle >> 5) & 7;
  if (qStyle == 0) {
    cover(69);
    eps = (tileComp->quantSteps[0] >> 3) & 0x1f;
    shift = guard + eps - 1;
    mu = 0; // make gcc happy
  } else {
    cover(70);
    shift = guard - 1 + tileComp->prec;
    mu = (double)(0x800 + (tileComp->quantSteps[0] & 0x7ff)) / 2048.0;
  }
  if (tileComp->transform == 0) {
    cover(71);
    shift += fracBits;
  }

  // copy (NL)LL into the upper-left corner of the data array, doing
  // the fixed point adjustment and dequantization along the way
  cb = subband->cbs;
  for (cbY = 0; cbY < subband->nYCBs; ++cbY) {
    for (cbX = 0; cbX < subband->nXCBs; ++cbX) {
      for (y = cb->y0, coeff0 = cb->coeffs;
	   y < cb->y1;
	   ++y, coeff0 += tileComp->cbW) {
	dataPtr = &tileComp->data[(y - subband->y0)
				  * (tileComp->x1 - tileComp->x0)
				  + (cb->x0 - subband->x0)];
	for (x = cb->x0, coeff = coeff0; x < cb->x1; ++x, ++coeff) {
	  val = (int)coeff->mag;
	  if (val != 0) {
	    shift2 = shift - (cb->nZeroBitPlanes + coeff->len);
	    if (shift2 > 0) {
	      cover(94);
	      val = (val << shift2) + (1 << (shift2 - 1));
	    } else {
	      cover(95);
	      val >>= -shift2;
	    }
	    if (qStyle == 0) {
	      cover(96);
	      if (tileComp->transform == 0) {
		cover(97);
		val &= -1 << fracBits;
	      }
	    } else {
	      cover(98);
	      val = (int)((double)val * mu);
	    }
	    if (coeff->flags & jpxCoeffSign) {
	      cover(99);
	      val = -val;
	    }
	  }
	  *dataPtr++ = val;
	}
      }
      ++cb;
    }
  }

  //----- IDWT for each level

  for (r = 1; r <= tileComp->nDecompLevels; ++r) {
    resLevel = &tileComp->resLevels[r];

    // (n)LL is already in the upper-left corner of the
    // tile-component data array -- interleave with (n)HL/LH/HH
    // and inverse transform to get (n-1)LL, which will be stored
    // in the upper-left corner of the tile-component data array
    if (r == tileComp->nDecompLevels) {
      cover(72);
      nx0 = tileComp->x0;
      ny0 = tileComp->y0;
      nx1 = tileComp->x1;
      ny1 = tileComp->y1;
    } else {
      cover(73);
      nx0 = tileComp->resLevels[r+1].x0;
      ny0 = tileComp->resLevels[r+1].y0;
      nx1 = tileComp->resLevels[r+1].x1;
      ny1 = tileComp->resLevels[r+1].y1;
    }
    inverseTransformLevel(tileComp, r, resLevel, nx0, ny0, nx1, ny1);
  }
}

// Do one level of the inverse transform:
// - take (n)LL from the tile-component data array
// - take (n)HL/LH/HH from <resLevel>
// - leave the resulting (n-1)LL in the tile-component data array
void JPXStream::inverseTransformLevel(JPXTileComp *tileComp,
				      Guint r, JPXResLevel *resLevel,
				      Guint nx0, Guint ny0,
				      Guint nx1, Guint ny1) {
  JPXPrecinct *precinct;
  JPXSubband *subband;
  JPXCodeBlock *cb;
  JPXCoeff *coeff0, *coeff;
  Guint qStyle, guard, eps, shift, t;
  int shift2;
  double mu;
  int val;
  int *dataPtr;
  Guint xo, yo;
  Guint x, y, sb, cbX, cbY;
  int xx, yy;

  //----- interleave

  // spread out LL
  for (yy = resLevel->y1 - 1; yy >= (int)resLevel->y0; --yy) {
    for (xx = resLevel->x1 - 1; xx >= (int)resLevel->x0; --xx) {
      tileComp->data[(2 * yy - ny0) * (tileComp->x1 - tileComp->x0)
		     + (2 * xx - nx0)] =
	  tileComp->data[(yy - resLevel->y0) * (tileComp->x1 - tileComp->x0)
			 + (xx - resLevel->x0)];
    }
  }

  // i-quant parameters
  qStyle = tileComp->quantStyle & 0x1f;
  guard = (tileComp->quantStyle >> 5) & 7;

  // interleave HL/LH/HH
  precinct = &resLevel->precincts[0];
  for (sb = 0; sb < 3; ++sb) {

    // i-quant parameters
    if (qStyle == 0) {
      cover(100);
      eps = (tileComp->quantSteps[3*r - 2 + sb] >> 3) & 0x1f;
      shift = guard + eps - 1;
      mu = 0; // make gcc happy
    } else {
      cover(101);
      shift = guard + tileComp->prec;
      if (sb == 2) {
	cover(102);
	++shift;
      }
      t = tileComp->quantSteps[qStyle == 1 ? 0 : (3*r - 2 + sb)];
      mu = (double)(0x800 + (t & 0x7ff)) / 2048.0;
    }
    if (tileComp->transform == 0) {
      cover(103);
      shift += fracBits;
    }

    // copy the subband coefficients into the data array, doing the
    // fixed point adjustment and dequantization along the way
    xo = (sb & 1) ? 0 : 1;
    yo = (sb > 0) ? 1 : 0;
    subband = &precinct->subbands[sb];
    cb = subband->cbs;
    for (cbY = 0; cbY < subband->nYCBs; ++cbY) {
      for (cbX = 0; cbX < subband->nXCBs; ++cbX) {
	for (y = cb->y0, coeff0 = cb->coeffs;
	     y < cb->y1;
	     ++y, coeff0 += tileComp->cbW) {
	  dataPtr = &tileComp->data[(2 * y + yo - ny0)
				    * (tileComp->x1 - tileComp->x0)
				    + (2 * cb->x0 + xo - nx0)];
	  for (x = cb->x0, coeff = coeff0; x < cb->x1; ++x, ++coeff) {
	    val = (int)coeff->mag;
	    if (val != 0) {
	      shift2 = shift - (cb->nZeroBitPlanes + coeff->len);
	      if (shift2 > 0) {
		cover(74);
		val = (val << shift2) + (1 << (shift2 - 1));
	      } else {
		cover(75);
		val >>= -shift2;
	      }
	      if (qStyle == 0) {
		cover(76);
		if (tileComp->transform == 0) {
		  val &= -1 << fracBits;
		}
	      } else {
		cover(77);
		val = (int)((double)val * mu);
	      }
	      if (coeff->flags & jpxCoeffSign) {
		cover(78);
		val = -val;
	      }
	    }
	    *dataPtr = val;
	    dataPtr += 2;
	  }
	}
	++cb;
      }
    }
  }

  //----- horizontal (row) transforms
  dataPtr = tileComp->data;
  for (y = 0; y < ny1 - ny0; ++y) {
    inverseTransform1D(tileComp, dataPtr, 1, nx0, nx1);
    dataPtr += tileComp->x1 - tileComp->x0;
  }

  //----- vertical (column) transforms
  dataPtr = tileComp->data;
  for (x = 0; x < nx1 - nx0; ++x) {
    inverseTransform1D(tileComp, dataPtr,
		       tileComp->x1 - tileComp->x0, ny0, ny1);
    ++dataPtr;
  }
}

void JPXStream::inverseTransform1D(JPXTileComp *tileComp,
				   int *data, Guint stride,
				   Guint i0, Guint i1) {
  int *buf;
  Guint offset, end, i;

  //----- special case for length = 1
  if (i1 - i0 == 1) {
    cover(79);
    if (i0 & 1) {
      cover(104);
      *data >>= 1;
    }

  } else {
    cover(80);

    // choose an offset: this makes even buf[] indexes correspond to
    // odd values of i, and vice versa
    offset = 3 + (i0 & 1);
    end = offset + i1 - i0;

    //----- gather
    buf = tileComp->buf;
    for (i = 0; i < i1 - i0; ++i) {
      buf[offset + i] = data[i * stride];
    }

    //----- extend right
    buf[end] = buf[end - 2];
    if (i1 - i0 == 2) {
      cover(81);
      buf[end+1] = buf[offset + 1];
      buf[end+2] = buf[offset];
      buf[end+3] = buf[offset + 1];
    } else {
      cover(82);
      buf[end+1] = buf[end - 3];
      if (i1 - i0 == 3) {
	cover(105);
	buf[end+2] = buf[offset + 1];
	buf[end+3] = buf[offset + 2];
      } else {
	cover(106);
	buf[end+2] = buf[end - 4];
	if (i1 - i0 == 4) {
	  cover(107);
	  buf[end+3] = buf[offset + 1];
	} else {
	  cover(108);
	  buf[end+3] = buf[end - 5];
	}
      }
    }

    //----- extend left
    buf[offset - 1] = buf[offset + 1];
    buf[offset - 2] = buf[offset + 2];
    buf[offset - 3] = buf[offset + 3];
    if (offset == 4) {
      cover(83);
      buf[0] = buf[offset + 4];
    }

    //----- 9-7 irreversible filter

    if (tileComp->transform == 0) {
      cover(84);
      // step 1 (even)
      for (i = 1; i <= end + 2; i += 2) {
	buf[i] = (int)(idwtKappa * buf[i]);
      }
      // step 2 (odd)
      for (i = 0; i <= end + 3; i += 2) {
	buf[i] = (int)(idwtIKappa * buf[i]);
      }
      // step 3 (even)
      for (i = 1; i <= end + 2; i += 2) {
	buf[i] = (int)(buf[i] - idwtDelta * (buf[i-1] + buf[i+1]));
      }
      // step 4 (odd)
      for (i = 2; i <= end + 1; i += 2) {
	buf[i] = (int)(buf[i] - idwtGamma * (buf[i-1] + buf[i+1]));
      }
      // step 5 (even)
      for (i = 3; i <= end; i += 2) {
	buf[i] = (int)(buf[i] - idwtBeta * (buf[i-1] + buf[i+1]));
      }
      // step 6 (odd)
      for (i = 4; i <= end - 1; i += 2) {
	buf[i] = (int)(buf[i] - idwtAlpha * (buf[i-1] + buf[i+1]));
      }

    //----- 5-3 reversible filter

    } else {
      cover(85);
      // step 1 (even)
      for (i = 3; i <= end; i += 2) {
	buf[i] -= (buf[i-1] + buf[i+1] + 2) >> 2;
      }
      // step 2 (odd)
      for (i = 4; i < end; i += 2) {
	buf[i] += (buf[i-1] + buf[i+1]) >> 1;
      }
    }

    //----- scatter
    for (i = 0; i < i1 - i0; ++i) {
      data[i * stride] = buf[offset + i];
    }
  }
}

// Inverse multi-component transform and DC level shift.  This also
// converts fixed point samples back to integers.
GBool JPXStream::inverseMultiCompAndDC(JPXTile *tile) {
  JPXTileComp *tileComp;
  int coeff, d0, d1, d2, t, minVal, maxVal, zeroVal;
  int *dataPtr;
  Guint j, comp, x, y;

  //----- inverse multi-component transform

  if (tile->multiComp == 1) {
    cover(86);
    if (img.nComps < 3 ||
	tile->tileComps[0].hSep != tile->tileComps[1].hSep ||
	tile->tileComps[0].vSep != tile->tileComps[1].vSep ||
	tile->tileComps[1].hSep != tile->tileComps[2].hSep ||
	tile->tileComps[1].vSep != tile->tileComps[2].vSep) {
      return gFalse;
    }

    // inverse irreversible multiple component transform
    if (tile->tileComps[0].transform == 0) {
      cover(87);
      j = 0;
      for (y = 0; y < tile->tileComps[0].y1 - tile->tileComps[0].y0; ++y) {
	for (x = 0; x < tile->tileComps[0].x1 - tile->tileComps[0].x0; ++x) {
	  d0 = tile->tileComps[0].data[j];
	  d1 = tile->tileComps[1].data[j];
	  d2 = tile->tileComps[2].data[j];
	  tile->tileComps[0].data[j] = (int)(d0 + 1.402 * d2 + 0.5);
	  tile->tileComps[1].data[j] =
	      (int)(d0 - 0.34413 * d1 - 0.71414 * d2 + 0.5);
	  tile->tileComps[2].data[j] = (int)(d0 + 1.772 * d1 + 0.5);
	  ++j;
	}
      }

    // inverse reversible multiple component transform
    } else {
      cover(88);
      j = 0;
      for (y = 0; y < tile->tileComps[0].y1 - tile->tileComps[0].y0; ++y) {
	for (x = 0; x < tile->tileComps[0].x1 - tile->tileComps[0].x0; ++x) {
	  d0 = tile->tileComps[0].data[j];
	  d1 = tile->tileComps[1].data[j];
	  d2 = tile->tileComps[2].data[j];
	  tile->tileComps[1].data[j] = t = d0 - ((d2 + d1) >> 2);
	  tile->tileComps[0].data[j] = d2 + t;
	  tile->tileComps[2].data[j] = d1 + t;
	  ++j;
	}
      }
    }
  }

  //----- DC level shift
  for (comp = 0; comp < img.nComps; ++comp) {
    tileComp = &tile->tileComps[comp];

    // signed: clip
    if (tileComp->sgned) {
      cover(89);
      minVal = -(1 << (tileComp->prec - 1));
      maxVal = (1 << (tileComp->prec - 1)) - 1;
      dataPtr = tileComp->data;
      for (y = 0; y < tileComp->y1 - tileComp->y0; ++y) {
	for (x = 0; x < tileComp->x1 - tileComp->x0; ++x) {
	  coeff = *dataPtr;
	  if (tileComp->transform == 0) {
	    cover(109);
	    coeff >>= fracBits;
	  }
	  if (coeff < minVal) {
	    cover(110);
	    coeff = minVal;
	  } else if (coeff > maxVal) {
	    cover(111);
	    coeff = maxVal;
	  }
	  *dataPtr++ = coeff;
	}
      }

    // unsigned: inverse DC level shift and clip
    } else {
      cover(90);
      maxVal = (1 << tileComp->prec) - 1;
      zeroVal = 1 << (tileComp->prec - 1);
      dataPtr = tileComp->data;
      for (y = 0; y < tileComp->y1 - tileComp->y0; ++y) {
	for (x = 0; x < tileComp->x1 - tileComp->x0; ++x) {
	  coeff = *dataPtr;
	  if (tileComp->transform == 0) {
	    cover(112);
	    coeff >>= fracBits;
	  }
	  coeff += zeroVal;
	  if (coeff < 0) {
	    cover(113);
	    coeff = 0;
	  } else if (coeff > maxVal) {
	    cover(114);
	    coeff = maxVal;
	  }
	  *dataPtr++ = coeff;
	}
      }
    }
  }

  return gTrue;
}

GBool JPXStream::readBoxHdr(Guint *boxType, Guint *boxLen, Guint *dataLen) {
  Guint len, lenH;

  if (!readULong(&len) ||
      !readULong(boxType)) {
    return gFalse;
  }
  if (len == 1) {
    if (!readULong(&lenH) || !readULong(&len)) {
      return gFalse;
    }
    if (lenH) {
      error(getPos(), "JPX stream contains a box larger than 2^32 bytes");
      return gFalse;
    }
    *boxLen = len;
    *dataLen = len - 16;
  } else if (len == 0) {
    *boxLen = 0;
    *dataLen = 0;
  } else {
    *boxLen = len;
    *dataLen = len - 8;
  }
  return gTrue;
}

int JPXStream::readMarkerHdr(int *segType, Guint *segLen) {
  int c;

  do {
    do {
      if ((c = str->getChar()) == EOF) {
	return gFalse;
      }
    } while (c != 0xff);
    do {
      if ((c = str->getChar()) == EOF) {
	return gFalse;
      }
    } while (c == 0xff);
  } while (c == 0x00);
  *segType = c;
  if ((c >= 0x30 && c <= 0x3f) ||
      c == 0x4f || c == 0x92 || c == 0x93 || c == 0xd9) {
    *segLen = 0;
    return gTrue;
  }
  return readUWord(segLen);
}

GBool JPXStream::readUByte(Guint *x) {
  int c0;

  if ((c0 = str->getChar()) == EOF) {
    return gFalse;
  }
  *x = (Guint)c0;
  return gTrue;
}

GBool JPXStream::readByte(int *x) {
 int c0;

  if ((c0 = str->getChar()) == EOF) {
    return gFalse;
  }
  *x = c0;
  if (c0 & 0x80) {
    *x |= -1 - 0xff;
  }
  return gTrue;
}

GBool JPXStream::readUWord(Guint *x) {
  int c0, c1;

  if ((c0 = str->getChar()) == EOF ||
      (c1 = str->getChar()) == EOF) {
    return gFalse;
  }
  *x = (Guint)((c0 << 8) | c1);
  return gTrue;
}

GBool JPXStream::readULong(Guint *x) {
  int c0, c1, c2, c3;

  if ((c0 = str->getChar()) == EOF ||
      (c1 = str->getChar()) == EOF ||
      (c2 = str->getChar()) == EOF ||
      (c3 = str->getChar()) == EOF) {
    return gFalse;
  }
  *x = (Guint)((c0 << 24) | (c1 << 16) | (c2 << 8) | c3);
  return gTrue;
}

GBool JPXStream::readNBytes(int nBytes, GBool signd, int *x) {
  int y, c, i;

  y = 0;
  for (i = 0; i < nBytes; ++i) {
    if ((c = str->getChar()) == EOF) {
      return gFalse;
    }
    y = (y << 8) + c;
  }
  if (signd) {
    if (y & (1 << (8 * nBytes - 1))) {
      y |= -1 << (8 * nBytes);
    }
  }
  *x = y;
  return gTrue;
}

GBool JPXStream::readBits(int nBits, Guint *x) {
  int c;

  while (bitBufLen < nBits) {
    if (byteCount == 0 || (c = str->getChar()) == EOF) {
      return gFalse;
    }
    --byteCount;
    if (bitBufSkip) {
      bitBuf = (bitBuf << 7) | (c & 0x7f);
      bitBufLen += 7;
    } else {
      bitBuf = (bitBuf << 8) | (c & 0xff);
      bitBufLen += 8;
    }
    bitBufSkip = c == 0xff;
  }
  *x = (bitBuf >> (bitBufLen - nBits)) & ((1 << nBits) - 1);
  bitBufLen -= nBits;
  return gTrue;
}

void JPXStream::startBitBuf(Guint byteCountA) {
  bitBufLen = 0;
  bitBufSkip = gFalse;
  byteCount = byteCountA;
}

Guint JPXStream::finishBitBuf() {
  if (bitBufSkip) {
    str->getChar();
    --byteCount;
  }
  return byteCount;
}
