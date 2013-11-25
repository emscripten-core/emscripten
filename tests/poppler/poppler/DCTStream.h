//========================================================================
//
// DCTStream.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2005 Jeff Muizelaar <jeff@infidigm.net>
// Copyright 2005 Martin Kretzschmar <martink@gnome.org>
// Copyright 2005-2007, 2009-2011 Albert Astals Cid <aacid@kde.org>
// Copyright 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2011 Daiki Ueno <ueno@unixuser.org>
// Copyright 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
//
//========================================================================

#ifndef DCTSTREAM_H
#define DCTSTREAM_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif


#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "poppler-config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "Error.h"
#include "Object.h"
#include "Decrypt.h"
#include "Stream.h"

extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}

struct str_src_mgr {
    struct jpeg_source_mgr pub;
    JOCTET buffer;
    Stream *str;
    int index;
};

struct str_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
  int width;
  int height;
};

class DCTStream: public FilterStream {
public:

  DCTStream(Stream *strA, int colorXformA, Object *dict, int recursion);
  virtual ~DCTStream();
  virtual StreamKind getKind() { return strDCT; }
  virtual void reset();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, const char *indent);
  virtual GBool isBinary(GBool last = gTrue);

private:
  void init();

  virtual GBool hasGetChars() { return true; }
  virtual int getChars(int nChars, Guchar *buffer);

  int colorXform;
  JSAMPLE *current;
  JSAMPLE *limit;
  struct jpeg_decompress_struct cinfo;
  struct str_error_mgr err;
  struct str_src_mgr src;
  JSAMPARRAY row_buffer;
};

#endif 
