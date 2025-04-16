/***************************************************************************/
/*                                                                         */
/*  ftoutln.c                                                              */
/*                                                                         */
/*    FreeType outline management (body).                                  */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010 by */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* All functions are declared in freetype.h.                             */
  /*                                                                       */
  /*************************************************************************/


#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H
#include FT_TRIGONOMETRY_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_outline


  static
  const FT_Outline  null_outline = { 0, 0, 0, 0, 0, 0 };


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Decompose( FT_Outline*              outline,
                        const FT_Outline_Funcs*  func_interface,
                        void*                    user )
  {
#undef SCALED
#define SCALED( x )  ( ( (x) << shift ) - delta )

    FT_Vector   v_last;
    FT_Vector   v_control;
    FT_Vector   v_start;

    FT_Vector*  point;
    FT_Vector*  limit;
    char*       tags;

    FT_Error    error;

    FT_Int   n;         /* index of contour in outline     */
    FT_UInt  first;     /* index of first point in contour */
    FT_Int   tag;       /* current point's state           */

    FT_Int   shift;
    FT_Pos   delta;


    if ( !outline || !func_interface )
      return FT_Err_Invalid_Argument;

    shift = func_interface->shift;
    delta = func_interface->delta;
    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      FT_Int  last;  /* index of last point in contour */


      FT_TRACE5(( "FT_Outline_Decompose: Outline %d\n", n ));

      last = outline->contours[n];
      if ( last < 0 )
        goto Invalid_Outline;
      limit = outline->points + last;

      v_start   = outline->points[first];
      v_start.x = SCALED( v_start.x );
      v_start.y = SCALED( v_start.y );

      v_last   = outline->points[last];
      v_last.x = SCALED( v_last.x );
      v_last.y = SCALED( v_last.y );

      v_control = v_start;

      point = outline->points + first;
      tags  = outline->tags   + first;
      tag   = FT_CURVE_TAG( tags[0] );

      /* A contour cannot start with a cubic control point! */
      if ( tag == FT_CURVE_TAG_CUBIC )
        goto Invalid_Outline;

      /* check first point to determine origin */
      if ( tag == FT_CURVE_TAG_CONIC )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( FT_CURVE_TAG( outline->tags[last] ) == FT_CURVE_TAG_ON )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
          limit--;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

          v_last = v_start;
        }
        point--;
        tags--;
      }

      FT_TRACE5(( "  move to (%.2f, %.2f)\n",
                  v_start.x / 64.0, v_start.y / 64.0 ));
      error = func_interface->move_to( &v_start, user );
      if ( error )
        goto Exit;

      while ( point < limit )
      {
        point++;
        tags++;

        tag = FT_CURVE_TAG( tags[0] );
        switch ( tag )
        {
        case FT_CURVE_TAG_ON:  /* emit a single line_to */
          {
            FT_Vector  vec;


            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            FT_TRACE5(( "  line to (%.2f, %.2f)\n",
                        vec.x / 64.0, vec.y / 64.0 ));
            error = func_interface->line_to( &vec, user );
            if ( error )
              goto Exit;
            continue;
          }

        case FT_CURVE_TAG_CONIC:  /* consume conic arcs */
          v_control.x = SCALED( point->x );
          v_control.y = SCALED( point->y );

        Do_Conic:
          if ( point < limit )
          {
            FT_Vector  vec;
            FT_Vector  v_middle;


            point++;
            tags++;
            tag = FT_CURVE_TAG( tags[0] );

            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            if ( tag == FT_CURVE_TAG_ON )
            {
              FT_TRACE5(( "  conic to (%.2f, %.2f)"
                          " with control (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          v_control.x / 64.0, v_control.y / 64.0 ));
              error = func_interface->conic_to( &v_control, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            if ( tag != FT_CURVE_TAG_CONIC )
              goto Invalid_Outline;

            v_middle.x = ( v_control.x + vec.x ) / 2;
            v_middle.y = ( v_control.y + vec.y ) / 2;

            FT_TRACE5(( "  conic to (%.2f, %.2f)"
                        " with control (%.2f, %.2f)\n",
                        v_middle.x / 64.0, v_middle.y / 64.0,
                        v_control.x / 64.0, v_control.y / 64.0 ));
            error = func_interface->conic_to( &v_control, &v_middle, user );
            if ( error )
              goto Exit;

            v_control = vec;
            goto Do_Conic;
          }

          FT_TRACE5(( "  conic to (%.2f, %.2f)"
                      " with control (%.2f, %.2f)\n",
                      v_start.x / 64.0, v_start.y / 64.0,
                      v_control.x / 64.0, v_control.y / 64.0 ));
          error = func_interface->conic_to( &v_control, &v_start, user );
          goto Close;

        default:  /* FT_CURVE_TAG_CUBIC */
          {
            FT_Vector  vec1, vec2;


            if ( point + 1 > limit                             ||
                 FT_CURVE_TAG( tags[1] ) != FT_CURVE_TAG_CUBIC )
              goto Invalid_Outline;

            point += 2;
            tags  += 2;

            vec1.x = SCALED( point[-2].x );
            vec1.y = SCALED( point[-2].y );

            vec2.x = SCALED( point[-1].x );
            vec2.y = SCALED( point[-1].y );

            if ( point <= limit )
            {
              FT_Vector  vec;


              vec.x = SCALED( point->x );
              vec.y = SCALED( point->y );

              FT_TRACE5(( "  cubic to (%.2f, %.2f)"
                          " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          vec1.x / 64.0, vec1.y / 64.0,
                          vec2.x / 64.0, vec2.y / 64.0 ));
              error = func_interface->cubic_to( &vec1, &vec2, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            FT_TRACE5(( "  cubic to (%.2f, %.2f)"
                        " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                        v_start.x / 64.0, v_start.y / 64.0,
                        vec1.x / 64.0, vec1.y / 64.0,
                        vec2.x / 64.0, vec2.y / 64.0 ));
            error = func_interface->cubic_to( &vec1, &vec2, &v_start, user );
            goto Close;
          }
        }
      }

      /* close the contour with a line segment */
      FT_TRACE5(( "  line to (%.2f, %.2f)\n",
                  v_start.x / 64.0, v_start.y / 64.0 ));
      error = func_interface->line_to( &v_start, user );

    Close:
      if ( error )
        goto Exit;

      first = last + 1;
    }

    FT_TRACE5(( "FT_Outline_Decompose: Done\n", n ));
    return FT_Err_Ok;

  Exit:
    FT_TRACE5(( "FT_Outline_Decompose: Error %d\n", error ));
    return error;

  Invalid_Outline:
    return FT_Err_Invalid_Outline;
  }


  FT_EXPORT_DEF( FT_Error )
  FT_Outline_New_Internal( FT_Memory    memory,
                           FT_UInt      numPoints,
                           FT_Int       numContours,
                           FT_Outline  *anoutline )
  {
    FT_Error  error;


    if ( !anoutline || !memory )
      return FT_Err_Invalid_Argument;

    *anoutline = null_outline;

    if ( FT_NEW_ARRAY( anoutline->points,   numPoints   ) ||
         FT_NEW_ARRAY( anoutline->tags,     numPoints   ) ||
         FT_NEW_ARRAY( anoutline->contours, numContours ) )
      goto Fail;

    anoutline->n_points    = (FT_UShort)numPoints;
    anoutline->n_contours  = (FT_Short)numContours;
    anoutline->flags      |= FT_OUTLINE_OWNER;

    return FT_Err_Ok;

  Fail:
    anoutline->flags |= FT_OUTLINE_OWNER;
    FT_Outline_Done_Internal( memory, anoutline );

    return error;
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_New( FT_Library   library,
                  FT_UInt      numPoints,
                  FT_Int       numContours,
                  FT_Outline  *anoutline )
  {
    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    return FT_Outline_New_Internal( library->memory, numPoints,
                                    numContours, anoutline );
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Check( FT_Outline*  outline )
  {
    if ( outline )
    {
      FT_Int  n_points   = outline->n_points;
      FT_Int  n_contours = outline->n_contours;
      FT_Int  end0, end;
      FT_Int  n;


      /* empty glyph? */
      if ( n_points == 0 && n_contours == 0 )
        return 0;

      /* check point and contour counts */
      if ( n_points <= 0 || n_contours <= 0 )
        goto Bad;

      end0 = end = -1;
      for ( n = 0; n < n_contours; n++ )
      {
        end = outline->contours[n];

        /* note that we don't accept empty contours */
        if ( end <= end0 || end >= n_points )
          goto Bad;

        end0 = end;
      }

      if ( end != n_points - 1 )
        goto Bad;

      /* XXX: check the tags array */
      return 0;
    }

  Bad:
    return FT_Err_Invalid_Argument;
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Copy( const FT_Outline*  source,
                   FT_Outline        *target )
  {
    FT_Int  is_owner;


    if ( !source            || !target            ||
         source->n_points   != target->n_points   ||
         source->n_contours != target->n_contours )
      return FT_Err_Invalid_Argument;

    if ( source == target )
      return FT_Err_Ok;

    FT_ARRAY_COPY( target->points, source->points, source->n_points );

    FT_ARRAY_COPY( target->tags, source->tags, source->n_points );

    FT_ARRAY_COPY( target->contours, source->contours, source->n_contours );

    /* copy all flags, except the `FT_OUTLINE_OWNER' one */
    is_owner      = target->flags & FT_OUTLINE_OWNER;
    target->flags = source->flags;

    target->flags &= ~FT_OUTLINE_OWNER;
    target->flags |= is_owner;

    return FT_Err_Ok;
  }


  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Done_Internal( FT_Memory    memory,
                            FT_Outline*  outline )
  {
    if ( memory && outline )
    {
      if ( outline->flags & FT_OUTLINE_OWNER )
      {
        FT_FREE( outline->points   );
        FT_FREE( outline->tags     );
        FT_FREE( outline->contours );
      }
      *outline = null_outline;

      return FT_Err_Ok;
    }
    else
      return FT_Err_Invalid_Argument;
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Done( FT_Library   library,
                   FT_Outline*  outline )
  {
    /* check for valid `outline' in FT_Outline_Done_Internal() */

    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    return FT_Outline_Done_Internal( library->memory, outline );
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( void )
  FT_Outline_Get_CBox( const FT_Outline*  outline,
                       FT_BBox           *acbox )
  {
    FT_Pos  xMin, yMin, xMax, yMax;


    if ( outline && acbox )
    {
      if ( outline->n_points == 0 )
      {
        xMin = 0;
        yMin = 0;
        xMax = 0;
        yMax = 0;
      }
      else
      {
        FT_Vector*  vec   = outline->points;
        FT_Vector*  limit = vec + outline->n_points;


        xMin = xMax = vec->x;
        yMin = yMax = vec->y;
        vec++;

        for ( ; vec < limit; vec++ )
        {
          FT_Pos  x, y;


          x = vec->x;
          if ( x < xMin ) xMin = x;
          if ( x > xMax ) xMax = x;

          y = vec->y;
          if ( y < yMin ) yMin = y;
          if ( y > yMax ) yMax = y;
        }
      }
      acbox->xMin = xMin;
      acbox->xMax = xMax;
      acbox->yMin = yMin;
      acbox->yMax = yMax;
    }
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( void )
  FT_Outline_Translate( const FT_Outline*  outline,
                        FT_Pos             xOffset,
                        FT_Pos             yOffset )
  {
    FT_UShort   n;
    FT_Vector*  vec;


    if ( !outline )
      return;

    vec = outline->points;

    for ( n = 0; n < outline->n_points; n++ )
    {
      vec->x += xOffset;
      vec->y += yOffset;
      vec++;
    }
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( void )
  FT_Outline_Reverse( FT_Outline*  outline )
  {
    FT_UShort  n;
    FT_Int     first, last;


    if ( !outline )
      return;

    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      last  = outline->contours[n];

      /* reverse point table */
      {
        FT_Vector*  p = outline->points + first;
        FT_Vector*  q = outline->points + last;
        FT_Vector   swap;


        while ( p < q )
        {
          swap = *p;
          *p   = *q;
          *q   = swap;
          p++;
          q--;
        }
      }

      /* reverse tags table */
      {
        char*  p = outline->tags + first;
        char*  q = outline->tags + last;
        char   swap;


        while ( p < q )
        {
          swap = *p;
          *p   = *q;
          *q   = swap;
          p++;
          q--;
        }
      }

      first = last + 1;
    }

    outline->flags ^= FT_OUTLINE_REVERSE_FILL;
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Render( FT_Library         library,
                     FT_Outline*        outline,
                     FT_Raster_Params*  params )
  {
    FT_Error     error;
    FT_Bool      update = FALSE;
    FT_Renderer  renderer;
    FT_ListNode  node;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( !outline || !params )
      return FT_Err_Invalid_Argument;

    renderer = library->cur_renderer;
    node     = library->renderers.head;

    params->source = (void*)outline;

    error = FT_Err_Cannot_Render_Glyph;
    while ( renderer )
    {
      error = renderer->raster_render( renderer->raster, params );
      if ( !error || FT_ERROR_BASE( error ) != FT_Err_Cannot_Render_Glyph )
        break;

      /* FT_Err_Cannot_Render_Glyph is returned if the render mode   */
      /* is unsupported by the current renderer for this glyph image */
      /* format                                                      */

      /* now, look for another renderer that supports the same */
      /* format                                                */
      renderer = FT_Lookup_Renderer( library, FT_GLYPH_FORMAT_OUTLINE,
                                     &node );
      update   = TRUE;
    }

    /* if we changed the current renderer for the glyph image format */
    /* we need to select it as the next current one                  */
    if ( !error && update && renderer )
      FT_Set_Renderer( library, renderer, 0, 0 );

    return error;
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Get_Bitmap( FT_Library        library,
                         FT_Outline*       outline,
                         const FT_Bitmap  *abitmap )
  {
    FT_Raster_Params  params;


    if ( !abitmap )
      return FT_Err_Invalid_Argument;

    /* other checks are delayed to FT_Outline_Render() */

    params.target = abitmap;
    params.flags  = 0;

    if ( abitmap->pixel_mode == FT_PIXEL_MODE_GRAY  ||
         abitmap->pixel_mode == FT_PIXEL_MODE_LCD   ||
         abitmap->pixel_mode == FT_PIXEL_MODE_LCD_V )
      params.flags |= FT_RASTER_FLAG_AA;

    return FT_Outline_Render( library, outline, &params );
  }


  /* documentation is in freetype.h */

  FT_EXPORT_DEF( void )
  FT_Vector_Transform( FT_Vector*        vector,
                       const FT_Matrix*  matrix )
  {
    FT_Pos  xz, yz;


    if ( !vector || !matrix )
      return;

    xz = FT_MulFix( vector->x, matrix->xx ) +
         FT_MulFix( vector->y, matrix->xy );

    yz = FT_MulFix( vector->x, matrix->yx ) +
         FT_MulFix( vector->y, matrix->yy );

    vector->x = xz;
    vector->y = yz;
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( void )
  FT_Outline_Transform( const FT_Outline*  outline,
                        const FT_Matrix*   matrix )
  {
    FT_Vector*  vec;
    FT_Vector*  limit;


    if ( !outline || !matrix )
      return;

    vec   = outline->points;
    limit = vec + outline->n_points;

    for ( ; vec < limit; vec++ )
      FT_Vector_Transform( vec, matrix );
  }


#if 0

#define FT_OUTLINE_GET_CONTOUR( outline, c, first, last )  \
  do {                                                     \
    (first) = ( c > 0 ) ? (outline)->points +              \
                            (outline)->contours[c - 1] + 1 \
                        : (outline)->points;               \
    (last) = (outline)->points + (outline)->contours[c];   \
  } while ( 0 )


  /* Is a point in some contour?                     */
  /*                                                 */
  /* We treat every point of the contour as if it    */
  /* it were ON.  That is, we allow false positives, */
  /* but disallow false negatives.  (XXX really?)    */
  static FT_Bool
  ft_contour_has( FT_Outline*  outline,
                  FT_Short     c,
                  FT_Vector*   point )
  {
    FT_Vector*  first;
    FT_Vector*  last;
    FT_Vector*  a;
    FT_Vector*  b;
    FT_UInt     n = 0;


    FT_OUTLINE_GET_CONTOUR( outline, c, first, last );

    for ( a = first; a <= last; a++ )
    {
      FT_Pos  x;
      FT_Int  intersect;


      b = ( a == last ) ? first : a + 1;

      intersect = ( a->y - point->y ) ^ ( b->y - point->y );

      /* a and b are on the same side */
      if ( intersect >= 0 )
      {
        if ( intersect == 0 && a->y == point->y )
        {
          if ( ( a->x <= point->x && b->x >= point->x ) ||
               ( a->x >= point->x && b->x <= point->x ) )
            return 1;
        }

        continue;
      }

      x = a->x + ( b->x - a->x ) * (point->y - a->y ) / ( b->y - a->y );

      if ( x < point->x )
        n++;
      else if ( x == point->x )
        return 1;
    }

    return ( n % 2 );
  }


  static FT_Bool
  ft_contour_enclosed( FT_Outline*  outline,
                       FT_UShort    c )
  {
    FT_Vector*  first;
    FT_Vector*  last;
    FT_Short    i;


    FT_OUTLINE_GET_CONTOUR( outline, c, first, last );

    for ( i = 0; i < outline->n_contours; i++ )
    {
      if ( i != c && ft_contour_has( outline, i, first ) )
      {
        FT_Vector*  pt;


        for ( pt = first + 1; pt <= last; pt++ )
          if ( !ft_contour_has( outline, i, pt ) )
            return 0;

        return 1;
      }
    }

    return 0;
  }


  /* This version differs from the public one in that each */
  /* part (contour not enclosed in another contour) of the */
  /* outline is checked for orientation.  This is          */
  /* necessary for some buggy CJK fonts.                   */
  static FT_Orientation
  ft_outline_get_orientation( FT_Outline*  outline )
  {
    FT_Short        i;
    FT_Vector*      first;
    FT_Vector*      last;
    FT_Orientation  orient = FT_ORIENTATION_NONE;


    first = outline->points;
    for ( i = 0; i < outline->n_contours; i++, first = last + 1 )
    {
      FT_Vector*  point;
      FT_Vector*  xmin_point;
      FT_Pos      xmin;


      last = outline->points + outline->contours[i];

      /* skip degenerate contours */
      if ( last < first + 2 )
        continue;

      if ( ft_contour_enclosed( outline, i ) )
        continue;

      xmin       = first->x;
      xmin_point = first;

      for ( point = first + 1; point <= last; point++ )
      {
        if ( point->x < xmin )
        {
          xmin       = point->x;
          xmin_point = point;
        }
      }

      /* check the orientation of the contour */
      {
        FT_Vector*      prev;
        FT_Vector*      next;
        FT_Orientation  o;


        prev = ( xmin_point == first ) ? last : xmin_point - 1;
        next = ( xmin_point == last ) ? first : xmin_point + 1;

        if ( FT_Atan2( prev->x - xmin_point->x, prev->y - xmin_point->y ) >
             FT_Atan2( next->x - xmin_point->x, next->y - xmin_point->y ) )
          o = FT_ORIENTATION_POSTSCRIPT;
        else
          o = FT_ORIENTATION_TRUETYPE;

        if ( orient == FT_ORIENTATION_NONE )
          orient = o;
        else if ( orient != o )
          return FT_ORIENTATION_NONE;
      }
    }

    return orient;
  }

#endif /* 0 */


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Error )
  FT_Outline_Embolden( FT_Outline*  outline,
                       FT_Pos       strength )
  {
    FT_Vector*  points;
    FT_Vector   v_prev, v_first, v_next, v_cur;
    FT_Angle    rotate, angle_in, angle_out;
    FT_Int      c, n, first;
    FT_Int      orientation;


    if ( !outline )
      return FT_Err_Invalid_Argument;

    strength /= 2;
    if ( strength == 0 )
      return FT_Err_Ok;

    orientation = FT_Outline_Get_Orientation( outline );
    if ( orientation == FT_ORIENTATION_NONE )
    {
      if ( outline->n_contours )
        return FT_Err_Invalid_Argument;
      else
        return FT_Err_Ok;
    }

    if ( orientation == FT_ORIENTATION_TRUETYPE )
      rotate = -FT_ANGLE_PI2;
    else
      rotate = FT_ANGLE_PI2;

    points = outline->points;

    first = 0;
    for ( c = 0; c < outline->n_contours; c++ )
    {
      int  last = outline->contours[c];


      v_first = points[first];
      v_prev  = points[last];
      v_cur   = v_first;

      for ( n = first; n <= last; n++ )
      {
        FT_Vector  in, out;
        FT_Angle   angle_diff;
        FT_Pos     d;
        FT_Fixed   scale;


        if ( n < last )
          v_next = points[n + 1];
        else
          v_next = v_first;

        /* compute the in and out vectors */
        in.x = v_cur.x - v_prev.x;
        in.y = v_cur.y - v_prev.y;

        out.x = v_next.x - v_cur.x;
        out.y = v_next.y - v_cur.y;

        angle_in   = FT_Atan2( in.x, in.y );
        angle_out  = FT_Atan2( out.x, out.y );
        angle_diff = FT_Angle_Diff( angle_in, angle_out );
        scale      = FT_Cos( angle_diff / 2 );

        if ( scale < 0x4000L && scale > -0x4000L )
          in.x = in.y = 0;
        else
        {
          d = FT_DivFix( strength, scale );

          FT_Vector_From_Polar( &in, d, angle_in + angle_diff / 2 - rotate );
        }

        outline->points[n].x = v_cur.x + strength + in.x;
        outline->points[n].y = v_cur.y + strength + in.y;

        v_prev = v_cur;
        v_cur  = v_next;
      }

      first = last + 1;
    }

    return FT_Err_Ok;
  }


  /* documentation is in ftoutln.h */

  FT_EXPORT_DEF( FT_Orientation )
  FT_Outline_Get_Orientation( FT_Outline*  outline )
  {
    FT_Pos      xmin       = 32768L;
    FT_Pos      xmin_ymin  = 32768L;
    FT_Pos      xmin_ymax  = -32768L;
    FT_Vector*  xmin_first = NULL;
    FT_Vector*  xmin_last  = NULL;

    short*      contour;

    FT_Vector*  first;
    FT_Vector*  last;
    FT_Vector*  prev;
    FT_Vector*  point;

    int             i;
    FT_Pos          ray_y[3];
    FT_Orientation  result[3] =
      { FT_ORIENTATION_NONE, FT_ORIENTATION_NONE, FT_ORIENTATION_NONE };


    if ( !outline || outline->n_points <= 0 )
      return FT_ORIENTATION_TRUETYPE;

    /* We use the nonzero winding rule to find the orientation.       */
    /* Since glyph outlines behave much more `regular' than arbitrary */
    /* cubic or quadratic curves, this test deals with the polygon    */
    /* only which is spanned up by the control points.                */

    first = outline->points;
    for ( contour = outline->contours;
          contour < outline->contours + outline->n_contours;
          contour++, first = last + 1 )
    {
      FT_Pos  contour_xmin = 32768L;
      FT_Pos  contour_xmax = -32768L;
      FT_Pos  contour_ymin = 32768L;
      FT_Pos  contour_ymax = -32768L;


      last = outline->points + *contour;

      /* skip degenerate contours */
      if ( last < first + 2 )
        continue;

      for ( point = first; point <= last; ++point )
      {
        if ( point->x < contour_xmin )
          contour_xmin = point->x;

        if ( point->x > contour_xmax )
          contour_xmax = point->x;

        if ( point->y < contour_ymin )
          contour_ymin = point->y;

        if ( point->y > contour_ymax )
          contour_ymax = point->y;
      }

      if ( contour_xmin < xmin          &&
           contour_xmin != contour_xmax &&
           contour_ymin != contour_ymax )
      {
        xmin       = contour_xmin;
        xmin_ymin  = contour_ymin;
        xmin_ymax  = contour_ymax;
        xmin_first = first;
        xmin_last  = last;
      }
    }

    if ( xmin == 32768L )
      return FT_ORIENTATION_TRUETYPE;

    ray_y[0] = ( xmin_ymin * 3 + xmin_ymax     ) >> 2;
    ray_y[1] = ( xmin_ymin     + xmin_ymax     ) >> 1;
    ray_y[2] = ( xmin_ymin     + xmin_ymax * 3 ) >> 2;

    for ( i = 0; i < 3; i++ )
    {
      FT_Pos      left_x;
      FT_Pos      right_x;
      FT_Vector*  left1;
      FT_Vector*  left2;
      FT_Vector*  right1;
      FT_Vector*  right2;


    RedoRay:
      left_x  = 32768L;
      right_x = -32768L;

      left1 = left2 = right1 = right2 = NULL;

      prev = xmin_last;
      for ( point = xmin_first; point <= xmin_last; prev = point, ++point )
      {
        FT_Pos  tmp_x;


        if ( point->y == ray_y[i] || prev->y == ray_y[i] )
        {
          ray_y[i]++;
          goto RedoRay;
        }

        if ( ( point->y < ray_y[i] && prev->y < ray_y[i] ) ||
             ( point->y > ray_y[i] && prev->y > ray_y[i] ) )
          continue;

        tmp_x = FT_MulDiv( point->x - prev->x,
                           ray_y[i] - prev->y,
                           point->y - prev->y ) + prev->x;

        if ( tmp_x < left_x )
        {
          left_x = tmp_x;
          left1  = prev;
          left2  = point;
        }

        if ( tmp_x > right_x )
        {
          right_x = tmp_x;
          right1  = prev;
          right2  = point;
        }
      }

      if ( left1 && right1 )
      {
        if ( left1->y < left2->y && right1->y > right2->y )
          result[i] = FT_ORIENTATION_TRUETYPE;
        else if ( left1->y > left2->y && right1->y < right2->y )
          result[i] = FT_ORIENTATION_POSTSCRIPT;
        else
          result[i] = FT_ORIENTATION_NONE;
      }
    }

    if ( result[0] != FT_ORIENTATION_NONE                     &&
         ( result[0] == result[1] || result[0] == result[2] ) )
      return result[0];

    if ( result[1] != FT_ORIENTATION_NONE && result[1] == result[2] )
      return result[1];

    return FT_ORIENTATION_TRUETYPE;
  }


/* END */
