//========================================================================
//
// NetPBMWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2009, 2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010, 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Brian Cameron <brian.cameron@oracle.com>
// Copyright (C) 2011 Thomas Freitag <Thomas.Freitag@alfa.de>
//
//========================================================================

#ifndef NETPBMWRITER_H
#define NETPBMWRITER_H

#include "poppler-config.h"

#include "ImgWriter.h"

// Writer for the NetPBM formats (PBM and PPM)
// This format is documented at:
//   http://netpbm.sourceforge.net/doc/pbm.html
//   http://netpbm.sourceforge.net/doc/ppm.html

class NetPBMWriter : public ImgWriter
{
public:

  /* RGB        - 3 bytes/pixel
   * MONOCHROME - 8 pixels/byte
   */
  enum Format { RGB, MONOCHROME };

  NetPBMWriter(Format formatA = RGB);
  ~NetPBMWriter() {};

  bool init(FILE *f, int width, int height, int hDPI, int vDPI);

  bool writePointers(unsigned char **rowPointers, int rowCount);
  bool writeRow(unsigned char **row);

  bool close();

private:
  FILE *file;
  Format format;
  int width;
};

#endif
