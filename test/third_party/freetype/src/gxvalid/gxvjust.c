/***************************************************************************/
/*                                                                         */
/*  gxvjust.c                                                              */
/*                                                                         */
/*    TrueTypeGX/AAT just table validation (body).                         */
/*                                                                         */
/*  Copyright 2005 by suzuki toshiya, Masatake YAMATO, Red Hat K.K.,       */
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

#include FT_SFNT_NAMES_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvjust

  /*
   * referred `just' table format specification:
   * http://developer.apple.com/fonts/TTRefMan/RM06/Chap6just.html
   * last updated 2000.
   * ----------------------------------------------
   * [JUST HEADER]: GXV_JUST_HEADER_SIZE
   * version     (fixed:  32bit) = 0x00010000
   * format      (uint16: 16bit) = 0 is only defined (2000)
   * horizOffset (uint16: 16bit)
   * vertOffset  (uint16: 16bit)
   * ----------------------------------------------
   */

  typedef struct  GXV_just_DataRec_
  {
    FT_UShort  wdc_offset_max;
    FT_UShort  wdc_offset_min;
    FT_UShort  pc_offset_max;
    FT_UShort  pc_offset_min;

  } GXV_just_DataRec, *GXV_just_Data;


#define  GXV_JUST_DATA( a )  GXV_TABLE_DATA( just, a )


  static void
  gxv_just_wdp_entry_validate( FT_Bytes       table,
                               FT_Bytes       limit,
                               GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_ULong   justClass;
    FT_Fixed   beforeGrowLimit;
    FT_Fixed   beforeShrinkGrowLimit;
    FT_Fixed   afterGrowLimit;
    FT_Fixed   afterShrinkGrowLimit;
    FT_UShort  growFlags;
    FT_UShort  shrinkFlags;


    GXV_LIMIT_CHECK( 4 + 4 + 4 + 4 + 4 + 2 + 2 );
    justClass             = FT_NEXT_ULONG( p );
    beforeGrowLimit       = FT_NEXT_ULONG( p );
    beforeShrinkGrowLimit = FT_NEXT_ULONG( p );
    afterGrowLimit        = FT_NEXT_ULONG( p );
    afterShrinkGrowLimit  = FT_NEXT_ULONG( p );
    growFlags             = FT_NEXT_USHORT( p );
    shrinkFlags           = FT_NEXT_USHORT( p );

    /* TODO: decode flags for human readability */

    valid->subtable_length = p - table;
  }


  static void
  gxv_just_wdc_entry_validate( FT_Bytes       table,
                               FT_Bytes       limit,
                               GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    FT_ULong  count, i;


    GXV_LIMIT_CHECK( 4 );
    count = FT_NEXT_ULONG( p );
    for ( i = 0; i < count; i++ )
    {
      GXV_TRACE(( "validating wdc pair %d/%d\n", i + 1, count ));
      gxv_just_wdp_entry_validate( p, limit, valid );
      p += valid->subtable_length;
    }

    valid->subtable_length = p - table;
  }


  static void
  gxv_just_widthDeltaClusters_validate( FT_Bytes       table,
                                        FT_Bytes       limit,
                                        GXV_Validator  valid )
  {
    FT_Bytes  p         = table ;
    FT_Bytes  wdc_end   = table + GXV_JUST_DATA( wdc_offset_max );
    FT_UInt   i;


    GXV_NAME_ENTER( "just justDeltaClusters" );

    if ( limit <= wdc_end )
      FT_INVALID_OFFSET;

    for ( i = 0; p <= wdc_end; i++ )
    {
      gxv_just_wdc_entry_validate( p, limit, valid );
      p += valid->subtable_length;
    }

    valid->subtable_length = p - table;

    GXV_EXIT;
  }


  static void
  gxv_just_actSubrecord_type0_validate( FT_Bytes       table,
                                        FT_Bytes       limit,
                                        GXV_Validator  valid )
  {
    FT_Bytes   p = table;

    FT_Fixed   lowerLimit;
    FT_Fixed   upperLimit;

    FT_UShort  order;
    FT_UShort  decomposedCount;

    FT_UInt    i;


    GXV_LIMIT_CHECK( 4 + 4 + 2 + 2 );
    lowerLimit      = FT_NEXT_ULONG( p );
    upperLimit      = FT_NEXT_ULONG( p );
    order           = FT_NEXT_USHORT( p );
    decomposedCount = FT_NEXT_USHORT( p );

    for ( i = 0; i < decomposedCount; i++ )
    {
      FT_UShort glyphs;


      GXV_LIMIT_CHECK( 2 );
      glyphs = FT_NEXT_USHORT( p );
    }

    valid->subtable_length = p - table;
  }


  static void
  gxv_just_actSubrecord_type1_validate( FT_Bytes       table,
                                        FT_Bytes       limit,
                                        GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  addGlyph;


    GXV_LIMIT_CHECK( 2 );
    addGlyph = FT_NEXT_USHORT( p );

    valid->subtable_length = p - table;
  }


  static void
  gxv_just_actSubrecord_type2_validate( FT_Bytes       table,
                                        FT_Bytes       limit,
                                        GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_Fixed   substThreshhold; /* Apple misspelled "Threshhold" */
    FT_UShort  addGlyph;
    FT_UShort  substGlyph;


    GXV_LIMIT_CHECK( 4 + 2 + 2 );
    substThreshhold = FT_NEXT_ULONG( p );
    addGlyph        = FT_NEXT_USHORT( p );
    substGlyph      = FT_NEXT_USHORT( p );

    valid->subtable_length = p - table;
  }


  static void
  gxv_just_actSubrecord_type4_validate( FT_Bytes       table,
                                        FT_Bytes       limit,
                                        GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    FT_ULong  variantsAxis;
    FT_Fixed  minimumLimit;
    FT_Fixed  noStretchValue;
    FT_Fixed  maximumLimit;


    GXV_LIMIT_CHECK( 4 + 4 + 4 + 4 );
    variantsAxis   = FT_NEXT_ULONG( p );
    minimumLimit   = FT_NEXT_ULONG( p );
    noStretchValue = FT_NEXT_ULONG( p );
    maximumLimit   = FT_NEXT_ULONG( p );

    valid->subtable_length = p - table;
  }


  static void
  gxv_just_actSubrecord_type5_validate( FT_Bytes       table,
                                        FT_Bytes       limit,
                                        GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  flags;
    FT_UShort  glyph;


    GXV_LIMIT_CHECK( 2 + 2 );
    flags = FT_NEXT_USHORT( p );
    glyph = FT_NEXT_USHORT( p );

    valid->subtable_length = p - table;
  }


  /* parse single actSubrecord */
  static void
  gxv_just_actSubrecord_validate( FT_Bytes       table,
                                  FT_Bytes       limit,
                                  GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  actionClass;
    FT_UShort  actionType;
    FT_ULong   actionLength;


    GXV_NAME_ENTER( "just actSubrecord" );

    GXV_LIMIT_CHECK( 2 + 2 + 4 );
    actionClass  = FT_NEXT_USHORT( p );
    actionType   = FT_NEXT_USHORT( p );
    actionLength = FT_NEXT_ULONG( p );

    if ( actionType == 0 )
      gxv_just_actSubrecord_type0_validate( p, limit, valid );
    else if ( actionType == 1 )
      gxv_just_actSubrecord_type1_validate( p, limit, valid );
    else if ( actionType == 2 )
      gxv_just_actSubrecord_type2_validate( p, limit, valid );
    else if ( actionType == 3 )
      ;                         /* Stretch glyph action: no actionData */
    else if ( actionType == 4 )
      gxv_just_actSubrecord_type4_validate( p, limit, valid );
    else if ( actionType == 5 )
      gxv_just_actSubrecord_type5_validate( p, limit, valid );
    else
      FT_INVALID_DATA;

    valid->subtable_length = actionLength;

    GXV_EXIT;
  }


  static void
  gxv_just_pcActionRecord_validate( FT_Bytes       table,
                                    FT_Bytes       limit,
                                    GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    FT_ULong  actionCount;
    FT_ULong  i;


    GXV_LIMIT_CHECK( 4 );
    actionCount = FT_NEXT_ULONG( p );
    GXV_TRACE(( "actionCount = %d\n", actionCount ));

    for ( i = 0; i < actionCount; i++ )
    {
      gxv_just_actSubrecord_validate( p, limit, valid );
      p += valid->subtable_length;
    }

    valid->subtable_length = p - table;

    GXV_EXIT;
  }


  static void
  gxv_just_pcTable_LookupValue_entry_validate( FT_UShort            glyph,
                                               GXV_LookupValueCPtr  value_p,
                                               GXV_Validator        valid )
  {
    FT_UNUSED( glyph );

    if ( value_p->u > GXV_JUST_DATA( pc_offset_max ) )
      GXV_JUST_DATA( pc_offset_max ) = value_p->u;
    if ( value_p->u < GXV_JUST_DATA( pc_offset_max ) )
      GXV_JUST_DATA( pc_offset_min ) = value_p->u;
  }


  static void
  gxv_just_pcLookupTable_validate( FT_Bytes       table,
                                   FT_Bytes       limit,
                                   GXV_Validator  valid )
  {
    FT_Bytes p = table;


    GXV_NAME_ENTER( "just pcLookupTable" );
    GXV_JUST_DATA( pc_offset_max ) = 0x0000;
    GXV_JUST_DATA( pc_offset_min ) = 0xFFFFU;

    valid->lookupval_sign = GXV_LOOKUPVALUE_UNSIGNED;
    valid->lookupval_func = gxv_just_pcTable_LookupValue_entry_validate;

    gxv_LookupTable_validate( p, limit, valid );

    /* subtable_length is set by gxv_LookupTable_validate() */

    GXV_EXIT;
  }


  static void
  gxv_just_postcompTable_validate( FT_Bytes       table,
                                   FT_Bytes       limit,
                                   GXV_Validator  valid )
  {
    FT_Bytes  p = table;


    GXV_NAME_ENTER( "just postcompTable" );

    gxv_just_pcLookupTable_validate( p, limit, valid );
    p += valid->subtable_length;

    gxv_just_pcActionRecord_validate( p, limit, valid );
    p += valid->subtable_length;

    valid->subtable_length = p - table;

    GXV_EXIT;
  }


  static void
  gxv_just_classTable_entry_validate(
    FT_Byte                         state,
    FT_UShort                       flags,
    GXV_StateTable_GlyphOffsetCPtr  glyphOffset_p,
    FT_Bytes                        table,
    FT_Bytes                        limit,
    GXV_Validator                   valid )
  {
    FT_UShort  setMark;
    FT_UShort  dontAdvance;
    FT_UShort  markClass;
    FT_UShort  currentClass;

    FT_UNUSED( state );
    FT_UNUSED( glyphOffset_p );
    FT_UNUSED( table );
    FT_UNUSED( limit );
    FT_UNUSED( valid );


    setMark      = (FT_UShort)( ( flags >> 15 ) & 1    );
    dontAdvance  = (FT_UShort)( ( flags >> 14 ) & 1    );
    markClass    = (FT_UShort)( ( flags >> 7  ) & 0x7F );
    currentClass = (FT_UShort)(   flags         & 0x7F );

    /* TODO: validate markClass & currentClass */
  }


  static void
  gxv_just_justClassTable_validate ( FT_Bytes       table,
                                     FT_Bytes       limit,
                                     GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  length;
    FT_UShort  coverage;
    FT_ULong   subFeatureFlags;


    GXV_NAME_ENTER( "just justClassTable" );

    GXV_LIMIT_CHECK( 2 + 2 + 4 );
    length          = FT_NEXT_USHORT( p );
    coverage        = FT_NEXT_USHORT( p );
    subFeatureFlags = FT_NEXT_ULONG( p );

    GXV_TRACE(( "  justClassTable: coverage = 0x%04x (%s)",
                coverage,
                ( 0x4000 & coverage ) == 0 ? "ascending" : "descending" ));

    valid->statetable.optdata               = NULL;
    valid->statetable.optdata_load_func     = NULL;
    valid->statetable.subtable_setup_func   = NULL;
    valid->statetable.entry_glyphoffset_fmt = GXV_GLYPHOFFSET_NONE;
    valid->statetable.entry_validate_func   =
      gxv_just_classTable_entry_validate;

    gxv_StateTable_validate( p, table + length, valid );

    /* subtable_length is set by gxv_LookupTable_validate() */

    GXV_EXIT;
  }


  static void
  gxv_just_wdcTable_LookupValue_validate( FT_UShort            glyph,
                                          GXV_LookupValueCPtr  value_p,
                                          GXV_Validator        valid )
  {
    FT_UNUSED( glyph );

    if ( value_p->u > GXV_JUST_DATA( wdc_offset_max ) )
      GXV_JUST_DATA( wdc_offset_max ) = value_p->u;
    if ( value_p->u < GXV_JUST_DATA( wdc_offset_min ) )
      GXV_JUST_DATA( wdc_offset_min ) = value_p->u;
  }


  static void
  gxv_just_justData_lookuptable_validate( FT_Bytes       table,
                                          FT_Bytes       limit,
                                          GXV_Validator  valid )
  {
    FT_Bytes  p = table;


    GXV_JUST_DATA( wdc_offset_max ) = 0x0000;
    GXV_JUST_DATA( wdc_offset_min ) = 0xFFFFU;

    valid->lookupval_sign = GXV_LOOKUPVALUE_UNSIGNED;
    valid->lookupval_func = gxv_just_wdcTable_LookupValue_validate;

    gxv_LookupTable_validate( p, limit, valid );

    /* subtable_length is set by gxv_LookupTable_validate() */

    GXV_EXIT;
  }


  /*
   * gxv_just_justData_validate() parses and validates horizData, vertData.
   */
  static void
  gxv_just_justData_validate( FT_Bytes       table,
                              FT_Bytes       limit,
                              GXV_Validator  valid )
  {
    /*
     * following 3 offsets are measured from the start of `just'
     * (which table points to), not justData
     */
    FT_UShort  justClassTableOffset;
    FT_UShort  wdcTableOffset;
    FT_UShort  pcTableOffset;
    FT_Bytes   p = table;

    GXV_ODTECT( 4, odtect );


    GXV_NAME_ENTER( "just justData" );

    GXV_ODTECT_INIT( odtect );
    GXV_LIMIT_CHECK( 2 + 2 + 2 );
    justClassTableOffset = FT_NEXT_USHORT( p );
    wdcTableOffset       = FT_NEXT_USHORT( p );
    pcTableOffset        = FT_NEXT_USHORT( p );

    GXV_TRACE(( " (justClassTableOffset = 0x%04x)\n", justClassTableOffset ));
    GXV_TRACE(( " (wdcTableOffset = 0x%04x)\n", wdcTableOffset ));
    GXV_TRACE(( " (pcTableOffset = 0x%04x)\n", pcTableOffset ));

    gxv_just_justData_lookuptable_validate( p, limit, valid );
    gxv_odtect_add_range( p, valid->subtable_length,
                          "just_LookupTable", odtect );

    if ( wdcTableOffset )
    {
      gxv_just_widthDeltaClusters_validate(
        valid->root->base + wdcTableOffset, limit, valid );
      gxv_odtect_add_range( valid->root->base + wdcTableOffset,
                            valid->subtable_length, "just_wdcTable", odtect );
    }

    if ( pcTableOffset )
    {
      gxv_just_postcompTable_validate( valid->root->base + pcTableOffset,
                                       limit, valid );
      gxv_odtect_add_range( valid->root->base + pcTableOffset,
                            valid->subtable_length, "just_pcTable", odtect );
    }

    if ( justClassTableOffset )
    {
      gxv_just_justClassTable_validate(
        valid->root->base + justClassTableOffset, limit, valid );
      gxv_odtect_add_range( valid->root->base + justClassTableOffset,
                            valid->subtable_length, "just_justClassTable",
                            odtect );
    }

    gxv_odtect_validate( odtect, valid );

    GXV_EXIT;
  }


  FT_LOCAL_DEF( void )
  gxv_just_validate( FT_Bytes      table,
                     FT_Face       face,
                     FT_Validator  ftvalid )
  {
    FT_Bytes           p     = table;
    FT_Bytes           limit = 0;
    FT_Offset          table_size;

    GXV_ValidatorRec   validrec;
    GXV_Validator      valid = &validrec;
    GXV_just_DataRec   justrec;
    GXV_just_Data      just = &justrec;

    FT_ULong           version;
    FT_UShort          format;
    FT_UShort          horizOffset;
    FT_UShort          vertOffset;

    GXV_ODTECT( 3, odtect );


    GXV_ODTECT_INIT( odtect );

    valid->root       = ftvalid;
    valid->table_data = just;
    valid->face       = face;

    FT_TRACE3(( "validating `just' table\n" ));
    GXV_INIT;

    limit      = valid->root->limit;
    table_size = limit - table;

    GXV_LIMIT_CHECK( 4 + 2 + 2 + 2 );
    version     = FT_NEXT_ULONG( p );
    format      = FT_NEXT_USHORT( p );
    horizOffset = FT_NEXT_USHORT( p );
    vertOffset  = FT_NEXT_USHORT( p );
    gxv_odtect_add_range( table, p - table, "just header", odtect );


    /* Version 1.0 (always:2000) */
    GXV_TRACE(( " (version = 0x%08x)\n", version ));
    if ( version != 0x00010000UL )
      FT_INVALID_FORMAT;

    /* format 0 (always:2000) */
    GXV_TRACE(( " (format = 0x%04x)\n", format ));
    if ( format != 0x0000 )
        FT_INVALID_FORMAT;

    GXV_TRACE(( " (horizOffset = %d)\n", horizOffset  ));
    GXV_TRACE(( " (vertOffset = %d)\n", vertOffset  ));


    /* validate justData */
    if ( 0 < horizOffset )
    {
      gxv_just_justData_validate( table + horizOffset, limit, valid );
      gxv_odtect_add_range( table + horizOffset, valid->subtable_length,
                            "horizJustData", odtect );
    }

    if ( 0 < vertOffset )
    {
      gxv_just_justData_validate( table + vertOffset, limit, valid );
      gxv_odtect_add_range( table + vertOffset, valid->subtable_length,
                            "vertJustData", odtect );
    }

    gxv_odtect_validate( odtect, valid );

    FT_TRACE4(( "\n" ));
  }


/* END */
