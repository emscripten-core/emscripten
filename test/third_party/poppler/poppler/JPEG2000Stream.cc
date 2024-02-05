//========================================================================
//
// JPEG2000Stream.cc
//
// A JPX stream decoder using OpenJPEG
//
// Copyright 2008-2010 Albert Astals Cid <aacid@kde.org>
//
// Licensed under GPLv2 or later
//
//========================================================================

#include "JPEG2000Stream.h"

JPXStream::JPXStream(Stream *strA) : FilterStream(strA)
{
  inited = gFalse;
  image = NULL;
  dinfo = NULL;
}

JPXStream::~JPXStream() {
  delete str;
  close();
}

void JPXStream::reset() {
  counter = 0;
}

void JPXStream::close() {
  if (image != NULL) {
    opj_image_destroy(image);
    image = NULL;
  }
  if (dinfo != NULL) {
    opj_destroy_decompress(dinfo);
    dinfo = NULL;
  }
}

int JPXStream::getPos() {
  return counter;
}

int JPXStream::getChars(int nChars, Guchar *buffer) {
  for (int i = 0; i < nChars; ++i) {
    const int c = doGetChar();
    if (likely(c != EOF)) buffer[i] = c;
    else return i;
  }
  return nChars;
}

int JPXStream::getChar() {
  return doGetChar();
}

#define BUFFER_INITIAL_SIZE 4096

void JPXStream::init()
{
  Object oLen;
  if (getDict()) getDict()->lookup("Length", &oLen);

  int bufSize = BUFFER_INITIAL_SIZE;
  if (oLen.isInt()) bufSize = oLen.getInt();
  oLen.free();

  
  int length = 0;
  unsigned char *buf = str->toUnsignedChars(&length, bufSize);
  init2(buf, length, CODEC_JP2);
  free(buf);

  counter = 0;
  inited = gTrue;
}

static void libopenjpeg_error_callback(const char *msg, void * /*client_data*/) {
  error(-1, "%s", msg);
}

static void libopenjpeg_warning_callback(const char *msg, void * /*client_data*/) {
  error(-1, "%s", msg);
}

void JPXStream::init2(unsigned char *buf, int bufLen, OPJ_CODEC_FORMAT format)
{
  opj_cio_t *cio = NULL;

  /* Use default decompression parameters */
  opj_dparameters_t parameters;
  opj_set_default_decoder_parameters(&parameters);

  /* Configure the event manager to receive errors and warnings */
  opj_event_mgr_t event_mgr;
  memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
  event_mgr.error_handler = libopenjpeg_error_callback;
  event_mgr.warning_handler = libopenjpeg_warning_callback;

  /* Get the decoder handle of the format */
  dinfo = opj_create_decompress(format);
  if (dinfo == NULL) goto error;

  /* Catch events using our callbacks */
  opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, NULL);

  /* Setup the decoder decoding parameters */
  opj_setup_decoder(dinfo, &parameters);

  /* Open a byte stream */
  cio = opj_cio_open((opj_common_ptr)dinfo, buf, bufLen);
  if (cio == NULL) goto error;

  /* Decode the stream and fill the image structure */
  image = opj_decode(dinfo, cio);

  /* Close the byte stream */
  opj_cio_close(cio);

  if (image == NULL) goto error;
  else return;

error:
  if (format == CODEC_JP2) {
    error(-1, "Did no succeed opening JPX Stream as JP2, trying as J2K.");
    init2(buf, bufLen, CODEC_J2K);
  } else if (format == CODEC_J2K) {
    error(-1, "Did no succeed opening JPX Stream as J2K, trying as JPT.");
    init2(buf, bufLen, CODEC_JPT);
  } else {
    error(-1, "Did no succeed opening JPX Stream.");
  }
}

int JPXStream::lookChar() {
  return doLookChar();
}

GooString *JPXStream::getPSFilter(int psLevel, char *indent) {
  return NULL;
}

GBool JPXStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}

void JPXStream::getImageParams(int *bitsPerComponent, StreamColorSpaceMode *csMode) {
  if (inited == gFalse) init();

  *bitsPerComponent = 8;
  if (image && image->numcomps == 3) *csMode = streamCSDeviceRGB;
  else *csMode = streamCSDeviceGray;
}

