//========================================================================
//
// JPXStream.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef JPXSTREAM_H
#define JPXSTREAM_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "Object.h"
#include "Stream.h"

class JArithmeticDecoder;
class JArithmeticDecoderStats;

//------------------------------------------------------------------------

enum JPXColorSpaceType {
  jpxCSBiLevel = 0,
  jpxCSYCbCr1 = 1,
  jpxCSYCbCr2 = 3,
  jpxCSYCBCr3 = 4,
  jpxCSPhotoYCC = 9,
  jpxCSCMY = 11,
  jpxCSCMYK = 12,
  jpxCSYCCK = 13,
  jpxCSCIELab = 14,
  jpxCSsRGB = 16,
  jpxCSGrayscale = 17,
  jpxCSBiLevel2 = 18,
  jpxCSCIEJab = 19,
  jpxCSCISesRGB = 20,
  jpxCSROMMRGB = 21,
  jpxCSsRGBYCbCr = 22,
  jpxCSYPbPr1125 = 23,
  jpxCSYPbPr1250 = 24
};

struct JPXColorSpecCIELab {
  Guint rl, ol, ra, oa, rb, ob, il;
};

struct JPXColorSpecEnumerated {
  JPXColorSpaceType type;	// color space type
  union {
    JPXColorSpecCIELab cieLab;
  };
};

struct JPXColorSpec {
  Guint meth;			// method
  int prec;			// precedence
  union {
    JPXColorSpecEnumerated enumerated;
  };
};

//------------------------------------------------------------------------

struct JPXPalette {
  Guint nEntries;		// number of entries in the palette
  Guint nComps;			// number of components in each entry
  Guint *bpc;			// bits per component, for each component
  int *c;			// color data:
				//   c[i*nComps+j] = entry i, component j
};

//------------------------------------------------------------------------

struct JPXCompMap {
  Guint nChannels;		// number of channels
  Guint *comp;			// codestream components mapped to each channel
  Guint *type;			// 0 for direct use, 1 for palette mapping
  Guint *pComp;			// palette components to use
};

//------------------------------------------------------------------------

struct JPXChannelDefn {
  Guint nChannels;		// number of channels
  Guint *idx;			// channel indexes
  Guint *type;			// channel types
  Guint *assoc;			// channel associations
};

//------------------------------------------------------------------------

struct JPXTagTreeNode {
  GBool finished;		// true if this node is finished
  Guint val;			// current value
};

//------------------------------------------------------------------------

struct JPXCoeff {
  Gushort flags;		// flag bits
  Gushort len;			// number of significant bits in mag
  Guint mag;			// magnitude value
};

// coefficient flags
#define jpxCoeffSignificantB  0
#define jpxCoeffTouchedB      1
#define jpxCoeffFirstMagRefB  2
#define jpxCoeffSignB         7
#define jpxCoeffSignificant   (1 << jpxCoeffSignificantB)
#define jpxCoeffTouched       (1 << jpxCoeffTouchedB)
#define jpxCoeffFirstMagRef   (1 << jpxCoeffFirstMagRefB)
#define jpxCoeffSign          (1 << jpxCoeffSignB)

//------------------------------------------------------------------------

struct JPXCodeBlock {
  //----- size
  Guint x0, y0, x1, y1;		// bounds

  //----- persistent state
  GBool seen;			// true if this code-block has already
				//   been seen
  Guint lBlock;			// base number of bits used for pkt data length
  Guint nextPass;		// next coding pass

  //---- info from first packet
  Guint nZeroBitPlanes;		// number of zero bit planes

  //----- info for the current packet
  Guint included;		// code-block inclusion in this packet:
				//   0=not included, 1=included
  Guint nCodingPasses;		// number of coding passes in this pkt
  Guint dataLen;		// pkt data length

  //----- coefficient data
  JPXCoeff *coeffs;		// the coefficients
  JArithmeticDecoder		// arithmetic decoder
    *arithDecoder;
  JArithmeticDecoderStats	// arithmetic decoder stats
    *stats;
};

//------------------------------------------------------------------------

struct JPXSubband {
  //----- computed
  Guint x0, y0, x1, y1;		// bounds
  Guint nXCBs, nYCBs;		// number of code-blocks in the x and y
				//   directions

  //----- tag trees
  Guint maxTTLevel;		// max tag tree level
  JPXTagTreeNode *inclusion;	// inclusion tag tree for each subband
  JPXTagTreeNode *zeroBitPlane;	// zero-bit plane tag tree for each
				//   subband

  //----- children
  JPXCodeBlock *cbs;		// the code-blocks (len = nXCBs * nYCBs)
};

//------------------------------------------------------------------------

struct JPXPrecinct {
  //----- computed
  Guint x0, y0, x1, y1;		// bounds of the precinct

  //----- children
  JPXSubband *subbands;		// the subbands
};

//------------------------------------------------------------------------

struct JPXResLevel {
  //----- from the COD and COC segments (main and tile)
  Guint precinctWidth;		// log2(precinct width)
  Guint precinctHeight;		// log2(precinct height)

  //----- computed
  Guint x0, y0, x1, y1;		// bounds of the tile-comp (for this res level)
  Guint bx0[3], by0[3],		// subband bounds
        bx1[3], by1[3];

  //---- children
  JPXPrecinct *precincts;	// the precincts
};

//------------------------------------------------------------------------

struct JPXTileComp {
  //----- from the SIZ segment
  GBool sgned;			// 1 for signed, 0 for unsigned
  Guint prec;			// precision, in bits
  Guint hSep;			// horizontal separation of samples
  Guint vSep;			// vertical separation of samples

  //----- from the COD and COC segments (main and tile)
  Guint style;			// coding style parameter (Scod / Scoc)
  Guint nDecompLevels;		// number of decomposition levels
  Guint codeBlockW;		// log2(code-block width)
  Guint codeBlockH;		// log2(code-block height)
  Guint codeBlockStyle;		// code-block style
  Guint transform;		// wavelet transformation

  //----- from the QCD and QCC segments (main and tile)
  Guint quantStyle;		// quantization style
  Guint *quantSteps;		// quantization step size for each subband
  Guint nQuantSteps;		// number of entries in quantSteps

  //----- computed
  Guint x0, y0, x1, y1;		// bounds of the tile-comp, in ref coords
  Guint cbW;			// code-block width
  Guint cbH;			// code-block height

  //----- image data
  int *data;			// the decoded image data
  int *buf;			// intermediate buffer for the inverse
				//   transform

  //----- children
  JPXResLevel *resLevels;	// the resolution levels
				//   (len = nDecompLevels + 1)
};

//------------------------------------------------------------------------

struct JPXTile {
  //----- from the COD segments (main and tile)
  Guint progOrder;		// progression order
  Guint nLayers;		// number of layers
  Guint multiComp;		// multiple component transformation

  //----- computed
  Guint x0, y0, x1, y1;		// bounds of the tile, in ref coords
  Guint maxNDecompLevels;	// max number of decomposition levels used
				//   in any component in this tile

  //----- progression order loop counters
  Guint comp;			//   component
  Guint res;			//   resolution level
  Guint precinct;		//   precinct
  Guint layer;			//   layer

  //----- children
  JPXTileComp *tileComps;	// the tile-components (len = JPXImage.nComps)
};

//------------------------------------------------------------------------

struct JPXImage {
  //----- from the SIZ segment
  Guint xSize, ySize;		// size of reference grid
  Guint xOffset, yOffset;	// image offset
  Guint xTileSize, yTileSize;	// size of tiles
  Guint xTileOffset,		// offset of first tile
        yTileOffset;
  Guint nComps;			// number of components

  //----- computed
  Guint nXTiles;		// number of tiles in x direction
  Guint nYTiles;		// number of tiles in y direction

  //----- children
  JPXTile *tiles;		// the tiles (len = nXTiles * nYTiles)
};

//------------------------------------------------------------------------

class JPXStream: public FilterStream {
public:

  JPXStream(Stream *strA);
  virtual ~JPXStream();
  virtual StreamKind getKind() { return strJPX; }
  virtual void reset();
  virtual void close();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual void getImageParams(int *bitsPerComponent,
			      StreamColorSpaceMode *csMode);

private:

  void fillReadBuf();
  void getImageParams2(int *bitsPerComponent, StreamColorSpaceMode *csMode);
  GBool readBoxes();
  GBool readColorSpecBox(Guint dataLen);
  GBool readCodestream(Guint len);
  GBool readTilePart();
  GBool readTilePartData(Guint tileIdx,
			 Guint tilePartLen, GBool tilePartToEOC);
  GBool readCodeBlockData(JPXTileComp *tileComp,
			  JPXResLevel *resLevel,
			  JPXPrecinct *precinct,
			  JPXSubband *subband,
			  Guint res, Guint sb,
			  JPXCodeBlock *cb);
  void inverseTransform(JPXTileComp *tileComp);
  void inverseTransformLevel(JPXTileComp *tileComp,
			     Guint r, JPXResLevel *resLevel,
			     Guint nx0, Guint ny0,
			     Guint nx1, Guint ny1);
  void inverseTransform1D(JPXTileComp *tileComp,
			  int *data, Guint stride,
			  Guint i0, Guint i1);
  GBool inverseMultiCompAndDC(JPXTile *tile);
  GBool readBoxHdr(Guint *boxType, Guint *boxLen, Guint *dataLen);
  int readMarkerHdr(int *segType, Guint *segLen);
  GBool readUByte(Guint *x);
  GBool readByte(int *x);
  GBool readUWord(Guint *x);
  GBool readULong(Guint *x);
  GBool readNBytes(int nBytes, GBool signd, int *x);
  GBool readBits(int nBits, Guint *x);
  void startBitBuf(Guint byteCountA);
  Guint finishBitBuf();

  Guint nComps;			// number of components
  Guint *bpc;			// bits per component, for each component
  Guint width, height;		// image size
  GBool haveImgHdr;		// set if a JP2/JPX image header has been
				//   found
  JPXColorSpec cs;		// color specification
  GBool haveCS;			// set if a color spec has been found
  JPXPalette palette;		// the palette
  GBool havePalette;		// set if a palette has been found
  JPXCompMap compMap;		// the component mapping
  GBool haveCompMap;		// set if a component mapping has been found
  JPXChannelDefn channelDefn;	// channel definition
  GBool haveChannelDefn;	// set if a channel defn has been found

  JPXImage img;			// JPEG2000 decoder data
  Guint bitBuf;			// buffer for bit reads
  int bitBufLen;		// number of bits in bitBuf
  GBool bitBufSkip;		// true if next bit should be skipped
				//   (for bit stuffing)
  Guint byteCount;		// number of available bytes left

  Guint curX, curY, curComp;	// current position for lookChar/getChar
  Guint readBuf;		// read buffer
  Guint readBufLen;		// number of valid bits in readBuf
};

#endif
