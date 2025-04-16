/***************************************************************************/
/*                                                                         */
/*  gxvprop.c                                                              */
/*                                                                         */
/*    TrueTypeGX/AAT prop table validation (body).                         */
/*                                                                         */
/*  Copyright 2004, 2005 by suzuki toshiya, Masatake YAMATO, Red Hat K.K., */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/* gxvalid is derived from both gxlayout module and otvalid module.        */
/* Development of gxlayout is supported by the Information-technology      */
/* Promotion Agency(IPA), Japan.                                           */
/*                                                                         */
/***************************************************************************/


#include "gxvalid.h"
#include "gxvcommn.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvprop


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      Data and Types                           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define GXV_PROP_HEADER_SIZE  ( 4 + 2 + 2 )
#define GXV_PROP_SIZE_MIN     GXV_PROP_HEADER_SIZE

  typedef struct  GXV_prop_DataRec_
  {
    FT_Fixed  version;

  } GXV_prop_DataRec, *GXV_prop_Data;

#define GXV_PROP_DATA( field )  GXV_TABLE_DATA( prop, field )

#define GXV_PROP_FLOATER                      0x8000U
#define GXV_PROP_USE_COMPLEMENTARY_BRACKET    0x1000U
#define GXV_PROP_COMPLEMENTARY_BRACKET_OFFSET 0x0F00U
#define GXV_PROP_ATTACHING_TO_RIGHT           0x0080U
#define GXV_PROP_RESERVED                     0x0060U
#define GXV_PROP_DIRECTIONALITY_CLASS         0x001FU


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      UTILITY FUNCTIONS                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  gxv_prop_zero_advance_validate( FT_UShort      gid,
                                  GXV_Validator  valid )
  {
    FT_Face       face;
    FT_Error      error;
    FT_GlyphSlot  glyph;


    GXV_NAME_ENTER( "zero advance" );

    face = valid->face;

    error = FT_Load_Glyph( face,
                           gid,
                           FT_LOAD_IGNORE_TRANSFORM );
    if ( error )
      FT_INVALID_GLYPH_ID;

    glyph = face->glyph;

    if ( glyph->advance.x != (FT_Pos)0 ||
         glyph->advance.y != (FT_Pos)0 )
      FT_INVALID_DATA;

    GXV_EXIT;
  }


  /* Pass 0 as GLYPH to check the default property */
  static void
  gxv_prop_property_validate( FT_UShort      property,
                              FT_UShort      glyph,
                              GXV_Validator  valid )
  {
    if ( glyph != 0 && ( property & GXV_PROP_FLOATER ) )
      gxv_prop_zero_advance_validate( glyph, valid );

    if ( property & GXV_PROP_USE_COMPLEMENTARY_BRACKET )
    {
      FT_UShort  offset;
      char       complement;


      offset = (FT_UShort)( property & GXV_PROP_COMPLEMENTARY_BRACKET_OFFSET );
      if ( offset == 0 )
        FT_INVALID_DATA;

      complement = (char)( offset >> 8 );
      if ( complement & 0x08 )
      {
        /* Top bit is set: negative */

        /* Calculate the absolute offset */
        complement = (char)( ( complement & 0x07 ) + 1 );

        /* The gid for complement must be greater than 0 */
        if ( glyph <= complement )
          FT_INVALID_DATA;
      }
      else
      {
        /* The gid for complement must be the face. */
        gxv_glyphid_validate( (FT_UShort)( glyph + complement ), valid );
      }
    }
    else
    {
      if ( property & GXV_PROP_COMPLEMENTARY_BRACKET_OFFSET )
        GXV_TRACE(( "glyph %d cannot have complementary bracketing\n",
                    glyph ));
    }

    /* this is introduced in version 2.0 */
    if ( property & GXV_PROP_ATTACHING_TO_RIGHT )
    {
      if ( GXV_PROP_DATA( version ) == 0x00010000UL )
        FT_INVALID_DATA;
    }

    if ( property & GXV_PROP_RESERVED )
      FT_INVALID_DATA;

    if ( ( property & GXV_PROP_DIRECTIONALITY_CLASS ) > 11 )
    {
      /* TODO: Too restricted. Use the validation level. */
      if ( GXV_PROP_DATA( version ) == 0x00010000UL ||
           GXV_PROP_DATA( version ) == 0x00020000UL )
        FT_INVALID_DATA;
    }
  }


  static void
  gxv_prop_LookupValue_validate( FT_UShort            glyph,
                                 GXV_LookupValueCPtr  value_p,
                                 GXV_Validator        valid )
  {
    gxv_prop_property_validate( value_p->u, glyph, valid );
  }


  /*
    +===============+ --------+
    | lookup header |         |
    +===============+         |
    | BinSrchHeader |         |
    +===============+         |
    | lastGlyph[0]  |         |
    +---------------+         |
    | firstGlyph[0] |         |    head of lookup table
    +---------------+         |             +
    | offset[0]     |    ->   |          offset            [byte]
    +===============+         |             +
    | lastGlyph[1]  |         | (glyphID - firstGlyph) * 2 [byte]
    +---------------+         |
    | firstGlyph[1] |         |
    +---------------+         |
    | offset[1]     |         |
    +===============+         |
                              |
     ...                      |
                              |
    16bit value array         |
    +===============+         |
    |     value     | <-------+
    ...
  */

  static GXV_LookupValueDesc
  gxv_prop_LookupFmt4_transit( FT_UShort            relative_gindex,
                               GXV_LookupValueCPtr  base_value_p,
                               FT_Bytes             lookuptbl_limit,
                               GXV_Validator        valid )
  {
    FT_Bytes             p;
    FT_Bytes             limit;
    FT_UShort            offset;
    GXV_LookupValueDesc  value;

    /* XXX: check range? */
    offset = (FT_UShort)( base_value_p->u +
                          relative_gindex * sizeof( FT_UShort ) );
    p      = valid->lookuptbl_head + offset;
    limit  = lookuptbl_limit;

    GXV_LIMIT_CHECK ( 2 );
    value.u = FT_NEXT_USHORT( p );

    return value;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         prop TABLE                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_LOCAL_DEF( void )
  gxv_prop_validate( FT_Bytes      table,
                     FT_Face       face,
                     FT_Validator  ftvalid )
  {
    FT_Bytes          p     = table;
    FT_Bytes          limit = 0;
    GXV_ValidatorRec  validrec;
    GXV_Validator     valid = &validrec;

    GXV_prop_DataRec  proprec;
    GXV_prop_Data     prop = &proprec;

    FT_Fixed          version;
    FT_UShort         format;
    FT_UShort         defaultProp;


    valid->root       = ftvalid;
    valid->table_data = prop;
    valid->face       = face;

    FT_TRACE3(( "validating `prop' table\n" ));
    GXV_INIT;

    GXV_LIMIT_CHECK( 4 + 2 + 2 );
    version     = FT_NEXT_ULONG( p );
    format      = FT_NEXT_USHORT( p );
    defaultProp = FT_NEXT_USHORT( p );

    /* only versions 1.0, 2.0, 3.0 are defined (1996) */
    if ( version != 0x00010000UL &&
         version != 0x00020000UL &&
         version != 0x00030000UL )
      FT_INVALID_FORMAT;


    /* only formats 0x0000, 0x0001 are defined (1996) */
    if ( format > 1 )
      FT_INVALID_FORMAT;

    gxv_prop_property_validate( defaultProp, 0, valid );

    if ( format == 0 )
    {
      FT_TRACE3(( "(format 0, no per-glyph properties, "
                  "remaining %d bytes are skipped)", limit - p ));
      goto Exit;
    }

    /* format == 1 */
    GXV_PROP_DATA( version ) = version;

    valid->lookupval_sign   = GXV_LOOKUPVALUE_UNSIGNED;
    valid->lookupval_func   = gxv_prop_LookupValue_validate;
    valid->lookupfmt4_trans = gxv_prop_LookupFmt4_transit;

    gxv_LookupTable_validate( p, limit, valid );

  Exit:
    FT_TRACE4(( "\n" ));
  }


/* END */
