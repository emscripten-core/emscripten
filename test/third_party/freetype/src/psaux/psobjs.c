/***************************************************************************/
/*                                                                         */
/*  psobjs.c                                                               */
/*                                                                         */
/*    Auxiliary functions for PostScript fonts (body).                     */
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
#include FT_INTERNAL_POSTSCRIPT_AUX_H
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_CALC_H

#include "psobjs.h"
#include "psconv.h"

#include "psauxerr.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_psobjs


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                             PS_TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ps_table_new                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a PS_Table.                                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    table  :: The address of the target table.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    count  :: The table size = the maximum number of elements.         */
  /*                                                                       */
  /*    memory :: The memory object to use for all subsequent              */
  /*              reallocations.                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_LOCAL_DEF( FT_Error )
  ps_table_new( PS_Table   table,
                FT_Int     count,
                FT_Memory  memory )
  {
    FT_Error  error;


    table->memory = memory;
    if ( FT_NEW_ARRAY( table->elements, count ) ||
         FT_NEW_ARRAY( table->lengths,  count ) )
      goto Exit;

    table->max_elems = count;
    table->init      = 0xDEADBEEFUL;
    table->num_elems = 0;
    table->block     = 0;
    table->capacity  = 0;
    table->cursor    = 0;

    *(PS_Table_FuncsRec*)&table->funcs = ps_table_funcs;

  Exit:
    if ( error )
      FT_FREE( table->elements );

    return error;
  }


  static void
  shift_elements( PS_Table  table,
                  FT_Byte*  old_base )
  {
    FT_PtrDist  delta  = table->block - old_base;
    FT_Byte**   offset = table->elements;
    FT_Byte**   limit  = offset + table->max_elems;


    for ( ; offset < limit; offset++ )
    {
      if ( offset[0] )
        offset[0] += delta;
    }
  }


  static FT_Error
  reallocate_t1_table( PS_Table  table,
                       FT_Long   new_size )
  {
    FT_Memory  memory   = table->memory;
    FT_Byte*   old_base = table->block;
    FT_Error   error;


    /* allocate new base block */
    if ( FT_ALLOC( table->block, new_size ) )
    {
      table->block = old_base;
      return error;
    }

    /* copy elements and shift offsets */
    if ( old_base )
    {
      FT_MEM_COPY( table->block, old_base, table->capacity );
      shift_elements( table, old_base );
      FT_FREE( old_base );
    }

    table->capacity = new_size;

    return PSaux_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ps_table_add                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Adds an object to a PS_Table, possibly growing its memory block.   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    table  :: The target table.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    idx    :: The index of the object in the table.                    */
  /*                                                                       */
  /*    object :: The address of the object to copy in memory.             */
  /*                                                                       */
  /*    length :: The length in bytes of the source object.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.  An error is returned if a  */
  /*    reallocation fails.                                                */
  /*                                                                       */
  FT_LOCAL_DEF( FT_Error )
  ps_table_add( PS_Table    table,
                FT_Int      idx,
                void*       object,
                FT_PtrDist  length )
  {
    if ( idx < 0 || idx >= table->max_elems )
    {
      FT_ERROR(( "ps_table_add: invalid index\n" ));
      return PSaux_Err_Invalid_Argument;
    }

    if ( length < 0 )
    {
      FT_ERROR(( "ps_table_add: invalid length\n" ));
      return PSaux_Err_Invalid_Argument;
    }

    /* grow the base block if needed */
    if ( table->cursor + length > table->capacity )
    {
      FT_Error   error;
      FT_Offset  new_size = table->capacity;
      FT_Long    in_offset;


      in_offset = (FT_Long)((FT_Byte*)object - table->block);
      if ( (FT_ULong)in_offset >= table->capacity )
        in_offset = -1;

      while ( new_size < table->cursor + length )
      {
        /* increase size by 25% and round up to the nearest multiple
           of 1024 */
        new_size += ( new_size >> 2 ) + 1;
        new_size  = FT_PAD_CEIL( new_size, 1024 );
      }

      error = reallocate_t1_table( table, new_size );
      if ( error )
        return error;

      if ( in_offset >= 0 )
        object = table->block + in_offset;
    }

    /* add the object to the base block and adjust offset */
    table->elements[idx] = table->block + table->cursor;
    table->lengths [idx] = length;
    FT_MEM_COPY( table->block + table->cursor, object, length );

    table->cursor += length;
    return PSaux_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ps_table_done                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a PS_TableRec (i.e., reallocate it to its current        */
  /*    cursor).                                                           */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    table :: The target table.                                         */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does NOT release the heap's memory block.  It is up  */
  /*    to the caller to clean it, or reference it in its own structures.  */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  ps_table_done( PS_Table  table )
  {
    FT_Memory  memory = table->memory;
    FT_Error   error;
    FT_Byte*   old_base = table->block;


    /* should never fail, because rec.cursor <= rec.size */
    if ( !old_base )
      return;

    if ( FT_ALLOC( table->block, table->cursor ) )
      return;
    FT_MEM_COPY( table->block, old_base, table->cursor );
    shift_elements( table, old_base );

    table->capacity = table->cursor;
    FT_FREE( old_base );

    FT_UNUSED( error );
  }


  FT_LOCAL_DEF( void )
  ps_table_release( PS_Table  table )
  {
    FT_Memory  memory = table->memory;


    if ( (FT_ULong)table->init == 0xDEADBEEFUL )
    {
      FT_FREE( table->block );
      FT_FREE( table->elements );
      FT_FREE( table->lengths );
      table->init = 0;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                            T1 PARSER                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  /* first character must be already part of the comment */

  static void
  skip_comment( FT_Byte*  *acur,
                FT_Byte*   limit )
  {
    FT_Byte*  cur = *acur;


    while ( cur < limit )
    {
      if ( IS_PS_NEWLINE( *cur ) )
        break;
      cur++;
    }

    *acur = cur;
  }


  static void
  skip_spaces( FT_Byte*  *acur,
               FT_Byte*   limit )
  {
    FT_Byte*  cur = *acur;


    while ( cur < limit )
    {
      if ( !IS_PS_SPACE( *cur ) )
      {
        if ( *cur == '%' )
          /* According to the PLRM, a comment is equal to a space. */
          skip_comment( &cur, limit );
        else
          break;
      }
      cur++;
    }

    *acur = cur;
  }


#define IS_OCTAL_DIGIT( c ) ( '0' <= (c) && (c) <= '7' )


  /* first character must be `(';                               */
  /* *acur is positioned at the character after the closing `)' */

  static FT_Error
  skip_literal_string( FT_Byte*  *acur,
                       FT_Byte*   limit )
  {
    FT_Byte*      cur   = *acur;
    FT_Int        embed = 0;
    FT_Error      error = PSaux_Err_Invalid_File_Format;
    unsigned int  i;


    while ( cur < limit )
    {
      FT_Byte  c = *cur;


      ++cur;

      if ( c == '\\' )
      {
        /* Red Book 3rd ed., section `Literal Text Strings', p. 29:     */
        /* A backslash can introduce three different types              */
        /* of escape sequences:                                         */
        /*   - a special escaped char like \r, \n, etc.                 */
        /*   - a one-, two-, or three-digit octal number                */
        /*   - none of the above in which case the backslash is ignored */

        if ( cur == limit )
          /* error (or to be ignored?) */
          break;

        switch ( *cur )
        {
          /* skip `special' escape */
        case 'n':
        case 'r':
        case 't':
        case 'b':
        case 'f':
        case '\\':
        case '(':
        case ')':
          ++cur;
          break;

        default:
          /* skip octal escape or ignore backslash */
          for ( i = 0; i < 3 && cur < limit; ++i )
          {
            if ( !IS_OCTAL_DIGIT( *cur ) )
              break;

            ++cur;
          }
        }
      }
      else if ( c == '(' )
        embed++;
      else if ( c == ')' )
      {
        embed--;
        if ( embed == 0 )
        {
          error = PSaux_Err_Ok;
          break;
        }
      }
    }

    *acur = cur;

    return error;
  }


  /* first character must be `<' */

  static FT_Error
  skip_string( FT_Byte*  *acur,
               FT_Byte*   limit )
  {
    FT_Byte*  cur = *acur;
    FT_Error  err =  PSaux_Err_Ok;


    while ( ++cur < limit )
    {
      /* All whitespace characters are ignored. */
      skip_spaces( &cur, limit );
      if ( cur >= limit )
        break;

      if ( !IS_PS_XDIGIT( *cur ) )
        break;
    }

    if ( cur < limit && *cur != '>' )
    {
      FT_ERROR(( "skip_string: missing closing delimiter `>'\n" ));
      err = PSaux_Err_Invalid_File_Format;
    }
    else
      cur++;

    *acur = cur;
    return err;
  }


  /* first character must be the opening brace that */
  /* starts the procedure                           */

  /* NB: [ and ] need not match:                    */
  /* `/foo {[} def' is a valid PostScript fragment, */
  /* even within a Type1 font                       */

  static FT_Error
  skip_procedure( FT_Byte*  *acur,
                  FT_Byte*   limit )
  {
    FT_Byte*  cur;
    FT_Int    embed = 0;
    FT_Error  error = PSaux_Err_Ok;


    FT_ASSERT( **acur == '{' );

    for ( cur = *acur; cur < limit && error == PSaux_Err_Ok; ++cur )
    {
      switch ( *cur )
      {
      case '{':
        ++embed;
        break;

      case '}':
        --embed;
        if ( embed == 0 )
        {
          ++cur;
          goto end;
        }
        break;

      case '(':
        error = skip_literal_string( &cur, limit );
        break;

      case '<':
        error = skip_string( &cur, limit );
        break;

      case '%':
        skip_comment( &cur, limit );
        break;
      }
    }

  end:
    if ( embed != 0 )
      error = PSaux_Err_Invalid_File_Format;

    *acur = cur;

    return error;
  }


  /***********************************************************************/
  /*                                                                     */
  /* All exported parsing routines handle leading whitespace and stop at */
  /* the first character which isn't part of the just handled token.     */
  /*                                                                     */
  /***********************************************************************/


  FT_LOCAL_DEF( void )
  ps_parser_skip_PS_token( PS_Parser  parser )
  {
    /* Note: PostScript allows any non-delimiting, non-whitespace        */
    /*       character in a name (PS Ref Manual, 3rd ed, p31).           */
    /*       PostScript delimiters are (, ), <, >, [, ], {, }, /, and %. */

    FT_Byte*  cur   = parser->cursor;
    FT_Byte*  limit = parser->limit;
    FT_Error  error = PSaux_Err_Ok;


    skip_spaces( &cur, limit );             /* this also skips comments */
    if ( cur >= limit )
      goto Exit;

    /* self-delimiting, single-character tokens */
    if ( *cur == '[' || *cur == ']' )
    {
      cur++;
      goto Exit;
    }

    /* skip balanced expressions (procedures and strings) */

    if ( *cur == '{' )                              /* {...} */
    {
      error = skip_procedure( &cur, limit );
      goto Exit;
    }

    if ( *cur == '(' )                              /* (...) */
    {
      error = skip_literal_string( &cur, limit );
      goto Exit;
    }

    if ( *cur == '<' )                              /* <...> */
    {
      if ( cur + 1 < limit && *(cur + 1) == '<' )   /* << */
      {
        cur++;
        cur++;
      }
      else
        error = skip_string( &cur, limit );

      goto Exit;
    }

    if ( *cur == '>' )
    {
      cur++;
      if ( cur >= limit || *cur != '>' )             /* >> */
      {
        FT_ERROR(( "ps_parser_skip_PS_token:"
                   " unexpected closing delimiter `>'\n" ));
        error = PSaux_Err_Invalid_File_Format;
        goto Exit;
      }
      cur++;
      goto Exit;
    }

    if ( *cur == '/' )
      cur++;

    /* anything else */
    while ( cur < limit )
    {
      /* *cur might be invalid (e.g., ')' or '}'), but this   */
      /* is handled by the test `cur == parser->cursor' below */
      if ( IS_PS_DELIM( *cur ) )
        break;

      cur++;
    }

  Exit:
    if ( cur == parser->cursor )
    {
      FT_ERROR(( "ps_parser_skip_PS_token:"
                 " current token is `%c' which is self-delimiting\n"
                 "                        "
                 " but invalid at this point\n",
                 *cur ));

      error = PSaux_Err_Invalid_File_Format;
    }

    parser->error  = error;
    parser->cursor = cur;
  }


  FT_LOCAL_DEF( void )
  ps_parser_skip_spaces( PS_Parser  parser )
  {
    skip_spaces( &parser->cursor, parser->limit );
  }


  /* `token' here means either something between balanced delimiters */
  /* or the next token; the delimiters are not removed.              */

  FT_LOCAL_DEF( void )
  ps_parser_to_token( PS_Parser  parser,
                      T1_Token   token )
  {
    FT_Byte*  cur;
    FT_Byte*  limit;
    FT_Int    embed;


    token->type  = T1_TOKEN_TYPE_NONE;
    token->start = 0;
    token->limit = 0;

    /* first of all, skip leading whitespace */
    ps_parser_skip_spaces( parser );

    cur   = parser->cursor;
    limit = parser->limit;

    if ( cur >= limit )
      return;

    switch ( *cur )
    {
      /************* check for literal string *****************/
    case '(':
      token->type  = T1_TOKEN_TYPE_STRING;
      token->start = cur;

      if ( skip_literal_string( &cur, limit ) == PSaux_Err_Ok )
        token->limit = cur;
      break;

      /************* check for programs/array *****************/
    case '{':
      token->type  = T1_TOKEN_TYPE_ARRAY;
      token->start = cur;

      if ( skip_procedure( &cur, limit ) == PSaux_Err_Ok )
        token->limit = cur;
      break;

      /************* check for table/array ********************/
      /* XXX: in theory we should also look for "<<"          */
      /*      since this is semantically equivalent to "[";   */
      /*      in practice it doesn't matter (?)               */
    case '[':
      token->type  = T1_TOKEN_TYPE_ARRAY;
      embed        = 1;
      token->start = cur++;

      /* we need this to catch `[ ]' */
      parser->cursor = cur;
      ps_parser_skip_spaces( parser );
      cur = parser->cursor;

      while ( cur < limit && !parser->error )
      {
        /* XXX: this is wrong because it does not      */
        /*      skip comments, procedures, and strings */
        if ( *cur == '[' )
          embed++;
        else if ( *cur == ']' )
        {
          embed--;
          if ( embed <= 0 )
          {
            token->limit = ++cur;
            break;
          }
        }

        parser->cursor = cur;
        ps_parser_skip_PS_token( parser );
        /* we need this to catch `[XXX ]' */
        ps_parser_skip_spaces  ( parser );
        cur = parser->cursor;
      }
      break;

      /* ************ otherwise, it is any token **************/
    default:
      token->start = cur;
      token->type  = ( *cur == '/' ? T1_TOKEN_TYPE_KEY : T1_TOKEN_TYPE_ANY );
      ps_parser_skip_PS_token( parser );
      cur = parser->cursor;
      if ( !parser->error )
        token->limit = cur;
    }

    if ( !token->limit )
    {
      token->start = 0;
      token->type  = T1_TOKEN_TYPE_NONE;
    }

    parser->cursor = cur;
  }


  /* NB: `tokens' can be NULL if we only want to count */
  /* the number of array elements                      */

  FT_LOCAL_DEF( void )
  ps_parser_to_token_array( PS_Parser  parser,
                            T1_Token   tokens,
                            FT_UInt    max_tokens,
                            FT_Int*    pnum_tokens )
  {
    T1_TokenRec  master;


    *pnum_tokens = -1;

    /* this also handles leading whitespace */
    ps_parser_to_token( parser, &master );

    if ( master.type == T1_TOKEN_TYPE_ARRAY )
    {
      FT_Byte*  old_cursor = parser->cursor;
      FT_Byte*  old_limit  = parser->limit;
      T1_Token  cur        = tokens;
      T1_Token  limit      = cur + max_tokens;


      /* don't include outermost delimiters */
      parser->cursor = master.start + 1;
      parser->limit  = master.limit - 1;

      while ( parser->cursor < parser->limit )
      {
        T1_TokenRec  token;


        ps_parser_to_token( parser, &token );
        if ( !token.type )
          break;

        if ( tokens != NULL && cur < limit )
          *cur = token;

        cur++;
      }

      *pnum_tokens = (FT_Int)( cur - tokens );

      parser->cursor = old_cursor;
      parser->limit  = old_limit;
    }
  }


  /* first character must be a delimiter or a part of a number */
  /* NB: `coords' can be NULL if we just want to skip the      */
  /*     array; in this case we ignore `max_coords'            */

  static FT_Int
  ps_tocoordarray( FT_Byte*  *acur,
                   FT_Byte*   limit,
                   FT_Int     max_coords,
                   FT_Short*  coords )
  {
    FT_Byte*  cur   = *acur;
    FT_Int    count = 0;
    FT_Byte   c, ender;


    if ( cur >= limit )
      goto Exit;

    /* check for the beginning of an array; otherwise, only one number */
    /* will be read                                                    */
    c     = *cur;
    ender = 0;

    if ( c == '[' )
      ender = ']';
    else if ( c == '{' )
      ender = '}';

    if ( ender )
      cur++;

    /* now, read the coordinates */
    while ( cur < limit )
    {
      FT_Short  dummy;
      FT_Byte*  old_cur;


      /* skip whitespace in front of data */
      skip_spaces( &cur, limit );
      if ( cur >= limit )
        goto Exit;

      if ( *cur == ender )
      {
        cur++;
        break;
      }

      old_cur = cur;

      if ( coords != NULL && count >= max_coords )
        break;

      /* call PS_Conv_ToFixed() even if coords == NULL */
      /* to properly parse number at `cur'             */
      *( coords != NULL ? &coords[count] : &dummy ) =
        (FT_Short)( PS_Conv_ToFixed( &cur, limit, 0 ) >> 16 );

      if ( old_cur == cur )
      {
        count = -1;
        goto Exit;
      }
      else
        count++;

      if ( !ender )
        break;
    }

  Exit:
    *acur = cur;
    return count;
  }


  /* first character must be a delimiter or a part of a number */
  /* NB: `values' can be NULL if we just want to skip the      */
  /*     array; in this case we ignore `max_values'            */

  static FT_Int
  ps_tofixedarray( FT_Byte*  *acur,
                   FT_Byte*   limit,
                   FT_Int     max_values,
                   FT_Fixed*  values,
                   FT_Int     power_ten )
  {
    FT_Byte*  cur   = *acur;
    FT_Int    count = 0;
    FT_Byte   c, ender;


    if ( cur >= limit )
      goto Exit;

    /* Check for the beginning of an array.  Otherwise, only one number */
    /* will be read.                                                    */
    c     = *cur;
    ender = 0;

    if ( c == '[' )
      ender = ']';
    else if ( c == '{' )
      ender = '}';

    if ( ender )
      cur++;

    /* now, read the values */
    while ( cur < limit )
    {
      FT_Fixed  dummy;
      FT_Byte*  old_cur;


      /* skip whitespace in front of data */
      skip_spaces( &cur, limit );
      if ( cur >= limit )
        goto Exit;

      if ( *cur == ender )
      {
        cur++;
        break;
      }

      old_cur = cur;

      if ( values != NULL && count >= max_values )
        break;

      /* call PS_Conv_ToFixed() even if coords == NULL */
      /* to properly parse number at `cur'             */
      *( values != NULL ? &values[count] : &dummy ) =
        PS_Conv_ToFixed( &cur, limit, power_ten );

      if ( old_cur == cur )
      {
        count = -1;
        goto Exit;
      }
      else
        count++;

      if ( !ender )
        break;
    }

  Exit:
    *acur = cur;
    return count;
  }


#if 0

  static FT_String*
  ps_tostring( FT_Byte**  cursor,
               FT_Byte*   limit,
               FT_Memory  memory )
  {
    FT_Byte*    cur = *cursor;
    FT_PtrDist  len = 0;
    FT_Int      count;
    FT_String*  result;
    FT_Error    error;


    /* XXX: some stupid fonts have a `Notice' or `Copyright' string     */
    /*      that simply doesn't begin with an opening parenthesis, even */
    /*      though they have a closing one!  E.g. "amuncial.pfb"        */
    /*                                                                  */
    /*      We must deal with these ill-fated cases there.  Note that   */
    /*      these fonts didn't work with the old Type 1 driver as the   */
    /*      notice/copyright was not recognized as a valid string token */
    /*      and made the old token parser commit errors.                */

    while ( cur < limit && ( *cur == ' ' || *cur == '\t' ) )
      cur++;
    if ( cur + 1 >= limit )
      return 0;

    if ( *cur == '(' )
      cur++;  /* skip the opening parenthesis, if there is one */

    *cursor = cur;
    count   = 0;

    /* then, count its length */
    for ( ; cur < limit; cur++ )
    {
      if ( *cur == '(' )
        count++;

      else if ( *cur == ')' )
      {
        count--;
        if ( count < 0 )
          break;
      }
    }

    len = cur - *cursor;
    if ( cur >= limit || FT_ALLOC( result, len + 1 ) )
      return 0;

    /* now copy the string */
    FT_MEM_COPY( result, *cursor, len );
    result[len] = '\0';
    *cursor = cur;
    return result;
  }

#endif /* 0 */


  static int
  ps_tobool( FT_Byte*  *acur,
             FT_Byte*   limit )
  {
    FT_Byte*  cur    = *acur;
    FT_Bool   result = 0;


    /* return 1 if we find `true', 0 otherwise */
    if ( cur + 3 < limit &&
         cur[0] == 't'   &&
         cur[1] == 'r'   &&
         cur[2] == 'u'   &&
         cur[3] == 'e'   )
    {
      result = 1;
      cur   += 5;
    }
    else if ( cur + 4 < limit &&
              cur[0] == 'f'   &&
              cur[1] == 'a'   &&
              cur[2] == 'l'   &&
              cur[3] == 's'   &&
              cur[4] == 'e'   )
    {
      result = 0;
      cur   += 6;
    }

    *acur = cur;
    return result;
  }


  /* load a simple field (i.e. non-table) into the current list of objects */

  FT_LOCAL_DEF( FT_Error )
  ps_parser_load_field( PS_Parser       parser,
                        const T1_Field  field,
                        void**          objects,
                        FT_UInt         max_objects,
                        FT_ULong*       pflags )
  {
    T1_TokenRec  token;
    FT_Byte*     cur;
    FT_Byte*     limit;
    FT_UInt      count;
    FT_UInt      idx;
    FT_Error     error;


    /* this also skips leading whitespace */
    ps_parser_to_token( parser, &token );
    if ( !token.type )
      goto Fail;

    count = 1;
    idx   = 0;
    cur   = token.start;
    limit = token.limit;

    /* we must detect arrays in /FontBBox */
    if ( field->type == T1_FIELD_TYPE_BBOX )
    {
      T1_TokenRec  token2;
      FT_Byte*     old_cur   = parser->cursor;
      FT_Byte*     old_limit = parser->limit;


      /* don't include delimiters */
      parser->cursor = token.start + 1;
      parser->limit  = token.limit - 1;

      ps_parser_to_token( parser, &token2 );
      parser->cursor = old_cur;
      parser->limit  = old_limit;

      if ( token2.type == T1_TOKEN_TYPE_ARRAY )
        goto FieldArray;
    }
    else if ( token.type == T1_TOKEN_TYPE_ARRAY )
    {
    FieldArray:
      /* if this is an array and we have no blend, an error occurs */
      if ( max_objects == 0 )
        goto Fail;

      count = max_objects;
      idx   = 1;

      /* don't include delimiters */
      cur++;
      limit--;
    }

    for ( ; count > 0; count--, idx++ )
    {
      FT_Byte*    q = (FT_Byte*)objects[idx] + field->offset;
      FT_Long     val;
      FT_String*  string;


      skip_spaces( &cur, limit );

      switch ( field->type )
      {
      case T1_FIELD_TYPE_BOOL:
        val = ps_tobool( &cur, limit );
        goto Store_Integer;

      case T1_FIELD_TYPE_FIXED:
        val = PS_Conv_ToFixed( &cur, limit, 0 );
        goto Store_Integer;

      case T1_FIELD_TYPE_FIXED_1000:
        val = PS_Conv_ToFixed( &cur, limit, 3 );
        goto Store_Integer;

      case T1_FIELD_TYPE_INTEGER:
        val = PS_Conv_ToInt( &cur, limit );
        /* fall through */

      Store_Integer:
        switch ( field->size )
        {
        case (8 / FT_CHAR_BIT):
          *(FT_Byte*)q = (FT_Byte)val;
          break;

        case (16 / FT_CHAR_BIT):
          *(FT_UShort*)q = (FT_UShort)val;
          break;

        case (32 / FT_CHAR_BIT):
          *(FT_UInt32*)q = (FT_UInt32)val;
          break;

        default:                /* for 64-bit systems */
          *(FT_Long*)q = val;
        }
        break;

      case T1_FIELD_TYPE_STRING:
      case T1_FIELD_TYPE_KEY:
        {
          FT_Memory  memory = parser->memory;
          FT_UInt    len    = (FT_UInt)( limit - cur );


          if ( cur >= limit )
            break;

          /* we allow both a string or a name   */
          /* for cases like /FontName (foo) def */
          if ( token.type == T1_TOKEN_TYPE_KEY )
          {
            /* don't include leading `/' */
            len--;
            cur++;
          }
          else if ( token.type == T1_TOKEN_TYPE_STRING )
          {
            /* don't include delimiting parentheses    */
            /* XXX we don't handle <<...>> here        */
            /* XXX should we convert octal escapes?    */
            /*     if so, what encoding should we use? */
            cur++;
            len -= 2;
          }
          else
          {
            FT_ERROR(( "ps_parser_load_field:"
                       " expected a name or string\n"
                       "                     "
                       " but found token of type %d instead\n",
                       token.type ));
            error = PSaux_Err_Invalid_File_Format;
            goto Exit;
          }

          /* for this to work (FT_String**)q must have been */
          /* initialized to NULL                            */
          if ( *(FT_String**)q != NULL )
          {
            FT_TRACE0(( "ps_parser_load_field: overwriting field %s\n",
                        field->ident ));
            FT_FREE( *(FT_String**)q );
            *(FT_String**)q = NULL;
          }

          if ( FT_ALLOC( string, len + 1 ) )
            goto Exit;

          FT_MEM_COPY( string, cur, len );
          string[len] = 0;

          *(FT_String**)q = string;
        }
        break;

      case T1_FIELD_TYPE_BBOX:
        {
          FT_Fixed  temp[4];
          FT_BBox*  bbox = (FT_BBox*)q;
          FT_Int    result;


          result = ps_tofixedarray( &cur, limit, 4, temp, 0 );

          if ( result < 0 )
          {
            FT_ERROR(( "ps_parser_load_field:"
                       " expected four integers in bounding box\n" ));
            error = PSaux_Err_Invalid_File_Format;
            goto Exit;
          }

          bbox->xMin = FT_RoundFix( temp[0] );
          bbox->yMin = FT_RoundFix( temp[1] );
          bbox->xMax = FT_RoundFix( temp[2] );
          bbox->yMax = FT_RoundFix( temp[3] );
        }
        break;

      default:
        /* an error occurred */
        goto Fail;
      }
    }

#if 0  /* obsolete -- keep for reference */
    if ( pflags )
      *pflags |= 1L << field->flag_bit;
#else
    FT_UNUSED( pflags );
#endif

    error = PSaux_Err_Ok;

  Exit:
    return error;

  Fail:
    error = PSaux_Err_Invalid_File_Format;
    goto Exit;
  }


#define T1_MAX_TABLE_ELEMENTS  32


  FT_LOCAL_DEF( FT_Error )
  ps_parser_load_field_table( PS_Parser       parser,
                              const T1_Field  field,
                              void**          objects,
                              FT_UInt         max_objects,
                              FT_ULong*       pflags )
  {
    T1_TokenRec  elements[T1_MAX_TABLE_ELEMENTS];
    T1_Token     token;
    FT_Int       num_elements;
    FT_Error     error = PSaux_Err_Ok;
    FT_Byte*     old_cursor;
    FT_Byte*     old_limit;
    T1_FieldRec  fieldrec = *(T1_Field)field;


    fieldrec.type = T1_FIELD_TYPE_INTEGER;
    if ( field->type == T1_FIELD_TYPE_FIXED_ARRAY ||
         field->type == T1_FIELD_TYPE_BBOX        )
      fieldrec.type = T1_FIELD_TYPE_FIXED;

    ps_parser_to_token_array( parser, elements,
                              T1_MAX_TABLE_ELEMENTS, &num_elements );
    if ( num_elements < 0 )
    {
      error = PSaux_Err_Ignore;
      goto Exit;
    }
    if ( (FT_UInt)num_elements > field->array_max )
      num_elements = field->array_max;

    old_cursor = parser->cursor;
    old_limit  = parser->limit;

    /* we store the elements count if necessary;           */
    /* we further assume that `count_offset' can't be zero */
    if ( field->type != T1_FIELD_TYPE_BBOX && field->count_offset != 0 )
      *(FT_Byte*)( (FT_Byte*)objects[0] + field->count_offset ) =
        (FT_Byte)num_elements;

    /* we now load each element, adjusting the field.offset on each one */
    token = elements;
    for ( ; num_elements > 0; num_elements--, token++ )
    {
      parser->cursor = token->start;
      parser->limit  = token->limit;
      ps_parser_load_field( parser, &fieldrec, objects, max_objects, 0 );
      fieldrec.offset += fieldrec.size;
    }

#if 0  /* obsolete -- keep for reference */
    if ( pflags )
      *pflags |= 1L << field->flag_bit;
#else
    FT_UNUSED( pflags );
#endif

    parser->cursor = old_cursor;
    parser->limit  = old_limit;

  Exit:
    return error;
  }


  FT_LOCAL_DEF( FT_Long )
  ps_parser_to_int( PS_Parser  parser )
  {
    ps_parser_skip_spaces( parser );
    return PS_Conv_ToInt( &parser->cursor, parser->limit );
  }


  /* first character must be `<' if `delimiters' is non-zero */

  FT_LOCAL_DEF( FT_Error )
  ps_parser_to_bytes( PS_Parser  parser,
                      FT_Byte*   bytes,
                      FT_Offset  max_bytes,
                      FT_Long*   pnum_bytes,
                      FT_Bool    delimiters )
  {
    FT_Error  error = PSaux_Err_Ok;
    FT_Byte*  cur;


    ps_parser_skip_spaces( parser );
    cur = parser->cursor;

    if ( cur >= parser->limit )
      goto Exit;

    if ( delimiters )
    {
      if ( *cur != '<' )
      {
        FT_ERROR(( "ps_parser_to_bytes: Missing starting delimiter `<'\n" ));
        error = PSaux_Err_Invalid_File_Format;
        goto Exit;
      }

      cur++;
    }

    *pnum_bytes = PS_Conv_ASCIIHexDecode( &cur,
                                          parser->limit,
                                          bytes,
                                          max_bytes );

    if ( delimiters )
    {
      if ( cur < parser->limit && *cur != '>' )
      {
        FT_ERROR(( "ps_parser_to_bytes: Missing closing delimiter `>'\n" ));
        error = PSaux_Err_Invalid_File_Format;
        goto Exit;
      }

      cur++;
    }

    parser->cursor = cur;

  Exit:
    return error;
  }


  FT_LOCAL_DEF( FT_Fixed )
  ps_parser_to_fixed( PS_Parser  parser,
                      FT_Int     power_ten )
  {
    ps_parser_skip_spaces( parser );
    return PS_Conv_ToFixed( &parser->cursor, parser->limit, power_ten );
  }


  FT_LOCAL_DEF( FT_Int )
  ps_parser_to_coord_array( PS_Parser  parser,
                            FT_Int     max_coords,
                            FT_Short*  coords )
  {
    ps_parser_skip_spaces( parser );
    return ps_tocoordarray( &parser->cursor, parser->limit,
                            max_coords, coords );
  }


  FT_LOCAL_DEF( FT_Int )
  ps_parser_to_fixed_array( PS_Parser  parser,
                            FT_Int     max_values,
                            FT_Fixed*  values,
                            FT_Int     power_ten )
  {
    ps_parser_skip_spaces( parser );
    return ps_tofixedarray( &parser->cursor, parser->limit,
                            max_values, values, power_ten );
  }


#if 0

  FT_LOCAL_DEF( FT_String* )
  T1_ToString( PS_Parser  parser )
  {
    return ps_tostring( &parser->cursor, parser->limit, parser->memory );
  }


  FT_LOCAL_DEF( FT_Bool )
  T1_ToBool( PS_Parser  parser )
  {
    return ps_tobool( &parser->cursor, parser->limit );
  }

#endif /* 0 */


  FT_LOCAL_DEF( void )
  ps_parser_init( PS_Parser  parser,
                  FT_Byte*   base,
                  FT_Byte*   limit,
                  FT_Memory  memory )
  {
    parser->error  = PSaux_Err_Ok;
    parser->base   = base;
    parser->limit  = limit;
    parser->cursor = base;
    parser->memory = memory;
    parser->funcs  = ps_parser_funcs;
  }


  FT_LOCAL_DEF( void )
  ps_parser_done( PS_Parser  parser )
  {
    FT_UNUSED( parser );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                            T1 BUILDER                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    t1_builder_init                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given glyph builder.                                 */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    builder :: A pointer to the glyph builder to initialize.           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: The current face object.                                */
  /*                                                                       */
  /*    size    :: The current size object.                                */
  /*                                                                       */
  /*    glyph   :: The current glyph object.                               */
  /*                                                                       */
  /*    hinting :: Whether hinting should be applied.                      */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  t1_builder_init( T1_Builder    builder,
                   FT_Face       face,
                   FT_Size       size,
                   FT_GlyphSlot  glyph,
                   FT_Bool       hinting )
  {
    builder->parse_state = T1_Parse_Start;
    builder->load_points = 1;

    builder->face   = face;
    builder->glyph  = glyph;
    builder->memory = face->memory;

    if ( glyph )
    {
      FT_GlyphLoader  loader = glyph->internal->loader;


      builder->loader  = loader;
      builder->base    = &loader->base.outline;
      builder->current = &loader->current.outline;
      FT_GlyphLoader_Rewind( loader );

      builder->hints_globals = size->internal;
      builder->hints_funcs   = 0;

      if ( hinting )
        builder->hints_funcs = glyph->internal->glyph_hints;
    }

    builder->pos_x = 0;
    builder->pos_y = 0;

    builder->left_bearing.x = 0;
    builder->left_bearing.y = 0;
    builder->advance.x      = 0;
    builder->advance.y      = 0;

    builder->funcs = t1_builder_funcs;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    t1_builder_done                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Finalizes a given glyph builder.  Its contents can still be used   */
  /*    after the call, but the function saves important information       */
  /*    within the corresponding glyph slot.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    builder :: A pointer to the glyph builder to finalize.             */
  /*                                                                       */
  FT_LOCAL_DEF( void )
  t1_builder_done( T1_Builder  builder )
  {
    FT_GlyphSlot  glyph = builder->glyph;


    if ( glyph )
      glyph->outline = *builder->base;
  }


  /* check that there is enough space for `count' more points */
  FT_LOCAL_DEF( FT_Error )
  t1_builder_check_points( T1_Builder  builder,
                           FT_Int      count )
  {
    return FT_GLYPHLOADER_CHECK_POINTS( builder->loader, count, 0 );
  }


  /* add a new point, do not check space */
  FT_LOCAL_DEF( void )
  t1_builder_add_point( T1_Builder  builder,
                        FT_Pos      x,
                        FT_Pos      y,
                        FT_Byte     flag )
  {
    FT_Outline*  outline = builder->current;


    if ( builder->load_points )
    {
      FT_Vector*  point   = outline->points + outline->n_points;
      FT_Byte*    control = (FT_Byte*)outline->tags + outline->n_points;


      point->x = FIXED_TO_INT( x );
      point->y = FIXED_TO_INT( y );
      *control = (FT_Byte)( flag ? FT_CURVE_TAG_ON : FT_CURVE_TAG_CUBIC );
    }
    outline->n_points++;
  }


  /* check space for a new on-curve point, then add it */
  FT_LOCAL_DEF( FT_Error )
  t1_builder_add_point1( T1_Builder  builder,
                         FT_Pos      x,
                         FT_Pos      y )
  {
    FT_Error  error;


    error = t1_builder_check_points( builder, 1 );
    if ( !error )
      t1_builder_add_point( builder, x, y, 1 );

    return error;
  }


  /* check space for a new contour, then add it */
  FT_LOCAL_DEF( FT_Error )
  t1_builder_add_contour( T1_Builder  builder )
  {
    FT_Outline*  outline = builder->current;
    FT_Error     error;


    /* this might happen in invalid fonts */
    if ( !outline )
    {
      FT_ERROR(( "t1_builder_add_contour: no outline to add points to\n" ));
      return PSaux_Err_Invalid_File_Format;
    }

    if ( !builder->load_points )
    {
      outline->n_contours++;
      return PSaux_Err_Ok;
    }

    error = FT_GLYPHLOADER_CHECK_POINTS( builder->loader, 0, 1 );
    if ( !error )
    {
      if ( outline->n_contours > 0 )
        outline->contours[outline->n_contours - 1] =
          (short)( outline->n_points - 1 );

      outline->n_contours++;
    }

    return error;
  }


  /* if a path was begun, add its first on-curve point */
  FT_LOCAL_DEF( FT_Error )
  t1_builder_start_point( T1_Builder  builder,
                          FT_Pos      x,
                          FT_Pos      y )
  {
    FT_Error  error = PSaux_Err_Invalid_File_Format;


    /* test whether we are building a new contour */

    if ( builder->parse_state == T1_Parse_Have_Path )
      error = PSaux_Err_Ok;
    else
    {
      builder->parse_state = T1_Parse_Have_Path;
      error = t1_builder_add_contour( builder );
      if ( !error )
        error = t1_builder_add_point1( builder, x, y );
    }

    return error;
  }


  /* close the current contour */
  FT_LOCAL_DEF( void )
  t1_builder_close_contour( T1_Builder  builder )
  {
    FT_Outline*  outline = builder->current;
    FT_Int       first;


    if ( !outline )
      return;

    first = outline->n_contours <= 1
            ? 0 : outline->contours[outline->n_contours - 2] + 1;

    /* We must not include the last point in the path if it */
    /* is located on the first point.                       */
    if ( outline->n_points > 1 )
    {
      FT_Vector*  p1      = outline->points + first;
      FT_Vector*  p2      = outline->points + outline->n_points - 1;
      FT_Byte*    control = (FT_Byte*)outline->tags + outline->n_points - 1;


      /* `delete' last point only if it coincides with the first */
      /* point and it is not a control point (which can happen). */
      if ( p1->x == p2->x && p1->y == p2->y )
        if ( *control == FT_CURVE_TAG_ON )
          outline->n_points--;
    }

    if ( outline->n_contours > 0 )
    {
      /* Don't add contours only consisting of one point, i.e.,  */
      /* check whether the first and the last point is the same. */
      if ( first == outline->n_points - 1 )
      {
        outline->n_contours--;
        outline->n_points--;
      }
      else
        outline->contours[outline->n_contours - 1] =
          (short)( outline->n_points - 1 );
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                            OTHER                              *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_LOCAL_DEF( void )
  t1_decrypt( FT_Byte*   buffer,
              FT_Offset  length,
              FT_UShort  seed )
  {
    PS_Conv_EexecDecode( &buffer,
                         buffer + length,
                         buffer,
                         length,
                         &seed );
  }


/* END */
