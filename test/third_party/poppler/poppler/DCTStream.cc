//========================================================================
//
// DCTStream.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2005 Jeff Muizelaar <jeff@infidigm.net>
// Copyright 2005-2010 Albert Astals Cid <aacid@kde.org>
// Copyright 2009 Ryszard Trojnacki <rysiek@menel.com>
// Copyright 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2011 Daiki Ueno <ueno@unixuser.org>
//
//========================================================================

#include "DCTStream.h"

static void str_init_source(j_decompress_ptr cinfo)
{
}

static boolean str_fill_input_buffer(j_decompress_ptr cinfo)
{
  int c;
  struct str_src_mgr * src = (struct str_src_mgr *)cinfo->src;
  if (src->index == 0) {
    c = 0xFF;
    src->index++;
  }
  else if (src->index == 1) {
    c = 0xD8;
    src->index++;
  }
  else c = src->str->getChar();
  if (c != EOF)
  {
    src->buffer = c;
    src->pub.next_input_byte = &src->buffer;
    src->pub.bytes_in_buffer = 1;
    return TRUE;
  }
  else return FALSE;
}

static void str_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  struct str_src_mgr * src = (struct str_src_mgr *)cinfo->src;
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      str_fill_input_buffer(cinfo);
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}

static void str_term_source(j_decompress_ptr cinfo)
{
}

DCTStream::DCTStream(Stream *strA, int colorXformA) :
  FilterStream(strA) {
  colorXform = colorXformA;
  init();
}

DCTStream::~DCTStream() {
  jpeg_destroy_decompress(&cinfo);
  delete str;
}

static void exitErrorHandler(jpeg_common_struct *error) {
  j_decompress_ptr cinfo = (j_decompress_ptr)error;
  str_error_mgr * err = (struct str_error_mgr *)cinfo->err;
  longjmp(err->setjmp_buffer, 1);
}

void DCTStream::init()
{
  jpeg_std_error(&err.pub);
  err.pub.error_exit = &exitErrorHandler;
  src.pub.init_source = str_init_source;
  src.pub.fill_input_buffer = str_fill_input_buffer;
  src.pub.skip_input_data = str_skip_input_data;
  src.pub.resync_to_restart = jpeg_resync_to_restart;
  src.pub.term_source = str_term_source;
  src.pub.bytes_in_buffer = 0;
  src.pub.next_input_byte = NULL;
  src.str = str;
  src.index = 0;
  current = NULL;
  limit = NULL;
  
  cinfo.err = &err.pub;
  if (!setjmp(err.setjmp_buffer)) {
    jpeg_create_decompress(&cinfo);
    cinfo.src = (jpeg_source_mgr *)&src;
  }
  row_buffer = NULL;
}

void DCTStream::reset() {
  int row_stride;

  str->reset();

  if (row_buffer)
  {
    jpeg_destroy_decompress(&cinfo);
    init();
  }

  // JPEG data has to start with 0xFF 0xD8
  // but some pdf like the one on 
  // https://bugs.freedesktop.org/show_bug.cgi?id=3299
  // does have some garbage before that this seeks for
  // the start marker...
  bool startFound = false;
  int c = 0, c2 = 0;
  while (!startFound)
  {
    if (!c)
    {
      c = str->getChar();
      if (c == -1)
      {
        error(-1, "Could not find start of jpeg data");
        return;
      }
      if (c != 0xFF) c = 0;
    }
    else
    {
      c2 = str->getChar();
      if (c2 != 0xD8)
      {
        c = 0;
        c2 = 0;
      }
      else startFound = true;
    }
  }

  if (!setjmp(err.setjmp_buffer)) {
    jpeg_read_header(&cinfo, TRUE);

    // figure out color transform
    if (colorXform == -1 && !cinfo.saw_Adobe_marker) {
      if (cinfo.num_components == 3) {
        if (cinfo.saw_JFIF_marker) {
	  colorXform = 1;
        } else if (cinfo.cur_comp_info[0]->component_id == 82 &&
		   cinfo.cur_comp_info[1]->component_id == 71 &&
		   cinfo.cur_comp_info[2]->component_id == 66) { // ASCII "RGB"
	  colorXform = 0;
	} else {
	  colorXform = 1;
	}
      } else {
        colorXform = 0;
      }
    } else if (cinfo.saw_Adobe_marker) {
      colorXform = cinfo.Adobe_transform;
    }

    switch (cinfo.num_components) {
    case 3:
	    cinfo.jpeg_color_space = colorXform ? JCS_YCbCr : JCS_RGB;
	    break;
    case 4:
	    cinfo.jpeg_color_space = colorXform ? JCS_YCCK : JCS_CMYK;
	    break;
    }

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    row_buffer = cinfo.mem->alloc_sarray((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
  }
}

// we can not go with inline since gcc
// refuses to inline because of setjmp
#define DO_GET_CHAR \
  if (current == limit) { \
    if (cinfo.output_scanline < cinfo.output_height) \
    { \
      if (!setjmp(err.setjmp_buffer)) \
      { \
        if (!jpeg_read_scanlines(&cinfo, row_buffer, 1)) c = EOF; \
        else { \
          current = &row_buffer[0][0]; \
          limit = &row_buffer[0][(cinfo.output_width - 1) * cinfo.output_components] + cinfo.output_components; \
          c = *current; \
          ++current; \
        } \
      } \
      else c = EOF; \
    } \
    else c = EOF; \
  } else { \
    c = *current; \
    ++current; \
  } \

int DCTStream::getChar() {
  int c;

  DO_GET_CHAR
  
  return c;
}

int DCTStream::getChars(int nChars, Guchar *buffer) {
  int c;
  for (int i = 0; i < nChars; ++i) {
    DO_GET_CHAR
    if (likely(c != EOF)) buffer[i] = c;
    else return i;
  }
  return nChars;
}

int DCTStream::lookChar() {
  return *current;
}

GooString *DCTStream::getPSFilter(int psLevel, char *indent) {
  GooString *s;

  if (psLevel < 2) {
    return NULL;
  }
  if (!(s = str->getPSFilter(psLevel, indent))) {
    return NULL;
  }
  s->append(indent)->append("<< >> /DCTDecode filter\n");
  return s;
}

GBool DCTStream::isBinary(GBool last) {
  return str->isBinary(gTrue);
}
