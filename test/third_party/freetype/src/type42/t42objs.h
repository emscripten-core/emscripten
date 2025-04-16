/***************************************************************************/
/*                                                                         */
/*  t42objs.h                                                              */
/*                                                                         */
/*    Type 42 objects manager (specification).                             */
/*                                                                         */
/*  Copyright 2002, 2003, 2006, 2007 by Roberto Alameda.                   */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __T42OBJS_H__
#define __T42OBJS_H__

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_INTERNAL_TYPE1_TYPES_H
#include "t42types.h"
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DRIVER_H
#include FT_SERVICE_POSTSCRIPT_CMAPS_H
#include FT_INTERNAL_POSTSCRIPT_HINTS_H


FT_BEGIN_HEADER


  /* Type42 size */
  typedef struct  T42_SizeRec_
  {
    FT_SizeRec  root;
    FT_Size     ttsize;

  } T42_SizeRec, *T42_Size;


  /* Type42 slot */
  typedef struct  T42_GlyphSlotRec_
  {
    FT_GlyphSlotRec  root;
    FT_GlyphSlot     ttslot;

  } T42_GlyphSlotRec, *T42_GlyphSlot;


  /* Type 42 driver */
  typedef struct  T42_DriverRec_
  {
    FT_DriverRec     root;
    FT_Driver_Class  ttclazz;
    void*            extension_component;

  } T42_DriverRec, *T42_Driver;


  /* */


  FT_LOCAL( FT_Error )
  T42_Face_Init( FT_Stream      stream,
                 T42_Face       face,
                 FT_Int         face_index,
                 FT_Int         num_params,
                 FT_Parameter*  params );


  FT_LOCAL( void )
  T42_Face_Done( T42_Face  face );


  FT_LOCAL( FT_Error )
  T42_Size_Init( T42_Size  size );


  FT_LOCAL( FT_Error )
  T42_Size_Request( T42_Size         size,
                    FT_Size_Request  req );


  FT_LOCAL( FT_Error )
  T42_Size_Select( T42_Size  size,
                   FT_ULong  strike_index );


  FT_LOCAL( void )
  T42_Size_Done( T42_Size  size );


  FT_LOCAL( FT_Error )
  T42_GlyphSlot_Init( T42_GlyphSlot  slot );


  FT_LOCAL( FT_Error )
  T42_GlyphSlot_Load( FT_GlyphSlot  glyph,
                      FT_Size       size,
                      FT_UInt       glyph_index,
                      FT_Int32      load_flags );

  FT_LOCAL( void )
  T42_GlyphSlot_Done( T42_GlyphSlot slot );


  FT_LOCAL( FT_Error )
  T42_Driver_Init( T42_Driver  driver );

  FT_LOCAL( void )
  T42_Driver_Done( T42_Driver  driver );

 /* */

FT_END_HEADER


#endif /* __T42OBJS_H__ */


/* END */
