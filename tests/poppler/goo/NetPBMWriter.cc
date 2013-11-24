//========================================================================
//
// NetPBMWriter.h
//
// Copyright 1998-2003 Glyph & Cog, LLC
//
//========================================================================
//
//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2007, 2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Rainer Keller <class321@gmx.de>
// Copyright (C) 2008 Timothy Lee <timothy.lee@siriushk.com>
// Copyright (C) 2008 Vasile Gaburici <gaburici@cs.umd.edu>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 William Bader <williambader@hotmail.com>
// Copyright (C) 2010 Jakob Voss <jakob.voss@gbv.de>
// Copyright (C) 2012, 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Thomas Fischer <fischer@unix-ag.uni-kl.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "poppler-config.h"

#include "NetPBMWriter.h"

// Writer for the NetPBM formats (PBM and PPM)
// This format is documented at:
//   http://netpbm.sourceforge.net/doc/pbm.html
//   http://netpbm.sourceforge.net/doc/ppm.html

NetPBMWriter::NetPBMWriter(Format formatA) : format(formatA)
{
}

bool NetPBMWriter::init(FILE *f, int widthA, int heightA, int hDPI, int vDPI)
{
  file = f;
  width = widthA;
  if (format == MONOCHROME) {
    fprintf(file, "P4\n");
    fprintf(file, "%d %d\n", widthA, heightA);
  } else {
    fprintf(file, "P6\n");
    fprintf(file, "%d %d\n", widthA, heightA);
    fprintf(file, "255\n");
  }
  return true;
}

bool NetPBMWriter::writePointers(unsigned char **rowPointers, int rowCount)
{
  for (int i = 0; i < rowCount; i++)
    writeRow(&rowPointers[i]);
  return true;
}

bool NetPBMWriter::writeRow(unsigned char **row)
{
  if (format == MONOCHROME) {
    // PBM uses 0 = white, 1 = black so we need to invert the colors
    int size = (width + 7)/8;
    for (int i = 0; i < size; i++)
      fputc((*row)[i] ^ 0xff, file);
  } else {
    fwrite(*row, 1, width*3, file);
  }
  return true;
}


bool NetPBMWriter::close()
{
  return true;
}

