/***************************************************************************/
/*                                                                         */
/*  ttmtx.h                                                                */
/*                                                                         */
/*    Load the metrics tables common to TTF and OTF fonts (specification). */
/*                                                                         */
/*  Copyright 2006 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTMTX_H__
#define __TTMTX_H__


#include <ft2build.h>
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_TRUETYPE_TYPES_H


FT_BEGIN_HEADER


  FT_LOCAL( FT_Error )
  tt_face_load_hhea( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical );


  FT_LOCAL( FT_Error )
  tt_face_load_hmtx( TT_Face    face,
                     FT_Stream  stream,
                     FT_Bool    vertical );


  FT_LOCAL( FT_Error )
  tt_face_get_metrics( TT_Face     face,
                       FT_Bool     vertical,
                       FT_UInt     gindex,
                       FT_Short*   abearing,
                       FT_UShort*  aadvance );

FT_END_HEADER

#endif /* __TTMTX_H__ */


/* END */
