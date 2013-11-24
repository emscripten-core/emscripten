//========================================================================
//
// JPEG2000Stream.h
//
// A JPX stream decoder using OpenJPEG
//
// Copyright 2008, 2010 Albert Astals Cid <aacid@kde.org>
// Copyright 2011 Daniel Glöckner <daniel-gl@gmx.net>
// Copyright 2013 Adrian Johnson <ajohnson@redneon.com>
//
// Licensed under GPLv2 or later
//
//========================================================================


#ifndef JPEG2000STREAM_H
#define JPEG2000STREAM_H

#include <openjpeg.h>

#include "goo/gtypes.h"
#include "Object.h"
#include "Stream.h"

class JPXStream: public FilterStream {
public:

  JPXStream(Stream *strA);
  virtual ~JPXStream();
  virtual StreamKind getKind() { return strJPX; }
  virtual void reset();
  virtual void close();
  virtual Goffset getPos();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, const char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual void getImageParams(int *bitsPerComponent, StreamColorSpaceMode *csMode);

private:
  void init();
  void init2(unsigned char *buf, int bufLen, OPJ_CODEC_FORMAT format);

  virtual GBool hasGetChars() { return true; }
  virtual int getChars(int nChars, Guchar *buffer);

  inline int doGetChar() {
    int result = doLookChar();
    if (++ccounter == ncomps) {
      ccounter = 0;
      ++counter;
    }
    return result;
  }

  inline int doLookChar() {
    if (unlikely(inited == gFalse)) init();

    if (unlikely(counter >= npixels)) return EOF;

    return ((unsigned char *)image->comps[ccounter].data)[counter];
  }

  opj_image_t *image;
  opj_dinfo_t *dinfo;
  int counter;
  int ccounter;
  int npixels;
  int ncomps;
  GBool inited;
};

#endif
