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
// Copyright (C) 2006, 2009, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Harry Roberts <harry.roberts@midnight-labs.org>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
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
#include <stdlib.h>
#include <limits.h>
#include "goo/gmem.h"
#include "SplashErrorCodes.h"
#include "SplashBitmap.h"
#include "poppler/Error.h"
#include "goo/JpegWriter.h"
#include "goo/PNGWriter.h"
#include "goo/ImgWriter.h"

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

SplashBitmap::SplashBitmap(int widthA, int heightA, int rowPadA,
			   SplashColorMode modeA, GBool alphaA,
			   GBool topDown) {
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
#endif
  }
  if (rowSize > 0) {
    rowSize += rowPad - 1;
    rowSize -= rowSize % rowPad;
  }
  data = (SplashColorPtr)gmallocn(rowSize, height);
  if (!topDown) {
    data += (height - 1) * rowSize;
    rowSize = -rowSize;
  }
  if (alphaA) {
    alpha = (Guchar *)gmallocn(width, height);
  } else {
    alpha = NULL;
  }
}


SplashBitmap::~SplashBitmap() {
  if (rowSize < 0) {
    gfree(data + (height - 1) * rowSize);
  } else {
    gfree(data);
  }
  gfree(alpha);
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
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(*p, f);
	++p;
      }
      row += rowSize;
    }
    break;

  case splashModeRGB8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(splashRGB8R(p), f);
	fputc(splashRGB8G(p), f);
	fputc(splashRGB8B(p), f);
	p += 3;
      }
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
    // PNM doesn't support CMYK
    error(-1, "unsupported SplashBitmap mode");
    return splashErrGeneric;
    break;
#endif
  }
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
#endif
  }
}

Guchar SplashBitmap::getAlpha(int x, int y) {
  return alpha[y * width + x];
}

SplashError SplashBitmap::writeImgFile(SplashImageFileFormat format, char *fileName, int hDPI, int vDPI) {
  FILE *f;
  SplashError e;

  if (!(f = fopen(fileName, "wb"))) {
    return splashErrOpenFile;
  }

  e = writeImgFile(format, f, hDPI, vDPI);
  
  fclose(f);
  return e;
}

SplashError SplashBitmap::writeImgFile(SplashImageFileFormat format, FILE *f, int hDPI, int vDPI) {
  ImgWriter *writer;
	SplashError e;
  
  switch (format) {
    #ifdef ENABLE_LIBPNG
    case splashFormatPng:
	  writer = new PNGWriter();
      break;
    #endif

    #ifdef ENABLE_LIBJPEG
    case splashFormatJpeg:
      writer = new JpegWriter();
      break;
    #endif
	
    default:
      // Not the greatest error message, but users of this function should
      // have already checked whether their desired format is compiled in.
      error(-1, "Support for this image type not compiled in");
      return splashErrGeneric;
  }

	e = writeImgFile(writer, f, hDPI, vDPI);
	delete writer;
	return e;
}

SplashError SplashBitmap::writeImgFile(ImgWriter *writer, FILE *f, int hDPI, int vDPI) {
  if (mode != splashModeRGB8 && mode != splashModeMono8 && mode != splashModeMono1 && mode != splashModeXBGR8) {
    error(-1, "unsupported SplashBitmap mode");
    return splashErrGeneric;
  }

  if (!writer->init(f, width, height, hDPI, vDPI)) {
    return splashErrGeneric;
  }

  switch (mode) {
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
  
  if (writer->close()) {
    return splashErrGeneric;
  }

  return splashOk;
}
