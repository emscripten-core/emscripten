/***************************************************************************/
/*                                                                         */
/*  t42objs.c                                                              */
/*                                                                         */
/*    Type 42 objects manager (body).                                      */
/*                                                                         */
/*  Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009               */
/*  by Roberto Alameda.                                                    */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "t42objs.h"
#include "t42parse.h"
#include "t42error.h"
#include FT_INTERNAL_DEBUG_H
#include FT_LIST_H
#include FT_TRUETYPE_IDS_H 


#undef  FT_COMPONENT
#define FT_COMPONENT  trace_t42


  static FT_Error
  T42_Open_Face( T42_Face  face )
  {
    T42_LoaderRec  loader;
    T42_Parser     parser;
    T1_Font        type1 = &face->type1;
    FT_Memory      memory = face->root.memory;
    FT_Error       error;

    PSAux_Service  psaux  = (PSAux_Service)face->psaux;


    t42_loader_init( &loader, face );

    parser = &loader.parser;

    if ( FT_ALLOC( face->ttf_data, 12 ) )
      goto Exit;

    error = t42_parser_init( parser,
                             face->root.stream,
                             memory,
                             psaux);
    if ( error )
      goto Exit;

    error = t42_parse_dict( face, &loader,
                            parser->base_dict, parser->base_len );
    if ( error )
      goto Exit;

    if ( type1->font_type != 42 )
    {
      error = T42_Err_Unknown_File_Format;
      goto Exit;
    }

    /* now, propagate the charstrings and glyphnames tables */
    /* to the Type1 data                                    */
    type1->num_glyphs = loader.num_glyphs;

    if ( !loader.charstrings.init )
    {
      FT_ERROR(( "T42_Open_Face: no charstrings array in face\n" ));
      error = T42_Err_Invalid_File_Format;
    }

    loader.charstrings.init  = 0;
    type1->charstrings_block = loader.charstrings.block;
    type1->charstrings       = loader.charstrings.elements;
    type1->charstrings_len   = loader.charstrings.lengths;

    /* we copy the glyph names `block' and `elements' fields; */
    /* the `lengths' field must be released later             */
    type1->glyph_names_block    = loader.glyph_names.block;
    type1->glyph_names          = (FT_String**)loader.glyph_names.elements;
    loader.glyph_names.block    = 0;
    loader.glyph_names.elements = 0;

    /* we must now build type1.encoding when we have a custom array */
    if ( type1->encoding_type == T1_ENCODING_TYPE_ARRAY )
    {
      FT_Int    charcode, idx, min_char, max_char;
      FT_Byte*  char_name;
      FT_Byte*  glyph_name;


      /* OK, we do the following: for each element in the encoding   */
      /* table, look up the index of the glyph having the same name  */
      /* as defined in the CharStrings array.                        */
      /* The index is then stored in type1.encoding.char_index, and  */
      /* the name in type1.encoding.char_name                        */

      min_char = 0;
      max_char = 0;

      charcode = 0;
      for ( ; charcode < loader.encoding_table.max_elems; charcode++ )
      {
        type1->encoding.char_index[charcode] = 0;
        type1->encoding.char_name [charcode] = (char *)".notdef";

        char_name = loader.encoding_table.elements[charcode];
        if ( char_name )
          for ( idx = 0; idx < type1->num_glyphs; idx++ )
          {
            glyph_name = (FT_Byte*)type1->glyph_names[idx];
            if ( ft_strcmp( (const char*)char_name,
                            (const char*)glyph_name ) == 0 )
            {
              type1->encoding.char_index[charcode] = (FT_UShort)idx;
              type1->encoding.char_name [charcode] = (char*)glyph_name;

              /* Change min/max encoded char only if glyph name is */
              /* not /.notdef                                      */
              if ( ft_strcmp( (const char*)".notdef",
                              (const char*)glyph_name ) != 0 )
              {
                if ( charcode < min_char )
                  min_char = charcode;
                if ( charcode >= max_char )
                  max_char = charcode + 1;
              }
              break;
            }
          }
      }

      type1->encoding.code_first = min_char;
      type1->encoding.code_last  = max_char;
      type1->encoding.num_chars  = loader.num_chars;
    }

  Exit:
    t42_loader_done( &loader );
    return error;
  }


  /***************** Driver Functions *************/


  FT_LOCAL_DEF( FT_Error )
  T42_Face_Init( FT_Stream      stream,
                 T42_Face       face,
                 FT_Int         face_index,
                 FT_Int         num_params,
                 FT_Parameter*  params )
  {
    FT_Error            error;
    FT_Service_PsCMaps  psnames;
    PSAux_Service       psaux;
    FT_Face             root  = (FT_Face)&face->root;
    T1_Font             type1 = &face->type1;
    PS_FontInfo         info  = &type1->font_info;

    FT_UNUSED( num_params );
    FT_UNUSED( params );
    FT_UNUSED( face_index );
    FT_UNUSED( stream );


    face->ttf_face       = NULL;
    face->root.num_faces = 1;

    FT_FACE_FIND_GLOBAL_SERVICE( face, psnames, POSTSCRIPT_CMAPS );
    face->psnames = psnames;

    face->psaux = FT_Get_Module_Interface( FT_FACE_LIBRARY( face ),
                                           "psaux" );
    psaux = (PSAux_Service)face->psaux;

    /* open the tokenizer, this will also check the font format */
    error = T42_Open_Face( face );
    if ( error )
      goto Exit;

    /* if we just wanted to check the format, leave successfully now */
    if ( face_index < 0 )
      goto Exit;

    /* check the face index */
    if ( face_index > 0 )
    {
      FT_ERROR(( "T42_Face_Init: invalid face index\n" ));
      error = T42_Err_Invalid_Argument;
      goto Exit;
    }

    /* Now load the font program into the face object */

    /* Init the face object fields */
    /* Now set up root face fields */

    root->num_glyphs   = type1->num_glyphs;
    root->num_charmaps = 0;
    root->face_index   = 0;

    root->face_flags = FT_FACE_FLAG_SCALABLE    |
                       FT_FACE_FLAG_HORIZONTAL  |
                       FT_FACE_FLAG_GLYPH_NAMES;

    if ( info->is_fixed_pitch )
      root->face_flags |= FT_FACE_FLAG_FIXED_WIDTH;

    /* We only set this flag if we have the patented bytecode interpreter. */
    /* There are no known `tricky' Type42 fonts that could be loaded with  */
    /* the unpatented interpreter.                                         */
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    root->face_flags |= FT_FACE_FLAG_HINTER;
#endif

    /* XXX: TODO -- add kerning with .afm support */

    /* get style name -- be careful, some broken fonts only */
    /* have a `/FontName' dictionary entry!                 */
    root->family_name = info->family_name;
    /* assume "Regular" style if we don't know better */
    root->style_name = (char *)"Regular";
    if ( root->family_name )
    {
      char*  full   = info->full_name;
      char*  family = root->family_name;


      if ( full )
      {
        while ( *full )
        {
          if ( *full == *family )
          {
            family++;
            full++;
          }
          else
          {
            if ( *full == ' ' || *full == '-' )
              full++;
            else if ( *family == ' ' || *family == '-' )
              family++;
            else
            {
              if ( !*family )
                root->style_name = full;
              break;
            }
          }
        }
      }
    }
    else
    {
      /* do we have a `/FontName'? */
      if ( type1->font_name )
        root->family_name = type1->font_name;
    }

    /* no embedded bitmap support */
    root->num_fixed_sizes = 0;
    root->available_sizes = 0;

    /* Load the TTF font embedded in the T42 font */
    {
      FT_Open_Args  args;


      args.flags       = FT_OPEN_MEMORY;
      args.memory_base = face->ttf_data;
      args.memory_size = face->ttf_size;

      if ( num_params )
      {
        args.flags     |= FT_OPEN_PARAMS;
        args.num_params = num_params;
        args.params     = params;
      }

      error = FT_Open_Face( FT_FACE_LIBRARY( face ),
                            &args, 0, &face->ttf_face );
    }

    if ( error )
      goto Exit;

    FT_Done_Size( face->ttf_face->size );

    /* Ignore info in FontInfo dictionary and use the info from the  */
    /* loaded TTF font.  The PostScript interpreter also ignores it. */
    root->bbox         = face->ttf_face->bbox;
    root->units_per_EM = face->ttf_face->units_per_EM;

    root->ascender  = face->ttf_face->ascender;
    root->descender = face->ttf_face->descender;
    root->height    = face->ttf_face->height;

    root->max_advance_width  = face->ttf_face->max_advance_width;
    root->max_advance_height = face->ttf_face->max_advance_height;

    root->underline_position  = (FT_Short)info->underline_position;
    root->underline_thickness = (FT_Short)info->underline_thickness;

    /* compute style flags */
    root->style_flags = 0;
    if ( info->italic_angle )
      root->style_flags |= FT_STYLE_FLAG_ITALIC;

    if ( face->ttf_face->style_flags & FT_STYLE_FLAG_BOLD )
      root->style_flags |= FT_STYLE_FLAG_BOLD;

    if ( face->ttf_face->face_flags & FT_FACE_FLAG_VERTICAL )
      root->face_flags |= FT_FACE_FLAG_VERTICAL;

    {
      if ( psnames && psaux )
      {
        FT_CharMapRec    charmap;
        T1_CMap_Classes  cmap_classes = psaux->t1_cmap_classes;
        FT_CMap_Class    clazz;


        charmap.face = root;

        /* first of all, try to synthesize a Unicode charmap */
        charmap.platform_id = TT_PLATFORM_MICROSOFT;
        charmap.encoding_id = TT_MS_ID_UNICODE_CS;
        charmap.encoding    = FT_ENCODING_UNICODE;

        error = FT_CMap_New( cmap_classes->unicode, NULL, &charmap, NULL );
        if ( error && FT_Err_No_Unicode_Glyph_Name != error )
          goto Exit;
        error = FT_Err_Ok;

        /* now, generate an Adobe Standard encoding when appropriate */
        charmap.platform_id = TT_PLATFORM_ADOBE;
        clazz               = NULL;

        switch ( type1->encoding_type )
        {
        case T1_ENCODING_TYPE_STANDARD:
          charmap.encoding    = FT_ENCODING_ADOBE_STANDARD;
          charmap.encoding_id = TT_ADOBE_ID_STANDARD;
          clazz               = cmap_classes->standard;
          break;

        case T1_ENCODING_TYPE_EXPERT:
          charmap.encoding    = FT_ENCODING_ADOBE_EXPERT;
          charmap.encoding_id = TT_ADOBE_ID_EXPERT;
          clazz               = cmap_classes->expert;
          break;

        case T1_ENCODING_TYPE_ARRAY:
          charmap.encoding    = FT_ENCODING_ADOBE_CUSTOM;
          charmap.encoding_id = TT_ADOBE_ID_CUSTOM;
          clazz               = cmap_classes->custom;
          break;

        case T1_ENCODING_TYPE_ISOLATIN1:
          charmap.encoding    = FT_ENCODING_ADOBE_LATIN_1;
          charmap.encoding_id = TT_ADOBE_ID_LATIN_1;
          clazz               = cmap_classes->unicode;
          break;

        default:
          ;
        }

        if ( clazz )
          error = FT_CMap_New( clazz, NULL, &charmap, NULL );

#if 0
        /* Select default charmap */
        if ( root->num_charmaps )
          root->charmap = root->charmaps[0];
#endif
      }
    }
  Exit:
    return error;
  }


  FT_LOCAL_DEF( void )
  T42_Face_Done( T42_Face  face )
  {
    T1_Font      type1;
    PS_FontInfo  info;
    FT_Memory    memory;


    if ( !face )
      return;

    type1  = &face->type1;
    info   = &type1->font_info;
    memory = face->root.memory;

    /* delete internal ttf face prior to freeing face->ttf_data */
    if ( face->ttf_face )
      FT_Done_Face( face->ttf_face );

    /* release font info strings */
    FT_FREE( info->version );
    FT_FREE( info->notice );
    FT_FREE( info->full_name );
    FT_FREE( info->family_name );
    FT_FREE( info->weight );

    /* release top dictionary */
    FT_FREE( type1->charstrings_len );
    FT_FREE( type1->charstrings );
    FT_FREE( type1->glyph_names );

    FT_FREE( type1->charstrings_block );
    FT_FREE( type1->glyph_names_block );

    FT_FREE( type1->encoding.char_index );
    FT_FREE( type1->encoding.char_name );
    FT_FREE( type1->font_name );

    FT_FREE( face->ttf_data );

#if 0
    /* release afm data if present */
    if ( face->afm_data )
      T1_Done_AFM( memory, (T1_AFM*)face->afm_data );
#endif

    /* release unicode map, if any */
    FT_FREE( face->unicode_map.maps );
    face->unicode_map.num_maps = 0;

    face->root.family_name = 0;
    face->root.style_name  = 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    T42_Driver_Init                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initializes a given Type 42 driver object.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    driver :: A handle to the target driver object.                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  FT_LOCAL_DEF( FT_Error )
  T42_Driver_Init( T42_Driver  driver )
  {
    FT_Module  ttmodule;


    ttmodule = FT_Get_Module( FT_MODULE(driver)->library, "truetype" );
    driver->ttclazz = (FT_Driver_Class)ttmodule->clazz;

    return T42_Err_Ok;
  }


  FT_LOCAL_DEF( void )
  T42_Driver_Done( T42_Driver  driver )
  {
    FT_UNUSED( driver );
  }


  FT_LOCAL_DEF( FT_Error )
  T42_Size_Init( T42_Size  size )
  {
    FT_Face   face = size->root.face;
    T42_Face  t42face = (T42_Face)face;
    FT_Size   ttsize;
    FT_Error  error   = T42_Err_Ok;


    error = FT_New_Size( t42face->ttf_face, &ttsize );
    size->ttsize = ttsize;

    FT_Activate_Size( ttsize );

    return error;
  }


  FT_LOCAL_DEF( FT_Error )
  T42_Size_Request( T42_Size         size,
                    FT_Size_Request  req )
  {
    T42_Face  face = (T42_Face)size->root.face;
    FT_Error  error;


    FT_Activate_Size( size->ttsize );

    error = FT_Request_Size( face->ttf_face, req );
    if ( !error )
      ( (FT_Size)size )->metrics = face->ttf_face->size->metrics;

    return error;
  }


  FT_LOCAL_DEF( FT_Error )
  T42_Size_Select( T42_Size  size,
                   FT_ULong  strike_index )
  {
    T42_Face  face = (T42_Face)size->root.face;
    FT_Error  error;


    FT_Activate_Size( size->ttsize );

    error = FT_Select_Size( face->ttf_face, (FT_Int)strike_index );
    if ( !error )
      ( (FT_Size)size )->metrics = face->ttf_face->size->metrics;

    return error;

  }


  FT_LOCAL_DEF( void )
  T42_Size_Done( T42_Size  size )
  {
    FT_Face      face    = size->root.face;
    T42_Face     t42face = (T42_Face)face;
    FT_ListNode  node;


    node = FT_List_Find( &t42face->ttf_face->sizes_list, size->ttsize );
    if ( node )
    {
      FT_Done_Size( size->ttsize );
      size->ttsize = NULL;
    }
  }


  FT_LOCAL_DEF( FT_Error )
  T42_GlyphSlot_Init( T42_GlyphSlot  slot )
  {
    FT_Face       face    = slot->root.face;
    T42_Face      t42face = (T42_Face)face;
    FT_GlyphSlot  ttslot;
    FT_Error      error   = T42_Err_Ok;


    if ( face->glyph == NULL )
    {
      /* First glyph slot for this face */
      slot->ttslot = t42face->ttf_face->glyph;
    }
    else
    {
      error = FT_New_GlyphSlot( t42face->ttf_face, &ttslot );
      slot->ttslot = ttslot;
    }

    return error;
  }


  FT_LOCAL_DEF( void )
  T42_GlyphSlot_Done( T42_GlyphSlot slot )
  {
    FT_Done_GlyphSlot( slot->ttslot );
  }


  static void
  t42_glyphslot_clear( FT_GlyphSlot  slot )
  {
    /* free bitmap if needed */
    ft_glyphslot_free_bitmap( slot );

    /* clear all public fields in the glyph slot */
    FT_ZERO( &slot->metrics );
    FT_ZERO( &slot->outline );
    FT_ZERO( &slot->bitmap );

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
  }


  FT_LOCAL_DEF( FT_Error )
  T42_GlyphSlot_Load( FT_GlyphSlot  glyph,
                      FT_Size       size,
                      FT_UInt       glyph_index,
                      FT_Int32      load_flags )
  {
    FT_Error         error;
    T42_GlyphSlot    t42slot = (T42_GlyphSlot)glyph;
    T42_Size         t42size = (T42_Size)size;
    FT_Driver_Class  ttclazz = ((T42_Driver)glyph->face->driver)->ttclazz;


    t42_glyphslot_clear( t42slot->ttslot );
    error = ttclazz->load_glyph( t42slot->ttslot,
                                 t42size->ttsize,
                                 glyph_index,
                                 load_flags | FT_LOAD_NO_BITMAP );

    if ( !error )
    {
      glyph->metrics = t42slot->ttslot->metrics;

      glyph->linearHoriAdvance = t42slot->ttslot->linearHoriAdvance;
      glyph->linearVertAdvance = t42slot->ttslot->linearVertAdvance;

      glyph->format  = t42slot->ttslot->format;
      glyph->outline = t42slot->ttslot->outline;

      glyph->bitmap      = t42slot->ttslot->bitmap;
      glyph->bitmap_left = t42slot->ttslot->bitmap_left;
      glyph->bitmap_top  = t42slot->ttslot->bitmap_top;

      glyph->num_subglyphs = t42slot->ttslot->num_subglyphs;
      glyph->subglyphs     = t42slot->ttslot->subglyphs;

      glyph->control_data  = t42slot->ttslot->control_data;
      glyph->control_len   = t42slot->ttslot->control_len;
    }

    return error;
  }


/* END */
