/***************************************************************************/
/*                                                                         */
/*  sfobjs.c                                                               */
/*                                                                         */
/*    SFNT object management (base).                                       */
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


#include <ft2build.h>
#include "sfobjs.h"
#include "ttload.h"
#include "ttcmap.h"
#include "ttkern.h"
#include FT_INTERNAL_SFNT_H
#include FT_INTERNAL_DEBUG_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TAGS_H
#include FT_SERVICE_POSTSCRIPT_CMAPS_H
#include FT_SFNT_NAMES_H
#include "sferrors.h"

#ifdef TT_CONFIG_OPTION_BDF
#include "ttbdf.h"
#endif


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_sfobjs



  /* convert a UTF-16 name entry to ASCII */
  static FT_String*
  tt_name_entry_ascii_from_utf16( TT_NameEntry  entry,
                                  FT_Memory     memory )
  {
    FT_String*  string = NULL;
    FT_UInt     len, code, n;
    FT_Byte*    read   = (FT_Byte*)entry->string;
    FT_Error    error;


    len = (FT_UInt)entry->stringLength / 2;

    if ( FT_NEW_ARRAY( string, len + 1 ) )
      return NULL;

    for ( n = 0; n < len; n++ )
    {
      code = FT_NEXT_USHORT( read );
      if ( code < 32 || code > 127 )
        code = '?';

      string[n] = (char)code;
    }

    string[len] = 0;

    return string;
  }


  /* convert an Apple Roman or symbol name entry to ASCII */
  static FT_String*
  tt_name_entry_ascii_from_other( TT_NameEntry  entry,
                                  FT_Memory     memory )
  {
    FT_String*  string = NULL;
    FT_UInt     len, code, n;
    FT_Byte*    read   = (FT_Byte*)entry->string;
    FT_Error    error;


    len = (FT_UInt)entry->stringLength;

    if ( FT_NEW_ARRAY( string, len + 1 ) )
      return NULL;

    for ( n = 0; n < len; n++ )
    {
      code = *read++;
      if ( code < 32 || code > 127 )
        code = '?';

      string[n] = (char)code;
    }

    string[len] = 0;

    return string;
  }


  typedef FT_String*  (*TT_NameEntry_ConvertFunc)( TT_NameEntry  entry,
                                                   FT_Memory     memory );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    tt_face_get_name                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Returns a given ENGLISH name record in ASCII.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the source face object.                      */
  /*                                                                       */
  /*    nameid :: The name id of the name record to return.                */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    name   :: The address of a string pointer.  NULL if no name is     */
  /*              present.                                                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  static FT_Error
  tt_face_get_name( TT_Face      face,
                    FT_UShort    nameid,
                    FT_String**  name )
  {
    FT_Memory         memory = face->root.memory;
    FT_Error          error  = SFNT_Err_Ok;
    FT_String*        result = NULL;
    FT_UShort         n;
    TT_NameEntryRec*  rec;
    FT_Int            found_apple         = -1;
    FT_Int            found_apple_roman   = -1;
    FT_Int            found_apple_english = -1;
    FT_Int            found_win           = -1;
    FT_Int            found_unicode       = -1;

    FT_Bool           is_english = 0;

    TT_NameEntry_ConvertFunc  convert;


    FT_ASSERT( name );

    rec = face->name_table.names;
    for ( n = 0; n < face->num_names; n++, rec++ )
    {
      /* According to the OpenType 1.3 specification, only Microsoft or  */
      /* Apple platform IDs might be used in the `name' table.  The      */
      /* `Unicode' platform is reserved for the `cmap' table, and the    */
      /* `ISO' one is deprecated.                                        */
      /*                                                                 */
      /* However, the Apple TrueType specification doesn't say the same  */
      /* thing and goes to suggest that all Unicode `name' table entries */
      /* should be coded in UTF-16 (in big-endian format I suppose).     */
      /*                                                                 */
      if ( rec->nameID == nameid && rec->stringLength > 0 )
      {
        switch ( rec->platformID )
        {
        case TT_PLATFORM_APPLE_UNICODE:
        case TT_PLATFORM_ISO:
          /* there is `languageID' to check there.  We should use this */
          /* field only as a last solution when nothing else is        */
          /* available.                                                */
          /*                                                           */
          found_unicode = n;
          break;

        case TT_PLATFORM_MACINTOSH:
          /* This is a bit special because some fonts will use either    */
          /* an English language id, or a Roman encoding id, to indicate */
          /* the English version of its font name.                       */
          /*                                                             */
          if ( rec->languageID == TT_MAC_LANGID_ENGLISH )
            found_apple_english = n;
          else if ( rec->encodingID == TT_MAC_ID_ROMAN )
            found_apple_roman = n;
          break;

        case TT_PLATFORM_MICROSOFT:
          /* we only take a non-English name when there is nothing */
          /* else available in the font                            */
          /*                                                       */
          if ( found_win == -1 || ( rec->languageID & 0x3FF ) == 0x009 )
          {
            switch ( rec->encodingID )
            {
            case TT_MS_ID_SYMBOL_CS:
            case TT_MS_ID_UNICODE_CS:
            case TT_MS_ID_UCS_4:
              is_english = FT_BOOL( ( rec->languageID & 0x3FF ) == 0x009 );
              found_win  = n;
              break;

            default:
              ;
            }
          }
          break;

        default:
          ;
        }
      }
    }

    found_apple = found_apple_roman;
    if ( found_apple_english >= 0 )
      found_apple = found_apple_english;

    /* some fonts contain invalid Unicode or Macintosh formatted entries; */
    /* we will thus favor names encoded in Windows formats if available   */
    /* (provided it is an English name)                                   */
    /*                                                                    */
    convert = NULL;
    if ( found_win >= 0 && !( found_apple >= 0 && !is_english ) )
    {
      rec = face->name_table.names + found_win;
      switch ( rec->encodingID )
      {
        /* all Unicode strings are encoded using UTF-16BE */
      case TT_MS_ID_UNICODE_CS:
      case TT_MS_ID_SYMBOL_CS:
        convert = tt_name_entry_ascii_from_utf16;
        break;

      case TT_MS_ID_UCS_4:
        /* Apparently, if this value is found in a name table entry, it is */
        /* documented as `full Unicode repertoire'.  Experience with the   */
        /* MsGothic font shipped with Windows Vista shows that this really */
        /* means UTF-16 encoded names (UCS-4 values are only used within   */
        /* charmaps).                                                      */
        convert = tt_name_entry_ascii_from_utf16;
        break;

      default:
        ;
      }
    }
    else if ( found_apple >= 0 )
    {
      rec     = face->name_table.names + found_apple;
      convert = tt_name_entry_ascii_from_other;
    }
    else if ( found_unicode >= 0 )
    {
      rec     = face->name_table.names + found_unicode;
      convert = tt_name_entry_ascii_from_utf16;
    }

    if ( rec && convert )
    {
      if ( rec->string == NULL )
      {
        FT_Stream  stream = face->name_table.stream;


        if ( FT_QNEW_ARRAY ( rec->string, rec->stringLength ) ||
             FT_STREAM_SEEK( rec->stringOffset )              ||
             FT_STREAM_READ( rec->string, rec->stringLength ) )
        {
          FT_FREE( rec->string );
          rec->stringLength = 0;
          result            = NULL;
          goto Exit;
        }
      }

      result = convert( rec, memory );
    }

  Exit:
    *name = result;
    return error;
  }


  static FT_Encoding
  sfnt_find_encoding( int  platform_id,
                      int  encoding_id )
  {
    typedef struct  TEncoding_
    {
      int          platform_id;
      int          encoding_id;
      FT_Encoding  encoding;

    } TEncoding;

    static
    const TEncoding  tt_encodings[] =
    {
      { TT_PLATFORM_ISO,           -1,                  FT_ENCODING_UNICODE },

      { TT_PLATFORM_APPLE_UNICODE, -1,                  FT_ENCODING_UNICODE },

      { TT_PLATFORM_MACINTOSH,     TT_MAC_ID_ROMAN,     FT_ENCODING_APPLE_ROMAN },

      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_SYMBOL_CS,  FT_ENCODING_MS_SYMBOL },
      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_UCS_4,      FT_ENCODING_UNICODE },
      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_UNICODE_CS, FT_ENCODING_UNICODE },
      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_SJIS,       FT_ENCODING_SJIS },
      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_GB2312,     FT_ENCODING_GB2312 },
      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_BIG_5,      FT_ENCODING_BIG5 },
      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_WANSUNG,    FT_ENCODING_WANSUNG },
      { TT_PLATFORM_MICROSOFT,     TT_MS_ID_JOHAB,      FT_ENCODING_JOHAB }
    };

    const TEncoding  *cur, *limit;


    cur   = tt_encodings;
    limit = cur + sizeof ( tt_encodings ) / sizeof ( tt_encodings[0] );

    for ( ; cur < limit; cur++ )
    {
      if ( cur->platform_id == platform_id )
      {
        if ( cur->encoding_id == encoding_id ||
             cur->encoding_id == -1          )
          return cur->encoding;
      }
    }

    return FT_ENCODING_NONE;
  }


  /* Fill in face->ttc_header.  If the font is not a TTC, it is */
  /* synthesized into a TTC with one offset table.              */
  static FT_Error
  sfnt_open_font( FT_Stream  stream,
                  TT_Face    face )
  {
    FT_Memory  memory = stream->memory;
    FT_Error   error;
    FT_ULong   tag, offset;

    static const FT_Frame_Field  ttc_header_fields[] =
    {
#undef  FT_STRUCTURE
#define FT_STRUCTURE  TTC_HeaderRec

      FT_FRAME_START( 8 ),
        FT_FRAME_LONG( version ),
        FT_FRAME_LONG( count   ),
      FT_FRAME_END
    };


    face->ttc_header.tag     = 0;
    face->ttc_header.version = 0;
    face->ttc_header.count   = 0;

    offset = FT_STREAM_POS();

    if ( FT_READ_ULONG( tag ) )
      return error;

    if ( tag != 0x00010000UL &&
         tag != TTAG_ttcf    &&
         tag != TTAG_OTTO    &&
         tag != TTAG_true    &&
         tag != TTAG_typ1    &&
         tag != 0x00020000UL )
      return SFNT_Err_Unknown_File_Format;

    face->ttc_header.tag = TTAG_ttcf;

    if ( tag == TTAG_ttcf )
    {
      FT_Int  n;


      FT_TRACE3(( "sfnt_open_font: file is a collection\n" ));

      if ( FT_STREAM_READ_FIELDS( ttc_header_fields, &face->ttc_header ) )
        return error;

      /* now read the offsets of each font in the file */
      if ( FT_NEW_ARRAY( face->ttc_header.offsets, face->ttc_header.count ) )
        return error;

      if ( FT_FRAME_ENTER( face->ttc_header.count * 4L ) )
        return error;

      for ( n = 0; n < face->ttc_header.count; n++ )
        face->ttc_header.offsets[n] = FT_GET_ULONG();

      FT_FRAME_EXIT();
    }
    else
    {
      FT_TRACE3(( "sfnt_open_font: synthesize TTC\n" ));

      face->ttc_header.version = 1 << 16;
      face->ttc_header.count   = 1;

      if ( FT_NEW( face->ttc_header.offsets ) )
        return error;

      face->ttc_header.offsets[0] = offset;
    }

    return error;
  }


  FT_LOCAL_DEF( FT_Error )
  sfnt_init_face( FT_Stream      stream,
                  TT_Face        face,
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    FT_Error        error;
    FT_Library      library = face->root.driver->root.library;
    SFNT_Service    sfnt;


    /* for now, parameters are unused */
    FT_UNUSED( num_params );
    FT_UNUSED( params );


    sfnt = (SFNT_Service)face->sfnt;
    if ( !sfnt )
    {
      sfnt = (SFNT_Service)FT_Get_Module_Interface( library, "sfnt" );
      if ( !sfnt )
        return SFNT_Err_Invalid_File_Format;

      face->sfnt       = sfnt;
      face->goto_table = sfnt->goto_table;
    }

    FT_FACE_FIND_GLOBAL_SERVICE( face, face->psnames, POSTSCRIPT_CMAPS );

    error = sfnt_open_font( stream, face );
    if ( error )
      return error;

    FT_TRACE2(( "sfnt_init_face: %08p, %ld\n", face, face_index ));

    if ( face_index < 0 )
      face_index = 0;

    if ( face_index >= face->ttc_header.count )
      return SFNT_Err_Invalid_Argument;

    if ( FT_STREAM_SEEK( face->ttc_header.offsets[face_index] ) )
      return error;

    /* check that we have a valid TrueType file */
    error = sfnt->load_font_dir( face, stream );
    if ( error )
      return error;

    face->root.num_faces  = face->ttc_header.count;
    face->root.face_index = face_index;

    return error;
  }


#define LOAD_( x )                                            \
  do {                                                        \
    FT_TRACE2(( "`" #x "' " ));                               \
    FT_TRACE3(( "-->\n" ));                                   \
                                                              \
    error = sfnt->load_##x( face, stream );                   \
                                                              \
    FT_TRACE2(( "%s\n", ( !error )                            \
                        ? "loaded"                            \
                        : ( error == SFNT_Err_Table_Missing ) \
                          ? "missing"                         \
                          : "failed to load" ));              \
    FT_TRACE3(( "\n" ));                                      \
  } while ( 0 )

#define LOADM_( x, vertical )                                 \
  do {                                                        \
    FT_TRACE2(( "`%s" #x "' ",                                \
                vertical ? "vertical " : "" ));               \
    FT_TRACE3(( "-->\n" ));                                   \
                                                              \
    error = sfnt->load_##x( face, stream, vertical );         \
                                                              \
    FT_TRACE2(( "%s\n", ( !error )                            \
                        ? "loaded"                            \
                        : ( error == SFNT_Err_Table_Missing ) \
                          ? "missing"                         \
                          : "failed to load" ));              \
    FT_TRACE3(( "\n" ));                                      \
  } while ( 0 )

#define GET_NAME( id, field )                                 \
  do {                                                        \
    error = tt_face_get_name( face, TT_NAME_ID_##id, field ); \
    if ( error )                                              \
      goto Exit;                                              \
  } while ( 0 )


  FT_LOCAL_DEF( FT_Error )
  sfnt_load_face( FT_Stream      stream,
                  TT_Face        face,
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params )
  {
    FT_Error      error;
#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
    FT_Error      psnames_error;
#endif
    FT_Bool       has_outline;
    FT_Bool       is_apple_sbit;
    FT_Bool       ignore_preferred_family = FALSE;
    FT_Bool       ignore_preferred_subfamily = FALSE;

    SFNT_Service  sfnt = (SFNT_Service)face->sfnt;

    FT_UNUSED( face_index );

    /* Check parameters */
    
    {
      FT_Int  i;


      for ( i = 0; i < num_params; i++ )
      {
        if ( params[i].tag == FT_PARAM_TAG_IGNORE_PREFERRED_FAMILY )
          ignore_preferred_family = TRUE;
        else if ( params[i].tag == FT_PARAM_TAG_IGNORE_PREFERRED_SUBFAMILY )
          ignore_preferred_subfamily = TRUE;
      }
    }

    /* Load tables */

    /* We now support two SFNT-based bitmapped font formats.  They */
    /* are recognized easily as they do not include a `glyf'       */
    /* table.                                                      */
    /*                                                             */
    /* The first format comes from Apple, and uses a table named   */
    /* `bhed' instead of `head' to store the font header (using    */
    /* the same format).  It also doesn't include horizontal and   */
    /* vertical metrics tables (i.e. `hhea' and `vhea' tables are  */
    /* missing).                                                   */
    /*                                                             */
    /* The other format comes from Microsoft, and is used with     */
    /* WinCE/PocketPC.  It looks like a standard TTF, except that  */
    /* it doesn't contain outlines.                                */
    /*                                                             */

    FT_TRACE2(( "sfnt_load_face: %08p\n\n", face ));

    /* do we have outlines in there? */
#ifdef FT_CONFIG_OPTION_INCREMENTAL
    has_outline   = FT_BOOL( face->root.internal->incremental_interface != 0 ||
                             tt_face_lookup_table( face, TTAG_glyf )    != 0 ||
                             tt_face_lookup_table( face, TTAG_CFF )     != 0 );
#else
    has_outline   = FT_BOOL( tt_face_lookup_table( face, TTAG_glyf ) != 0 ||
                             tt_face_lookup_table( face, TTAG_CFF )  != 0 );
#endif

    is_apple_sbit = 0;

    /* if this font doesn't contain outlines, we try to load */
    /* a `bhed' table                                        */
    if ( !has_outline && sfnt->load_bhed )
    {
      LOAD_( bhed );
      is_apple_sbit = FT_BOOL( !error );
    }

    /* load the font header (`head' table) if this isn't an Apple */
    /* sbit font file                                             */
    if ( !is_apple_sbit )
    {
      LOAD_( head );
      if ( error )
        goto Exit;
    }

    if ( face->header.Units_Per_EM == 0 )
    {
      error = SFNT_Err_Invalid_Table;

      goto Exit;
    }

    /* the following tables are often not present in embedded TrueType */
    /* fonts within PDF documents, so don't check for them.            */
    LOAD_( maxp );
    LOAD_( cmap );

    /* the following tables are optional in PCL fonts -- */
    /* don't check for errors                            */
    LOAD_( name );
    LOAD_( post );

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
    psnames_error = error;
#endif

    /* do not load the metrics headers and tables if this is an Apple */
    /* sbit font file                                                 */
    if ( !is_apple_sbit )
    {
      /* load the `hhea' and `hmtx' tables */
      LOADM_( hhea, 0 );
      if ( !error )
      {
        LOADM_( hmtx, 0 );
        if ( error == SFNT_Err_Table_Missing )
        {
          error = SFNT_Err_Hmtx_Table_Missing;

#ifdef FT_CONFIG_OPTION_INCREMENTAL
          /* If this is an incrementally loaded font and there are */
          /* overriding metrics, tolerate a missing `hmtx' table.  */
          if ( face->root.internal->incremental_interface          &&
               face->root.internal->incremental_interface->funcs->
                 get_glyph_metrics                                 )
          {
            face->horizontal.number_Of_HMetrics = 0;
            error = SFNT_Err_Ok;
          }
#endif
        }
      }
      else if ( error == SFNT_Err_Table_Missing )
      {
        /* No `hhea' table necessary for SFNT Mac fonts. */
        if ( face->format_tag == TTAG_true )
        {
          FT_TRACE2(( "This is an SFNT Mac font.\n" ));
          has_outline = 0;
          error = SFNT_Err_Ok;
        }
        else
        {
          error = SFNT_Err_Horiz_Header_Missing;

#ifdef FT_CONFIG_OPTION_INCREMENTAL
          /* If this is an incrementally loaded font and there are */
          /* overriding metrics, tolerate a missing `hhea' table.  */
          if ( face->root.internal->incremental_interface          &&
               face->root.internal->incremental_interface->funcs->
                 get_glyph_metrics                                 )
          {
            face->horizontal.number_Of_HMetrics = 0;
            error = SFNT_Err_Ok;
          }
#endif

        }
      }

      if ( error )
        goto Exit;

      /* try to load the `vhea' and `vmtx' tables */
      LOADM_( hhea, 1 );
      if ( !error )
      {
        LOADM_( hmtx, 1 );
        if ( !error )
          face->vertical_info = 1;
      }

      if ( error && error != SFNT_Err_Table_Missing )
        goto Exit;

      LOAD_( os2 );
      if ( error )
      {
        /* we treat the table as missing if there are any errors */
        face->os2.version = 0xFFFFU;
      }
    }

    /* the optional tables */

    /* embedded bitmap support */
    if ( sfnt->load_eblc )
    {
      LOAD_( eblc );
      if ( error )
      {
        /* a font which contains neither bitmaps nor outlines is */
        /* still valid (although rather useless in most cases);  */
        /* however, you can find such stripped fonts in PDFs     */
        if ( error == SFNT_Err_Table_Missing )
          error = SFNT_Err_Ok;
        else
          goto Exit;
      }
    }

    LOAD_( pclt );
    if ( error )
    {
      if ( error != SFNT_Err_Table_Missing )
        goto Exit;

      face->pclt.Version = 0;
    }

    /* consider the kerning and gasp tables as optional */
    LOAD_( gasp );
    LOAD_( kern );

    face->root.num_glyphs = face->max_profile.numGlyphs;

    /* Bit 8 of the `fsSelection' field in the `OS/2' table denotes  */
    /* a WWS-only font face.  `WWS' stands for `weight', width', and */
    /* `slope', a term used by Microsoft's Windows Presentation      */
    /* Foundation (WPF).  This flag has been introduced in version   */
    /* 1.5 of the OpenType specification (May 2008).                 */

    face->root.family_name = NULL;
    face->root.style_name  = NULL;
    if ( face->os2.version != 0xFFFFU && face->os2.fsSelection & 256 )
    {
      if ( !ignore_preferred_family )
        GET_NAME( PREFERRED_FAMILY, &face->root.family_name );
      if ( !face->root.family_name )
        GET_NAME( FONT_FAMILY, &face->root.family_name );

      if ( !ignore_preferred_subfamily )
        GET_NAME( PREFERRED_SUBFAMILY, &face->root.style_name );
      if ( !face->root.style_name )
        GET_NAME( FONT_SUBFAMILY, &face->root.style_name );
    }
    else
    {
      GET_NAME( WWS_FAMILY, &face->root.family_name );
      if ( !face->root.family_name && !ignore_preferred_family )
        GET_NAME( PREFERRED_FAMILY, &face->root.family_name );
      if ( !face->root.family_name )
        GET_NAME( FONT_FAMILY, &face->root.family_name );

      GET_NAME( WWS_SUBFAMILY, &face->root.style_name );
      if ( !face->root.style_name && !ignore_preferred_subfamily )
        GET_NAME( PREFERRED_SUBFAMILY, &face->root.style_name );
      if ( !face->root.style_name )
        GET_NAME( FONT_SUBFAMILY, &face->root.style_name );
    }

    /* now set up root fields */
    {
      FT_Face  root  = &face->root;
      FT_Long  flags = root->face_flags;


      /*********************************************************************/
      /*                                                                   */
      /* Compute face flags.                                               */
      /*                                                                   */
      if ( has_outline == TRUE )
        flags |= FT_FACE_FLAG_SCALABLE;   /* scalable outlines */

      /* The sfnt driver only supports bitmap fonts natively, thus we */
      /* don't set FT_FACE_FLAG_HINTER.                               */
      flags |= FT_FACE_FLAG_SFNT       |  /* SFNT file format  */
               FT_FACE_FLAG_HORIZONTAL;   /* horizontal data   */

#ifdef TT_CONFIG_OPTION_POSTSCRIPT_NAMES
      if ( psnames_error == SFNT_Err_Ok               &&
           face->postscript.FormatType != 0x00030000L )
        flags |= FT_FACE_FLAG_GLYPH_NAMES;
#endif

      /* fixed width font? */
      if ( face->postscript.isFixedPitch )
        flags |= FT_FACE_FLAG_FIXED_WIDTH;

      /* vertical information? */
      if ( face->vertical_info )
        flags |= FT_FACE_FLAG_VERTICAL;

      /* kerning available ? */
      if ( TT_FACE_HAS_KERNING( face ) )
        flags |= FT_FACE_FLAG_KERNING;

#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
      /* Don't bother to load the tables unless somebody asks for them. */
      /* No need to do work which will (probably) not be used.          */
      if ( tt_face_lookup_table( face, TTAG_glyf ) != 0 &&
           tt_face_lookup_table( face, TTAG_fvar ) != 0 &&
           tt_face_lookup_table( face, TTAG_gvar ) != 0 )
        flags |= FT_FACE_FLAG_MULTIPLE_MASTERS;
#endif

      root->face_flags = flags;

      /*********************************************************************/
      /*                                                                   */
      /* Compute style flags.                                              */
      /*                                                                   */

      flags = 0;
      if ( has_outline == TRUE && face->os2.version != 0xFFFFU )
      {
        /* We have an OS/2 table; use the `fsSelection' field.  Bit 9 */
        /* indicates an oblique font face.  This flag has been        */
        /* introduced in version 1.5 of the OpenType specification.   */

        if ( face->os2.fsSelection & 512 )       /* bit 9 */
          flags |= FT_STYLE_FLAG_ITALIC;
        else if ( face->os2.fsSelection & 1 )    /* bit 0 */
          flags |= FT_STYLE_FLAG_ITALIC;

        if ( face->os2.fsSelection & 32 )        /* bit 5 */
          flags |= FT_STYLE_FLAG_BOLD;
      }
      else
      {
        /* this is an old Mac font, use the header field */

        if ( face->header.Mac_Style & 1 )
          flags |= FT_STYLE_FLAG_BOLD;

        if ( face->header.Mac_Style & 2 )
          flags |= FT_STYLE_FLAG_ITALIC;
      }

      root->style_flags = flags;

      /*********************************************************************/
      /*                                                                   */
      /* Polish the charmaps.                                              */
      /*                                                                   */
      /*   Try to set the charmap encoding according to the platform &     */
      /*   encoding ID of each charmap.                                    */
      /*                                                                   */

      tt_face_build_cmaps( face );  /* ignore errors */


      /* set the encoding fields */
      {
        FT_Int  m;


        for ( m = 0; m < root->num_charmaps; m++ )
        {
          FT_CharMap  charmap = root->charmaps[m];


          charmap->encoding = sfnt_find_encoding( charmap->platform_id,
                                                  charmap->encoding_id );

#if 0
          if ( root->charmap     == NULL &&
               charmap->encoding == FT_ENCODING_UNICODE )
          {
            /* set 'root->charmap' to the first Unicode encoding we find */
            root->charmap = charmap;
          }
#endif
        }
      }

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS

      /*
       *  Now allocate the root array of FT_Bitmap_Size records and
       *  populate them.  Unfortunately, it isn't possible to indicate bit
       *  depths in the FT_Bitmap_Size record.  This is a design error.
       */
      {
        FT_UInt  i, count;


#ifndef FT_CONFIG_OPTION_OLD_INTERNALS
        count = face->sbit_num_strikes;
#else
        count = (FT_UInt)face->num_sbit_strikes;
#endif

        if ( count > 0 )
        {
          FT_Memory        memory   = face->root.stream->memory;
          FT_UShort        em_size  = face->header.Units_Per_EM;
          FT_Short         avgwidth = face->os2.xAvgCharWidth;
          FT_Size_Metrics  metrics;


          if ( em_size == 0 || face->os2.version == 0xFFFFU )
          {
            avgwidth = 0;
            em_size = 1;
          }

          if ( FT_NEW_ARRAY( root->available_sizes, count ) )
            goto Exit;

          for ( i = 0; i < count; i++ )
          {
            FT_Bitmap_Size*  bsize = root->available_sizes + i;


            error = sfnt->load_strike_metrics( face, i, &metrics );
            if ( error )
              goto Exit;

            bsize->height = (FT_Short)( metrics.height >> 6 );
            bsize->width = (FT_Short)(
                ( avgwidth * metrics.x_ppem + em_size / 2 ) / em_size );

            bsize->x_ppem = metrics.x_ppem << 6;
            bsize->y_ppem = metrics.y_ppem << 6;

            /* assume 72dpi */
            bsize->size   = metrics.y_ppem << 6;
          }

          root->face_flags     |= FT_FACE_FLAG_FIXED_SIZES;
          root->num_fixed_sizes = (FT_Int)count;
        }
      }

#endif /* TT_CONFIG_OPTION_EMBEDDED_BITMAPS */

      /* a font with no bitmaps and no outlines is scalable; */
      /* it has only empty glyphs then                       */
      if ( !FT_HAS_FIXED_SIZES( root ) && !FT_IS_SCALABLE( root ) )
        root->face_flags |= FT_FACE_FLAG_SCALABLE;


      /*********************************************************************/
      /*                                                                   */
      /*  Set up metrics.                                                  */
      /*                                                                   */
      if ( FT_IS_SCALABLE( root ) )
      {
        /* XXX What about if outline header is missing */
        /*     (e.g. sfnt wrapped bitmap)?             */
        root->bbox.xMin    = face->header.xMin;
        root->bbox.yMin    = face->header.yMin;
        root->bbox.xMax    = face->header.xMax;
        root->bbox.yMax    = face->header.yMax;
        root->units_per_EM = face->header.Units_Per_EM;


        /* XXX: Computing the ascender/descender/height is very different */
        /*      from what the specification tells you.  Apparently, we    */
        /*      must be careful because                                   */
        /*                                                                */
        /*      - not all fonts have an OS/2 table; in this case, we take */
        /*        the values in the horizontal header.  However, these    */
        /*        values very often are not reliable.                     */
        /*                                                                */
        /*      - otherwise, the correct typographic values are in the    */
        /*        sTypoAscender, sTypoDescender & sTypoLineGap fields.    */
        /*                                                                */
        /*        However, certain fonts have these fields set to 0.      */
        /*        Rather, they have usWinAscent & usWinDescent correctly  */
        /*        set (but with different values).                        */
        /*                                                                */
        /*      As an example, Arial Narrow is implemented through four   */
        /*      files ARIALN.TTF, ARIALNI.TTF, ARIALNB.TTF & ARIALNBI.TTF */
        /*                                                                */
        /*      Strangely, all fonts have the same values in their        */
        /*      sTypoXXX fields, except ARIALNB which sets them to 0.     */
        /*                                                                */
        /*      On the other hand, they all have different                */
        /*      usWinAscent/Descent values -- as a conclusion, the OS/2   */
        /*      table cannot be used to compute the text height reliably! */
        /*                                                                */

        /* The ascender/descender/height are computed from the OS/2 table */
        /* when found.  Otherwise, they're taken from the horizontal      */
        /* header.                                                        */
        /*                                                                */

        root->ascender  = face->horizontal.Ascender;
        root->descender = face->horizontal.Descender;

        root->height    = (FT_Short)( root->ascender - root->descender +
                                      face->horizontal.Line_Gap );

#if 0
        /* if the line_gap is 0, we add an extra 15% to the text height --  */
        /* this computation is based on various versions of Times New Roman */
        if ( face->horizontal.Line_Gap == 0 )
          root->height = (FT_Short)( ( root->height * 115 + 50 ) / 100 );
#endif /* 0 */

#if 0
        /* some fonts have the OS/2 "sTypoAscender", "sTypoDescender" & */
        /* "sTypoLineGap" fields set to 0, like ARIALNB.TTF             */
        if ( face->os2.version != 0xFFFFU && root->ascender )
        {
          FT_Int  height;


          root->ascender  =  face->os2.sTypoAscender;
          root->descender = -face->os2.sTypoDescender;

          height = root->ascender + root->descender + face->os2.sTypoLineGap;
          if ( height > root->height )
            root->height = height;
        }
#endif /* 0 */

        root->max_advance_width  = face->horizontal.advance_Width_Max;
        root->max_advance_height = (FT_Short)( face->vertical_info
                                     ? face->vertical.advance_Height_Max
                                     : root->height );

        /* See http://www.microsoft.com/OpenType/OTSpec/post.htm -- */
        /* Adjust underline position from top edge to centre of     */
        /* stroke to convert TrueType meaning to FreeType meaning.  */
        root->underline_position  = face->postscript.underlinePosition -
                                    face->postscript.underlineThickness / 2;
        root->underline_thickness = face->postscript.underlineThickness;
      }

    }

  Exit:
    FT_TRACE2(( "sfnt_load_face: done\n" ));

    return error;
  }


#undef LOAD_
#undef LOADM_
#undef GET_NAME


  FT_LOCAL_DEF( void )
  sfnt_done_face( TT_Face  face )
  {
    FT_Memory     memory;
    SFNT_Service  sfnt;


    if ( !face )
      return;

    memory = face->root.memory;
    sfnt   = (SFNT_Service)face->sfnt;

    if ( sfnt )
    {
      /* destroy the postscript names table if it is loaded */
      if ( sfnt->free_psnames )
        sfnt->free_psnames( face );

      /* destroy the embedded bitmaps table if it is loaded */
      if ( sfnt->free_eblc )
        sfnt->free_eblc( face );
    }

#ifdef TT_CONFIG_OPTION_BDF
    /* freeing the embedded BDF properties */
    tt_face_free_bdf_props( face );
#endif

    /* freeing the kerning table */
    tt_face_done_kern( face );

    /* freeing the collection table */
    FT_FREE( face->ttc_header.offsets );
    face->ttc_header.count = 0;

    /* freeing table directory */
    FT_FREE( face->dir_tables );
    face->num_tables = 0;

    {
      FT_Stream  stream = FT_FACE_STREAM( face );


      /* simply release the 'cmap' table frame */
      FT_FRAME_RELEASE( face->cmap_table );
      face->cmap_size = 0;
    }

    /* freeing the horizontal metrics */
#ifndef FT_CONFIG_OPTION_OLD_INTERNALS
    {
      FT_Stream  stream = FT_FACE_STREAM( face );


      FT_FRAME_RELEASE( face->horz_metrics );
      FT_FRAME_RELEASE( face->vert_metrics );
      face->horz_metrics_size = 0;
      face->vert_metrics_size = 0;
    }
#else
    FT_FREE( face->horizontal.long_metrics );
    FT_FREE( face->horizontal.short_metrics );
#endif

    /* freeing the vertical ones, if any */
    if ( face->vertical_info )
    {
      FT_FREE( face->vertical.long_metrics  );
      FT_FREE( face->vertical.short_metrics );
      face->vertical_info = 0;
    }

    /* freeing the gasp table */
    FT_FREE( face->gasp.gaspRanges );
    face->gasp.numRanges = 0;

    /* freeing the name table */
    if ( sfnt )
      sfnt->free_name( face );

    /* freeing family and style name */
    FT_FREE( face->root.family_name );
    FT_FREE( face->root.style_name );

    /* freeing sbit size table */
    FT_FREE( face->root.available_sizes );
    face->root.num_fixed_sizes = 0;

    FT_FREE( face->postscript_name );

    face->sfnt = 0;
  }


/* END */
