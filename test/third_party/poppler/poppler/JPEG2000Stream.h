//========================================================================
//
// JPEG2000Stream.h
//
// A JPX stream decoder using OpenJPEG
//
// Copyright 2008, 2010 Albert Astals Cid <aacid@kde.org>
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
  virtual int getPos();
  virtual int getChar();
  virtual int lookChar();
  virtual GooString *getPSFilter(int psLevel, char *indent);
  virtual GBool isBinary(GBool last = gTrue);
  virtual void getImageParams(int *bitsPerComponent, StreamColorSpaceMode *csMode);

private:
  void init();
  void init2(unsigned char *buf, int bufLen, OPJ_CODEC_FORMAT format);

  virtual GBool hasGetChars() { return true; }
  virtual int getChars(int nChars, Guchar *buffer);

  inline int doGetChar() {
    int result = doLookChar();
    ++counter;
    return result;
  }

  inline int doLookChar() {
    if (inited == gFalse) init();

    if (!image) return EOF;

    int w = image->comps[0].w;
    int h = image->comps[0].h;

    int y = (counter / image->numcomps) / w;
    int x = (counter / image->numcomps) % w;
    if (y >= h) return EOF;

    int component = counter % image->numcomps;

    int adjust = 0;
    if (image->comps[component].prec > 8) {
      adjust = image->comps[component].prec - 8;
    }

    if (unlikely(image->comps[component].data == NULL)) return EOF;

    int r = image->comps[component].data[y * w + x];
    r += (image->comps[component].sgnd ? 1 << (image->comps[0].prec - 1) : 0);

    unsigned char rc = (unsigned char) ((r >> adjust)+((r >> (adjust-1))%2));

    return rc;
  }

  opj_image_t *image;
  opj_dinfo_t *dinfo;
  int counter;
  GBool inited;
};

#endif
