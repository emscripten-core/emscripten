/***************************************************************************/
/*                                                                         */
/*  ftobjs.c                                                               */
/*                                                                         */
/*    The FreeType private base classes (body).                            */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,   */
/*            2010 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
#include FT_LIST_H
#include FT_OUTLINE_H
#include FT_INTERNAL_VALIDATE_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_RFORK_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_SFNT_H    /* for SFNT_Load_Table_Func */
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include FT_TRUETYPE_IDS_H

#include FT_SERVICE_SFNT_H
#include FT_SERVICE_POSTSCRIPT_NAME_H
#include FT_SERVICE_GLYPH_DICT_H
#include FT_SERVICE_TT_CMAP_H
#include FT_SERVICE_KERNING_H
#include FT_SERVICE_TRUETYPE_ENGINE_H

#ifdef FT_CONFIG_OPTION_MAC_FONTS
#include "ftbase.h"
#endif

#define GRID_FIT_METRICS


  FT_BASE_DEF( FT_Pointer )
  ft_service_list_lookup( FT_ServiceDesc  service_descriptors,
                          const char*     service_id )
  {
    FT_Pointer      result = NULL;
    FT_ServiceDesc  desc   = service_descriptors;


    if ( desc && service_id )
    {
      for ( ; desc->serv_id != NULL; desc++ )
      {
        if ( ft_strcmp( desc->serv_id, service_id ) == 0 )
        {
          result = (FT_Pointer)desc->serv_data;
          break;
        }
      }
    }

    return result;
  }


  FT_BASE_DEF( void )
  ft_validator_init( FT_Validator        valid,
                     const FT_Byte*      base,
                     const FT_Byte*      limit,
                     FT_ValidationLevel  level )
  {
    valid->base  = base;
    valid->limit = limit;
    valid->level = level;
    valid->error = FT_Err_Ok;
  }


  FT_BASE_DEF( FT_Int )
  ft_validator_run( FT_Validator  valid )
  {
    /* This function doesn't work!  None should call it. */
    FT_UNUSED( valid );

    return -1;
  }


  FT_BASE_DEF( void )
  ft_validator_error( FT_Validator  valid,
                      FT_Error      error )
  {
    /* since the cast below also disables the compiler's */
    /* type check, we introduce a dummy variable, which  */
    /* will be optimized away                            */
    volatile ft_jmp_buf* jump_buffer = &valid->jump_buffer;


    valid->error = error;

    /* throw away volatileness; use `jump_buffer' or the  */
    /* compiler may warn about an unused local variable   */
    ft_longjmp( *(ft_jmp_buf*) jump_buffer, 1 );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           S T R E A M                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /* create a new input stream from an FT_Open_Args structure */
  /*                                                          */
  FT_BASE_DEF( FT_Error )
  FT_Stream_New( FT_Library           library,
                 const FT_Open_Args*  args,
                 FT_Stream           *astream )
  {
    FT_Error   error;
    FT_Memory  memory;
    FT_Stream  stream;


    *astream = 0;

    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( !args )
      return FT_Err_Invalid_Argument;

    memory = library->memory;

    if ( FT_NEW( stream ) )
      goto Exit;

    stream->memory = memory;

    if ( args->flags & FT_OPEN_MEMORY )
    {
      /* create a memory-based stream */
      FT_Stream_OpenMemory( stream,
                            (const FT_Byte*)args->memory_base,
                            args->memory_size );
    }
    else if ( args->flags & FT_OPEN_PATHNAME )
    {
      /* create a normal system stream */
      error = FT_Stream_Open( stream, args->pathname );
      stream->pathname.pointer = args->pathname;
    }
    else if ( ( args->flags & FT_OPEN_STREAM ) && args->stream )
    {
      /* use an existing, user-provided stream */

      /* in this case, we do not need to allocate a new stream object */
      /* since the caller is responsible for closing it himself       */
      FT_FREE( stream );
      stream = args->stream;
    }
    else
      error = FT_Err_Invalid_Argument;

    if ( error )
      FT_FREE( stream );
    else
      stream->memory = memory;  /* just to be certain */

    *astream = stream;

  Exit:
    return error;
  }


  FT_BASE_DEF( void )
  FT_Stream_Free( FT_Stream  stream,
                  FT_Int     external )
  {
    if ( stream )
    {
      FT_Memory  memory = stream->memory;


      FT_Stream_Close( stream );

      if ( !external )
        FT_FREE( stream );
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_objs


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****               FACE, SIZE & GLYPH SLOT OBJECTS                   ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  static FT_Error
  ft_glyphslot_init( FT_GlyphSlot  slot )
  {
    FT_Driver         driver   = slot->face->driver;
    FT_Driver_Class   clazz    = driver->clazz;
    FT_Memory         memory   = driver->root.memory;
    FT_Error          error    = FT_Err_Ok;
    FT_Slot_Internal  internal = NULL;


    slot->library = driver->root.library;

    if ( FT_NEW( internal ) )
      goto Exit;

    slot->internal = internal;

    if ( FT_DRIVER_USES_OUTLINES( driver ) )
      error = FT_GlyphLoader_New( memory, &internal->loader );

    if ( !error && clazz->init_slot )
      error = clazz->init_slot( slot );

  Exit:
    return error;
  }


  FT_BASE_DEF( void )
  ft_glyphslot_free_bitmap( FT_GlyphSlot  slot )
  {
    if ( slot->internal && ( slot->internal->flags & FT_GLYPH_OWN_BITMAP ) )
    {
      FT_Memory  memory = FT_FACE_MEMORY( slot->face );


      FT_FREE( slot->bitmap.buffer );
      slot->internal->flags &= ~FT_GLYPH_OWN_BITMAP;
    }
    else
    {
      /* assume that the bitmap buffer was stolen or not */
      /* allocated from the heap                         */
      slot->bitmap.buffer = NULL;
    }
  }


  FT_BASE_DEF( void )
  ft_glyphslot_set_bitmap( FT_GlyphSlot  slot,
                           FT_Byte*      buffer )
  {
    ft_glyphslot_free_bitmap( slot );

    slot->bitmap.buffer = buffer;

    FT_ASSERT( (slot->internal->flags & FT_GLYPH_OWN_BITMAP) == 0 );
  }


  FT_BASE_DEF( FT_Error )
  ft_glyphslot_alloc_bitmap( FT_GlyphSlot  slot,
                             FT_ULong      size )
  {
    FT_Memory  memory = FT_FACE_MEMORY( slot->face );
    FT_Error   error;


    if ( slot->internal->flags & FT_GLYPH_OWN_BITMAP )
      FT_FREE( slot->bitmap.buffer );
    else
      slot->internal->flags |= FT_GLYPH_OWN_BITMAP;

    (void)FT_ALLOC( slot->bitmap.buffer, size );
    return error;
  }


  static void
  ft_glyphslot_clear( FT_GlyphSlot  slot )
  {
    /* free bitmap if needed */
    ft_glyphslot_free_bitmap( slot );

    /* clear all public fields in the glyph slot */
    FT_ZERO( &slot->metrics );
    FT_ZERO( &slot->outline );

    slot->bitmap.width      = 0;
    slot->bitmap.rows       = 0;
    slot->bitmap.pitch      = 0;
    slot->bitmap.pixel_mode = 0;
    /* `slot->bitmap.buffer' has been handled by ft_glyphslot_free_bitmap */

    slot->bitmap_left   = 0;
    slot->bitmap_top    = 0;
    slot->num_subglyphs = 0;
    slot->subglyphs     = 0;
    slot->control_data  = 0;
    slot->control_len   = 0;
    slot->other         = 0;
    slot->format        = FT_GLYPH_FORMAT_NONE;

    slot->linearHoriAdvance = 0;
    slot->linearVertAdvance = 0;
    slot->lsb_delta         = 0;
    slot->rsb_delta         = 0;
  }


  static void
  ft_glyphslot_done( FT_GlyphSlot  slot )
  {
    FT_Driver        driver = slot->face->driver;
    FT_Driver_Class  clazz  = driver->clazz;
    FT_Memory        memory = driver->root.memory;


    if ( clazz->done_slot )
      clazz->done_slot( slot );

    /* free bitmap buffer if needed */
    ft_glyphslot_free_bitmap( slot );

    /* slot->internal might be NULL in out-of-memory situations */
    if ( slot->internal )
    {
      /* free glyph loader */
      if ( FT_DRIVER_USES_OUTLINES( driver ) )
      {
        FT_GlyphLoader_Done( slot->internal->loader );
        slot->internal->loader = 0;
      }

      FT_FREE( slot->internal );
    }
  }


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( FT_Error )
  FT_New_GlyphSlot( FT_Face        face,
                    FT_GlyphSlot  *aslot )
  {
    FT_Error         error;
    FT_Driver        driver;
    FT_Driver_Class  clazz;
    FT_Memory        memory;
    FT_GlyphSlot     slot;


    if ( !face || !face->driver )
      return FT_Err_Invalid_Argument;

    driver = face->driver;
    clazz  = driver->clazz;
    memory = driver->root.memory;

    FT_TRACE4(( "FT_New_GlyphSlot: Creating new slot object\n" ));
    if ( !FT_ALLOC( slot, clazz->slot_object_size ) )
    {
      slot->face = face;

      error = ft_glyphslot_init( slot );
      if ( error )
      {
        ft_glyphslot_done( slot );
        FT_FREE( slot );
        goto Exit;
      }

      slot->next  = face->glyph;
      face->glyph = slot;

      if ( aslot )
        *aslot = slot;
    }
    else if ( aslot )
      *aslot = 0;


  Exit:
    FT_TRACE4(( "FT_New_GlyphSlot: Return %d\n", error ));
    return error;
  }


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( void )
  FT_Done_GlyphSlot( FT_GlyphSlot  slot )
  {
    if ( slot )
    {
      FT_Driver     driver = slot->face->driver;
      FT_Memory     memory = driver->root.memory;
      FT_GlyphSlot  prev;
      FT_GlyphSlot  cur;


      /* Remove slot from its parent face's list */
      prev = NULL;
      cur  = slot->face->glyph;

      while ( cur )
      {
        if ( cur == slot )
        {
          if ( !prev )
            slot->face->glyph = cur->next;
          else
            prev->next = cur->next;

          ft_glyphslot_done( slot );
          FT_FREE( slot );
          break;
        }
        prev = cur;
        cur  = cur->next;
      }
    }
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( void )
  FT_Set_Transform( FT_Face     face,
                    FT_Matrix*  matrix,
                    FT_Vector*  delta )
  {
    FT_Face_Internal  internal;


    if ( !face )
      return;

    internal = face->internal;

    internal->transform_flags = 0;

    if ( !matrix )
    {
      internal->transform_matrix.xx = 0x10000L;
      internal->transform_matrix.xy = 0;
      internal->transform_matrix.yx = 0;
      internal->transform_matrix.yy = 0x10000L;
      matrix = &internal->transform_matrix;
    }
    else
      internal->transform_matrix = *matrix;

    /* set transform_flags bit flag 0 if `matrix' isn't the identity */
    if ( ( matrix->xy | matrix->yx ) ||
         matrix->xx != 0x10000L      ||
         matrix->yy != 0x10000L      )
      internal->transform_flags |= 1;

    if ( !delta )
    {
      internal->transform_delta.x = 0;
      internal->transform_delta.y = 0;
      delta = &internal->transform_delta;
    }
    else
      internal->transform_delta = *delta;

    /* set transform_flags bit flag 1 if `delta' isn't the null vector */
    if ( delta->x | delta->y )
      internal->transform_flags |= 2;
  }


  static FT_Renderer
  ft_lookup_glyph_renderer( FT_GlyphSlot  slot );


#ifdef GRID_FIT_METRICS
  static void
  ft_glyphslot_grid_fit_metrics( FT_GlyphSlot  slot,
                                 FT_Bool       vertical )
  {
    FT_Glyph_Metrics*  metrics = &slot->metrics;
    FT_Pos             right, bottom;


    if ( vertical )
    {
      metrics->horiBearingX = FT_PIX_FLOOR( metrics->horiBearingX );
      metrics->horiBearingY = FT_PIX_CEIL ( metrics->horiBearingY );

      right  = FT_PIX_CEIL( metrics->vertBearingX + metrics->width );
      bottom = FT_PIX_CEIL( metrics->vertBearingY + metrics->height );

      metrics->vertBearingX = FT_PIX_FLOOR( metrics->vertBearingX );
      metrics->vertBearingY = FT_PIX_FLOOR( metrics->vertBearingY );

      metrics->width  = right - metrics->vertBearingX;
      metrics->height = bottom - metrics->vertBearingY;
    }
    else
    {
      metrics->vertBearingX = FT_PIX_FLOOR( metrics->vertBearingX );
      metrics->vertBearingY = FT_PIX_FLOOR( metrics->vertBearingY );

      right  = FT_PIX_CEIL ( metrics->horiBearingX + metrics->width );
      bottom = FT_PIX_FLOOR( metrics->horiBearingY - metrics->height );

      metrics->horiBearingX = FT_PIX_FLOOR( metrics->horiBearingX );
      metrics->horiBearingY = FT_PIX_CEIL ( metrics->horiBearingY );

      metrics->width  = right - metrics->horiBearingX;
      metrics->height = metrics->horiBearingY - bottom;
    }

    metrics->horiAdvance = FT_PIX_ROUND( metrics->horiAdvance );
    metrics->vertAdvance = FT_PIX_ROUND( metrics->vertAdvance );
  }
#endif /* GRID_FIT_METRICS */


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Load_Glyph( FT_Face   face,
                 FT_UInt   glyph_index,
                 FT_Int32  load_flags )
  {
    FT_Error      error;
    FT_Driver     driver;
    FT_GlyphSlot  slot;
    FT_Library    library;
    FT_Bool       autohint = FALSE;
    FT_Module     hinter;


    if ( !face || !face->size || !face->glyph )
      return FT_Err_Invalid_Face_Handle;

    /* The validity test for `glyph_index' is performed by the */
    /* font drivers.                                           */

    slot = face->glyph;
    ft_glyphslot_clear( slot );

    driver  = face->driver;
    library = driver->root.library;
    hinter  = library->auto_hinter;

    /* resolve load flags dependencies */

    if ( load_flags & FT_LOAD_NO_RECURSE )
      load_flags |= FT_LOAD_NO_SCALE         |
                    FT_LOAD_IGNORE_TRANSFORM;

    if ( load_flags & FT_LOAD_NO_SCALE )
    {
      load_flags |= FT_LOAD_NO_HINTING |
                    FT_LOAD_NO_BITMAP;

      load_flags &= ~FT_LOAD_RENDER;
    }

    /*
     * Determine whether we need to auto-hint or not.
     * The general rules are:
     *
     * - Do only auto-hinting if we have a hinter module, a scalable font
     *   format dealing with outlines, and no transforms except simple
     *   slants and/or rotations by integer multiples of 90 degrees.
     *
     * - Then, auto-hint if FT_LOAD_FORCE_AUTOHINT is set or if we don't
     *   have a native font hinter.
     *
     * - Otherwise, auto-hint for LIGHT hinting mode.
     *
     * - Exception: The font is `tricky' and requires the native hinter to
     *   load properly.
     */

    if ( hinter                                           &&
         !( load_flags & FT_LOAD_NO_HINTING )             &&
         !( load_flags & FT_LOAD_NO_AUTOHINT )            &&
         FT_DRIVER_IS_SCALABLE( driver )                  &&
         FT_DRIVER_USES_OUTLINES( driver )                &&
         !FT_IS_TRICKY( face )                            &&
         ( ( face->internal->transform_matrix.yx == 0 &&
             face->internal->transform_matrix.xx != 0 ) ||
           ( face->internal->transform_matrix.xx == 0 &&
             face->internal->transform_matrix.yx != 0 ) ) )
    {
      if ( ( load_flags & FT_LOAD_FORCE_AUTOHINT ) ||
           !FT_DRIVER_HAS_HINTER( driver )         )
        autohint = TRUE;
      else
      {
        FT_Render_Mode  mode = FT_LOAD_TARGET_MODE( load_flags );


        if ( mode == FT_RENDER_MODE_LIGHT             ||
             face->internal->ignore_unpatented_hinter )
          autohint = TRUE;
      }
    }

    if ( autohint )
    {
      FT_AutoHinter_Service  hinting;


      /* try to load embedded bitmaps first if available            */
      /*                                                            */
      /* XXX: This is really a temporary hack that should disappear */
      /*      promptly with FreeType 2.1!                           */
      /*                                                            */
      if ( FT_HAS_FIXED_SIZES( face )             &&
          ( load_flags & FT_LOAD_NO_BITMAP ) == 0 )
      {
        error = driver->clazz->load_glyph( slot, face->size,
                                           glyph_index,
                                           load_flags | FT_LOAD_SBITS_ONLY );

        if ( !error && slot->format == FT_GLYPH_FORMAT_BITMAP )
          goto Load_Ok;
      }

      {
        FT_Face_Internal  internal        = face->internal;
        FT_Int            transform_flags = internal->transform_flags;


        /* since the auto-hinter calls FT_Load_Glyph by itself, */
        /* make sure that glyphs aren't transformed             */
        internal->transform_flags = 0;

        /* load auto-hinted outline */
        hinting = (FT_AutoHinter_Service)hinter->clazz->module_interface;

        error   = hinting->load_glyph( (FT_AutoHinter)hinter,
                                       slot, face->size,
                                       glyph_index, load_flags );

        internal->transform_flags = transform_flags;
      }
    }
    else
    {
      error = driver->clazz->load_glyph( slot,
                                         face->size,
                                         glyph_index,
                                         load_flags );
      if ( error )
        goto Exit;

      if ( slot->format == FT_GLYPH_FORMAT_OUTLINE )
      {
        /* check that the loaded outline is correct */
        error = FT_Outline_Check( &slot->outline );
        if ( error )
          goto Exit;

#ifdef GRID_FIT_METRICS
        if ( !( load_flags & FT_LOAD_NO_HINTING ) )
          ft_glyphslot_grid_fit_metrics( slot,
              FT_BOOL( load_flags & FT_LOAD_VERTICAL_LAYOUT ) );
#endif
      }
    }

  Load_Ok:
    /* compute the advance */
    if ( load_flags & FT_LOAD_VERTICAL_LAYOUT )
    {
      slot->advance.x = 0;
      slot->advance.y = slot->metrics.vertAdvance;
    }
    else
    {
      slot->advance.x = slot->metrics.horiAdvance;
      slot->advance.y = 0;
    }

    /* compute the linear advance in 16.16 pixels */
    if ( ( load_flags & FT_LOAD_LINEAR_DESIGN ) == 0 &&
         ( FT_IS_SCALABLE( face ) )                  )
    {
      FT_Size_Metrics*  metrics = &face->size->metrics;


      /* it's tricky! */
      slot->linearHoriAdvance = FT_MulDiv( slot->linearHoriAdvance,
                                           metrics->x_scale, 64 );

      slot->linearVertAdvance = FT_MulDiv( slot->linearVertAdvance,
                                           metrics->y_scale, 64 );
    }

    if ( ( load_flags & FT_LOAD_IGNORE_TRANSFORM ) == 0 )
    {
      FT_Face_Internal  internal = face->internal;


      /* now, transform the glyph image if needed */
      if ( internal->transform_flags )
      {
        /* get renderer */
        FT_Renderer  renderer = ft_lookup_glyph_renderer( slot );


        if ( renderer )
          error = renderer->clazz->transform_glyph(
                                     renderer, slot,
                                     &internal->transform_matrix,
                                     &internal->transform_delta );
        else if ( slot->format == FT_GLYPH_FORMAT_OUTLINE )
        {
          /* apply `standard' transformation if no renderer is available */
          if ( &internal->transform_matrix )
            FT_Outline_Transform( &slot->outline,
                                  &internal->transform_matrix );

          if ( &internal->transform_delta )
            FT_Outline_Translate( &slot->outline,
                                  internal->transform_delta.x,
                                  internal->transform_delta.y );
        }

        /* transform advance */
        FT_Vector_Transform( &slot->advance, &internal->transform_matrix );
      }
    }

    FT_TRACE5(( "  x advance: %d\n" , slot->advance.x ));
    FT_TRACE5(( "  y advance: %d\n" , slot->advance.y ));

    FT_TRACE5(( "  linear x advance: %d\n" , slot->linearHoriAdvance ));
    FT_TRACE5(( "  linear y advance: %d\n" , slot->linearVertAdvance ));

    /* do we need to render the image now? */
    if ( !error                                    &&
         slot->format != FT_GLYPH_FORMAT_BITMAP    &&
         slot->format != FT_GLYPH_FORMAT_COMPOSITE &&
         load_flags & FT_LOAD_RENDER )
    {
      FT_Render_Mode  mode = FT_LOAD_TARGET_MODE( load_flags );


      if ( mode == FT_RENDER_MODE_NORMAL      &&
           (load_flags & FT_LOAD_MONOCHROME ) )
        mode = FT_RENDER_MODE_MONO;

      error = FT_Render_Glyph( slot, mode );
    }

  Exit:
    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Load_Char( FT_Face   face,
                FT_ULong  char_code,
                FT_Int32  load_flags )
  {
    FT_UInt  glyph_index;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    glyph_index = (FT_UInt)char_code;
    if ( face->charmap )
      glyph_index = FT_Get_Char_Index( face, char_code );

    return FT_Load_Glyph( face, glyph_index, load_flags );
  }


  /* destructor for sizes list */
  static void
  destroy_size( FT_Memory  memory,
                FT_Size    size,
                FT_Driver  driver )
  {
    /* finalize client-specific data */
    if ( size->generic.finalizer )
      size->generic.finalizer( size );

    /* finalize format-specific stuff */
    if ( driver->clazz->done_size )
      driver->clazz->done_size( size );

    FT_FREE( size->internal );
    FT_FREE( size );
  }


  static void
  ft_cmap_done_internal( FT_CMap  cmap );


  static void
  destroy_charmaps( FT_Face    face,
                    FT_Memory  memory )
  {
    FT_Int  n;


    if ( !face )
      return;

    for ( n = 0; n < face->num_charmaps; n++ )
    {
      FT_CMap  cmap = FT_CMAP( face->charmaps[n] );


      ft_cmap_done_internal( cmap );

      face->charmaps[n] = NULL;
    }

    FT_FREE( face->charmaps );
    face->num_charmaps = 0;
  }


  /* destructor for faces list */
  static void
  destroy_face( FT_Memory  memory,
                FT_Face    face,
                FT_Driver  driver )
  {
    FT_Driver_Class  clazz = driver->clazz;


    /* discard auto-hinting data */
    if ( face->autohint.finalizer )
      face->autohint.finalizer( face->autohint.data );

    /* Discard glyph slots for this face.                           */
    /* Beware!  FT_Done_GlyphSlot() changes the field `face->glyph' */
    while ( face->glyph )
      FT_Done_GlyphSlot( face->glyph );

    /* discard all sizes for this face */
    FT_List_Finalize( &face->sizes_list,
                      (FT_List_Destructor)destroy_size,
                      memory,
                      driver );
    face->size = 0;

    /* now discard client data */
    if ( face->generic.finalizer )
      face->generic.finalizer( face );

    /* discard charmaps */
    destroy_charmaps( face, memory );

    /* finalize format-specific stuff */
    if ( clazz->done_face )
      clazz->done_face( face );

    /* close the stream for this face if needed */
    FT_Stream_Free(
      face->stream,
      ( face->face_flags & FT_FACE_FLAG_EXTERNAL_STREAM ) != 0 );

    face->stream = 0;

    /* get rid of it */
    if ( face->internal )
    {
      FT_FREE( face->internal );
    }
    FT_FREE( face );
  }


  static void
  Destroy_Driver( FT_Driver  driver )
  {
    FT_List_Finalize( &driver->faces_list,
                      (FT_List_Destructor)destroy_face,
                      driver->root.memory,
                      driver );

    /* check whether we need to drop the driver's glyph loader */
    if ( FT_DRIVER_USES_OUTLINES( driver ) )
      FT_GlyphLoader_Done( driver->glyph_loader );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    find_unicode_charmap                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function finds a Unicode charmap, if there is one.            */
  /*    And if there is more than one, it tries to favour the more         */
  /*    extensive one, i.e., one that supports UCS-4 against those which   */
  /*    are limited to the BMP (said UCS-2 encoding.)                      */
  /*                                                                       */
  /*    This function is called from open_face() (just below), and also    */
  /*    from FT_Select_Charmap( ..., FT_ENCODING_UNICODE ).                */
  /*                                                                       */
  static FT_Error
  find_unicode_charmap( FT_Face  face )
  {
    FT_CharMap*  first;
    FT_CharMap*  cur;


    /* caller should have already checked that `face' is valid */
    FT_ASSERT( face );

    first = face->charmaps;

    if ( !first )
      return FT_Err_Invalid_CharMap_Handle;

    /*
     *  The original TrueType specification(s) only specified charmap
     *  formats that are capable of mapping 8 or 16 bit character codes to
     *  glyph indices.
     *
     *  However, recent updates to the Apple and OpenType specifications
     *  introduced new formats that are capable of mapping 32-bit character
     *  codes as well.  And these are already used on some fonts, mainly to
     *  map non-BMP Asian ideographs as defined in Unicode.
     *
     *  For compatibility purposes, these fonts generally come with
     *  *several* Unicode charmaps:
     *
     *   - One of them in the "old" 16-bit format, that cannot access
     *     all glyphs in the font.
     *
     *   - Another one in the "new" 32-bit format, that can access all
     *     the glyphs.
     *
     *  This function has been written to always favor a 32-bit charmap
     *  when found.  Otherwise, a 16-bit one is returned when found.
     */

    /* Since the `interesting' table, with IDs (3,10), is normally the */
    /* last one, we loop backwards.  This loses with type1 fonts with  */
    /* non-BMP characters (<.0001%), this wins with .ttf with non-BMP  */
    /* chars (.01% ?), and this is the same about 99.99% of the time!  */

    cur = first + face->num_charmaps;  /* points after the last one */

    for ( ; --cur >= first; )
    {
      if ( cur[0]->encoding == FT_ENCODING_UNICODE )
      {
        /* XXX If some new encodings to represent UCS-4 are added, */
        /*     they should be added here.                          */
        if ( ( cur[0]->platform_id == TT_PLATFORM_MICROSOFT &&
               cur[0]->encoding_id == TT_MS_ID_UCS_4        )     ||
             ( cur[0]->platform_id == TT_PLATFORM_APPLE_UNICODE &&
               cur[0]->encoding_id == TT_APPLE_ID_UNICODE_32    ) )
        {
#ifdef FT_MAX_CHARMAP_CACHEABLE
          if ( cur - first > FT_MAX_CHARMAP_CACHEABLE )
          {
            FT_ERROR(( "find_unicode_charmap: UCS-4 cmap is found "
                       "at too late position (%d)\n", cur - first ));
            continue;
          }
#endif
          face->charmap = cur[0];
          return FT_Err_Ok;
        }
      }
    }

    /* We do not have any UCS-4 charmap.                */
    /* Do the loop again and search for UCS-2 charmaps. */
    cur = first + face->num_charmaps;

    for ( ; --cur >= first; )
    {
      if ( cur[0]->encoding == FT_ENCODING_UNICODE )
      {
#ifdef FT_MAX_CHARMAP_CACHEABLE
        if ( cur - first > FT_MAX_CHARMAP_CACHEABLE )
        {
          FT_ERROR(( "find_unicode_charmap: UCS-2 cmap is found "
                     "at too late position (%d)\n", cur - first ));
          continue;
        }
#endif
        face->charmap = cur[0];
        return FT_Err_Ok;
      }
    }

    return FT_Err_Invalid_CharMap_Handle;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    find_variant_selector_charmap                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function finds the variant selector charmap, if there is one. */
  /*    There can only be one (platform=0, specific=5, format=14).         */
  /*                                                                       */
  static FT_CharMap
  find_variant_selector_charmap( FT_Face  face )
  {
    FT_CharMap*  first;
    FT_CharMap*  end;
    FT_CharMap*  cur;


    /* caller should have already checked that `face' is valid */
    FT_ASSERT( face );

    first = face->charmaps;

    if ( !first )
      return NULL;

    end = first + face->num_charmaps;  /* points after the last one */

    for ( cur = first; cur < end; ++cur )
    {
      if ( cur[0]->platform_id == TT_PLATFORM_APPLE_UNICODE    &&
           cur[0]->encoding_id == TT_APPLE_ID_VARIANT_SELECTOR &&
           FT_Get_CMap_Format( cur[0] ) == 14                  )
      {
#ifdef FT_MAX_CHARMAP_CACHEABLE
        if ( cur - first > FT_MAX_CHARMAP_CACHEABLE )
        {
          FT_ERROR(( "find_unicode_charmap: UVS cmap is found "
                     "at too late position (%d)\n", cur - first ));
          continue;
        }
#endif
        return cur[0];
      }
    }

    return NULL;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    open_face                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function does some work for FT_Open_Face().                   */
  /*                                                                       */
  static FT_Error
  open_face( FT_Driver      driver,
             FT_Stream      stream,
             FT_Long        face_index,
             FT_Int         num_params,
             FT_Parameter*  params,
             FT_Face       *aface )
  {
    FT_Memory         memory;
    FT_Driver_Class   clazz;
    FT_Face           face = 0;
    FT_Error          error, error2;
    FT_Face_Internal  internal = NULL;


    clazz  = driver->clazz;
    memory = driver->root.memory;

    /* allocate the face object and perform basic initialization */
    if ( FT_ALLOC( face, clazz->face_object_size ) )
      goto Fail;

    if ( FT_NEW( internal ) )
      goto Fail;

    face->internal = internal;

    face->driver   = driver;
    face->memory   = memory;
    face->stream   = stream;

#ifdef FT_CONFIG_OPTION_INCREMENTAL
    {
      int  i;


      face->internal->incremental_interface = 0;
      for ( i = 0; i < num_params && !face->internal->incremental_interface;
            i++ )
        if ( params[i].tag == FT_PARAM_TAG_INCREMENTAL )
          face->internal->incremental_interface =
            (FT_Incremental_Interface)params[i].data;
    }
#endif

    if ( clazz->init_face )
      error = clazz->init_face( stream,
                                face,
                                (FT_Int)face_index,
                                num_params,
                                params );
    if ( error )
      goto Fail;

    /* select Unicode charmap by default */
    error2 = find_unicode_charmap( face );

    /* if no Unicode charmap can be found, FT_Err_Invalid_CharMap_Handle */
    /* is returned.                                                      */

    /* no error should happen, but we want to play safe */
    if ( error2 && error2 != FT_Err_Invalid_CharMap_Handle )
    {
      error = error2;
      goto Fail;
    }

    *aface = face;

  Fail:
    if ( error )
    {
      destroy_charmaps( face, memory );
      if ( clazz->done_face )
        clazz->done_face( face );
      FT_FREE( internal );
      FT_FREE( face );
      *aface = 0;
    }

    return error;
  }


  /* there's a Mac-specific extended implementation of FT_New_Face() */
  /* in src/base/ftmac.c                                             */

#if !defined( FT_MACINTOSH ) || defined( DARWIN_NO_CARBON )

  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_New_Face( FT_Library   library,
               const char*  pathname,
               FT_Long      face_index,
               FT_Face     *aface )
  {
    FT_Open_Args  args;


    /* test for valid `library' and `aface' delayed to FT_Open_Face() */
    if ( !pathname )
      return FT_Err_Invalid_Argument;

    args.flags    = FT_OPEN_PATHNAME;
    args.pathname = (char*)pathname;
    args.stream   = NULL;

    return FT_Open_Face( library, &args, face_index, aface );
  }

#endif  /* defined( FT_MACINTOSH ) && !defined( DARWIN_NO_CARBON ) */


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_New_Memory_Face( FT_Library      library,
                      const FT_Byte*  file_base,
                      FT_Long         file_size,
                      FT_Long         face_index,
                      FT_Face        *aface )
  {
    FT_Open_Args  args;


    /* test for valid `library' and `face' delayed to FT_Open_Face() */
    if ( !file_base )
      return FT_Err_Invalid_Argument;

    args.flags       = FT_OPEN_MEMORY;
    args.memory_base = file_base;
    args.memory_size = file_size;
    args.stream      = NULL;

    return FT_Open_Face( library, &args, face_index, aface );
  }


#ifdef FT_CONFIG_OPTION_MAC_FONTS

  /* The behavior here is very similar to that in base/ftmac.c, but it     */
  /* is designed to work on non-mac systems, so no mac specific calls.     */
  /*                                                                       */
  /* We look at the file and determine if it is a mac dfont file or a mac  */
  /* resource file, or a macbinary file containing a mac resource file.    */
  /*                                                                       */
  /* Unlike ftmac I'm not going to look at a `FOND'.  I don't really see   */
  /* the point, especially since there may be multiple `FOND' resources.   */
  /* Instead I'll just look for `sfnt' and `POST' resources, ordered as    */
  /* they occur in the file.                                               */
  /*                                                                       */
  /* Note that multiple `POST' resources do not mean multiple postscript   */
  /* fonts; they all get jammed together to make what is essentially a     */
  /* pfb file.                                                             */
  /*                                                                       */
  /* We aren't interested in `NFNT' or `FONT' bitmap resources.            */
  /*                                                                       */
  /* As soon as we get an `sfnt' load it into memory and pass it off to    */
  /* FT_Open_Face.                                                         */
  /*                                                                       */
  /* If we have a (set of) `POST' resources, massage them into a (memory)  */
  /* pfb file and pass that to FT_Open_Face.  (As with ftmac.c I'm not     */
  /* going to try to save the kerning info.  After all that lives in the   */
  /* `FOND' which isn't in the file containing the `POST' resources so     */
  /* we don't really have access to it.                                    */


  /* Finalizer for a memory stream; gets called by FT_Done_Face(). */
  /* It frees the memory it uses.                                  */
  /* From ftmac.c.                                                 */
  static void
  memory_stream_close( FT_Stream  stream )
  {
    FT_Memory  memory = stream->memory;


    FT_FREE( stream->base );

    stream->size  = 0;
    stream->base  = 0;
    stream->close = 0;
  }


  /* Create a new memory stream from a buffer and a size. */
  /* From ftmac.c.                                        */
  static FT_Error
  new_memory_stream( FT_Library           library,
                     FT_Byte*             base,
                     FT_ULong             size,
                     FT_Stream_CloseFunc  close,
                     FT_Stream           *astream )
  {
    FT_Error   error;
    FT_Memory  memory;
    FT_Stream  stream;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( !base )
      return FT_Err_Invalid_Argument;

    *astream = 0;
    memory = library->memory;
    if ( FT_NEW( stream ) )
      goto Exit;

    FT_Stream_OpenMemory( stream, base, size );

    stream->close = close;

    *astream = stream;

  Exit:
    return error;
  }


  /* Create a new FT_Face given a buffer and a driver name. */
  /* from ftmac.c */
  FT_LOCAL_DEF( FT_Error )
  open_face_from_buffer( FT_Library   library,
                         FT_Byte*     base,
                         FT_ULong     size,
                         FT_Long      face_index,
                         const char*  driver_name,
                         FT_Face     *aface )
  {
    FT_Open_Args  args;
    FT_Error      error;
    FT_Stream     stream = NULL;
    FT_Memory     memory = library->memory;


    error = new_memory_stream( library,
                               base,
                               size,
                               memory_stream_close,
                               &stream );
    if ( error )
    {
      FT_FREE( base );
      return error;
    }

    args.flags = FT_OPEN_STREAM;
    args.stream = stream;
    if ( driver_name )
    {
      args.flags = args.flags | FT_OPEN_DRIVER;
      args.driver = FT_Get_Module( library, driver_name );
    }

#ifdef FT_MACINTOSH
    /* At this point, face_index has served its purpose;      */
    /* whoever calls this function has already used it to     */
    /* locate the correct font data.  We should not propagate */
    /* this index to FT_Open_Face() (unless it is negative).  */

    if ( face_index > 0 )
      face_index = 0;
#endif

    error = FT_Open_Face( library, &args, face_index, aface );

    if ( error == FT_Err_Ok )
      (*aface)->face_flags &= ~FT_FACE_FLAG_EXTERNAL_STREAM;
    else
#ifdef FT_MACINTOSH
      FT_Stream_Free( stream, 0 );
#else
    {
      FT_Stream_Close( stream );
      FT_FREE( stream );
    }
#endif

    return error;
  }


  /* Look up `TYP1' or `CID ' table from sfnt table directory.       */
  /* `offset' and `length' must exclude the binary header in tables. */

  /* Type 1 and CID-keyed font drivers should recognize sfnt-wrapped */
  /* format too.  Here, since we can't expect that the TrueType font */
  /* driver is loaded unconditially, we must parse the font by       */
  /* ourselves.  We are only interested in the name of the table and */
  /* the offset.                                                     */

  static FT_Error
  ft_lookup_PS_in_sfnt_stream( FT_Stream  stream,
                               FT_Long    face_index,
                               FT_ULong*  offset,
                               FT_ULong*  length,
                               FT_Bool*   is_sfnt_cid )
  {
    FT_Error   error;
    FT_UShort  numTables;
    FT_Long    pstable_index;
    FT_ULong   tag;
    int        i;


    *offset = 0;
    *length = 0;
    *is_sfnt_cid = FALSE;

    /* TODO: support for sfnt-wrapped PS/CID in TTC format */

    /* version check for 'typ1' (should be ignored?) */
    if ( FT_READ_ULONG( tag ) )
      return error;
    if ( tag != TTAG_typ1 )
      return FT_Err_Unknown_File_Format;

    if ( FT_READ_USHORT( numTables ) )
      return error;
    if ( FT_STREAM_SKIP( 2 * 3 ) ) /* skip binary search header */
      return error;

    pstable_index = -1;
    *is_sfnt_cid  = FALSE;

    for ( i = 0; i < numTables; i++ )
    {
      if ( FT_READ_ULONG( tag )     || FT_STREAM_SKIP( 4 )      ||
           FT_READ_ULONG( *offset ) || FT_READ_ULONG( *length ) )
        return error;

      if ( tag == TTAG_CID )
      {
        pstable_index++;
        *offset += 22;
        *length -= 22;
        *is_sfnt_cid = TRUE;
        if ( face_index < 0 )
          return FT_Err_Ok;
      }
      else if ( tag == TTAG_TYP1 )
      {
        pstable_index++;
        *offset += 24;
        *length -= 24;
        *is_sfnt_cid = FALSE;
        if ( face_index < 0 )
          return FT_Err_Ok;
      }
      if ( face_index >= 0 && pstable_index == face_index )
        return FT_Err_Ok;
    }
    return FT_Err_Table_Missing;
  }


  FT_LOCAL_DEF( FT_Error )
  open_face_PS_from_sfnt_stream( FT_Library     library,
                                 FT_Stream      stream,
                                 FT_Long        face_index,
                                 FT_Int         num_params,
                                 FT_Parameter  *params,
                                 FT_Face       *aface )
  {
    FT_Error   error;
    FT_Memory  memory = library->memory;
    FT_ULong   offset, length;
    FT_Long    pos;
    FT_Bool    is_sfnt_cid;
    FT_Byte*   sfnt_ps;

    FT_UNUSED( num_params );
    FT_UNUSED( params );


    pos = FT_Stream_Pos( stream );

    error = ft_lookup_PS_in_sfnt_stream( stream,
                                         face_index,
                                         &offset,
                                         &length,
                                         &is_sfnt_cid );
    if ( error )
      goto Exit;

    if ( FT_Stream_Seek( stream, pos + offset ) )
      goto Exit;

    if ( FT_ALLOC( sfnt_ps, (FT_Long)length ) )
      goto Exit;

    error = FT_Stream_Read( stream, (FT_Byte *)sfnt_ps, length );
    if ( error )
      goto Exit;

    error = open_face_from_buffer( library,
                                   sfnt_ps,
                                   length,
                                   face_index < 0 ? face_index : 0,
                                   is_sfnt_cid ? "cid" : "type1",
                                   aface );
  Exit:
    {
      FT_Error  error1;


      if ( error == FT_Err_Unknown_File_Format )
      {
        error1 = FT_Stream_Seek( stream, pos );
        if ( error1 )
          return error1;
      }

      return error;
    }
  }


#if !defined( FT_MACINTOSH ) || defined( DARWIN_NO_CARBON )

  /* The resource header says we've got resource_cnt `POST' (type1) */
  /* resources in this file.  They all need to be coalesced into    */
  /* one lump which gets passed on to the type1 driver.             */
  /* Here can be only one PostScript font in a file so face_index   */
  /* must be 0 (or -1).                                             */
  /*                                                                */
  static FT_Error
  Mac_Read_POST_Resource( FT_Library  library,
                          FT_Stream   stream,
                          FT_Long    *offsets,
                          FT_Long     resource_cnt,
                          FT_Long     face_index,
                          FT_Face    *aface )
  {
    FT_Error   error  = FT_Err_Cannot_Open_Resource;
    FT_Memory  memory = library->memory;
    FT_Byte*   pfb_data;
    int        i, type, flags;
    FT_Long    len;
    FT_Long    pfb_len, pfb_pos, pfb_lenpos;
    FT_Long    rlen, temp;


    if ( face_index == -1 )
      face_index = 0;
    if ( face_index != 0 )
      return error;

    /* Find the length of all the POST resources, concatenated.  Assume */
    /* worst case (each resource in its own section).                   */
    pfb_len = 0;
    for ( i = 0; i < resource_cnt; ++i )
    {
      error = FT_Stream_Seek( stream, offsets[i] );
      if ( error )
        goto Exit;
      if ( FT_READ_LONG( temp ) )
        goto Exit;
      pfb_len += temp + 6;
    }

    if ( FT_ALLOC( pfb_data, (FT_Long)pfb_len + 2 ) )
      goto Exit;

    pfb_data[0] = 0x80;
    pfb_data[1] = 1;            /* Ascii section */
    pfb_data[2] = 0;            /* 4-byte length, fill in later */
    pfb_data[3] = 0;
    pfb_data[4] = 0;
    pfb_data[5] = 0;
    pfb_pos     = 6;
    pfb_lenpos  = 2;

    len = 0;
    type = 1;
    for ( i = 0; i < resource_cnt; ++i )
    {
      error = FT_Stream_Seek( stream, offsets[i] );
      if ( error )
        goto Exit2;
      if ( FT_READ_LONG( rlen ) )
        goto Exit;
      if ( FT_READ_USHORT( flags ) )
        goto Exit;
      FT_TRACE3(( "POST fragment[%d]: offsets=0x%08x, rlen=0x%08x, flags=0x%04x\n",
                   i, offsets[i], rlen, flags ));

      /* postpone the check of rlen longer than buffer until FT_Stream_Read() */
      if ( ( flags >> 8 ) == 0 )        /* Comment, should not be loaded */
        continue;

      /* the flags are part of the resource, so rlen >= 2.  */
      /* but some fonts declare rlen = 0 for empty fragment */
      if ( rlen > 2 )
        rlen -= 2;
      else
        rlen = 0;

      if ( ( flags >> 8 ) == type )
        len += rlen;
      else
      {
        if ( pfb_lenpos + 3 > pfb_len + 2 )
          goto Exit2;
        pfb_data[pfb_lenpos    ] = (FT_Byte)( len );
        pfb_data[pfb_lenpos + 1] = (FT_Byte)( len >> 8 );
        pfb_data[pfb_lenpos + 2] = (FT_Byte)( len >> 16 );
        pfb_data[pfb_lenpos + 3] = (FT_Byte)( len >> 24 );

        if ( ( flags >> 8 ) == 5 )      /* End of font mark */
          break;

        if ( pfb_pos + 6 > pfb_len + 2 )
          goto Exit2;
        pfb_data[pfb_pos++] = 0x80;

        type = flags >> 8;
        len = rlen;

        pfb_data[pfb_pos++] = (FT_Byte)type;
        pfb_lenpos          = pfb_pos;
        pfb_data[pfb_pos++] = 0;        /* 4-byte length, fill in later */
        pfb_data[pfb_pos++] = 0;
        pfb_data[pfb_pos++] = 0;
        pfb_data[pfb_pos++] = 0;
      }

      error = FT_Err_Cannot_Open_Resource;
      if ( pfb_pos > pfb_len || pfb_pos + rlen > pfb_len )
        goto Exit2;

      error = FT_Stream_Read( stream, (FT_Byte *)pfb_data + pfb_pos, rlen );
      if ( error )
        goto Exit2;
      pfb_pos += rlen;
    }

    if ( pfb_pos + 2 > pfb_len + 2 )
      goto Exit2;
    pfb_data[pfb_pos++] = 0x80;
    pfb_data[pfb_pos++] = 3;

    if ( pfb_lenpos + 3 > pfb_len + 2 )
      goto Exit2;
    pfb_data[pfb_lenpos    ] = (FT_Byte)( len );
    pfb_data[pfb_lenpos + 1] = (FT_Byte)( len >> 8 );
    pfb_data[pfb_lenpos + 2] = (FT_Byte)( len >> 16 );
    pfb_data[pfb_lenpos + 3] = (FT_Byte)( len >> 24 );

    return open_face_from_buffer( library,
                                  pfb_data,
                                  pfb_pos,
                                  face_index,
                                  "type1",
                                  aface );

  Exit2:
    FT_FREE( pfb_data );

  Exit:
    return error;
  }


  /* The resource header says we've got resource_cnt `sfnt'      */
  /* (TrueType/OpenType) resources in this file.  Look through   */
  /* them for the one indicated by face_index, load it into mem, */
  /* pass it on the the truetype driver and return it.           */
  /*                                                             */
  static FT_Error
  Mac_Read_sfnt_Resource( FT_Library  library,
                          FT_Stream   stream,
                          FT_Long    *offsets,
                          FT_Long     resource_cnt,
                          FT_Long     face_index,
                          FT_Face    *aface )
  {
    FT_Memory  memory = library->memory;
    FT_Byte*   sfnt_data;
    FT_Error   error;
    FT_Long    flag_offset;
    FT_Long    rlen;
    int        is_cff;
    FT_Long    face_index_in_resource = 0;


    if ( face_index == -1 )
      face_index = 0;
    if ( face_index >= resource_cnt )
      return FT_Err_Cannot_Open_Resource;

    flag_offset = offsets[face_index];
    error = FT_Stream_Seek( stream, flag_offset );
    if ( error )
      goto Exit;

    if ( FT_READ_LONG( rlen ) )
      goto Exit;
    if ( rlen == -1 )
      return FT_Err_Cannot_Open_Resource;

    error = open_face_PS_from_sfnt_stream( library,
                                           stream,
                                           face_index,
                                           0, NULL,
                                           aface );
    if ( !error )
      goto Exit;

    /* rewind sfnt stream before open_face_PS_from_sfnt_stream() */
    if ( FT_Stream_Seek( stream, flag_offset + 4 ) )
      goto Exit;

    if ( FT_ALLOC( sfnt_data, (FT_Long)rlen ) )
      return error;
    error = FT_Stream_Read( stream, (FT_Byte *)sfnt_data, rlen );
    if ( error )
      goto Exit;

    is_cff = rlen > 4 && !ft_memcmp( sfnt_data, "OTTO", 4 );
    error = open_face_from_buffer( library,
                                   sfnt_data,
                                   rlen,
                                   face_index_in_resource,
                                   is_cff ? "cff" : "truetype",
                                   aface );

  Exit:
    return error;
  }


  /* Check for a valid resource fork header, or a valid dfont    */
  /* header.  In a resource fork the first 16 bytes are repeated */
  /* at the location specified by bytes 4-7.  In a dfont bytes   */
  /* 4-7 point to 16 bytes of zeroes instead.                    */
  /*                                                             */
  static FT_Error
  IsMacResource( FT_Library  library,
                 FT_Stream   stream,
                 FT_Long     resource_offset,
                 FT_Long     face_index,
                 FT_Face    *aface )
  {
    FT_Memory  memory = library->memory;
    FT_Error   error;
    FT_Long    map_offset, rdara_pos;
    FT_Long    *data_offsets;
    FT_Long    count;


    error = FT_Raccess_Get_HeaderInfo( library, stream, resource_offset,
                                       &map_offset, &rdara_pos );
    if ( error )
      return error;

    error = FT_Raccess_Get_DataOffsets( library, stream,
                                        map_offset, rdara_pos,
                                        TTAG_POST,
                                        &data_offsets, &count );
    if ( !error )
    {
      error = Mac_Read_POST_Resource( library, stream, data_offsets, count,
                                      face_index, aface );
      FT_FREE( data_offsets );
      /* POST exists in an LWFN providing a single face */
      if ( !error )
        (*aface)->num_faces = 1;
      return error;
    }

    error = FT_Raccess_Get_DataOffsets( library, stream,
                                        map_offset, rdara_pos,
                                        TTAG_sfnt,
                                        &data_offsets, &count );
    if ( !error )
    {
      FT_Long  face_index_internal = face_index % count;


      error = Mac_Read_sfnt_Resource( library, stream, data_offsets, count,
                                      face_index_internal, aface );
      FT_FREE( data_offsets );
      if ( !error )
        (*aface)->num_faces = count;
    }

    return error;
  }


  /* Check for a valid macbinary header, and if we find one   */
  /* check that the (flattened) resource fork in it is valid. */
  /*                                                          */
  static FT_Error
  IsMacBinary( FT_Library  library,
               FT_Stream   stream,
               FT_Long     face_index,
               FT_Face    *aface )
  {
    unsigned char  header[128];
    FT_Error       error;
    FT_Long        dlen, offset;


    if ( NULL == stream )
      return FT_Err_Invalid_Stream_Operation;

    error = FT_Stream_Seek( stream, 0 );
    if ( error )
      goto Exit;

    error = FT_Stream_Read( stream, (FT_Byte*)header, 128 );
    if ( error )
      goto Exit;

    if (            header[ 0] !=  0 ||
                    header[74] !=  0 ||
                    header[82] !=  0 ||
                    header[ 1] ==  0 ||
                    header[ 1] >  33 ||
                    header[63] !=  0 ||
         header[2 + header[1]] !=  0 )
      return FT_Err_Unknown_File_Format;

    dlen = ( header[0x53] << 24 ) |
           ( header[0x54] << 16 ) |
           ( header[0x55] <<  8 ) |
             header[0x56];
#if 0
    rlen = ( header[0x57] << 24 ) |
           ( header[0x58] << 16 ) |
           ( header[0x59] <<  8 ) |
             header[0x5a];
#endif /* 0 */
    offset = 128 + ( ( dlen + 127 ) & ~127 );

    return IsMacResource( library, stream, offset, face_index, aface );

  Exit:
    return error;
  }


  static FT_Error
  load_face_in_embedded_rfork( FT_Library           library,
                               FT_Stream            stream,
                               FT_Long              face_index,
                               FT_Face             *aface,
                               const FT_Open_Args  *args )
  {

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_raccess

    FT_Memory  memory = library->memory;
    FT_Error   error  = FT_Err_Unknown_File_Format;
    int        i;

    char *     file_names[FT_RACCESS_N_RULES];
    FT_Long    offsets[FT_RACCESS_N_RULES];
    FT_Error   errors[FT_RACCESS_N_RULES];
    FT_Bool    is_darwin_vfs, vfs_rfork_has_no_font = FALSE; /* not tested */

    FT_Open_Args  args2;
    FT_Stream     stream2 = 0;


    FT_Raccess_Guess( library, stream,
                      args->pathname, file_names, offsets, errors );

    for ( i = 0; i < FT_RACCESS_N_RULES; i++ )
    {
      is_darwin_vfs = raccess_rule_by_darwin_vfs( i );
      if ( is_darwin_vfs && vfs_rfork_has_no_font )
      {
        FT_TRACE3(( "Skip rule %d: darwin vfs resource fork"
                    " is already checked and"
                    " no font is found\n", i ));
        continue;
      }  

      if ( errors[i] )
      {
        FT_TRACE3(( "Error[%d] has occurred in rule %d\n", errors[i], i ));
        continue;
      }

      args2.flags    = FT_OPEN_PATHNAME;
      args2.pathname = file_names[i] ? file_names[i] : args->pathname;

      FT_TRACE3(( "Try rule %d: %s (offset=%d) ...",
                  i, args2.pathname, offsets[i] ));

      error = FT_Stream_New( library, &args2, &stream2 );
      if ( is_darwin_vfs && error == FT_Err_Cannot_Open_Stream )
        vfs_rfork_has_no_font = TRUE;

      if ( error )
      {
        FT_TRACE3(( "failed\n" ));
        continue;
      }

      error = IsMacResource( library, stream2, offsets[i],
                             face_index, aface );
      FT_Stream_Free( stream2, 0 );

      FT_TRACE3(( "%s\n", error ? "failed": "successful" ));

      if ( !error )
          break;
      else if ( is_darwin_vfs )
          vfs_rfork_has_no_font = TRUE;
    }

    for (i = 0; i < FT_RACCESS_N_RULES; i++)
    {
      if ( file_names[i] )
        FT_FREE( file_names[i] );
    }

    /* Caller (load_mac_face) requires FT_Err_Unknown_File_Format. */
    if ( error )
      error = FT_Err_Unknown_File_Format;

    return error;

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_objs

  }


  /* Check for some macintosh formats without Carbon framework.    */
  /* Is this a macbinary file?  If so look at the resource fork.   */
  /* Is this a mac dfont file?                                     */
  /* Is this an old style resource fork? (in data)                 */
  /* Else call load_face_in_embedded_rfork to try extra rules      */
  /* (defined in `ftrfork.c').                                     */
  /*                                                               */
  static FT_Error
  load_mac_face( FT_Library           library,
                 FT_Stream            stream,
                 FT_Long              face_index,
                 FT_Face             *aface,
                 const FT_Open_Args  *args )
  {
    FT_Error error;
    FT_UNUSED( args );


    error = IsMacBinary( library, stream, face_index, aface );
    if ( FT_ERROR_BASE( error ) == FT_Err_Unknown_File_Format )
    {

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_raccess

      FT_TRACE3(( "Try as dfont: %s ...", args->pathname ));

      error = IsMacResource( library, stream, 0, face_index, aface );

      FT_TRACE3(( "%s\n", error ? "failed" : "successful" ));

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_objs

    }

    if ( ( FT_ERROR_BASE( error ) == FT_Err_Unknown_File_Format      ||
           FT_ERROR_BASE( error ) == FT_Err_Invalid_Stream_Operation ) &&
         ( args->flags & FT_OPEN_PATHNAME )                            )
      error = load_face_in_embedded_rfork( library, stream,
                                           face_index, aface, args );
    return error;
  }
#endif

#endif  /* !FT_MACINTOSH && FT_CONFIG_OPTION_MAC_FONTS */


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Open_Face( FT_Library           library,
                const FT_Open_Args*  args,
                FT_Long              face_index,
                FT_Face             *aface )
  {
    FT_Error     error;
    FT_Driver    driver;
    FT_Memory    memory;
    FT_Stream    stream = NULL;
    FT_Face      face   = NULL;
    FT_ListNode  node   = NULL;
    FT_Bool      external_stream;
    FT_Module*   cur;
    FT_Module*   limit;


    /* test for valid `library' delayed to */
    /* FT_Stream_New()                     */

    if ( ( !aface && face_index >= 0 ) || !args )
      return FT_Err_Invalid_Argument;

    external_stream = FT_BOOL( ( args->flags & FT_OPEN_STREAM ) &&
                               args->stream                     );

    /* create input stream */
    error = FT_Stream_New( library, args, &stream );
    if ( error )
      goto Fail3;

    memory = library->memory;

    /* If the font driver is specified in the `args' structure, use */
    /* it.  Otherwise, we scan the list of registered drivers.      */
    if ( ( args->flags & FT_OPEN_DRIVER ) && args->driver )
    {
      driver = FT_DRIVER( args->driver );

      /* not all modules are drivers, so check... */
      if ( FT_MODULE_IS_DRIVER( driver ) )
      {
        FT_Int         num_params = 0;
        FT_Parameter*  params     = 0;


        if ( args->flags & FT_OPEN_PARAMS )
        {
          num_params = args->num_params;
          params     = args->params;
        }

        error = open_face( driver, stream, face_index,
                           num_params, params, &face );
        if ( !error )
          goto Success;
      }
      else
        error = FT_Err_Invalid_Handle;

      FT_Stream_Free( stream, external_stream );
      goto Fail;
    }
    else
    {
      /* check each font driver for an appropriate format */
      cur   = library->modules;
      limit = cur + library->num_modules;


      for ( ; cur < limit; cur++ )
      {
        /* not all modules are font drivers, so check... */
        if ( FT_MODULE_IS_DRIVER( cur[0] ) )
        {
          FT_Int         num_params = 0;
          FT_Parameter*  params     = 0;


          driver = FT_DRIVER( cur[0] );

          if ( args->flags & FT_OPEN_PARAMS )
          {
            num_params = args->num_params;
            params     = args->params;
          }

          error = open_face( driver, stream, face_index,
                             num_params, params, &face );
          if ( !error )
            goto Success;

#ifdef FT_CONFIG_OPTION_MAC_FONTS
          if ( ft_strcmp( cur[0]->clazz->module_name, "truetype" ) == 0 &&
               FT_ERROR_BASE( error ) == FT_Err_Table_Missing           )
          {
            /* TrueType but essential tables are missing */
            if ( FT_Stream_Seek( stream, 0 ) )
              break;

            error = open_face_PS_from_sfnt_stream( library,
                                                   stream,
                                                   face_index,
                                                   num_params,
                                                   params,
                                                   aface );
            if ( !error )
            {
              FT_Stream_Free( stream, external_stream );
              return error;
            }
          }
#endif

          if ( FT_ERROR_BASE( error ) != FT_Err_Unknown_File_Format )
            goto Fail3;
        }
      }

  Fail3:
    /* If we are on the mac, and we get an FT_Err_Invalid_Stream_Operation */
    /* it may be because we have an empty data fork, so we need to check   */
    /* the resource fork.                                                  */
    if ( FT_ERROR_BASE( error ) != FT_Err_Cannot_Open_Stream       &&
         FT_ERROR_BASE( error ) != FT_Err_Unknown_File_Format      &&
         FT_ERROR_BASE( error ) != FT_Err_Invalid_Stream_Operation )
      goto Fail2;

#if !defined( FT_MACINTOSH ) && defined( FT_CONFIG_OPTION_MAC_FONTS )
    error = load_mac_face( library, stream, face_index, aface, args );
    if ( !error )
    {
      /* We don't want to go to Success here.  We've already done that. */
      /* On the other hand, if we succeeded we still need to close this */
      /* stream (we opened a different stream which extracted the       */
      /* interesting information out of this stream here.  That stream  */
      /* will still be open and the face will point to it).             */
      FT_Stream_Free( stream, external_stream );
      return error;
    }

    if ( FT_ERROR_BASE( error ) != FT_Err_Unknown_File_Format )
      goto Fail2;
#endif  /* !FT_MACINTOSH && FT_CONFIG_OPTION_MAC_FONTS */

      /* no driver is able to handle this format */
      error = FT_Err_Unknown_File_Format;

  Fail2:
      FT_Stream_Free( stream, external_stream );
      goto Fail;
    }

  Success:
    FT_TRACE4(( "FT_Open_Face: New face object, adding to list\n" ));

    /* set the FT_FACE_FLAG_EXTERNAL_STREAM bit for FT_Done_Face */
    if ( external_stream )
      face->face_flags |= FT_FACE_FLAG_EXTERNAL_STREAM;

    /* add the face object to its driver's list */
    if ( FT_NEW( node ) )
      goto Fail;

    node->data = face;
    /* don't assume driver is the same as face->driver, so use */
    /* face->driver instead.                                   */
    FT_List_Add( &face->driver->faces_list, node );

    /* now allocate a glyph slot object for the face */
    FT_TRACE4(( "FT_Open_Face: Creating glyph slot\n" ));

    if ( face_index >= 0 )
    {
      error = FT_New_GlyphSlot( face, NULL );
      if ( error )
        goto Fail;

      /* finally, allocate a size object for the face */
      {
        FT_Size  size;


        FT_TRACE4(( "FT_Open_Face: Creating size object\n" ));

        error = FT_New_Size( face, &size );
        if ( error )
          goto Fail;

        face->size = size;
      }
    }

    /* some checks */

    if ( FT_IS_SCALABLE( face ) )
    {
      if ( face->height < 0 )
        face->height = (FT_Short)-face->height;

      if ( !FT_HAS_VERTICAL( face ) )
        face->max_advance_height = (FT_Short)face->height;
    }

    if ( FT_HAS_FIXED_SIZES( face ) )
    {
      FT_Int  i;


      for ( i = 0; i < face->num_fixed_sizes; i++ )
      {
        FT_Bitmap_Size*  bsize = face->available_sizes + i;


        if ( bsize->height < 0 )
          bsize->height = (FT_Short)-bsize->height;
        if ( bsize->x_ppem < 0 )
          bsize->x_ppem = (FT_Short)-bsize->x_ppem;
        if ( bsize->y_ppem < 0 )
          bsize->y_ppem = -bsize->y_ppem;
      }
    }

    /* initialize internal face data */
    {
      FT_Face_Internal  internal = face->internal;


      internal->transform_matrix.xx = 0x10000L;
      internal->transform_matrix.xy = 0;
      internal->transform_matrix.yx = 0;
      internal->transform_matrix.yy = 0x10000L;

      internal->transform_delta.x = 0;
      internal->transform_delta.y = 0;

      internal->refcount = 1;
    }

    if ( aface )
      *aface = face;
    else
      FT_Done_Face( face );

    goto Exit;

  Fail:
    FT_Done_Face( face );

  Exit:
    FT_TRACE4(( "FT_Open_Face: Return %d\n", error ));

    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Attach_File( FT_Face      face,
                  const char*  filepathname )
  {
    FT_Open_Args  open;


    /* test for valid `face' delayed to FT_Attach_Stream() */

    if ( !filepathname )
      return FT_Err_Invalid_Argument;

    open.stream   = NULL;
    open.flags    = FT_OPEN_PATHNAME;
    open.pathname = (char*)filepathname;

    return FT_Attach_Stream( face, &open );
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Attach_Stream( FT_Face        face,
                    FT_Open_Args*  parameters )
  {
    FT_Stream  stream;
    FT_Error   error;
    FT_Driver  driver;

    FT_Driver_Class  clazz;


    /* test for valid `parameters' delayed to FT_Stream_New() */

    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    driver = face->driver;
    if ( !driver )
      return FT_Err_Invalid_Driver_Handle;

    error = FT_Stream_New( driver->root.library, parameters, &stream );
    if ( error )
      goto Exit;

    /* we implement FT_Attach_Stream in each driver through the */
    /* `attach_file' interface                                  */

    error = FT_Err_Unimplemented_Feature;
    clazz = driver->clazz;
    if ( clazz->attach_file )
      error = clazz->attach_file( face, stream );

    /* close the attached stream */
    FT_Stream_Free( stream,
                    (FT_Bool)( parameters->stream &&
                               ( parameters->flags & FT_OPEN_STREAM ) ) );

  Exit:
    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Reference_Face( FT_Face  face )
  {
    face->internal->refcount++;

    return FT_Err_Ok;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Done_Face( FT_Face  face )
  {
    FT_Error     error;
    FT_Driver    driver;
    FT_Memory    memory;
    FT_ListNode  node;


    error = FT_Err_Invalid_Face_Handle;
    if ( face && face->driver )
    {
      face->internal->refcount--;
      if ( face->internal->refcount > 0 )
        error = FT_Err_Ok;
      else
      {
        driver = face->driver;
        memory = driver->root.memory;

        /* find face in driver's list */
        node = FT_List_Find( &driver->faces_list, face );
        if ( node )
        {
          /* remove face object from the driver's list */
          FT_List_Remove( &driver->faces_list, node );
          FT_FREE( node );

          /* now destroy the object proper */
          destroy_face( memory, face, driver );
          error = FT_Err_Ok;
        }
      }
    }

    return error;
  }


  /* documentation is in ftobjs.h */

  FT_EXPORT_DEF( FT_Error )
  FT_New_Size( FT_Face   face,
               FT_Size  *asize )
  {
    FT_Error         error;
    FT_Memory        memory;
    FT_Driver        driver;
    FT_Driver_Class  clazz;

    FT_Size          size = 0;
    FT_ListNode      node = 0;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    if ( !asize )
      return FT_Err_Invalid_Size_Handle;

    if ( !face->driver )
      return FT_Err_Invalid_Driver_Handle;

    *asize = 0;

    driver = face->driver;
    clazz  = driver->clazz;
    memory = face->memory;

    /* Allocate new size object and perform basic initialisation */
    if ( FT_ALLOC( size, clazz->size_object_size ) || FT_NEW( node ) )
      goto Exit;

    size->face = face;

    /* for now, do not use any internal fields in size objects */
    size->internal = 0;

    if ( clazz->init_size )
      error = clazz->init_size( size );

    /* in case of success, add to the face's list */
    if ( !error )
    {
      *asize     = size;
      node->data = size;
      FT_List_Add( &face->sizes_list, node );
    }

  Exit:
    if ( error )
    {
      FT_FREE( node );
      FT_FREE( size );
    }

    return error;
  }


  /* documentation is in ftobjs.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Done_Size( FT_Size  size )
  {
    FT_Error     error;
    FT_Driver    driver;
    FT_Memory    memory;
    FT_Face      face;
    FT_ListNode  node;


    if ( !size )
      return FT_Err_Invalid_Size_Handle;

    face = size->face;
    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    driver = face->driver;
    if ( !driver )
      return FT_Err_Invalid_Driver_Handle;

    memory = driver->root.memory;

    error = FT_Err_Ok;
    node  = FT_List_Find( &face->sizes_list, size );
    if ( node )
    {
      FT_List_Remove( &face->sizes_list, node );
      FT_FREE( node );

      if ( face->size == size )
      {
        face->size = 0;
        if ( face->sizes_list.head )
          face->size = (FT_Size)(face->sizes_list.head->data);
      }

      destroy_size( memory, size, driver );
    }
    else
      error = FT_Err_Invalid_Size_Handle;

    return error;
  }


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( FT_Error )
  FT_Match_Size( FT_Face          face,
                 FT_Size_Request  req,
                 FT_Bool          ignore_width,
                 FT_ULong*        size_index )
  {
    FT_Int   i;
    FT_Long  w, h;


    if ( !FT_HAS_FIXED_SIZES( face ) )
      return FT_Err_Invalid_Face_Handle;

    /* FT_Bitmap_Size doesn't provide enough info... */
    if ( req->type != FT_SIZE_REQUEST_TYPE_NOMINAL )
      return FT_Err_Unimplemented_Feature;

    w = FT_REQUEST_WIDTH ( req );
    h = FT_REQUEST_HEIGHT( req );

    if ( req->width && !req->height )
      h = w;
    else if ( !req->width && req->height )
      w = h;

    w = FT_PIX_ROUND( w );
    h = FT_PIX_ROUND( h );

    for ( i = 0; i < face->num_fixed_sizes; i++ )
    {
      FT_Bitmap_Size*  bsize = face->available_sizes + i;


      if ( h != FT_PIX_ROUND( bsize->y_ppem ) )
        continue;

      if ( w == FT_PIX_ROUND( bsize->x_ppem ) || ignore_width )
      {
        if ( size_index )
          *size_index = (FT_ULong)i;

        return FT_Err_Ok;
      }
    }

    return FT_Err_Invalid_Pixel_Size;
  }


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( void )
  ft_synthesize_vertical_metrics( FT_Glyph_Metrics*  metrics,
                                  FT_Pos             advance )
  {
    FT_Pos  height = metrics->height;


    /* compensate for glyph with bbox above/below the baseline */
    if ( metrics->horiBearingY < 0 )
    {
      if ( height < metrics->horiBearingY )
        height = metrics->horiBearingY;
    }
    else if ( metrics->horiBearingY > 0 )
      height -= metrics->horiBearingY;

    /* the factor 1.2 is a heuristical value */
    if ( !advance )
      advance = height * 12 / 10;

    metrics->vertBearingX = metrics->horiBearingX - metrics->horiAdvance / 2;
    metrics->vertBearingY = ( advance - height ) / 2;
    metrics->vertAdvance  = advance;
  }


  static void
  ft_recompute_scaled_metrics( FT_Face           face,
                               FT_Size_Metrics*  metrics )
  {
    /* Compute root ascender, descender, test height, and max_advance */

#ifdef GRID_FIT_METRICS
    metrics->ascender    = FT_PIX_CEIL( FT_MulFix( face->ascender,
                                                   metrics->y_scale ) );

    metrics->descender   = FT_PIX_FLOOR( FT_MulFix( face->descender,
                                                    metrics->y_scale ) );

    metrics->height      = FT_PIX_ROUND( FT_MulFix( face->height,
                                                    metrics->y_scale ) );

    metrics->max_advance = FT_PIX_ROUND( FT_MulFix( face->max_advance_width,
                                                    metrics->x_scale ) );
#else /* !GRID_FIT_METRICS */
    metrics->ascender    = FT_MulFix( face->ascender,
                                      metrics->y_scale );

    metrics->descender   = FT_MulFix( face->descender,
                                      metrics->y_scale );

    metrics->height      = FT_MulFix( face->height,
                                      metrics->y_scale );

    metrics->max_advance = FT_MulFix( face->max_advance_width,
                                      metrics->x_scale );
#endif /* !GRID_FIT_METRICS */
  }


  FT_BASE_DEF( void )
  FT_Select_Metrics( FT_Face   face,
                     FT_ULong  strike_index )
  {
    FT_Size_Metrics*  metrics;
    FT_Bitmap_Size*   bsize;


    metrics = &face->size->metrics;
    bsize   = face->available_sizes + strike_index;

    metrics->x_ppem = (FT_UShort)( ( bsize->x_ppem + 32 ) >> 6 );
    metrics->y_ppem = (FT_UShort)( ( bsize->y_ppem + 32 ) >> 6 );

    if ( FT_IS_SCALABLE( face ) )
    {
      metrics->x_scale = FT_DivFix( bsize->x_ppem,
                                    face->units_per_EM );
      metrics->y_scale = FT_DivFix( bsize->y_ppem,
                                    face->units_per_EM );

      ft_recompute_scaled_metrics( face, metrics );
    }
    else
    {
      metrics->x_scale     = 1L << 16;
      metrics->y_scale     = 1L << 16;
      metrics->ascender    = bsize->y_ppem;
      metrics->descender   = 0;
      metrics->height      = bsize->height << 6;
      metrics->max_advance = bsize->x_ppem;
    }
  }


  FT_BASE_DEF( void )
  FT_Request_Metrics( FT_Face          face,
                      FT_Size_Request  req )
  {
    FT_Size_Metrics*  metrics;


    metrics = &face->size->metrics;

    if ( FT_IS_SCALABLE( face ) )
    {
      FT_Long  w = 0, h = 0, scaled_w = 0, scaled_h = 0;


      switch ( req->type )
      {
      case FT_SIZE_REQUEST_TYPE_NOMINAL:
        w = h = face->units_per_EM;
        break;

      case FT_SIZE_REQUEST_TYPE_REAL_DIM:
        w = h = face->ascender - face->descender;
        break;

      case FT_SIZE_REQUEST_TYPE_BBOX:
        w = face->bbox.xMax - face->bbox.xMin;
        h = face->bbox.yMax - face->bbox.yMin;
        break;

      case FT_SIZE_REQUEST_TYPE_CELL:
        w = face->max_advance_width;
        h = face->ascender - face->descender;
        break;

      case FT_SIZE_REQUEST_TYPE_SCALES:
        metrics->x_scale = (FT_Fixed)req->width;
        metrics->y_scale = (FT_Fixed)req->height;
        if ( !metrics->x_scale )
          metrics->x_scale = metrics->y_scale;
        else if ( !metrics->y_scale )
          metrics->y_scale = metrics->x_scale;
        goto Calculate_Ppem;

      case FT_SIZE_REQUEST_TYPE_MAX:
        break;
      }

      /* to be on the safe side */
      if ( w < 0 )
        w = -w;

      if ( h < 0 )
        h = -h;

      scaled_w = FT_REQUEST_WIDTH ( req );
      scaled_h = FT_REQUEST_HEIGHT( req );

      /* determine scales */
      if ( req->width )
      {
        metrics->x_scale = FT_DivFix( scaled_w, w );

        if ( req->height )
        {
          metrics->y_scale = FT_DivFix( scaled_h, h );

          if ( req->type == FT_SIZE_REQUEST_TYPE_CELL )
          {
            if ( metrics->y_scale > metrics->x_scale )
              metrics->y_scale = metrics->x_scale;
            else
              metrics->x_scale = metrics->y_scale;
          }
        }
        else
        {
          metrics->y_scale = metrics->x_scale;
          scaled_h = FT_MulDiv( scaled_w, h, w );
        }
      }
      else
      {
        metrics->x_scale = metrics->y_scale = FT_DivFix( scaled_h, h );
        scaled_w = FT_MulDiv( scaled_h, w, h );
      }

  Calculate_Ppem:
      /* calculate the ppems */
      if ( req->type != FT_SIZE_REQUEST_TYPE_NOMINAL )
      {
        scaled_w = FT_MulFix( face->units_per_EM, metrics->x_scale );
        scaled_h = FT_MulFix( face->units_per_EM, metrics->y_scale );
      }

      metrics->x_ppem = (FT_UShort)( ( scaled_w + 32 ) >> 6 );
      metrics->y_ppem = (FT_UShort)( ( scaled_h + 32 ) >> 6 );

      ft_recompute_scaled_metrics( face, metrics );
    }
    else
    {
      FT_ZERO( metrics );
      metrics->x_scale = 1L << 16;
      metrics->y_scale = 1L << 16;
    }
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Select_Size( FT_Face  face,
                  FT_Int   strike_index )
  {
    FT_Driver_Class  clazz;


    if ( !face || !FT_HAS_FIXED_SIZES( face ) )
      return FT_Err_Invalid_Face_Handle;

    if ( strike_index < 0 || strike_index >= face->num_fixed_sizes )
      return FT_Err_Invalid_Argument;

    clazz = face->driver->clazz;

    if ( clazz->select_size )
      return clazz->select_size( face->size, (FT_ULong)strike_index );

    FT_Select_Metrics( face, (FT_ULong)strike_index );

    return FT_Err_Ok;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Request_Size( FT_Face          face,
                   FT_Size_Request  req )
  {
    FT_Driver_Class  clazz;
    FT_ULong         strike_index;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    if ( !req || req->width < 0 || req->height < 0 ||
         req->type >= FT_SIZE_REQUEST_TYPE_MAX )
      return FT_Err_Invalid_Argument;

    clazz = face->driver->clazz;

    if ( clazz->request_size )
      return clazz->request_size( face->size, req );

    /*
     * The reason that a driver doesn't have `request_size' defined is
     * either that the scaling here suffices or that the supported formats
     * are bitmap-only and size matching is not implemented.
     *
     * In the latter case, a simple size matching is done.
     */
    if ( !FT_IS_SCALABLE( face ) && FT_HAS_FIXED_SIZES( face ) )
    {
      FT_Error  error;


      error = FT_Match_Size( face, req, 0, &strike_index );
      if ( error )
        return error;

      FT_TRACE3(( "FT_Request_Size: bitmap strike %lu matched\n",
                  strike_index ));

      return FT_Select_Size( face, (FT_Int)strike_index );
    }

    FT_Request_Metrics( face, req );

    return FT_Err_Ok;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Set_Char_Size( FT_Face     face,
                    FT_F26Dot6  char_width,
                    FT_F26Dot6  char_height,
                    FT_UInt     horz_resolution,
                    FT_UInt     vert_resolution )
  {
    FT_Size_RequestRec  req;


    if ( !char_width )
      char_width = char_height;
    else if ( !char_height )
      char_height = char_width;

    if ( !horz_resolution )
      horz_resolution = vert_resolution;
    else if ( !vert_resolution )
      vert_resolution = horz_resolution;

    if ( char_width  < 1 * 64 )
      char_width  = 1 * 64;
    if ( char_height < 1 * 64 )
      char_height = 1 * 64;

    if ( !horz_resolution )
      horz_resolution = vert_resolution = 72;

    req.type           = FT_SIZE_REQUEST_TYPE_NOMINAL;
    req.width          = char_width;
    req.height         = char_height;
    req.horiResolution = horz_resolution;
    req.vertResolution = vert_resolution;

    return FT_Request_Size( face, &req );
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Set_Pixel_Sizes( FT_Face  face,
                      FT_UInt  pixel_width,
                      FT_UInt  pixel_height )
  {
    FT_Size_RequestRec  req;


    if ( pixel_width == 0 )
      pixel_width = pixel_height;
    else if ( pixel_height == 0 )
      pixel_height = pixel_width;

    if ( pixel_width  < 1 )
      pixel_width  = 1;
    if ( pixel_height < 1 )
      pixel_height = 1;

    /* use `>=' to avoid potential compiler warning on 16bit platforms */
    if ( pixel_width  >= 0xFFFFU )
      pixel_width  = 0xFFFFU;
    if ( pixel_height >= 0xFFFFU )
      pixel_height = 0xFFFFU;

    req.type           = FT_SIZE_REQUEST_TYPE_NOMINAL;
    req.width          = pixel_width << 6;
    req.height         = pixel_height << 6;
    req.horiResolution = 0;
    req.vertResolution = 0;

    return FT_Request_Size( face, &req );
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Get_Kerning( FT_Face     face,
                  FT_UInt     left_glyph,
                  FT_UInt     right_glyph,
                  FT_UInt     kern_mode,
                  FT_Vector  *akerning )
  {
    FT_Error   error = FT_Err_Ok;
    FT_Driver  driver;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    if ( !akerning )
      return FT_Err_Invalid_Argument;

    driver = face->driver;

    akerning->x = 0;
    akerning->y = 0;

    if ( driver->clazz->get_kerning )
    {
      error = driver->clazz->get_kerning( face,
                                          left_glyph,
                                          right_glyph,
                                          akerning );
      if ( !error )
      {
        if ( kern_mode != FT_KERNING_UNSCALED )
        {
          akerning->x = FT_MulFix( akerning->x, face->size->metrics.x_scale );
          akerning->y = FT_MulFix( akerning->y, face->size->metrics.y_scale );

          if ( kern_mode != FT_KERNING_UNFITTED )
          {
            /* we scale down kerning values for small ppem values */
            /* to avoid that rounding makes them too big.         */
            /* `25' has been determined heuristically.            */
            if ( face->size->metrics.x_ppem < 25 )
              akerning->x = FT_MulDiv( akerning->x,
                                       face->size->metrics.x_ppem, 25 );
            if ( face->size->metrics.y_ppem < 25 )
              akerning->y = FT_MulDiv( akerning->y,
                                       face->size->metrics.y_ppem, 25 );

            akerning->x = FT_PIX_ROUND( akerning->x );
            akerning->y = FT_PIX_ROUND( akerning->y );
          }
        }
      }
    }

    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Get_Track_Kerning( FT_Face    face,
                        FT_Fixed   point_size,
                        FT_Int     degree,
                        FT_Fixed*  akerning )
  {
    FT_Service_Kerning  service;
    FT_Error            error = FT_Err_Ok;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    if ( !akerning )
      return FT_Err_Invalid_Argument;

    FT_FACE_FIND_SERVICE( face, service, KERNING );
    if ( !service )
      return FT_Err_Unimplemented_Feature;

    error = service->get_track( face,
                                point_size,
                                degree,
                                akerning );

    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Select_Charmap( FT_Face      face,
                     FT_Encoding  encoding )
  {
    FT_CharMap*  cur;
    FT_CharMap*  limit;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    if ( encoding == FT_ENCODING_NONE )
      return FT_Err_Invalid_Argument;

    /* FT_ENCODING_UNICODE is special.  We try to find the `best' Unicode */
    /* charmap available, i.e., one with UCS-4 characters, if possible.   */
    /*                                                                    */
    /* This is done by find_unicode_charmap() above, to share code.       */
    if ( encoding == FT_ENCODING_UNICODE )
      return find_unicode_charmap( face );

    cur = face->charmaps;
    if ( !cur )
      return FT_Err_Invalid_CharMap_Handle;

    limit = cur + face->num_charmaps;

    for ( ; cur < limit; cur++ )
    {
      if ( cur[0]->encoding == encoding )
      {
#ifdef FT_MAX_CHARMAP_CACHEABLE
        if ( cur - face->charmaps > FT_MAX_CHARMAP_CACHEABLE )
        {
          FT_ERROR(( "FT_Select_Charmap: requested charmap is found (%d), "
                     "but in too late position to cache\n",
                     cur - face->charmaps ));
          continue;
        }
#endif
        face->charmap = cur[0];
        return 0;
      }
    }

    return FT_Err_Invalid_Argument;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Set_Charmap( FT_Face     face,
                  FT_CharMap  charmap )
  {
    FT_CharMap*  cur;
    FT_CharMap*  limit;


    if ( !face )
      return FT_Err_Invalid_Face_Handle;

    cur = face->charmaps;
    if ( !cur )
      return FT_Err_Invalid_CharMap_Handle;
    if ( FT_Get_CMap_Format( charmap ) == 14 )
      return FT_Err_Invalid_Argument;

    limit = cur + face->num_charmaps;

    for ( ; cur < limit; cur++ )
    {
      if ( cur[0] == charmap )
      {
#ifdef FT_MAX_CHARMAP_CACHEABLE
        if ( cur - face->charmaps > FT_MAX_CHARMAP_CACHEABLE )
        {
          FT_ERROR(( "FT_Set_Charmap: requested charmap is found (%d), "
                     "but in too late position to cache\n",
                     cur - face->charmaps ));
          continue;
        }
#endif
        face->charmap = cur[0];
        return 0;
      }
    }
    return FT_Err_Invalid_Argument;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Int )
  FT_Get_Charmap_Index( FT_CharMap  charmap )
  {
    FT_Int  i;


    if ( !charmap || !charmap->face )
      return -1;

    for ( i = 0; i < charmap->face->num_charmaps; i++ )
      if ( charmap->face->charmaps[i] == charmap )
        break;

    FT_ASSERT( i < charmap->face->num_charmaps );

#ifdef FT_MAX_CHARMAP_CACHEABLE
    if ( i > FT_MAX_CHARMAP_CACHEABLE )
    {
      FT_ERROR(( "FT_Get_Charmap_Index: requested charmap is found (%d), "
                 "but in too late position to cache\n",
                 i ));
      return -i;
    }
#endif
    return i;
  }


  static void
  ft_cmap_done_internal( FT_CMap  cmap )
  {
    FT_CMap_Class  clazz  = cmap->clazz;
    FT_Face        face   = cmap->charmap.face;
    FT_Memory      memory = FT_FACE_MEMORY(face);


    if ( clazz->done )
      clazz->done( cmap );

    FT_FREE( cmap );
  }


  FT_BASE_DEF( void )
  FT_CMap_Done( FT_CMap  cmap )
  {
    if ( cmap )
    {
      FT_Face    face   = cmap->charmap.face;
      FT_Memory  memory = FT_FACE_MEMORY( face );
      FT_Error   error;
      FT_Int     i, j;


      for ( i = 0; i < face->num_charmaps; i++ )
      {
        if ( (FT_CMap)face->charmaps[i] == cmap )
        {
          FT_CharMap  last_charmap = face->charmaps[face->num_charmaps - 1];


          if ( FT_RENEW_ARRAY( face->charmaps,
                               face->num_charmaps,
                               face->num_charmaps - 1 ) )
            return;

          /* remove it from our list of charmaps */
          for ( j = i + 1; j < face->num_charmaps; j++ )
          {
            if ( j == face->num_charmaps - 1 )
              face->charmaps[j - 1] = last_charmap;
            else
              face->charmaps[j - 1] = face->charmaps[j];
          }

          face->num_charmaps--;

          if ( (FT_CMap)face->charmap == cmap )
            face->charmap = NULL;

          ft_cmap_done_internal( cmap );

          break;
        }
      }
    }
  }


  FT_BASE_DEF( FT_Error )
  FT_CMap_New( FT_CMap_Class  clazz,
               FT_Pointer     init_data,
               FT_CharMap     charmap,
               FT_CMap       *acmap )
  {
    FT_Error   error = FT_Err_Ok;
    FT_Face    face;
    FT_Memory  memory;
    FT_CMap    cmap;


    if ( clazz == NULL || charmap == NULL || charmap->face == NULL )
      return FT_Err_Invalid_Argument;

    face   = charmap->face;
    memory = FT_FACE_MEMORY( face );

    if ( !FT_ALLOC( cmap, clazz->size ) )
    {
      cmap->charmap = *charmap;
      cmap->clazz   = clazz;

      if ( clazz->init )
      {
        error = clazz->init( cmap, init_data );
        if ( error )
          goto Fail;
      }

      /* add it to our list of charmaps */
      if ( FT_RENEW_ARRAY( face->charmaps,
                           face->num_charmaps,
                           face->num_charmaps + 1 ) )
        goto Fail;

      face->charmaps[face->num_charmaps++] = (FT_CharMap)cmap;
    }

  Exit:
    if ( acmap )
      *acmap = cmap;

    return error;

  Fail:
    ft_cmap_done_internal( cmap );
    cmap = NULL;
    goto Exit;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_UInt )
  FT_Get_Char_Index( FT_Face   face,
                     FT_ULong  charcode )
  {
    FT_UInt  result = 0;


    if ( face && face->charmap )
    {
      FT_CMap  cmap = FT_CMAP( face->charmap );


      if ( charcode > 0xFFFFFFFFUL )
      {
        FT_TRACE1(( "FT_Get_Char_Index: too large charcode" ));
        FT_TRACE1(( " 0x%x is truncated\n", charcode ));
      }
      result = cmap->clazz->char_index( cmap, (FT_UInt32)charcode );
    }
    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_ULong )
  FT_Get_First_Char( FT_Face   face,
                     FT_UInt  *agindex )
  {
    FT_ULong  result = 0;
    FT_UInt   gindex = 0;


    if ( face && face->charmap && face->num_glyphs )
    {
      gindex = FT_Get_Char_Index( face, 0 );
      if ( gindex == 0 || gindex >= (FT_UInt)face->num_glyphs )
        result = FT_Get_Next_Char( face, 0, &gindex );
    }

    if ( agindex )
      *agindex = gindex;

    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_ULong )
  FT_Get_Next_Char( FT_Face   face,
                    FT_ULong  charcode,
                    FT_UInt  *agindex )
  {
    FT_ULong  result = 0;
    FT_UInt   gindex = 0;


    if ( face && face->charmap && face->num_glyphs )
    {
      FT_UInt32  code = (FT_UInt32)charcode;
      FT_CMap    cmap = FT_CMAP( face->charmap );


      do {
        gindex = cmap->clazz->char_next( cmap, &code );
      } while ( gindex >= (FT_UInt)face->num_glyphs );

      result = ( gindex == 0 ) ? 0 : code;
    }

    if ( agindex )
      *agindex = gindex;

    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_UInt )
  FT_Face_GetCharVariantIndex( FT_Face   face,
                               FT_ULong  charcode,
                               FT_ULong  variantSelector )
  {
    FT_UInt  result = 0;


    if ( face && face->charmap &&
        face->charmap->encoding == FT_ENCODING_UNICODE )
    {
      FT_CharMap  charmap = find_variant_selector_charmap( face );
      FT_CMap     ucmap = FT_CMAP( face->charmap );


      if ( charmap != NULL )
      {
        FT_CMap  vcmap = FT_CMAP( charmap );


        if ( charcode > 0xFFFFFFFFUL )
        {
          FT_TRACE1(( "FT_Get_Char_Index: too large charcode" ));
          FT_TRACE1(( " 0x%x is truncated\n", charcode ));
        }
        if ( variantSelector > 0xFFFFFFFFUL )
        {
          FT_TRACE1(( "FT_Get_Char_Index: too large variantSelector" ));
          FT_TRACE1(( " 0x%x is truncated\n", variantSelector ));
        }

        result = vcmap->clazz->char_var_index( vcmap, ucmap,
                                               (FT_UInt32)charcode,
                                               (FT_UInt32)variantSelector );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Int )
  FT_Face_GetCharVariantIsDefault( FT_Face   face,
                                   FT_ULong  charcode,
                                   FT_ULong  variantSelector )
  {
    FT_Int  result = -1;


    if ( face )
    {
      FT_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap != NULL )
      {
        FT_CMap  vcmap = FT_CMAP( charmap );


        if ( charcode > 0xFFFFFFFFUL )
        {
          FT_TRACE1(( "FT_Get_Char_Index: too large charcode" ));
          FT_TRACE1(( " 0x%x is truncated\n", charcode ));
        }
        if ( variantSelector > 0xFFFFFFFFUL )
        {
          FT_TRACE1(( "FT_Get_Char_Index: too large variantSelector" ));
          FT_TRACE1(( " 0x%x is truncated\n", variantSelector ));
        }

        result = vcmap->clazz->char_var_default( vcmap,
                                                 (FT_UInt32)charcode,
                                                 (FT_UInt32)variantSelector );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_UInt32* )
  FT_Face_GetVariantSelectors( FT_Face  face )
  {
    FT_UInt32  *result = NULL;


    if ( face )
    {
      FT_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap != NULL )
      {
        FT_CMap    vcmap  = FT_CMAP( charmap );
        FT_Memory  memory = FT_FACE_MEMORY( face );


        result = vcmap->clazz->variant_list( vcmap, memory );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_UInt32* )
  FT_Face_GetVariantsOfChar( FT_Face   face,
                             FT_ULong  charcode )
  {
    FT_UInt32  *result = NULL;


    if ( face )
    {
      FT_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap != NULL )
      {
        FT_CMap    vcmap  = FT_CMAP( charmap );
        FT_Memory  memory = FT_FACE_MEMORY( face );


        if ( charcode > 0xFFFFFFFFUL )
        {
          FT_TRACE1(( "FT_Get_Char_Index: too large charcode" ));
          FT_TRACE1(( " 0x%x is truncated\n", charcode ));
        }

        result = vcmap->clazz->charvariant_list( vcmap, memory,
                                                 (FT_UInt32)charcode );
      }
    }
    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_UInt32* )
  FT_Face_GetCharsOfVariant( FT_Face   face,
                             FT_ULong  variantSelector )
  {
    FT_UInt32  *result = NULL;


    if ( face )
    {
      FT_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap != NULL )
      {
        FT_CMap    vcmap  = FT_CMAP( charmap );
        FT_Memory  memory = FT_FACE_MEMORY( face );


        if ( variantSelector > 0xFFFFFFFFUL )
        {
          FT_TRACE1(( "FT_Get_Char_Index: too large variantSelector" ));
          FT_TRACE1(( " 0x%x is truncated\n", variantSelector ));
        }

        result = vcmap->clazz->variantchar_list( vcmap, memory,
                                                 (FT_UInt32)variantSelector );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_UInt )
  FT_Get_Name_Index( FT_Face     face,
                     FT_String*  glyph_name )
  {
    FT_UInt  result = 0;


    if ( face && FT_HAS_GLYPH_NAMES( face ) )
    {
      FT_Service_GlyphDict  service;


      FT_FACE_LOOKUP_SERVICE( face,
                              service,
                              GLYPH_DICT );

      if ( service && service->name_index )
        result = service->name_index( face, glyph_name );
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Get_Glyph_Name( FT_Face     face,
                     FT_UInt     glyph_index,
                     FT_Pointer  buffer,
                     FT_UInt     buffer_max )
  {
    FT_Error  error = FT_Err_Invalid_Argument;


    /* clean up buffer */
    if ( buffer && buffer_max > 0 )
      ((FT_Byte*)buffer)[0] = 0;

    if ( face                                     &&
         (FT_Long)glyph_index <= face->num_glyphs &&
         FT_HAS_GLYPH_NAMES( face )               )
    {
      FT_Service_GlyphDict  service;


      FT_FACE_LOOKUP_SERVICE( face,
                              service,
                              GLYPH_DICT );

      if ( service && service->get_name )
        error = service->get_name( face, glyph_index, buffer, buffer_max );
    }

    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( const char* )
  FT_Get_Postscript_Name( FT_Face  face )
  {
    const char*  result = NULL;


    if ( !face )
      goto Exit;

    if ( !result )
    {
      FT_Service_PsFontName  service;


      FT_FACE_LOOKUP_SERVICE( face,
                              service,
                              POSTSCRIPT_FONT_NAME );

      if ( service && service->get_ps_font_name )
        result = service->get_ps_font_name( face );
    }

  Exit:
    return result;
  }


  /* documentation is in tttables.h */

  FT_EXPORT_DEF( void* )
  FT_Get_Sfnt_Table( FT_Face      face,
                     FT_Sfnt_Tag  tag )
  {
    void*                  table = 0;
    FT_Service_SFNT_Table  service;


    if ( face && FT_IS_SFNT( face ) )
    {
      FT_FACE_FIND_SERVICE( face, service, SFNT_TABLE );
      if ( service != NULL )
        table = service->get_table( face, tag );
    }

    return table;
  }


  /* documentation is in tttables.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Load_Sfnt_Table( FT_Face    face,
                      FT_ULong   tag,
                      FT_Long    offset,
                      FT_Byte*   buffer,
                      FT_ULong*  length )
  {
    FT_Service_SFNT_Table  service;


    if ( !face || !FT_IS_SFNT( face ) )
      return FT_Err_Invalid_Face_Handle;

    FT_FACE_FIND_SERVICE( face, service, SFNT_TABLE );
    if ( service == NULL )
      return FT_Err_Unimplemented_Feature;

    return service->load_table( face, tag, offset, buffer, length );
  }


  /* documentation is in tttables.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Sfnt_Table_Info( FT_Face    face,
                      FT_UInt    table_index,
                      FT_ULong  *tag,
                      FT_ULong  *length )
  {
    FT_Service_SFNT_Table  service;
    FT_ULong               offset;


    if ( !face || !FT_IS_SFNT( face ) )
      return FT_Err_Invalid_Face_Handle;

    FT_FACE_FIND_SERVICE( face, service, SFNT_TABLE );
    if ( service == NULL )
      return FT_Err_Unimplemented_Feature;

    return service->table_info( face, table_index, tag, &offset, length );
  }


  /* documentation is in tttables.h */

  FT_EXPORT_DEF( FT_ULong )
  FT_Get_CMap_Language_ID( FT_CharMap  charmap )
  {
    FT_Service_TTCMaps  service;
    FT_Face             face;
    TT_CMapInfo         cmap_info;


    if ( !charmap || !charmap->face )
      return 0;

    face = charmap->face;
    FT_FACE_FIND_SERVICE( face, service, TT_CMAP );
    if ( service == NULL )
      return 0;
    if ( service->get_cmap_info( charmap, &cmap_info ))
      return 0;

    return cmap_info.language;
  }


  /* documentation is in tttables.h */

  FT_EXPORT_DEF( FT_Long )
  FT_Get_CMap_Format( FT_CharMap  charmap )
  {
    FT_Service_TTCMaps  service;
    FT_Face             face;
    TT_CMapInfo         cmap_info;


    if ( !charmap || !charmap->face )
      return -1;

    face = charmap->face;
    FT_FACE_FIND_SERVICE( face, service, TT_CMAP );
    if ( service == NULL )
      return -1;
    if ( service->get_cmap_info( charmap, &cmap_info ))
      return -1;

    return cmap_info.format;
  }


  /* documentation is in ftsizes.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Activate_Size( FT_Size  size )
  {
    FT_Face  face;


    if ( size == NULL )
      return FT_Err_Invalid_Argument;

    face = size->face;
    if ( face == NULL || face->driver == NULL )
      return FT_Err_Invalid_Argument;

    /* we don't need anything more complex than that; all size objects */
    /* are already listed by the face                                  */
    face->size = size;

    return FT_Err_Ok;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        R E N D E R E R S                        ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /* lookup a renderer by glyph format in the library's list */
  FT_BASE_DEF( FT_Renderer )
  FT_Lookup_Renderer( FT_Library       library,
                      FT_Glyph_Format  format,
                      FT_ListNode*     node )
  {
    FT_ListNode  cur;
    FT_Renderer  result = 0;


    if ( !library )
      goto Exit;

    cur = library->renderers.head;

    if ( node )
    {
      if ( *node )
        cur = (*node)->next;
      *node = 0;
    }

    while ( cur )
    {
      FT_Renderer  renderer = FT_RENDERER( cur->data );


      if ( renderer->glyph_format == format )
      {
        if ( node )
          *node = cur;

        result = renderer;
        break;
      }
      cur = cur->next;
    }

  Exit:
    return result;
  }


  static FT_Renderer
  ft_lookup_glyph_renderer( FT_GlyphSlot  slot )
  {
    FT_Face      face    = slot->face;
    FT_Library   library = FT_FACE_LIBRARY( face );
    FT_Renderer  result  = library->cur_renderer;


    if ( !result || result->glyph_format != slot->format )
      result = FT_Lookup_Renderer( library, slot->format, 0 );

    return result;
  }


  static void
  ft_set_current_renderer( FT_Library  library )
  {
    FT_Renderer  renderer;


    renderer = FT_Lookup_Renderer( library, FT_GLYPH_FORMAT_OUTLINE, 0 );
    library->cur_renderer = renderer;
  }


  static FT_Error
  ft_add_renderer( FT_Module  module )
  {
    FT_Library   library = module->library;
    FT_Memory    memory  = library->memory;
    FT_Error     error;
    FT_ListNode  node    = NULL;


    if ( FT_NEW( node ) )
      goto Exit;

    {
      FT_Renderer         render = FT_RENDERER( module );
      FT_Renderer_Class*  clazz  = (FT_Renderer_Class*)module->clazz;


      render->clazz        = clazz;
      render->glyph_format = clazz->glyph_format;

      /* allocate raster object if needed */
      if ( clazz->glyph_format == FT_GLYPH_FORMAT_OUTLINE &&
           clazz->raster_class->raster_new                )
      {
        error = clazz->raster_class->raster_new( memory, &render->raster );
        if ( error )
          goto Fail;

        render->raster_render = clazz->raster_class->raster_render;
        render->render        = clazz->render_glyph;
      }

      /* add to list */
      node->data = module;
      FT_List_Add( &library->renderers, node );

      ft_set_current_renderer( library );
    }

  Fail:
    if ( error )
      FT_FREE( node );

  Exit:
    return error;
  }


  static void
  ft_remove_renderer( FT_Module  module )
  {
    FT_Library   library = module->library;
    FT_Memory    memory  = library->memory;
    FT_ListNode  node;


    node = FT_List_Find( &library->renderers, module );
    if ( node )
    {
      FT_Renderer  render = FT_RENDERER( module );


      /* release raster object, if any */
      if ( render->clazz->glyph_format == FT_GLYPH_FORMAT_OUTLINE &&
           render->raster                                         )
        render->clazz->raster_class->raster_done( render->raster );

      /* remove from list */
      FT_List_Remove( &library->renderers, node );
      FT_FREE( node );

      ft_set_current_renderer( library );
    }
  }


  /* documentation is in ftrender.h */

  FT_EXPORT_DEF( FT_Renderer )
  FT_Get_Renderer( FT_Library       library,
                   FT_Glyph_Format  format )
  {
    /* test for valid `library' delayed to FT_Lookup_Renderer() */

    return FT_Lookup_Renderer( library, format, 0 );
  }


  /* documentation is in ftrender.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Set_Renderer( FT_Library     library,
                   FT_Renderer    renderer,
                   FT_UInt        num_params,
                   FT_Parameter*  parameters )
  {
    FT_ListNode  node;
    FT_Error     error = FT_Err_Ok;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( !renderer )
      return FT_Err_Invalid_Argument;

    node = FT_List_Find( &library->renderers, renderer );
    if ( !node )
    {
      error = FT_Err_Invalid_Argument;
      goto Exit;
    }

    FT_List_Up( &library->renderers, node );

    if ( renderer->glyph_format == FT_GLYPH_FORMAT_OUTLINE )
      library->cur_renderer = renderer;

    if ( num_params > 0 )
    {
      FT_Renderer_SetModeFunc  set_mode = renderer->clazz->set_mode;


      for ( ; num_params > 0; num_params-- )
      {
        error = set_mode( renderer, parameters->tag, parameters->data );
        if ( error )
          break;
      }
    }

  Exit:
    return error;
  }


  FT_BASE_DEF( FT_Error )
  FT_Render_Glyph_Internal( FT_Library      library,
                            FT_GlyphSlot    slot,
                            FT_Render_Mode  render_mode )
  {
    FT_Error     error = FT_Err_Ok;
    FT_Renderer  renderer;


    /* if it is already a bitmap, no need to do anything */
    switch ( slot->format )
    {
    case FT_GLYPH_FORMAT_BITMAP:   /* already a bitmap, don't do anything */
      break;

    default:
      {
        FT_ListNode  node   = 0;
        FT_Bool      update = 0;


        /* small shortcut for the very common case */
        if ( slot->format == FT_GLYPH_FORMAT_OUTLINE )
        {
          renderer = library->cur_renderer;
          node     = library->renderers.head;
        }
        else
          renderer = FT_Lookup_Renderer( library, slot->format, &node );

        error = FT_Err_Unimplemented_Feature;
        while ( renderer )
        {
          error = renderer->render( renderer, slot, render_mode, NULL );
          if ( !error                                               ||
               FT_ERROR_BASE( error ) != FT_Err_Cannot_Render_Glyph )
            break;

          /* FT_Err_Cannot_Render_Glyph is returned if the render mode   */
          /* is unsupported by the current renderer for this glyph image */
          /* format.                                                     */

          /* now, look for another renderer that supports the same */
          /* format.                                               */
          renderer = FT_Lookup_Renderer( library, slot->format, &node );
          update   = 1;
        }

        /* if we changed the current renderer for the glyph image format */
        /* we need to select it as the next current one                  */
        if ( !error && update && renderer )
          FT_Set_Renderer( library, renderer, 0, 0 );
      }
    }

    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Render_Glyph( FT_GlyphSlot    slot,
                   FT_Render_Mode  render_mode )
  {
    FT_Library  library;


    if ( !slot || !slot->face )
      return FT_Err_Invalid_Argument;

    library = FT_FACE_LIBRARY( slot->face );

    return FT_Render_Glyph_Internal( library, slot, render_mode );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                         M O D U L E S                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    Destroy_Module                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroys a given module object.  For drivers, this also destroys   */
  /*    all child faces.                                                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*     module :: A handle to the target driver object.                   */
  /*                                                                       */
  /* <Note>                                                                */
  /*     The driver _must_ be LOCKED!                                      */
  /*                                                                       */
  static void
  Destroy_Module( FT_Module  module )
  {
    FT_Memory         memory  = module->memory;
    FT_Module_Class*  clazz   = module->clazz;
    FT_Library        library = module->library;


    /* finalize client-data - before anything else */
    if ( module->generic.finalizer )
      module->generic.finalizer( module );

    if ( library && library->auto_hinter == module )
      library->auto_hinter = 0;

    /* if the module is a renderer */
    if ( FT_MODULE_IS_RENDERER( module ) )
      ft_remove_renderer( module );

    /* if the module is a font driver, add some steps */
    if ( FT_MODULE_IS_DRIVER( module ) )
      Destroy_Driver( FT_DRIVER( module ) );

    /* finalize the module object */
    if ( clazz->module_done )
      clazz->module_done( module );

    /* discard it */
    FT_FREE( module );
  }


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Add_Module( FT_Library              library,
                 const FT_Module_Class*  clazz )
  {
    FT_Error   error;
    FT_Memory  memory;
    FT_Module  module;
    FT_UInt    nn;


#define FREETYPE_VER_FIXED  ( ( (FT_Long)FREETYPE_MAJOR << 16 ) | \
                                FREETYPE_MINOR                  )

    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( !clazz )
      return FT_Err_Invalid_Argument;

    /* check freetype version */
    if ( clazz->module_requires > FREETYPE_VER_FIXED )
      return FT_Err_Invalid_Version;

    /* look for a module with the same name in the library's table */
    for ( nn = 0; nn < library->num_modules; nn++ )
    {
      module = library->modules[nn];
      if ( ft_strcmp( module->clazz->module_name, clazz->module_name ) == 0 )
      {
        /* this installed module has the same name, compare their versions */
        if ( clazz->module_version <= module->clazz->module_version )
          return FT_Err_Lower_Module_Version;

        /* remove the module from our list, then exit the loop to replace */
        /* it by our new version..                                        */
        FT_Remove_Module( library, module );
        break;
      }
    }

    memory = library->memory;
    error  = FT_Err_Ok;

    if ( library->num_modules >= FT_MAX_MODULES )
    {
      error = FT_Err_Too_Many_Drivers;
      goto Exit;
    }

    /* allocate module object */
    if ( FT_ALLOC( module, clazz->module_size ) )
      goto Exit;

    /* base initialization */
    module->library = library;
    module->memory  = memory;
    module->clazz   = (FT_Module_Class*)clazz;

    /* check whether the module is a renderer - this must be performed */
    /* before the normal module initialization                         */
    if ( FT_MODULE_IS_RENDERER( module ) )
    {
      /* add to the renderers list */
      error = ft_add_renderer( module );
      if ( error )
        goto Fail;
    }

    /* is the module a auto-hinter? */
    if ( FT_MODULE_IS_HINTER( module ) )
      library->auto_hinter = module;

    /* if the module is a font driver */
    if ( FT_MODULE_IS_DRIVER( module ) )
    {
      /* allocate glyph loader if needed */
      FT_Driver  driver = FT_DRIVER( module );


      driver->clazz = (FT_Driver_Class)module->clazz;
      if ( FT_DRIVER_USES_OUTLINES( driver ) )
      {
        error = FT_GlyphLoader_New( memory, &driver->glyph_loader );
        if ( error )
          goto Fail;
      }
    }

    if ( clazz->module_init )
    {
      error = clazz->module_init( module );
      if ( error )
        goto Fail;
    }

    /* add module to the library's table */
    library->modules[library->num_modules++] = module;

  Exit:
    return error;

  Fail:
    if ( FT_MODULE_IS_DRIVER( module ) )
    {
      FT_Driver  driver = FT_DRIVER( module );


      if ( FT_DRIVER_USES_OUTLINES( driver ) )
        FT_GlyphLoader_Done( driver->glyph_loader );
    }

    if ( FT_MODULE_IS_RENDERER( module ) )
    {
      FT_Renderer  renderer = FT_RENDERER( module );


      if ( renderer->clazz->glyph_format == FT_GLYPH_FORMAT_OUTLINE && 
           renderer->raster                                         )
        renderer->clazz->raster_class->raster_done( renderer->raster );
    }

    FT_FREE( module );
    goto Exit;
  }


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( FT_Module )
  FT_Get_Module( FT_Library   library,
                 const char*  module_name )
  {
    FT_Module   result = 0;
    FT_Module*  cur;
    FT_Module*  limit;


    if ( !library || !module_name )
      return result;

    cur   = library->modules;
    limit = cur + library->num_modules;

    for ( ; cur < limit; cur++ )
      if ( ft_strcmp( cur[0]->clazz->module_name, module_name ) == 0 )
      {
        result = cur[0];
        break;
      }

    return result;
  }


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( const void* )
  FT_Get_Module_Interface( FT_Library   library,
                           const char*  mod_name )
  {
    FT_Module  module;


    /* test for valid `library' delayed to FT_Get_Module() */

    module = FT_Get_Module( library, mod_name );

    return module ? module->clazz->module_interface : 0;
  }


  FT_BASE_DEF( FT_Pointer )
  ft_module_get_service( FT_Module    module,
                         const char*  service_id )
  {
    FT_Pointer  result = NULL;

    if ( module )
    {
      FT_ASSERT( module->clazz && module->clazz->get_interface );

     /* first, look for the service in the module
      */
      if ( module->clazz->get_interface )
        result = module->clazz->get_interface( module, service_id );

      if ( result == NULL )
      {
       /* we didn't find it, look in all other modules then
        */
        FT_Library  library = module->library;
        FT_Module*  cur     = library->modules;
        FT_Module*  limit   = cur + library->num_modules;

        for ( ; cur < limit; cur++ )
        {
          if ( cur[0] != module )
          {
            FT_ASSERT( cur[0]->clazz );

            if ( cur[0]->clazz->get_interface )
            {
              result = cur[0]->clazz->get_interface( cur[0], service_id );
              if ( result != NULL )
                break;
            }
          }
        }
      }
    }

    return result;
  }


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Remove_Module( FT_Library  library,
                    FT_Module   module )
  {
    /* try to find the module from the table, then remove it from there */

    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( module )
    {
      FT_Module*  cur   = library->modules;
      FT_Module*  limit = cur + library->num_modules;


      for ( ; cur < limit; cur++ )
      {
        if ( cur[0] == module )
        {
          /* remove it from the table */
          library->num_modules--;
          limit--;
          while ( cur < limit )
          {
            cur[0] = cur[1];
            cur++;
          }
          limit[0] = 0;

          /* destroy the module */
          Destroy_Module( module );

          return FT_Err_Ok;
        }
      }
    }
    return FT_Err_Invalid_Driver_Handle;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                         L I B R A R Y                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Reference_Library( FT_Library  library )
  {
    library->refcount++;

    return FT_Err_Ok;
  }


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( FT_Error )
  FT_New_Library( FT_Memory    memory,
                  FT_Library  *alibrary )
  {
    FT_Library  library = NULL;
    FT_Error    error;


    if ( !memory )
      return FT_Err_Invalid_Argument;

#ifdef FT_DEBUG_LEVEL_ERROR
    /* init debugging support */
    ft_debug_init();
#endif

    /* first of all, allocate the library object */
    if ( FT_NEW( library ) )
      return error;

    library->memory = memory;

#ifdef FT_CONFIG_OPTION_PIC
    /* initialize position independent code containers */
    error = ft_pic_container_init( library );
    if ( error )
      goto Fail;
#endif

    /* allocate the render pool */
    library->raster_pool_size = FT_RENDER_POOL_SIZE;
#if FT_RENDER_POOL_SIZE > 0
    if ( FT_ALLOC( library->raster_pool, FT_RENDER_POOL_SIZE ) )
      goto Fail;
#endif

    library->version_major = FREETYPE_MAJOR;
    library->version_minor = FREETYPE_MINOR;
    library->version_patch = FREETYPE_PATCH;

    library->refcount = 1;

    /* That's ok now */
    *alibrary = library;

    return FT_Err_Ok;

  Fail:
#ifdef FT_CONFIG_OPTION_PIC
    ft_pic_container_destroy( library );
#endif
    FT_FREE( library );
    return error;
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( void )
  FT_Library_Version( FT_Library   library,
                      FT_Int      *amajor,
                      FT_Int      *aminor,
                      FT_Int      *apatch )
  {
    FT_Int  major = 0;
    FT_Int  minor = 0;
    FT_Int  patch = 0;


    if ( library )
    {
      major = library->version_major;
      minor = library->version_minor;
      patch = library->version_patch;
    }

    if ( amajor )
      *amajor = major;

    if ( aminor )
      *aminor = minor;

    if ( apatch )
      *apatch = patch;
  }


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Done_Library( FT_Library  library )
  {
    FT_Memory  memory;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    library->refcount--;
    if ( library->refcount > 0 )
      goto Exit;

    memory = library->memory;

    /* Discard client-data */
    if ( library->generic.finalizer )
      library->generic.finalizer( library );

    /*
     * Close all faces in the library.  If we don't do this, we can have
     * some subtle memory leaks.
     *
     * Example:
     *
     *  - the cff font driver uses the pshinter module in cff_size_done
     *  - if the pshinter module is destroyed before the cff font driver,
     *    opened FT_Face objects managed by the driver are not properly
     *    destroyed, resulting in a memory leak
     *
     * Some faces are dependent on other faces, like Type42 faces that
     * depend on TrueType faces synthesized internally.
     *
     * The order of drivers should be specified in driver_name[].
     */
    {
      FT_UInt      m, n;
      const char*  driver_name[] = { "type42", NULL };


      for ( m = 0;
            m < sizeof ( driver_name ) / sizeof ( driver_name[0] );
            m++ )
      {
        for ( n = 0; n < library->num_modules; n++ )
        {
          FT_Module    module      = library->modules[n];
          const char*  module_name = module->clazz->module_name;
          FT_List      faces;


          if ( driver_name[m]                                &&
               ft_strcmp( module_name, driver_name[m] ) != 0 )
            continue;

          if ( ( module->clazz->module_flags & FT_MODULE_FONT_DRIVER ) == 0 )
            continue;

          FT_TRACE7(( "FT_Done_Library: close faces for %s\n", module_name ));

          faces = &FT_DRIVER( module )->faces_list;
          while ( faces->head )
          {
            FT_Done_Face( FT_FACE( faces->head->data ) );
            if ( faces->head )
              FT_TRACE0(( "FT_Done_Library: failed to free some faces\n" ));
          }
        }
      }
    }

    /* Close all other modules in the library */
#if 1
    /* XXX Modules are removed in the reversed order so that  */
    /* type42 module is removed before truetype module.  This */
    /* avoids double free in some occasions.  It is a hack.   */
    while ( library->num_modules > 0 )
      FT_Remove_Module( library,
                        library->modules[library->num_modules - 1] );
#else
    {
      FT_UInt  n;


      for ( n = 0; n < library->num_modules; n++ )
      {
        FT_Module  module = library->modules[n];


        if ( module )
        {
          Destroy_Module( module );
          library->modules[n] = 0;
        }
      }
    }
#endif

    /* Destroy raster objects */
    FT_FREE( library->raster_pool );
    library->raster_pool_size = 0;

#ifdef FT_CONFIG_OPTION_PIC
    /* Destroy pic container contents */
    ft_pic_container_destroy( library );
#endif

    FT_FREE( library );

  Exit:
    return FT_Err_Ok;
  }


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( void )
  FT_Set_Debug_Hook( FT_Library         library,
                     FT_UInt            hook_index,
                     FT_DebugHook_Func  debug_hook )
  {
    if ( library && debug_hook &&
         hook_index <
           ( sizeof ( library->debug_hooks ) / sizeof ( void* ) ) )
      library->debug_hooks[hook_index] = debug_hook;
  }


  /* documentation is in ftmodapi.h */

  FT_EXPORT_DEF( FT_TrueTypeEngineType )
  FT_Get_TrueType_Engine_Type( FT_Library  library )
  {
    FT_TrueTypeEngineType  result = FT_TRUETYPE_ENGINE_TYPE_NONE;


    if ( library )
    {
      FT_Module  module = FT_Get_Module( library, "truetype" );


      if ( module )
      {
        FT_Service_TrueTypeEngine  service;


        service = (FT_Service_TrueTypeEngine)
                    ft_module_get_service( module,
                                           FT_SERVICE_ID_TRUETYPE_ENGINE );
        if ( service )
          result = service->engine_type;
      }
    }

    return result;
  }


#ifdef FT_CONFIG_OPTION_OLD_INTERNALS

  FT_BASE_DEF( FT_Error )
  ft_stub_set_char_sizes( FT_Size     size,
                          FT_F26Dot6  width,
                          FT_F26Dot6  height,
                          FT_UInt     horz_res,
                          FT_UInt     vert_res )
  {
    FT_Size_RequestRec  req;
    FT_Driver           driver = size->face->driver;


    if ( driver->clazz->request_size )
    {
      req.type   = FT_SIZE_REQUEST_TYPE_NOMINAL;
      req.width  = width;
      req.height = height;

      if ( horz_res == 0 )
        horz_res = vert_res;

      if ( vert_res == 0 )
        vert_res = horz_res;

      if ( horz_res == 0 )
        horz_res = vert_res = 72;

      req.horiResolution = horz_res;
      req.vertResolution = vert_res;

      return driver->clazz->request_size( size, &req );
    }

    return 0;
  }


  FT_BASE_DEF( FT_Error )
  ft_stub_set_pixel_sizes( FT_Size  size,
                           FT_UInt  width,
                           FT_UInt  height )
  {
    FT_Size_RequestRec  req;
    FT_Driver           driver = size->face->driver;


    if ( driver->clazz->request_size )
    {
      req.type           = FT_SIZE_REQUEST_TYPE_NOMINAL;
      req.width          = width  << 6;
      req.height         = height << 6;
      req.horiResolution = 0;
      req.vertResolution = 0;

      return driver->clazz->request_size( size, &req );
    }

    return 0;
  }

#endif /* FT_CONFIG_OPTION_OLD_INTERNALS */


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Get_SubGlyph_Info( FT_GlyphSlot  glyph,
                        FT_UInt       sub_index,
                        FT_Int       *p_index,
                        FT_UInt      *p_flags,
                        FT_Int       *p_arg1,
                        FT_Int       *p_arg2,
                        FT_Matrix    *p_transform )
  {
    FT_Error  error = FT_Err_Invalid_Argument;


    if ( glyph                                      &&
         glyph->subglyphs                           &&
         glyph->format == FT_GLYPH_FORMAT_COMPOSITE &&
         sub_index < glyph->num_subglyphs           )
    {
      FT_SubGlyph  subg = glyph->subglyphs + sub_index;


      *p_index     = subg->index;
      *p_flags     = subg->flags;
      *p_arg1      = subg->arg1;
      *p_arg2      = subg->arg2;
      *p_transform = subg->transform;
    }

    return error;
  }


/* END */
