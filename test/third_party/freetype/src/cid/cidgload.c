/***************************************************************************/
/*                                                                         */
/*  cidgload.c                                                             */
/*                                                                         */
/*    CID-keyed Type1 Glyph Loader (body).                                 */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010 by */
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
#include "cidload.h"
#include "cidgload.h"
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_OUTLINE_H
#include FT_INTERNAL_CALC_H

#include "ciderrs.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_cidgload


  FT_CALLBACK_DEF( FT_Error )
  cid_load_glyph( T1_Decoder  decoder,
                  FT_UInt     glyph_index )
  {
    CID_Face       face = (CID_Face)decoder->builder.face;
    CID_FaceInfo   cid  = &face->cid;
    FT_Byte*       p;
    FT_UInt        fd_select;
    FT_Stream      stream       = face->cid_stream;
    FT_Error       error        = CID_Err_Ok;
    FT_Byte*       charstring   = 0;
    FT_Memory      memory       = face->root.memory;
    FT_ULong       glyph_length = 0;
    PSAux_Service  psaux        = (PSAux_Service)face->psaux;

#ifdef FT_CONFIG_OPTION_INCREMENTAL
    FT_Incremental_InterfaceRec *inc =
                                  face->root.internal->incremental_interface;
#endif


    FT_TRACE4(( "cid_load_glyph: glyph index %d\n", glyph_index ));

#ifdef FT_CONFIG_OPTION_INCREMENTAL

    /* For incremental fonts get the character data using */
    /* the callback function.                             */
    if ( inc )
    {
      FT_Data  glyph_data;


      error = inc->funcs->get_glyph_data( inc->object,
                                          glyph_index, &glyph_data );
      if ( error )
        goto Exit;

      p         = (FT_Byte*)glyph_data.pointer;
      fd_select = (FT_UInt)cid_get_offset( &p, (FT_Byte)cid->fd_bytes );

      if ( glyph_data.length != 0 )
      {
        glyph_length = glyph_data.length - cid->fd_bytes;
        (void)FT_ALLOC( charstring, glyph_length );
        if ( !error )
          ft_memcpy( charstring, glyph_data.pointer + cid->fd_bytes,
                     glyph_length );
      }

      inc->funcs->free_glyph_data( inc->object, &glyph_data );

      if ( error )
        goto Exit;
    }

    else

#endif /* FT_CONFIG_OPTION_INCREMENTAL */

    /* For ordinary fonts read the CID font dictionary index */
    /* and charstring offset from the CIDMap.                */
    {
      FT_UInt   entry_len = cid->fd_bytes + cid->gd_bytes;
      FT_ULong  off1;


      if ( FT_STREAM_SEEK( cid->data_offset + cid->cidmap_offset +
                           glyph_index * entry_len )               ||
           FT_FRAME_ENTER( 2 * entry_len )                         )
        goto Exit;

      p            = (FT_Byte*)stream->cursor;
      fd_select    = (FT_UInt) cid_get_offset( &p, (FT_Byte)cid->fd_bytes );
      off1         = (FT_ULong)cid_get_offset( &p, (FT_Byte)cid->gd_bytes );
      p           += cid->fd_bytes;
      glyph_length = cid_get_offset( &p, (FT_Byte)cid->gd_bytes ) - off1;
      FT_FRAME_EXIT();

      if ( fd_select >= (FT_UInt)cid->num_dicts )
      {
        error = CID_Err_Invalid_Offset;
        goto Exit;
      }
      if ( glyph_length == 0 )
        goto Exit;
      if ( FT_ALLOC( charstring, glyph_length ) )
        goto Exit;
      if ( FT_STREAM_READ_AT( cid->data_offset + off1,
                              charstring, glyph_length ) )
        goto Exit;
    }

    /* Now set up the subrs array and parse the charstrings. */
    {
      CID_FaceDict  dict;
      CID_Subrs     cid_subrs = face->subrs + fd_select;
      FT_Int        cs_offset;


      /* Set up subrs */
      decoder->num_subrs = cid_subrs->num_subrs;
      decoder->subrs     = cid_subrs->code;
      decoder->subrs_len = 0;

      /* Set up font matrix */
      dict                 = cid->font_dicts + fd_select;

      decoder->font_matrix = dict->font_matrix;
      decoder->font_offset = dict->font_offset;
      decoder->lenIV       = dict->private_dict.lenIV;

      /* Decode the charstring. */

      /* Adjustment for seed bytes. */
      cs_offset = ( decoder->lenIV >= 0 ? decoder->lenIV : 0 );

      /* Decrypt only if lenIV >= 0. */
      if ( decoder->lenIV >= 0 )
        psaux->t1_decrypt( charstring, glyph_length, 4330 );

      error = decoder->funcs.parse_charstrings(
                decoder, charstring + cs_offset,
                (FT_Int)glyph_length - cs_offset );
    }

    FT_FREE( charstring );

#ifdef FT_CONFIG_OPTION_INCREMENTAL

    /* Incremental fonts can optionally override the metrics. */
    if ( !error && inc && inc->funcs->get_glyph_metrics )
    {
      FT_Incremental_MetricsRec  metrics;


      metrics.bearing_x = FIXED_TO_INT( decoder->builder.left_bearing.x );
      metrics.bearing_y = 0;
      metrics.advance   = FIXED_TO_INT( decoder->builder.advance.x );
      metrics.advance_v = FIXED_TO_INT( decoder->builder.advance.y );

      error = inc->funcs->get_glyph_metrics( inc->object,
                                             glyph_index, FALSE, &metrics );

      decoder->builder.left_bearing.x = INT_TO_FIXED( metrics.bearing_x );
      decoder->builder.advance.x      = INT_TO_FIXED( metrics.advance );
      decoder->builder.advance.y      = INT_TO_FIXED( metrics.advance_v );
    }

#endif /* FT_CONFIG_OPTION_INCREMENTAL */

  Exit:
    return error;
  }


#if 0


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /**********                                                      *********/
  /**********                                                      *********/
  /**********            COMPUTE THE MAXIMUM ADVANCE WIDTH         *********/
  /**********                                                      *********/
  /**********    The following code is in charge of computing      *********/
  /**********    the maximum advance width of the font.  It        *********/
  /**********    quickly processes each glyph charstring to        *********/
  /**********    extract the value from either a `sbw' or `seac'   *********/
  /**********    operator.                                         *********/
  /**********                                                      *********/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  FT_LOCAL_DEF( FT_Error )
  cid_face_compute_max_advance( CID_Face  face,
                                FT_Int*   max_advance )
  {
    FT_Error       error;
    T1_DecoderRec  decoder;
    FT_Int         glyph_index;

    PSAux_Service  psaux = (PSAux_Service)face->psaux;


    *max_advance = 0;

    /* Initialize load decoder */
    error = psaux->t1_decoder_funcs->init( &decoder,
                                           (FT_Face)face,
                                           0, /* size       */
                                           0, /* glyph slot */
                                           0, /* glyph names! XXX */
                                           0, /* blend == 0 */
                                           0, /* hinting == 0 */
                                           cid_load_glyph );
    if ( error )
      return error;

    /* TODO: initialize decoder.len_buildchar and decoder.buildchar */
    /*       if we ever support CID-keyed multiple master fonts     */

    decoder.builder.metrics_only = 1;
    decoder.builder.load_points  = 0;

    /* for each glyph, parse the glyph charstring and extract */
    /* the advance width                                      */
    for ( glyph_index = 0; glyph_index < face->root.num_glyphs;
          glyph_index++ )
    {
      /* now get load the unscaled outline */
      error = cid_load_glyph( &decoder, glyph_index );
      /* ignore the error if one occurred - skip to next glyph */
    }

    *max_advance = FIXED_TO_INT( decoder.builder.advance.x );

    psaux->t1_decoder_funcs->done( &decoder );

    return CID_Err_Ok;
  }


#endif /* 0 */


  FT_LOCAL_DEF( FT_Error )
  cid_slot_load_glyph( FT_GlyphSlot  cidglyph,      /* CID_GlyphSlot */
                       FT_Size       cidsize,       /* CID_Size      */
                       FT_UInt       glyph_index,
                       FT_Int32      load_flags )
  {
    CID_GlyphSlot  glyph = (CID_GlyphSlot)cidglyph;
    FT_Error       error;
    T1_DecoderRec  decoder;
    CID_Face       face = (CID_Face)cidglyph->face;
    FT_Bool        hinting;

    PSAux_Service  psaux = (PSAux_Service)face->psaux;
    FT_Matrix      font_matrix;
    FT_Vector      font_offset;


    if ( glyph_index >= (FT_UInt)face->root.num_glyphs )
    {
      error = CID_Err_Invalid_Argument;
      goto Exit;
    }

    if ( load_flags & FT_LOAD_NO_RECURSE )
      load_flags |= FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING;

    glyph->x_scale = cidsize->metrics.x_scale;
    glyph->y_scale = cidsize->metrics.y_scale;

    cidglyph->outline.n_points   = 0;
    cidglyph->outline.n_contours = 0;

    hinting = FT_BOOL( ( load_flags & FT_LOAD_NO_SCALE   ) == 0 &&
                       ( load_flags & FT_LOAD_NO_HINTING ) == 0 );

    cidglyph->format = FT_GLYPH_FORMAT_OUTLINE;

    error = psaux->t1_decoder_funcs->init( &decoder,
                                           cidglyph->face,
                                           cidsize,
                                           cidglyph,
                                           0, /* glyph names -- XXX */
                                           0, /* blend == 0 */
                                           hinting,
                                           FT_LOAD_TARGET_MODE( load_flags ),
                                           cid_load_glyph );
    if ( error )
      goto Exit;

    /* TODO: initialize decoder.len_buildchar and decoder.buildchar */
    /*       if we ever support CID-keyed multiple master fonts     */

    /* set up the decoder */
    decoder.builder.no_recurse = FT_BOOL(
      ( ( load_flags & FT_LOAD_NO_RECURSE ) != 0 ) );

    error = cid_load_glyph( &decoder, glyph_index );
    if ( error )
      goto Exit;

    font_matrix = decoder.font_matrix;
    font_offset = decoder.font_offset;

    /* save new glyph tables */
    psaux->t1_decoder_funcs->done( &decoder );

    /* now set the metrics -- this is rather simple, as    */
    /* the left side bearing is the xMin, and the top side */
    /* bearing the yMax                                    */
    cidglyph->outline.flags &= FT_OUTLINE_OWNER;
    cidglyph->outline.flags |= FT_OUTLINE_REVERSE_FILL;

    /* for composite glyphs, return only left side bearing and */
    /* advance width                                           */
    if ( load_flags & FT_LOAD_NO_RECURSE )
    {
      FT_Slot_Internal  internal = cidglyph->internal;


      cidglyph->metrics.horiBearingX =
        FIXED_TO_INT( decoder.builder.left_bearing.x );
      cidglyph->metrics.horiAdvance =
        FIXED_TO_INT( decoder.builder.advance.x );

      internal->glyph_matrix      = font_matrix;
      internal->glyph_delta       = font_offset;
      internal->glyph_transformed = 1;
    }
    else
    {
      FT_BBox            cbox;
      FT_Glyph_Metrics*  metrics = &cidglyph->metrics;
      FT_Vector          advance;


      /* copy the _unscaled_ advance width */
      metrics->horiAdvance =
        FIXED_TO_INT( decoder.builder.advance.x );
      cidglyph->linearHoriAdvance =
        FIXED_TO_INT( decoder.builder.advance.x );
      cidglyph->internal->glyph_transformed = 0;

      /* make up vertical ones */
      metrics->vertAdvance        = ( face->cid.font_bbox.yMax -
                                      face->cid.font_bbox.yMin ) >> 16;
      cidglyph->linearVertAdvance = metrics->vertAdvance;

      cidglyph->format            = FT_GLYPH_FORMAT_OUTLINE;

      if ( cidsize->metrics.y_ppem < 24 )
        cidglyph->outline.flags |= FT_OUTLINE_HIGH_PRECISION;

      /* apply the font matrix */
      FT_Outline_Transform( &cidglyph->outline, &font_matrix );

      FT_Outline_Translate( &cidglyph->outline,
                            font_offset.x,
                            font_offset.y );

      advance.x = metrics->horiAdvance;
      advance.y = 0;
      FT_Vector_Transform( &advance, &font_matrix );
      metrics->horiAdvance = advance.x + font_offset.x;

      advance.x = 0;
      advance.y = metrics->vertAdvance;
      FT_Vector_Transform( &advance, &font_matrix );
      metrics->vertAdvance = advance.y + font_offset.y;

      if ( ( load_flags & FT_LOAD_NO_SCALE ) == 0 )
      {
        /* scale the outline and the metrics */
        FT_Int       n;
        FT_Outline*  cur = decoder.builder.base;
        FT_Vector*   vec = cur->points;
        FT_Fixed     x_scale = glyph->x_scale;
        FT_Fixed     y_scale = glyph->y_scale;


        /* First of all, scale the points */
        if ( !hinting || !decoder.builder.hints_funcs )
          for ( n = cur->n_points; n > 0; n--, vec++ )
          {
            vec->x = FT_MulFix( vec->x, x_scale );
            vec->y = FT_MulFix( vec->y, y_scale );
          }

        /* Then scale the metrics */
        metrics->horiAdvance = FT_MulFix( metrics->horiAdvance, x_scale );
        metrics->vertAdvance = FT_MulFix( metrics->vertAdvance, y_scale );
      }

      /* compute the other metrics */
      FT_Outline_Get_CBox( &cidglyph->outline, &cbox );

      metrics->width  = cbox.xMax - cbox.xMin;
      metrics->height = cbox.yMax - cbox.yMin;

      metrics->horiBearingX = cbox.xMin;
      metrics->horiBearingY = cbox.yMax;

      if ( load_flags & FT_LOAD_VERTICAL_LAYOUT ) 
      {
        /* make up vertical ones */
        ft_synthesize_vertical_metrics( metrics,
                                        metrics->vertAdvance );
      }
    }

  Exit:
    return error;
  }


/* END */
