//========================================================================
//
// JPEG2000Stream.cc
//
// A JPX stream decoder using OpenJPEG
//
// Copyright 2008-2010, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright 2011 Daniel Glöckner <daniel-gl@gmx.net>
// Copyright 2013 Adrian Johnson <ajohnson@redneon.com>
//
// Licensed under GPLv2 or later
//
//========================================================================

#include "JPEG2000Stream.h"

#include "config.h"

JPXStream::JPXStream(Stream *strA) : FilterStream(strA)
{
  inited = gFalse;
  image = NULL;
  dinfo = NULL;
  npixels = 0;
  ncomps = 0;
}

JPXStream::~JPXStream() {
  delete str;
  close();
}

void JPXStream::reset() {
  counter = 0;
  ccounter = 0;
}

void JPXStream::close() {
  if (image != NULL) {
    opj_image_destroy(image);
    image = NULL;
    npixels = 0;
  }
  if (dinfo != NULL) {
    opj_destroy_decompress(dinfo);
    dinfo = NULL;
  }
}

Goffset JPXStream::getPos() {
  return counter * ncomps + ccounter;
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

  if (image) {
    npixels = image->comps[0].w * image->comps[0].h;
    ncomps = image->numcomps;
    for (int component = 0; component < ncomps; component++) {
      if (image->comps[component].data == NULL) {
        close();
        break;
      }
      unsigned char *cdata = (unsigned char *)image->comps[component].data;
      int adjust = 0;
      if (image->comps[component].prec > 8)
	adjust = image->comps[component].prec - 8;
      int sgndcorr = 0;
      if (image->comps[component].sgnd)
	sgndcorr = 1 << (image->comps[0].prec - 1);
      for (int i = 0; i < npixels; i++) {
	int r = image->comps[component].data[i];
	r += sgndcorr;
	if (adjust) {
	  r = (r >> adjust)+((r >> (adjust-1))%2);
	  if (unlikely(r > 255))
	    r = 255;
        }
	*(cdata++) = r;
      }
    }
  } else
    npixels = 0;

  counter = 0;
  ccounter = 0;
  inited = gTrue;
}

static void libopenjpeg_error_callback(const char *msg, void * /*client_data*/) {
  error(errSyntaxError, -1, "{0:s}", msg);
}

static void libopenjpeg_warning_callback(const char *msg, void * /*client_data*/) {
  error(errSyntaxWarning, -1, "{0:s}", msg);
}

void JPXStream::init2(unsigned char *buf, int bufLen, OPJ_CODEC_FORMAT format)
{
  opj_cio_t *cio = NULL;

  /* Use default decompression parameters */
  opj_dparameters_t parameters;
  opj_set_default_decoder_parameters(&parameters);
#ifdef WITH_OPENJPEG_IGNORE_PCLR_CMAP_CDEF_FLAG
  parameters.flags = OPJ_DPARAMETERS_IGNORE_PCLR_CMAP_CDEF_FLAG;
#endif

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
    error(errSyntaxWarning, -1, "Did no succeed opening JPX Stream as JP2, trying as J2K.");
    init2(buf, bufLen, CODEC_J2K);
  } else if (format == CODEC_J2K) {
    error(errSyntaxWarning, -1, "Did no succeed opening JPX Stream as J2K, trying as JPT.");
    init2(buf, bufLen, CODEC_JPT);
  } else {
    error(errSyntaxError, -1, "Did no succeed opening JPX Stream.");
  }
}

int JPXStream::lookChar() {
  return doLookChar();
}

GooString *JPXStream::getPSFilter(int psLevel, const char *indent) {
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

