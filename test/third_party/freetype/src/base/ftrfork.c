/***************************************************************************/
/*                                                                         */
/*  ftrfork.c                                                              */
/*                                                                         */
/*    Embedded resource forks accessor (body).                             */
/*                                                                         */
/*  Copyright 2004, 2005, 2006, 2007, 2008, 2009, 2010 by                  */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  FT_Raccess_Get_HeaderInfo() and raccess_guess_darwin_hfsplus() are     */
/*  derived from ftobjs.c.                                                 */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/


#include <ft2build.h>
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_RFORK_H


#undef  FT_COMPONENT
#define FT_COMPONENT  trace_raccess


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****               Resource fork directory access                    ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  FT_BASE_DEF( FT_Error )
  FT_Raccess_Get_HeaderInfo( FT_Library  library,
                             FT_Stream   stream,
                             FT_Long     rfork_offset,
                             FT_Long    *map_offset,
                             FT_Long    *rdata_pos )
  {
    FT_Error       error;
    unsigned char  head[16], head2[16];
    FT_Long        map_pos, rdata_len;
    int            allzeros, allmatch, i;
    FT_Long        type_list;

    FT_UNUSED( library );


    error = FT_Stream_Seek( stream, rfork_offset );
    if ( error )
      return error;

    error = FT_Stream_Read( stream, (FT_Byte *)head, 16 );
    if ( error )
      return error;

    *rdata_pos = rfork_offset + ( ( head[0] << 24 ) |
                                  ( head[1] << 16 ) |
                                  ( head[2] <<  8 ) |
                                    head[3]         );
    map_pos    = rfork_offset + ( ( head[4] << 24 ) |
                                  ( head[5] << 16 ) |
                                  ( head[6] <<  8 ) |
                                    head[7]         );
    rdata_len = ( head[ 8] << 24 ) |
                ( head[ 9] << 16 ) |
                ( head[10] <<  8 ) |
                  head[11];

    /* map_len = head[12] .. head[15] */

    if ( *rdata_pos + rdata_len != map_pos || map_pos == rfork_offset )
      return FT_Err_Unknown_File_Format;

    error = FT_Stream_Seek( stream, map_pos );
    if ( error )
      return error;

    head2[15] = (FT_Byte)( head[15] + 1 );       /* make it be different */

    error = FT_Stream_Read( stream, (FT_Byte*)head2, 16 );
    if ( error )
      return error;

    allzeros = 1;
    allmatch = 1;
    for ( i = 0; i < 16; ++i )
    {
      if ( head2[i] != 0 )
        allzeros = 0;
      if ( head2[i] != head[i] )
        allmatch = 0;
    }
    if ( !allzeros && !allmatch )
      return FT_Err_Unknown_File_Format;

    /* If we have reached this point then it is probably a mac resource */
    /* file.  Now, does it contain any interesting resources?           */
    /* Skip handle to next resource map, the file resource number, and  */
    /* attributes.                                                      */
    (void)FT_STREAM_SKIP( 4        /* skip handle to next resource map */
                          + 2      /* skip file resource number */
                          + 2 );   /* skip attributes */

    if ( FT_READ_USHORT( type_list ) )
      return error;
    if ( type_list == -1 )
      return FT_Err_Unknown_File_Format;

    error = FT_Stream_Seek( stream, map_pos + type_list );
    if ( error )
      return error;

    *map_offset = map_pos + type_list;
    return FT_Err_Ok;
  }


  static int
  ft_raccess_sort_ref_by_id( FT_RFork_Ref*  a,
                             FT_RFork_Ref*  b )
  {
    if ( a->res_id < b->res_id )
      return -1;
    else if ( a->res_id > b->res_id )
      return 1;
    else
      return 0;
  }


  FT_BASE_DEF( FT_Error )
  FT_Raccess_Get_DataOffsets( FT_Library  library,
                              FT_Stream   stream,
                              FT_Long     map_offset,
                              FT_Long     rdata_pos,
                              FT_Long     tag,
                              FT_Long   **offsets,
                              FT_Long    *count )
  {
    FT_Error      error;
    int           i, j, cnt, subcnt;
    FT_Long       tag_internal, rpos;
    FT_Memory     memory = library->memory;
    FT_Long       temp;
    FT_Long       *offsets_internal;
    FT_RFork_Ref  *ref;


    error = FT_Stream_Seek( stream, map_offset );
    if ( error )
      return error;

    if ( FT_READ_USHORT( cnt ) )
      return error;
    cnt++;

    for ( i = 0; i < cnt; ++i )
    {
      if ( FT_READ_LONG( tag_internal ) ||
           FT_READ_USHORT( subcnt )     ||
           FT_READ_USHORT( rpos )       )
        return error;

      FT_TRACE2(( "Resource tags: %c%c%c%c\n",
                  (char)( 0xff & ( tag_internal >> 24 ) ),
                  (char)( 0xff & ( tag_internal >> 16 ) ),
                  (char)( 0xff & ( tag_internal >>  8 ) ),
                  (char)( 0xff & ( tag_internal >>  0 ) ) ));

      if ( tag_internal == tag )
      {
        *count = subcnt + 1;
        rpos  += map_offset;

        error = FT_Stream_Seek( stream, rpos );
        if ( error )
          return error;

        if ( FT_NEW_ARRAY( ref, *count ) )
          return error;

        for ( j = 0; j < *count; ++j )
        {
          if ( FT_READ_USHORT( ref[j].res_id ) )
            goto Exit;
          if ( FT_STREAM_SKIP( 2 ) ) /* resource name */
            goto Exit;
          if ( FT_READ_LONG( temp ) )
            goto Exit;
          if ( FT_STREAM_SKIP( 4 ) ) /* mbz */
            goto Exit;

          ref[j].offset = temp & 0xFFFFFFL;
        }

        ft_qsort( ref, *count, sizeof ( FT_RFork_Ref ),
                  ( int(*)(const void*, const void*) )
                  ft_raccess_sort_ref_by_id );

        if ( FT_NEW_ARRAY( offsets_internal, *count ) )
          goto Exit;

        /* XXX: duplicated reference ID,
         *      gap between reference IDs are acceptable?
         *      further investigation on Apple implementation is needed.
         */
        for ( j = 0; j < *count; ++j )
          offsets_internal[j] = rdata_pos + ref[j].offset;

        *offsets = offsets_internal;
        error    = FT_Err_Ok;

      Exit:
        FT_FREE( ref );
        return error;
      }
    }

    return FT_Err_Cannot_Open_Resource;
  }


#ifdef FT_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                     Guessing functions                          ****/
  /****                                                                 ****/
  /****            When you add a new guessing function,                ****/
  /****           update FT_RACCESS_N_RULES in ftrfork.h.               ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  typedef FT_Error
  (*raccess_guess_func)( FT_Library  library,
                         FT_Stream   stream,
                         char       *base_file_name,
                         char      **result_file_name,
                         FT_Long    *result_offset );


  static FT_Error
  raccess_guess_apple_double( FT_Library  library,
                              FT_Stream   stream,
                              char       *base_file_name,
                              char      **result_file_name,
                              FT_Long    *result_offset );

  static FT_Error
  raccess_guess_apple_single( FT_Library  library,
                              FT_Stream   stream,
                              char       *base_file_name,
                              char      **result_file_name,
                              FT_Long    *result_offset );

  static FT_Error
  raccess_guess_darwin_ufs_export( FT_Library  library,
                                   FT_Stream   stream,
                                   char       *base_file_name,
                                   char      **result_file_name,
                                   FT_Long    *result_offset );

  static FT_Error
  raccess_guess_darwin_newvfs( FT_Library  library,
                               FT_Stream   stream,
                               char       *base_file_name,
                               char      **result_file_name,
                               FT_Long    *result_offset );

  static FT_Error
  raccess_guess_darwin_hfsplus( FT_Library  library,
                                FT_Stream   stream,
                                char       *base_file_name,
                                char      **result_file_name,
                                FT_Long    *result_offset );

  static FT_Error
  raccess_guess_vfat( FT_Library  library,
                      FT_Stream   stream,
                      char       *base_file_name,
                      char      **result_file_name,
                      FT_Long    *result_offset );

  static FT_Error
  raccess_guess_linux_cap( FT_Library  library,
                           FT_Stream   stream,
                           char       *base_file_name,
                           char      **result_file_name,
                           FT_Long    *result_offset );

  static FT_Error
  raccess_guess_linux_double( FT_Library  library,
                              FT_Stream   stream,
                              char       *base_file_name,
                              char      **result_file_name,
                              FT_Long    *result_offset );

  static FT_Error
  raccess_guess_linux_netatalk( FT_Library  library,
                                FT_Stream   stream,
                                char       *base_file_name,
                                char      **result_file_name,
                                FT_Long    *result_offset );


  /*************************************************************************/
  /****                                                                 ****/
  /****                       Helper functions                          ****/
  /****                                                                 ****/
  /*************************************************************************/

  static FT_Error
  raccess_guess_apple_generic( FT_Library  library,
                               FT_Stream   stream,
                               char       *base_file_name,
                               FT_Int32    magic,
                               FT_Long    *result_offset );

  static FT_Error
  raccess_guess_linux_double_from_file_name( FT_Library  library,
                                             char *      file_name,
                                             FT_Long    *result_offset );

  static char *
  raccess_make_file_name( FT_Memory    memory,
                          const char  *original_name,
                          const char  *insertion );


  typedef enum  FT_RFork_Rule_ {
    FT_RFork_Rule_invalid = -2,
    FT_RFork_Rule_uknown, /* -1 */
    FT_RFork_Rule_apple_double,
    FT_RFork_Rule_apple_single,
    FT_RFork_Rule_darwin_ufs_export,
    FT_RFork_Rule_darwin_newvfs,
    FT_RFork_Rule_darwin_hfsplus,
    FT_RFork_Rule_vfat,
    FT_RFork_Rule_linux_cap,
    FT_RFork_Rule_linux_double,
    FT_RFork_Rule_linux_netatalk
  } FT_RFork_Rule;

  /* For fast translation between rule index and rule type,
   * the macros FT_RFORK_xxx should be kept consistent with
   * the raccess_guess_funcs table
   */
  typedef struct raccess_guess_rec_ {
    raccess_guess_func  func;
    FT_RFork_Rule       type;
  } raccess_guess_rec;

  static raccess_guess_rec  raccess_guess_table[FT_RACCESS_N_RULES] =
  {
    { raccess_guess_apple_double,	FT_RFork_Rule_apple_double, },
    { raccess_guess_apple_single,	FT_RFork_Rule_apple_single, },
    { raccess_guess_darwin_ufs_export,	FT_RFork_Rule_darwin_ufs_export, },
    { raccess_guess_darwin_newvfs,	FT_RFork_Rule_darwin_newvfs, },
    { raccess_guess_darwin_hfsplus,	FT_RFork_Rule_darwin_hfsplus, },
    { raccess_guess_vfat,		FT_RFork_Rule_vfat, },
    { raccess_guess_linux_cap,		FT_RFork_Rule_linux_cap, },
    { raccess_guess_linux_double,	FT_RFork_Rule_linux_double, },
    { raccess_guess_linux_netatalk,	FT_RFork_Rule_linux_netatalk, },
  };

  FT_BASE_DEF( void )
  FT_Raccess_Guess( FT_Library  library,
                    FT_Stream   stream,
                    char*       base_name,
                    char      **new_names,
                    FT_Long    *offsets,
                    FT_Error   *errors )
  {
    FT_Long  i;


    for ( i = 0; i < FT_RACCESS_N_RULES; i++ )
    {
      new_names[i] = NULL;
      if ( NULL != stream )
        errors[i] = FT_Stream_Seek( stream, 0 );
      else
        errors[i] = FT_Err_Ok;

      if ( errors[i] )
        continue ;

      errors[i] = (raccess_guess_table[i].func)( library,
                                                 stream, base_name,
                                                 &(new_names[i]),
                                                 &(offsets[i]) );
    }

    return;
  }


  static FT_RFork_Rule
  raccess_get_rule_type_from_rule_index( FT_UInt  rule_index )
  {
    if ( rule_index >= FT_RACCESS_N_RULES )
      return FT_RFork_Rule_invalid;

    return raccess_guess_table[rule_index].type;
  }


  FT_LOCAL_DEF( FT_Bool )
  raccess_rule_by_darwin_vfs( FT_UInt  rule_index )
  {
    switch( raccess_get_rule_type_from_rule_index( rule_index ) )
    {
      case FT_RFork_Rule_darwin_newvfs:
      case FT_RFork_Rule_darwin_hfsplus:
        return TRUE;

      default:
        return FALSE;
    }
  }


  static FT_Error
  raccess_guess_apple_double( FT_Library  library,
                              FT_Stream   stream,
                              char       *base_file_name,
                              char      **result_file_name,
                              FT_Long    *result_offset )
  {
    FT_Int32  magic = ( 0x00 << 24 ) |
                      ( 0x05 << 16 ) |
                      ( 0x16 <<  8 ) |
                        0x07;


    *result_file_name = NULL;
    if ( NULL == stream )
      return FT_Err_Cannot_Open_Stream;

    return raccess_guess_apple_generic( library, stream, base_file_name,
                                        magic, result_offset );
  }


  static FT_Error
  raccess_guess_apple_single( FT_Library  library,
                              FT_Stream   stream,
                              char       *base_file_name,
                              char      **result_file_name,
                              FT_Long    *result_offset )
  {
    FT_Int32  magic = ( 0x00 << 24 ) |
                      ( 0x05 << 16 ) |
                      ( 0x16 <<  8 ) |
                        0x00;


    *result_file_name = NULL;
    if ( NULL == stream )
      return FT_Err_Cannot_Open_Stream;

    return raccess_guess_apple_generic( library, stream, base_file_name,
                                        magic, result_offset );
  }


  static FT_Error
  raccess_guess_darwin_ufs_export( FT_Library  library,
                                   FT_Stream   stream,
                                   char       *base_file_name,
                                   char      **result_file_name,
                                   FT_Long    *result_offset )
  {
    char*      newpath;
    FT_Error   error;
    FT_Memory  memory;

    FT_UNUSED( stream );


    memory  = library->memory;
    newpath = raccess_make_file_name( memory, base_file_name, "._" );
    if ( !newpath )
      return FT_Err_Out_Of_Memory;

    error = raccess_guess_linux_double_from_file_name( library, newpath,
                                                       result_offset );
    if ( !error )
      *result_file_name = newpath;
    else
      FT_FREE( newpath );

    return error;
  }


  static FT_Error
  raccess_guess_darwin_hfsplus( FT_Library  library,
                                FT_Stream   stream,
                                char       *base_file_name,
                                char      **result_file_name,
                                FT_Long    *result_offset )
  {
    /*
      Only meaningful on systems with hfs+ drivers (or Macs).
     */
    FT_Error   error;
    char*      newpath;
    FT_Memory  memory;
    FT_Long    base_file_len = ft_strlen( base_file_name );

    FT_UNUSED( stream );


    memory = library->memory;

    if ( base_file_len + 6 > FT_INT_MAX )
      return FT_Err_Array_Too_Large;

    if ( FT_ALLOC( newpath, base_file_len + 6 ) )
      return error;

    FT_MEM_COPY( newpath, base_file_name, base_file_len );
    FT_MEM_COPY( newpath + base_file_len, "/rsrc", 6 );

    *result_file_name = newpath;
    *result_offset    = 0;

    return FT_Err_Ok;
  }


  static FT_Error
  raccess_guess_darwin_newvfs( FT_Library  library,
                               FT_Stream   stream,
                               char       *base_file_name,
                               char      **result_file_name,
                               FT_Long    *result_offset )
  {
    /*
      Only meaningful on systems with Mac OS X (> 10.1).
     */
    FT_Error   error;
    char*      newpath;
    FT_Memory  memory;
    FT_Long    base_file_len = ft_strlen( base_file_name );

    FT_UNUSED( stream );


    memory = library->memory;

    if ( base_file_len + 18 > FT_INT_MAX )
      return FT_Err_Array_Too_Large;

    if ( FT_ALLOC( newpath, base_file_len + 18 ) )
      return error;

    FT_MEM_COPY( newpath, base_file_name, base_file_len );
    FT_MEM_COPY( newpath + base_file_len, "/..namedfork/rsrc", 18 );

    *result_file_name = newpath;
    *result_offset    = 0;

    return FT_Err_Ok;
  }


  static FT_Error
  raccess_guess_vfat( FT_Library  library,
                      FT_Stream   stream,
                      char       *base_file_name,
                      char      **result_file_name,
                      FT_Long    *result_offset )
  {
    char*      newpath;
    FT_Memory  memory;

    FT_UNUSED( stream );


    memory = library->memory;

    newpath = raccess_make_file_name( memory, base_file_name,
                                      "resource.frk/" );
    if ( !newpath )
      return FT_Err_Out_Of_Memory;

    *result_file_name = newpath;
    *result_offset    = 0;

    return FT_Err_Ok;
  }


  static FT_Error
  raccess_guess_linux_cap( FT_Library  library,
                           FT_Stream   stream,
                           char       *base_file_name,
                           char      **result_file_name,
                           FT_Long    *result_offset )
  {
    char*      newpath;
    FT_Memory  memory;

    FT_UNUSED( stream );


    memory = library->memory;

    newpath = raccess_make_file_name( memory, base_file_name, ".resource/" );
    if ( !newpath )
      return FT_Err_Out_Of_Memory;

    *result_file_name = newpath;
    *result_offset    = 0;

    return FT_Err_Ok;
  }


  static FT_Error
  raccess_guess_linux_double( FT_Library  library,
                              FT_Stream   stream,
                              char       *base_file_name,
                              char      **result_file_name,
                              FT_Long    *result_offset )
  {
    char*      newpath;
    FT_Error   error;
    FT_Memory  memory;

    FT_UNUSED( stream );


    memory = library->memory;

    newpath = raccess_make_file_name( memory, base_file_name, "%" );
    if ( !newpath )
      return FT_Err_Out_Of_Memory;

    error = raccess_guess_linux_double_from_file_name( library, newpath,
                                                       result_offset );
    if ( !error )
      *result_file_name = newpath;
    else
      FT_FREE( newpath );

    return error;
  }


  static FT_Error
  raccess_guess_linux_netatalk( FT_Library  library,
                                FT_Stream   stream,
                                char       *base_file_name,
                                char      **result_file_name,
                                FT_Long    *result_offset )
  {
    char*      newpath;
    FT_Error   error;
    FT_Memory  memory;

    FT_UNUSED( stream );


    memory = library->memory;

    newpath = raccess_make_file_name( memory, base_file_name,
                                      ".AppleDouble/" );
    if ( !newpath )
      return FT_Err_Out_Of_Memory;

    error = raccess_guess_linux_double_from_file_name( library, newpath,
                                                       result_offset );
    if ( !error )
      *result_file_name = newpath;
    else
      FT_FREE( newpath );

    return error;
  }


  static FT_Error
  raccess_guess_apple_generic( FT_Library  library,
                               FT_Stream   stream,
                               char       *base_file_name,
                               FT_Int32    magic,
                               FT_Long    *result_offset )
  {
    FT_Int32   magic_from_stream;
    FT_Error   error;
    FT_Int32   version_number = 0;
    FT_UShort  n_of_entries;

    int        i;
    FT_UInt32  entry_id, entry_offset, entry_length = 0;

    const FT_UInt32  resource_fork_entry_id = 0x2;

    FT_UNUSED( library );
    FT_UNUSED( base_file_name );
    FT_UNUSED( version_number );
    FT_UNUSED( entry_length   );


    if ( FT_READ_LONG( magic_from_stream ) )
      return error;
    if ( magic_from_stream != magic )
      return FT_Err_Unknown_File_Format;

    if ( FT_READ_LONG( version_number ) )
      return error;

    /* filler */
    error = FT_Stream_Skip( stream, 16 );
    if ( error )
      return error;

    if ( FT_READ_USHORT( n_of_entries ) )
      return error;
    if ( n_of_entries == 0 )
      return FT_Err_Unknown_File_Format;

    for ( i = 0; i < n_of_entries; i++ )
    {
      if ( FT_READ_LONG( entry_id ) )
        return error;
      if ( entry_id == resource_fork_entry_id )
      {
        if ( FT_READ_LONG( entry_offset ) ||
             FT_READ_LONG( entry_length ) )
          continue;
        *result_offset = entry_offset;

        return FT_Err_Ok;
      }
      else
      {
        error = FT_Stream_Skip( stream, 4 + 4 );    /* offset + length */
        if ( error )
          return error;
      }
    }

    return FT_Err_Unknown_File_Format;
  }


  static FT_Error
  raccess_guess_linux_double_from_file_name( FT_Library  library,
                                             char       *file_name,
                                             FT_Long    *result_offset )
  {
    FT_Open_Args  args2;
    FT_Stream     stream2;
    char *        nouse = NULL;
    FT_Error      error;


    args2.flags    = FT_OPEN_PATHNAME;
    args2.pathname = file_name;
    error = FT_Stream_New( library, &args2, &stream2 );
    if ( error )
      return error;

    error = raccess_guess_apple_double( library, stream2, file_name,
                                        &nouse, result_offset );

    FT_Stream_Free( stream2, 0 );

    return error;
  }


  static char*
  raccess_make_file_name( FT_Memory    memory,
                          const char  *original_name,
                          const char  *insertion )
  {
    char*        new_name = NULL;
    const char*  tmp;
    const char*  slash;
    size_t       new_length;
    FT_Error     error = FT_Err_Ok;

    FT_UNUSED( error );


    new_length = ft_strlen( original_name ) + ft_strlen( insertion );
    if ( FT_ALLOC( new_name, new_length + 1 ) )
      return NULL;

    tmp = ft_strrchr( original_name, '/' );
    if ( tmp )
    {
      ft_strncpy( new_name, original_name, tmp - original_name + 1 );
      new_name[tmp - original_name + 1] = '\0';
      slash = tmp + 1;
    }
    else
    {
      slash       = original_name;
      new_name[0] = '\0';
    }

    ft_strcat( new_name, insertion );
    ft_strcat( new_name, slash );

    return new_name;
  }


#else   /* !FT_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK */


  /*************************************************************************/
  /*                  Dummy function; just sets errors                     */
  /*************************************************************************/

  FT_BASE_DEF( void )
  FT_Raccess_Guess( FT_Library  library,
                    FT_Stream   stream,
                    char       *base_name,
                    char      **new_names,
                    FT_Long    *offsets,
                    FT_Error   *errors )
  {
    int  i;

    FT_UNUSED( library );
    FT_UNUSED( stream );
    FT_UNUSED( base_name );


    for ( i = 0; i < FT_RACCESS_N_RULES; i++ )
    {
      new_names[i] = NULL;
      offsets[i]   = 0;
      errors[i]    = FT_Err_Unimplemented_Feature;
    }
  }


#endif  /* !FT_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK */


/* END */
