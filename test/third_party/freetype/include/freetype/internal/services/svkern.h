/***************************************************************************/
/*                                                                         */
/*  svkern.h                                                               */
/*                                                                         */
/*    The FreeType Kerning service (specification).                        */
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


#ifndef __SVKERN_H__
#define __SVKERN_H__

#include FT_INTERNAL_SERVICE_H
#include FT_TRUETYPE_TABLES_H


FT_BEGIN_HEADER

#define FT_SERVICE_ID_KERNING  "kerning"


  typedef FT_Error
  (*FT_Kerning_TrackGetFunc)( FT_Face    face,
                              FT_Fixed   point_size,
                              FT_Int     degree,
                              FT_Fixed*  akerning );

  FT_DEFINE_SERVICE( Kerning )
  {
    FT_Kerning_TrackGetFunc  get_track;
  };

  /* */


FT_END_HEADER


#endif /* __SVKERN_H__ */


/* END */
