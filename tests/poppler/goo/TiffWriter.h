//========================================================================
//
// TiffWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2010, 2012 William Bader <williambader@hotmail.com>
// Copyright (C) 2011, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Pino Toscano <pino@kde.org>
//
//========================================================================

#ifndef TIFFWRITER_H
#define TIFFWRITER_H

#include "poppler-config.h"

#ifdef ENABLE_LIBTIFF

#include <sys/types.h>
#include "ImgWriter.h"

struct TiffWriterPrivate;

class TiffWriter : public ImgWriter
{
public:
  /* RGB                 - 3 bytes/pixel
   * RGBA_PREMULTIPLIED  - 4 bytes/pixel premultiplied by alpha
   * GRAY                - 1 byte/pixel
   * MONOCHROME          - 8 pixels/byte
   * CMYK                - 4 bytes/pixel
   */
  enum Format { RGB, RGBA_PREMULTIPLIED, GRAY, MONOCHROME, CMYK };

  TiffWriter(Format format = RGB);
  ~TiffWriter();

  void setCompressionString(const char *compressionStringArg);

  bool init(FILE *openedFile, int width, int height, int hDPI, int vDPI);

  bool writePointers(unsigned char **rowPointers, int rowCount);
  bool writeRow(unsigned char **rowData);

  bool supportCMYK() { return true; }

  bool close();

private:
  TiffWriter(const TiffWriter &other);
  TiffWriter& operator=(const TiffWriter &other);

  TiffWriterPrivate *priv;
};

#endif

#endif
