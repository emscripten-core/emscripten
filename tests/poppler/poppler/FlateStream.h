//========================================================================
//
// FlateStream.h
//
// Copyright (C) 2005, Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2010, 2011, Albert Astals Cid <aacid@kde.org>
//
// This file is under the GPLv2 or later license
//
//========================================================================

#ifndef FLATESTREAM_H
#define FLATESTREAM_H

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
#include <zlib.h>
}

class FlateStream: public FilterStream {
public:

  FlateStream(Stream *strA, int predictor, int columns, int colors, int bits);
  virtual ~FlateStream();
  virtual StreamKind getKind() { return strFlate; }
  virtual void reset();
  virtual int getChar();
  virtual int lookChar();
  virtual int getRawChar();
  virtual void getRawChars(int nChars, int *buffer);
  virtual GooString *getPSFilter(int psLevel, const char *indent);
  virtual GBool isBinary(GBool last = gTrue);

private:
  inline int doGetRawChar() {
    if (fill_buffer())
      return EOF;

    return out_buf[out_pos++];
  }

  int fill_buffer(void);
  z_stream d_stream;
  StreamPredictor *pred;
  int status;
  /* in_buf currently needs to be 1 or we over read from EmbedStreams */
  unsigned char in_buf[1];
  unsigned char out_buf[4096];
  int out_pos;
  int out_buf_len;
};

#endif
