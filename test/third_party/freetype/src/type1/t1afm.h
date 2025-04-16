/***************************************************************************/
/*                                                                         */
/*  t1afm.h                                                                */
/*                                                                         */
/*    AFM support for Type 1 fonts (specification).                        */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2006 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __T1AFM_H__
#define __T1AFM_H__

#include <ft2build.h>
#include "t1objs.h"
#include FT_INTERNAL_TYPE1_TYPES_H

FT_BEGIN_HEADER


  FT_LOCAL( FT_Error )
  T1_Read_Metrics( FT_Face    face,
                   FT_Stream  stream );

  FT_LOCAL( void )
  T1_Done_Metrics( FT_Memory     memory,
                   AFM_FontInfo  fi );

  FT_LOCAL( void )
  T1_Get_Kerning( AFM_FontInfo  fi,
                  FT_UInt       glyph1,
                  FT_UInt       glyph2,
                  FT_Vector*    kerning );

  FT_LOCAL( FT_Error )
  T1_Get_Track_Kerning( FT_Face    face,
                        FT_Fixed   ptsize,
                        FT_Int     degree,
                        FT_Fixed*  kerning );

FT_END_HEADER

#endif /* __T1AFM_H__ */


/* END */
