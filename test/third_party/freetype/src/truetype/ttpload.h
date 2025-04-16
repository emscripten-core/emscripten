/***************************************************************************/
/*                                                                         */
/*  ttpload.h                                                              */
/*                                                                         */
/*    TrueType-specific tables loader (specification).                     */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2005, 2006 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTPLOAD_H__
#define __TTPLOAD_H__


#include <ft2build.h>
#include FT_INTERNAL_TRUETYPE_TYPES_H


FT_BEGIN_HEADER


  FT_LOCAL( FT_Error )
  tt_face_load_loca( TT_Face    face,
                     FT_Stream  stream );

  FT_LOCAL( FT_ULong )
  tt_face_get_location( TT_Face   face,
                        FT_UInt   gindex,
                        FT_UInt  *asize );

  FT_LOCAL( void )
  tt_face_done_loca( TT_Face  face );

  FT_LOCAL( FT_Error )
  tt_face_load_cvt( TT_Face    face,
                    FT_Stream  stream );

  FT_LOCAL( FT_Error )
  tt_face_load_fpgm( TT_Face    face,
                     FT_Stream  stream );


  FT_LOCAL( FT_Error )
  tt_face_load_prep( TT_Face    face,
                     FT_Stream  stream );


  FT_LOCAL( FT_Error )
  tt_face_load_hdmx( TT_Face    face,
                     FT_Stream  stream );


  FT_LOCAL( void )
  tt_face_free_hdmx( TT_Face  face );


  FT_LOCAL( FT_Byte* )
  tt_face_get_device_metrics( TT_Face    face,
                              FT_UInt    ppem,
                              FT_UInt    gindex );

FT_END_HEADER

#endif /* __TTPLOAD_H__ */


/* END */
