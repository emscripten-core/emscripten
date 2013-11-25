//========================================================================
//
// PNGWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Warren Toomey <wkt@tuhs.org>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009, 2011-2013 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010, 2011, 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Pino Toscano <pino@kde.org>
//
//========================================================================

#ifndef PNGWRITER_H
#define PNGWRITER_H

#include "poppler-config.h"

#ifdef ENABLE_LIBPNG

#include "ImgWriter.h"

struct PNGWriterPrivate;

class PNGWriter : public ImgWriter
{
public:

  /* RGB        - 3 bytes/pixel
   * RGBA       - 4 bytes/pixel
   * GRAY       - 1 byte/pixel
   * MONOCHROME - 8 pixels/byte
   */
  enum Format { RGB, RGBA, GRAY, MONOCHROME };

  PNGWriter(Format format = RGB);
  ~PNGWriter();

  void setICCProfile(const char *name, unsigned char *data, int size);
  void setSRGBProfile();


  bool init(FILE *f, int width, int height, int hDPI, int vDPI);

  bool writePointers(unsigned char **rowPointers, int rowCount);
  bool writeRow(unsigned char **row);

  bool close();

private:
  PNGWriter(const PNGWriter &other);
  PNGWriter& operator=(const PNGWriter &other);

  PNGWriterPrivate *priv;
};

#endif

#endif
