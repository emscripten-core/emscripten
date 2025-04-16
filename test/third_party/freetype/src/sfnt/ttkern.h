/***************************************************************************/
/*                                                                         */
/*  ttkern.h                                                               */
/*                                                                         */
/*    Load the basic TrueType kerning table.  This doesn't handle          */
/*    kerning data within the GPOS table at the moment.                    */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2005, 2007 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTKERN_H__
#define __TTKERN_H__


#include <ft2build.h>
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_TRUETYPE_TYPES_H


FT_BEGIN_HEADER


  FT_LOCAL( FT_Error  )
  tt_face_load_kern( TT_Face    face,
                     FT_Stream  stream );

  FT_LOCAL( void )
  tt_face_done_kern( TT_Face  face );

  FT_LOCAL( FT_Int )
  tt_face_get_kerning( TT_Face     face,
                       FT_UInt     left_glyph,
                       FT_UInt     right_glyph );

#define TT_FACE_HAS_KERNING( face )  ( (face)->kern_avail_bits != 0 )


FT_END_HEADER

#endif /* __TTKERN_H__ */


/* END */
