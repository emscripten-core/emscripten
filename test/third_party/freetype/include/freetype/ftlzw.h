/***************************************************************************/
/*                                                                         */
/*  ftlzw.h                                                                */
/*                                                                         */
/*    LZW-compressed stream support.                                       */
/*                                                                         */
/*  Copyright 2004, 2006 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTLZW_H__
#define __FTLZW_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    lzw                                                                */
  /*                                                                       */
  /* <Title>                                                               */
  /*    LZW Streams                                                        */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Using LZW-compressed font files.                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains the declaration of LZW-specific functions.   */
  /*                                                                       */
  /*************************************************************************/

 /************************************************************************
  *
  * @function:
  *   FT_Stream_OpenLZW
  *
  * @description:
  *   Open a new stream to parse LZW-compressed font files.  This is
  *   mainly used to support the compressed `*.pcf.Z' fonts that come
  *   with XFree86.
  *
  * @input:
  *   stream :: The target embedding stream.
  *
  *   source :: The source stream.
  *
  * @return:
  *   FreeType error code.  0~means success.
  *
  * @note:
  *   The source stream must be opened _before_ calling this function.
  *
  *   Calling the internal function `FT_Stream_Close' on the new stream will
  *   *not* call `FT_Stream_Close' on the source stream.  None of the stream
  *   objects will be released to the heap.
  *
  *   The stream implementation is very basic and resets the decompression
  *   process each time seeking backwards is needed within the stream
  *
  *   In certain builds of the library, LZW compression recognition is
  *   automatically handled when calling @FT_New_Face or @FT_Open_Face.
  *   This means that if no font driver is capable of handling the raw
  *   compressed file, the library will try to open a LZW stream from it
  *   and re-open the face with it.
  *
  *   This function may return `FT_Err_Unimplemented_Feature' if your build
  *   of FreeType was not compiled with LZW support.
  */
  FT_EXPORT( FT_Error )
  FT_Stream_OpenLZW( FT_Stream  stream,
                     FT_Stream  source );

 /* */


FT_END_HEADER

#endif /* __FTLZW_H__ */


/* END */
