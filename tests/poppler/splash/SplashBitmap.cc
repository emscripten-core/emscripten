//========================================================================
//
// SplashBitmap.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2009, 2010, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010, 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Harry Roberts <harry.roberts@midnight-labs.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2010 William Bader <williambader@hotmail.com>
// Copyright (C) 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Anthony Wesley <awesley@smartnetworks.com.au>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "goo/gmem.h"
#include "SplashErrorCodes.h"
#include "SplashBitmap.h"
#include "poppler/Error.h"
#include "goo/JpegWriter.h"
#include "goo/PNGWriter.h"
#include "goo/TiffWriter.h"
#include "goo/ImgWriter.h"
#include "goo/GooList.h"

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

SplashBitmap::SplashBitmap(int widthA, int heightA, int rowPadA,
			   SplashColorMode modeA, GBool alphaA,
			   GBool topDown, GooList *separationListA) {
  width = widthA;
  height = heightA;
  mode = modeA;
  rowPad = rowPadA;
  switch (mode) {
  case splashModeMono1:
    if (width > 0) {
      rowSize = (width + 7) >> 3;
    } else {
      rowSize = -1;
    }
    break;
  case splashModeMono8:
    if (width > 0) {
      rowSize = width;
    } else {
      rowSize = -1;
    }
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    if (width > 0 && width <= INT_MAX / 3) {
      rowSize = width * 3;
    } else {
      rowSize = -1;
    }
    break;
  case splashModeXBGR8:
    if (width > 0 && width <= INT_MAX / 4) {
      rowSize = width * 4;
    } else {
      rowSize = -1;
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    if (width > 0 && width <= INT_MAX / 4) {
      rowSize = width * 4;
    } else {
      rowSize = -1;
    }
    break;
  case splashModeDeviceN8:
    if (width > 0 && width <= INT_MAX / 4) {
      rowSize = width * (SPOT_NCOMPS + 4);
    } else {
      rowSize = -1;
    }
    break;
#endif
  }
  if (rowSize > 0) {
    rowSize += rowPad - 1;
    rowSize -= rowSize % rowPad;
  }
  data = (SplashColorPtr)gmallocn_checkoverflow(rowSize, height);
  if (data != NULL) {
    if (!topDown) {
      data += (height - 1) * rowSize;
      rowSize = -rowSize;
    }
    if (alphaA) {
      alpha = (Guchar *)gmallocn(width, height);
    } else {
      alpha = NULL;
    }
  } else {
    alpha = NULL;
  }
  separationList = new GooList();
  if (separationListA != NULL)
    for (int i = 0; i < separationListA->getLength(); i++)
      separationList->append(((GfxSeparationColorSpace *) separationListA->get(i))->copy());
}

SplashBitmap *SplashBitmap::copy(SplashBitmap *src) {
  SplashBitmap *result = new SplashBitmap(src->getWidth(), src->getHeight(), src->getRowPad(), 
    src->getMode(), src->getAlphaPtr() != NULL, src->getRowSize() >= 0, src->getSeparationList());
  Guchar *dataSource = src->getDataPtr();
  Guchar *dataDest = result->getDataPtr();
  int amount = src->getRowSize();
  if (amount < 0) {
    dataSource = dataSource + (src->getHeight() - 1) * amount;
    dataDest = dataDest + (src->getHeight() - 1) * amount;
    amount *= -src->getHeight();
  } else {
    amount *= src->getHeight();
  }
  memcpy(dataDest, dataSource, amount);
  if (src->getAlphaPtr() != NULL) {
    memcpy(result->getAlphaPtr(), src->getAlphaPtr(), src->getWidth() * src->getHeight());
  }
  return result;
}

SplashBitmap::~SplashBitmap() {
  if (data) {
    if (rowSize < 0) {
      gfree(data + (height - 1) * rowSize);
    } else {
      gfree(data);
    }
  }
  gfree(alpha);
  deleteGooList(separationList, GfxSeparationColorSpace);
}


SplashError SplashBitmap::writePNMFile(char *fileName) {
  FILE *f;
  SplashError e;

  if (!(f = fopen(fileName, "wb"))) {
    return splashErrOpenFile;
  }

  e = this->writePNMFile(f);
  
  fclose(f);
  return e;
}


SplashError SplashBitmap::writePNMFile(FILE *f) {
  SplashColorPtr row, p;
  int x, y;

  switch (mode) {

  case splashModeMono1:
    fprintf(f, "P4\n%d %d\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; x += 8) {
	fputc(*p ^ 0xff, f);
	++p;
      }
      row += rowSize;
    }
    break;

  case splashModeMono8:
    fprintf(f, "P5\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      fwrite(row, 1, width, f);
      row += rowSize;
    }
    break;

  case splashModeRGB8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      fwrite(row, 1, 3 * width, f);
      row += rowSize;
    }
    break;

  case splashModeXBGR8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(splashBGR8R(p), f);
	fputc(splashBGR8G(p), f);
	fputc(splashBGR8B(p), f);
	p += 4;
      }
      row += rowSize;
    }
    break;


  case splashModeBGR8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(splashBGR8R(p), f);
	fputc(splashBGR8G(p), f);
	fputc(splashBGR8B(p), f);
	p += 3;
      }
      row += rowSize;
    }
    break;

#if SPLASH_CMYK
  case splashModeCMYK8:
  case splashModeDeviceN8:
    // PNM doesn't support CMYK
    error(errInternal, -1, "unsupported SplashBitmap mode");
    return splashErrGeneric;
    break;
#endif
  }
  return splashOk;
}

SplashError SplashBitmap::writeAlphaPGMFile(char *fileName) {
  FILE *f;

  if (!alpha) {
    return splashErrModeMismatch;
  }
  if (!(f = fopen(fileName, "wb"))) {
    return splashErrOpenFile;
  }
  fprintf(f, "P5\n%d %d\n255\n", width, height);
  fwrite(alpha, 1, width * height, f);
  fclose(f);
  return splashOk;
}

void SplashBitmap::getPixel(int x, int y, SplashColorPtr pixel) {
  SplashColorPtr p;

  if (y < 0 || y >= height || x < 0 || x >= width) {
    return;
  }
  switch (mode) {
  case splashModeMono1:
    p = &data[y * rowSize + (x >> 3)];
    pixel[0] = (p[0] & (0x80 >> (x & 7))) ? 0xff : 0x00;
    break;
  case splashModeMono8:
    p = &data[y * rowSize + x];
    pixel[0] = p[0];
    break;
  case splashModeRGB8:
    p = &data[y * rowSize + 3 * x];
    pixel[0] = p[0];
    pixel[1] = p[1];
    pixel[2] = p[2];
    break;
  case splashModeXBGR8:
    p = &data[y * rowSize + 4 * x];
    pixel[0] = p[2];
    pixel[1] = p[1];
    pixel[2] = p[0];
    pixel[3] = p[3];
    break;
  case splashModeBGR8:
    p = &data[y * rowSize + 3 * x];
    pixel[0] = p[2];
    pixel[1] = p[1];
    pixel[2] = p[0];
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    p = &data[y * rowSize + 4 * x];
    pixel[0] = p[0];
    pixel[1] = p[1];
    pixel[2] = p[2];
    pixel[3] = p[3];
    break;
  case splashModeDeviceN8:
    p = &data[y * rowSize + (SPOT_NCOMPS + 4) * x];
    for (int cp = 0; cp < SPOT_NCOMPS + 4; cp++)
      pixel[cp] = p[cp];
    break;
#endif
  }
}

Guchar SplashBitmap::getAlpha(int x, int y) {
  return alpha[y * width + x];
}

SplashColorPtr SplashBitmap::takeData() {
  SplashColorPtr data2;

  data2 = data;
  data = NULL;
  return data2;
}

SplashError SplashBitmap::writeImgFile(SplashImageFileFormat format, char *fileName, int hDPI, int vDPI, const char *compressionString) {
  FILE *f;
  SplashError e;

  if (!(f = fopen(fileName, "wb"))) {
    return splashErrOpenFile;
  }

  e = writeImgFile(format, f, hDPI, vDPI, compressionString);
  
  fclose(f);
  return e;
}

SplashError SplashBitmap::writeImgFile(SplashImageFileFormat format, FILE *f, int hDPI, int vDPI, const char *compressionString) {
  ImgWriter *writer;
	SplashError e;
  
  switch (format) {
    #ifdef ENABLE_LIBPNG
    case splashFormatPng:
	  writer = new PNGWriter();
      break;
    #endif

    #ifdef ENABLE_LIBJPEG
    #ifdef SPLASH_CMYK
    case splashFormatJpegCMYK:
      writer = new JpegWriter(JpegWriter::CMYK);
      break;
    #endif
    case splashFormatJpeg:
      writer = new JpegWriter();
      break;
    #endif
	
    #ifdef ENABLE_LIBTIFF
    case splashFormatTiff:
      switch (mode) {
      case splashModeMono1:
        writer = new TiffWriter(TiffWriter::MONOCHROME);
        break;
      case splashModeMono8:
        writer = new TiffWriter(TiffWriter::GRAY);
        break;
      case splashModeRGB8:
      case splashModeBGR8:
        writer = new TiffWriter(TiffWriter::RGB);
        break;
#if SPLASH_CMYK
      case splashModeCMYK8:
      case splashModeDeviceN8:
        writer = new TiffWriter(TiffWriter::CMYK);
        break;
#endif
      default:
        fprintf(stderr, "TiffWriter: Mode %d not supported\n", mode);
        writer = new TiffWriter();
      }
      if (writer) {
        ((TiffWriter *)writer)->setCompressionString(compressionString);
      }
      break;
    #endif

    default:
      // Not the greatest error message, but users of this function should
      // have already checked whether their desired format is compiled in.
      error(errInternal, -1, "Support for this image type not compiled in");
      return splashErrGeneric;
  }

	e = writeImgFile(writer, f, hDPI, vDPI);
	delete writer;
	return e;
}

#include "poppler/GfxState_helpers.h"

void SplashBitmap::getRGBLine(int yl, SplashColorPtr line) {
  SplashColor col;
  double c, m, y, k, c1, m1, y1, k1, r, g, b;

  for (int x = 0; x < width; x++) {
    getPixel(x, yl, col);
    c = byteToDbl(col[0]);
    m = byteToDbl(col[1]);
    y = byteToDbl(col[2]);
    k = byteToDbl(col[3]);
#if SPLASH_CMYK
    if (separationList->getLength() > 0) {
      for (int i = 0; i < separationList->getLength(); i++) {
        if (col[i+4] > 0) {
          GfxCMYK cmyk;
          GfxColor input;
          input.c[0] = byteToCol(col[i+4]);
          GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)separationList->get(i);
          sepCS->getCMYK(&input, &cmyk);
          col[0] = colToByte(cmyk.c);
          col[1] = colToByte(cmyk.m);
          col[2] = colToByte(cmyk.y);
          col[3] = colToByte(cmyk.k);
          c += byteToDbl(col[0]);
          m += byteToDbl(col[1]);
          y += byteToDbl(col[2]);
          k += byteToDbl(col[3]);
        }
      }
      if (c > 1) c = 1;
      if (m > 1) m = 1;
      if (y > 1) y = 1;
      if (k > 1) k = 1;
    }
#endif
    c1 = 1 - c;
    m1 = 1 - m;
    y1 = 1 - y;
    k1 = 1 - k;
    cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
    *line++ = dblToByte(clip01(r));
    *line++ = dblToByte(clip01(g));
    *line++ = dblToByte(clip01(b));
  }
}

void SplashBitmap::getXBGRLine(int yl, SplashColorPtr line) {
  SplashColor col;
  double c, m, y, k, c1, m1, y1, k1, r, g, b;

  for (int x = 0; x < width; x++) {
    getPixel(x, yl, col);
    c = byteToDbl(col[0]);
    m = byteToDbl(col[1]);
    y = byteToDbl(col[2]);
    k = byteToDbl(col[3]);
#if SPLASH_CMYK
    if (separationList->getLength() > 0) {
      for (int i = 0; i < separationList->getLength(); i++) {
        if (col[i+4] > 0) {
          GfxCMYK cmyk;
          GfxColor input;
          input.c[0] = byteToCol(col[i+4]);
          GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)separationList->get(i);
          sepCS->getCMYK(&input, &cmyk);
          col[0] = colToByte(cmyk.c);
          col[1] = colToByte(cmyk.m);
          col[2] = colToByte(cmyk.y);
          col[3] = colToByte(cmyk.k);
          c += byteToDbl(col[0]);
          m += byteToDbl(col[1]);
          y += byteToDbl(col[2]);
          k += byteToDbl(col[3]);
        }
      }
      if (c > 1) c = 1;
      if (m > 1) m = 1;
      if (y > 1) y = 1;
      if (k > 1) k = 1;
    }
#endif
    c1 = 1 - c;
    m1 = 1 - m;
    y1 = 1 - y;
    k1 = 1 - k;
    cmykToRGBMatrixMultiplication(c, m, y, k, c1, m1, y1, k1, r, g, b);
    *line++ = dblToByte(clip01(b));
    *line++ = dblToByte(clip01(g));
    *line++ = dblToByte(clip01(r));
    *line++ = 255;
  }
}

GBool SplashBitmap::convertToXBGR() {
  if (mode == splashModeXBGR8)
    return gTrue;
  
  int newrowSize = width * 4;
  SplashColorPtr newdata = (SplashColorPtr)gmallocn_checkoverflow(newrowSize, height);
  if (newdata != NULL) {
    for (int y = 0; y < height; y++) {
      unsigned char *row = newdata + y * newrowSize;
      getXBGRLine(y, row);
    }
    if (rowSize < 0) {
      gfree(data + (height - 1) * rowSize);
    } else {
      gfree(data);
    }
    data = newdata;
    rowSize = newrowSize;
    mode = splashModeXBGR8;
  }
  return newdata != NULL;
}

#if SPLASH_CMYK
void SplashBitmap::getCMYKLine(int yl, SplashColorPtr line) {
  SplashColor col;

  for (int x = 0; x < width; x++) {
    getPixel(x, yl, col);
    if (separationList->getLength() > 0) {
      double c, m, y, k;
      c = byteToDbl(col[0]);
      m = byteToDbl(col[1]);
      y = byteToDbl(col[2]);
      k = byteToDbl(col[3]);
      for (int i = 0; i < separationList->getLength(); i++) {
        if (col[i+4] > 0) {
          GfxCMYK cmyk;
          GfxColor input;
          input.c[0] = byteToCol(col[i+4]);
          GfxSeparationColorSpace *sepCS = (GfxSeparationColorSpace *)separationList->get(i);
          sepCS->getCMYK(&input, &cmyk);
          col[0] = colToByte(cmyk.c);
          col[1] = colToByte(cmyk.m);
          col[2] = colToByte(cmyk.y);
          col[3] = colToByte(cmyk.k);
          c += byteToDbl(col[0]);
          m += byteToDbl(col[1]);
          y += byteToDbl(col[2]);
          k += byteToDbl(col[3]);
        }
      }
      col[0] = dblToByte(clip01(c));
      col[1] = dblToByte(clip01(m));
      col[2] = dblToByte(clip01(y));
      col[3] = dblToByte(clip01(k));
    }
    *line++ = col[0];
    *line++ = col[1];
    *line++ = col[2];
    *line++ = col[3];
  }
}
#endif

SplashError SplashBitmap::writeImgFile(ImgWriter *writer, FILE *f, int hDPI, int vDPI) {
  if (mode != splashModeRGB8 && mode != splashModeMono8 && mode != splashModeMono1 && mode != splashModeXBGR8 && mode != splashModeBGR8
#if SPLASH_CMYK
      && mode != splashModeCMYK8 && mode != splashModeDeviceN8
#endif
     ) {
    error(errInternal, -1, "unsupported SplashBitmap mode");
    return splashErrGeneric;
  }

  if (!writer->init(f, width, height, hDPI, vDPI)) {
    return splashErrGeneric;
  }

  switch (mode) {
#if SPLASH_CMYK
    case splashModeCMYK8:
      if (writer->supportCMYK()) {
        SplashColorPtr row;
        unsigned char **row_pointers = new unsigned char*[height];
        row = data;

        for (int y = 0; y < height; ++y) {
          row_pointers[y] = row;
          row += rowSize;
        }
        if (!writer->writePointers(row_pointers, height)) {
          delete[] row_pointers;
          return splashErrGeneric;
        }
        delete[] row_pointers;
      } else {
        unsigned char *row = new unsigned char[3 * width];
        for (int y = 0; y < height; y++) {
          getRGBLine(y, row);
          if (!writer->writeRow(&row)) {
            delete[] row;
            return splashErrGeneric;
          }
        }
        delete[] row;
      }
    break;
    case splashModeDeviceN8:
      if (writer->supportCMYK()) {
        unsigned char *row = new unsigned char[4 * width];
        for (int y = 0; y < height; y++) {
          getCMYKLine(y, row);
          if (!writer->writeRow(&row)) {
            delete[] row;
            return splashErrGeneric;
          }
        }
        delete[] row;
      } else {
        unsigned char *row = new unsigned char[3 * width];
        for (int y = 0; y < height; y++) {
          getRGBLine(y, row);
          if (!writer->writeRow(&row)) {
            delete[] row;
            return splashErrGeneric;
          }
        }
        delete[] row;
      }
    break;
#endif
    case splashModeRGB8:
    {
      SplashColorPtr row;
      unsigned char **row_pointers = new unsigned char*[height];
      row = data;

      for (int y = 0; y < height; ++y) {
        row_pointers[y] = row;
        row += rowSize;
      }
      if (!writer->writePointers(row_pointers, height)) {
        delete[] row_pointers;
        return splashErrGeneric;
      }
      delete[] row_pointers;
    }
    break;
    
    case splashModeBGR8:
    {
      unsigned char *row = new unsigned char[3 * width];
      for (int y = 0; y < height; y++) {
        // Convert into a PNG row
        for (int x = 0; x < width; x++) {
          row[3*x] = data[y * rowSize + x * 3 + 2];
          row[3*x+1] = data[y * rowSize + x * 3 + 1];
          row[3*x+2] = data[y * rowSize + x * 3];
        }

        if (!writer->writeRow(&row)) {
          delete[] row;
          return splashErrGeneric;
        }
      }
      delete[] row;
    }
    break;
    
    case splashModeXBGR8:
    {
      unsigned char *row = new unsigned char[3 * width];
      for (int y = 0; y < height; y++) {
        // Convert into a PNG row
        for (int x = 0; x < width; x++) {
          row[3*x] = data[y * rowSize + x * 4 + 2];
          row[3*x+1] = data[y * rowSize + x * 4 + 1];
          row[3*x+2] = data[y * rowSize + x * 4];
        }

        if (!writer->writeRow(&row)) {
          delete[] row;
          return splashErrGeneric;
        }
      }
      delete[] row;
    }
    break;
    
    case splashModeMono8:
    {
      unsigned char *row = new unsigned char[3 * width];
      for (int y = 0; y < height; y++) {
        // Convert into a PNG row
        for (int x = 0; x < width; x++) {
          row[3*x] = data[y * rowSize + x];
          row[3*x+1] = data[y * rowSize + x];
          row[3*x+2] = data[y * rowSize + x];
        }

        if (!writer->writeRow(&row)) {
          delete[] row;
          return splashErrGeneric;
        }
      }
      delete[] row;
    }
    break;
    
    case splashModeMono1:
    {
      unsigned char *row = new unsigned char[3 * width];
      for (int y = 0; y < height; y++) {
        // Convert into a PNG row
        for (int x = 0; x < width; x++) {
          getPixel(x, y, &row[3*x]);
          row[3*x+1] = row[3*x];
          row[3*x+2] = row[3*x];
        }

        if (!writer->writeRow(&row)) {
          delete[] row;
          return splashErrGeneric;
        }
      }
      delete[] row;
    }
    break;
    
    default:
    // can't happen
    break;
  }
  
  if (!writer->close()) {
    return splashErrGeneric;
  }

  return splashOk;
}
