/***************************************************************************/
/*                                                                         */
/*  ftstream.c                                                             */
/*                                                                         */
/*    I/O stream support (body).                                           */
/*                                                                         */
/*  Copyright 2000-2001, 2002, 2004, 2005, 2006, 2008, 2009, 2010 by       */
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
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_DEBUG_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_stream


  FT_BASE_DEF( void )
  FT_Stream_OpenMemory( FT_Stream       stream,
                        const FT_Byte*  base,
                        FT_ULong        size )
  {
    stream->base   = (FT_Byte*) base;
    stream->size   = size;
    stream->pos    = 0;
    stream->cursor = 0;
    stream->read   = 0;
    stream->close  = 0;
  }


  FT_BASE_DEF( void )
  FT_Stream_Close( FT_Stream  stream )
  {
    if ( stream && stream->close )
      stream->close( stream );
  }


  FT_BASE_DEF( FT_Error )
  FT_Stream_Seek( FT_Stream  stream,
                  FT_ULong   pos )
  {
    FT_Error  error = FT_Err_Ok;


    if ( stream->read )
    {
      if ( stream->read( stream, pos, 0, 0 ) )
      {
        FT_ERROR(( "FT_Stream_Seek:"
                   " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
                   pos, stream->size ));

        error = FT_Err_Invalid_Stream_Operation;
      }
    }
    /* note that seeking to the first position after the file is valid */
    else if ( pos > stream->size )
    {
      FT_ERROR(( "FT_Stream_Seek:"
                 " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
                 pos, stream->size ));

      error = FT_Err_Invalid_Stream_Operation;
    }

    if ( !error )
      stream->pos = pos;

    return error;
  }


  FT_BASE_DEF( FT_Error )
  FT_Stream_Skip( FT_Stream  stream,
                  FT_Long    distance )
  {
    if ( distance < 0 )
      return FT_Err_Invalid_Stream_Operation;

    return FT_Stream_Seek( stream, (FT_ULong)( stream->pos + distance ) );
  }


  FT_BASE_DEF( FT_Long )
  FT_Stream_Pos( FT_Stream  stream )
  {
    return stream->pos;
  }


  FT_BASE_DEF( FT_Error )
  FT_Stream_Read( FT_Stream  stream,
                  FT_Byte*   buffer,
                  FT_ULong   count )
  {
    return FT_Stream_ReadAt( stream, stream->pos, buffer, count );
  }


  FT_BASE_DEF( FT_Error )
  FT_Stream_ReadAt( FT_Stream  stream,
                    FT_ULong   pos,
                    FT_Byte*   buffer,
                    FT_ULong   count )
  {
    FT_Error  error = FT_Err_Ok;
    FT_ULong  read_bytes;


    if ( pos >= stream->size )
    {
      FT_ERROR(( "FT_Stream_ReadAt:"
                 " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
                 pos, stream->size ));

      return FT_Err_Invalid_Stream_Operation;
    }

    if ( stream->read )
      read_bytes = stream->read( stream, pos, buffer, count );
    else
    {
      read_bytes = stream->size - pos;
      if ( read_bytes > count )
        read_bytes = count;

      FT_MEM_COPY( buffer, stream->base + pos, read_bytes );
    }

    stream->pos = pos + read_bytes;

    if ( read_bytes < count )
    {
      FT_ERROR(( "FT_Stream_ReadAt:"
                 " invalid read; expected %lu bytes, got %lu\n",
                 count, read_bytes ));

      error = FT_Err_Invalid_Stream_Operation;
    }

    return error;
  }


  FT_BASE_DEF( FT_ULong )
  FT_Stream_TryRead( FT_Stream  stream,
                     FT_Byte*   buffer,
                     FT_ULong   count )
  {
    FT_ULong  read_bytes = 0;


    if ( stream->pos >= stream->size )
      goto Exit;

    if ( stream->read )
      read_bytes = stream->read( stream, stream->pos, buffer, count );
    else
    {
      read_bytes = stream->size - stream->pos;
      if ( read_bytes > count )
        read_bytes = count;

      FT_MEM_COPY( buffer, stream->base + stream->pos, read_bytes );
    }

    stream->pos += read_bytes;

  Exit:
    return read_bytes;
  }


  FT_BASE_DEF( FT_Error )
  FT_Stream_ExtractFrame( FT_Stream  stream,
                          FT_ULong   count,
                          FT_Byte**  pbytes )
  {
    FT_Error  error;


    error = FT_Stream_EnterFrame( stream, count );
    if ( !error )
    {
      *pbytes = (FT_Byte*)stream->cursor;

      /* equivalent to FT_Stream_ExitFrame(), with no memory block release */
      stream->cursor = 0;
      stream->limit  = 0;
    }

    return error;
  }


  FT_BASE_DEF( void )
  FT_Stream_ReleaseFrame( FT_Stream  stream,
                          FT_Byte**  pbytes )
  {
    if ( stream && stream->read )
    {
      FT_Memory  memory = stream->memory;

#ifdef FT_DEBUG_MEMORY
      ft_mem_free( memory, *pbytes );
      *pbytes = NULL;
#else
      FT_FREE( *pbytes );
#endif
    }
    *pbytes = 0;
  }


  FT_BASE_DEF( FT_Error )
  FT_Stream_EnterFrame( FT_Stream  stream,
                        FT_ULong   count )
  {
    FT_Error  error = FT_Err_Ok;
    FT_ULong  read_bytes;


    /* check for nested frame access */
    FT_ASSERT( stream && stream->cursor == 0 );

    if ( stream->read )
    {
      /* allocate the frame in memory */
      FT_Memory  memory = stream->memory;


      /* simple sanity check */
      if ( count > stream->size )
      {
        FT_ERROR(( "FT_Stream_EnterFrame:"
                   " frame size (%lu) larger than stream size (%lu)\n",
                   count, stream->size ));

        error = FT_Err_Invalid_Stream_Operation;
        goto Exit;
      }

#ifdef FT_DEBUG_MEMORY
      /* assume _ft_debug_file and _ft_debug_lineno are already set */
      stream->base = (unsigned char*)ft_mem_qalloc( memory, count, &error );
      if ( error )
        goto Exit;
#else
      if ( FT_QALLOC( stream->base, count ) )
        goto Exit;
#endif
      /* read it */
      read_bytes = stream->read( stream, stream->pos,
                                 stream->base, count );
      if ( read_bytes < count )
      {
        FT_ERROR(( "FT_Stream_EnterFrame:"
                   " invalid read; expected %lu bytes, got %lu\n",
                   count, read_bytes ));

        FT_FREE( stream->base );
        error = FT_Err_Invalid_Stream_Operation;
      }
      stream->cursor = stream->base;
      stream->limit  = stream->cursor + count;
      stream->pos   += read_bytes;
    }
    else
    {
      /* check current and new position */
      if ( stream->pos >= stream->size        ||
           stream->size - stream->pos < count )
      {
        FT_ERROR(( "FT_Stream_EnterFrame:"
                   " invalid i/o; pos = 0x%lx, count = %lu, size = 0x%lx\n",
                   stream->pos, count, stream->size ));

        error = FT_Err_Invalid_Stream_Operation;
        goto Exit;
      }

      /* set cursor */
      stream->cursor = stream->base + stream->pos;
      stream->limit  = stream->cursor + count;
      stream->pos   += count;
    }

  Exit:
    return error;
  }


  FT_BASE_DEF( void )
  FT_Stream_ExitFrame( FT_Stream  stream )
  {
    /* IMPORTANT: The assertion stream->cursor != 0 was removed, given    */
    /*            that it is possible to access a frame of length 0 in    */
    /*            some weird fonts (usually, when accessing an array of   */
    /*            0 records, like in some strange kern tables).           */
    /*                                                                    */
    /*  In this case, the loader code handles the 0-length table          */
    /*  gracefully; however, stream.cursor is really set to 0 by the      */
    /*  FT_Stream_EnterFrame() call, and this is not an error.            */
    /*                                                                    */
    FT_ASSERT( stream );

    if ( stream->read )
    {
      FT_Memory  memory = stream->memory;

#ifdef FT_DEBUG_MEMORY
      ft_mem_free( memory, stream->base );
      stream->base = NULL;
#else
      FT_FREE( stream->base );
#endif
    }
    stream->cursor = 0;
    stream->limit  = 0;
  }


  FT_BASE_DEF( FT_Char )
  FT_Stream_GetChar( FT_Stream  stream )
  {
    FT_Char  result;


    FT_ASSERT( stream && stream->cursor );

    result = 0;
    if ( stream->cursor < stream->limit )
      result = *stream->cursor++;

    return result;
  }


  FT_BASE_DEF( FT_Short )
  FT_Stream_GetShort( FT_Stream  stream )
  {
    FT_Byte*  p;
    FT_Short  result;


    FT_ASSERT( stream && stream->cursor );

    result         = 0;
    p              = stream->cursor;
    if ( p + 1 < stream->limit )
      result       = FT_NEXT_SHORT( p );
    stream->cursor = p;

    return result;
  }


  FT_BASE_DEF( FT_Short )
  FT_Stream_GetShortLE( FT_Stream  stream )
  {
    FT_Byte*  p;
    FT_Short  result;


    FT_ASSERT( stream && stream->cursor );

    result         = 0;
    p              = stream->cursor;
    if ( p + 1 < stream->limit )
      result       = FT_NEXT_SHORT_LE( p );
    stream->cursor = p;

    return result;
  }


  FT_BASE_DEF( FT_Long )
  FT_Stream_GetOffset( FT_Stream  stream )
  {
    FT_Byte*  p;
    FT_Long   result;


    FT_ASSERT( stream && stream->cursor );

    result         = 0;
    p              = stream->cursor;
    if ( p + 2 < stream->limit )
      result       = FT_NEXT_OFF3( p );
    stream->cursor = p;
    return result;
  }


  FT_BASE_DEF( FT_Long )
  FT_Stream_GetLong( FT_Stream  stream )
  {
    FT_Byte*  p;
    FT_Long   result;


    FT_ASSERT( stream && stream->cursor );

    result         = 0;
    p              = stream->cursor;
    if ( p + 3 < stream->limit )
      result       = FT_NEXT_LONG( p );
    stream->cursor = p;
    return result;
  }


  FT_BASE_DEF( FT_Long )
  FT_Stream_GetLongLE( FT_Stream  stream )
  {
    FT_Byte*  p;
    FT_Long   result;


    FT_ASSERT( stream && stream->cursor );

    result         = 0;
    p              = stream->cursor;
    if ( p + 3 < stream->limit )
      result       = FT_NEXT_LONG_LE( p );
    stream->cursor = p;
    return result;
  }


  FT_BASE_DEF( FT_Char )
  FT_Stream_ReadChar( FT_Stream  stream,
                      FT_Error*  error )
  {
    FT_Byte  result = 0;


    FT_ASSERT( stream );

    *error = FT_Err_Ok;

    if ( stream->read )
    {
      if ( stream->read( stream, stream->pos, &result, 1L ) != 1L )
        goto Fail;
    }
    else
    {
      if ( stream->pos < stream->size )
        result = stream->base[stream->pos];
      else
        goto Fail;
    }
    stream->pos++;

    return result;

  Fail:
    *error = FT_Err_Invalid_Stream_Operation;
    FT_ERROR(( "FT_Stream_ReadChar:"
               " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
               stream->pos, stream->size ));

    return 0;
  }


  FT_BASE_DEF( FT_Short )
  FT_Stream_ReadShort( FT_Stream  stream,
                       FT_Error*  error )
  {
    FT_Byte   reads[2];
    FT_Byte*  p = 0;
    FT_Short  result = 0;


    FT_ASSERT( stream );

    *error = FT_Err_Ok;

    if ( stream->pos + 1 < stream->size )
    {
      if ( stream->read )
      {
        if ( stream->read( stream, stream->pos, reads, 2L ) != 2L )
          goto Fail;

        p = reads;
      }
      else
      {
        p = stream->base + stream->pos;
      }

      if ( p )
        result = FT_NEXT_SHORT( p );
    }
    else
      goto Fail;

    stream->pos += 2;

    return result;

  Fail:
    *error = FT_Err_Invalid_Stream_Operation;
    FT_ERROR(( "FT_Stream_ReadShort:"
               " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
               stream->pos, stream->size ));

    return 0;
  }


  FT_BASE_DEF( FT_Short )
  FT_Stream_ReadShortLE( FT_Stream  stream,
                         FT_Error*  error )
  {
    FT_Byte   reads[2];
    FT_Byte*  p = 0;
    FT_Short  result = 0;


    FT_ASSERT( stream );

    *error = FT_Err_Ok;

    if ( stream->pos + 1 < stream->size )
    {
      if ( stream->read )
      {
        if ( stream->read( stream, stream->pos, reads, 2L ) != 2L )
          goto Fail;

        p = reads;
      }
      else
      {
        p = stream->base + stream->pos;
      }

      if ( p )
        result = FT_NEXT_SHORT_LE( p );
    }
    else
      goto Fail;

    stream->pos += 2;

    return result;

  Fail:
    *error = FT_Err_Invalid_Stream_Operation;
    FT_ERROR(( "FT_Stream_ReadShortLE:"
               " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
               stream->pos, stream->size ));

    return 0;
  }


  FT_BASE_DEF( FT_Long )
  FT_Stream_ReadOffset( FT_Stream  stream,
                        FT_Error*  error )
  {
    FT_Byte   reads[3];
    FT_Byte*  p = 0;
    FT_Long   result = 0;


    FT_ASSERT( stream );

    *error = FT_Err_Ok;

    if ( stream->pos + 2 < stream->size )
    {
      if ( stream->read )
      {
        if (stream->read( stream, stream->pos, reads, 3L ) != 3L )
          goto Fail;

        p = reads;
      }
      else
      {
        p = stream->base + stream->pos;
      }

      if ( p )
        result = FT_NEXT_OFF3( p );
    }
    else
      goto Fail;

    stream->pos += 3;

    return result;

  Fail:
    *error = FT_Err_Invalid_Stream_Operation;
    FT_ERROR(( "FT_Stream_ReadOffset:"
               " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
               stream->pos, stream->size ));

    return 0;
  }


  FT_BASE_DEF( FT_Long )
  FT_Stream_ReadLong( FT_Stream  stream,
                      FT_Error*  error )
  {
    FT_Byte   reads[4];
    FT_Byte*  p = 0;
    FT_Long   result = 0;


    FT_ASSERT( stream );

    *error = FT_Err_Ok;

    if ( stream->pos + 3 < stream->size )
    {
      if ( stream->read )
      {
        if ( stream->read( stream, stream->pos, reads, 4L ) != 4L )
          goto Fail;

        p = reads;
      }
      else
      {
        p = stream->base + stream->pos;
      }

      if ( p )
        result = FT_NEXT_LONG( p );
    }
    else
      goto Fail;

    stream->pos += 4;

    return result;

  Fail:
    *error = FT_Err_Invalid_Stream_Operation;
    FT_ERROR(( "FT_Stream_ReadLong:"
               " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
               stream->pos, stream->size ));

    return 0;
  }


  FT_BASE_DEF( FT_Long )
  FT_Stream_ReadLongLE( FT_Stream  stream,
                        FT_Error*  error )
  {
    FT_Byte   reads[4];
    FT_Byte*  p = 0;
    FT_Long   result = 0;


    FT_ASSERT( stream );

    *error = FT_Err_Ok;

    if ( stream->pos + 3 < stream->size )
    {
      if ( stream->read )
      {
        if ( stream->read( stream, stream->pos, reads, 4L ) != 4L )
          goto Fail;

        p = reads;
      }
      else
      {
        p = stream->base + stream->pos;
      }

      if ( p )
        result = FT_NEXT_LONG_LE( p );
    }
    else
      goto Fail;

    stream->pos += 4;

    return result;

  Fail:
    *error = FT_Err_Invalid_Stream_Operation;
    FT_ERROR(( "FT_Stream_ReadLongLE:"
               " invalid i/o; pos = 0x%lx, size = 0x%lx\n",
               stream->pos, stream->size ));

    return 0;
  }


  FT_BASE_DEF( FT_Error )
  FT_Stream_ReadFields( FT_Stream              stream,
                        const FT_Frame_Field*  fields,
                        void*                  structure )
  {
    FT_Error  error;
    FT_Bool   frame_accessed = 0;
    FT_Byte*  cursor;

    if ( !fields || !stream )
      return FT_Err_Invalid_Argument;

    cursor = stream->cursor;

    error = FT_Err_Ok;
    do
    {
      FT_ULong  value;
      FT_Int    sign_shift;
      FT_Byte*  p;


      switch ( fields->value )
      {
      case ft_frame_start:  /* access a new frame */
        error = FT_Stream_EnterFrame( stream, fields->offset );
        if ( error )
          goto Exit;

        frame_accessed = 1;
        cursor         = stream->cursor;
        fields++;
        continue;  /* loop! */

      case ft_frame_bytes:  /* read a byte sequence */
      case ft_frame_skip:   /* skip some bytes      */
        {
          FT_UInt  len = fields->size;


          if ( cursor + len > stream->limit )
          {
            error = FT_Err_Invalid_Stream_Operation;
            goto Exit;
          }

          if ( fields->value == ft_frame_bytes )
          {
            p = (FT_Byte*)structure + fields->offset;
            FT_MEM_COPY( p, cursor, len );
          }
          cursor += len;
          fields++;
          continue;
        }

      case ft_frame_byte:
      case ft_frame_schar:  /* read a single byte */
        value = FT_NEXT_BYTE(cursor);
        sign_shift = 24;
        break;

      case ft_frame_short_be:
      case ft_frame_ushort_be:  /* read a 2-byte big-endian short */
        value = FT_NEXT_USHORT(cursor);
        sign_shift = 16;
        break;

      case ft_frame_short_le:
      case ft_frame_ushort_le:  /* read a 2-byte little-endian short */
        value = FT_NEXT_USHORT_LE(cursor);
        sign_shift = 16;
        break;

      case ft_frame_long_be:
      case ft_frame_ulong_be:  /* read a 4-byte big-endian long */
        value = FT_NEXT_ULONG(cursor);
        sign_shift = 0;
        break;

      case ft_frame_long_le:
      case ft_frame_ulong_le:  /* read a 4-byte little-endian long */
        value = FT_NEXT_ULONG_LE(cursor);
        sign_shift = 0;
        break;

      case ft_frame_off3_be:
      case ft_frame_uoff3_be:  /* read a 3-byte big-endian long */
        value = FT_NEXT_UOFF3(cursor);
        sign_shift = 8;
        break;

      case ft_frame_off3_le:
      case ft_frame_uoff3_le:  /* read a 3-byte little-endian long */
        value = FT_NEXT_UOFF3_LE(cursor);
        sign_shift = 8;
        break;

      default:
        /* otherwise, exit the loop */
        stream->cursor = cursor;
        goto Exit;
      }

      /* now, compute the signed value is necessary */
      if ( fields->value & FT_FRAME_OP_SIGNED )
        value = (FT_ULong)( (FT_Int32)( value << sign_shift ) >> sign_shift );

      /* finally, store the value in the object */

      p = (FT_Byte*)structure + fields->offset;
      switch ( fields->size )
      {
      case (8 / FT_CHAR_BIT):
        *(FT_Byte*)p = (FT_Byte)value;
        break;

      case (16 / FT_CHAR_BIT):
        *(FT_UShort*)p = (FT_UShort)value;
        break;

      case (32 / FT_CHAR_BIT):
        *(FT_UInt32*)p = (FT_UInt32)value;
        break;

      default:  /* for 64-bit systems */
        *(FT_ULong*)p = (FT_ULong)value;
      }

      /* go to next field */
      fields++;
    }
    while ( 1 );

  Exit:
    /* close the frame if it was opened by this read */
    if ( frame_accessed )
      FT_Stream_ExitFrame( stream );

    return error;
  }


/* END */
