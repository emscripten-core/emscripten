/***************************************************************************/
/*                                                                         */
/*  gxvmort1.c                                                             */
/*                                                                         */
/*    TrueTypeGX/AAT mort table validation                                 */
/*    body for type1 (Contextual Substitution) subtable.                   */
/*                                                                         */
/*  Copyright 2005, 2007 by suzuki toshiya, Masatake YAMATO, Red Hat K.K., */
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


#include "gxvmort.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvmort


  typedef struct  GXV_mort_subtable_type1_StateOptRec_
  {
    FT_UShort  substitutionTable;
    FT_UShort  substitutionTable_length;

  }  GXV_mort_subtable_type1_StateOptRec,
    *GXV_mort_subtable_type1_StateOptRecData;

#define GXV_MORT_SUBTABLE_TYPE1_HEADER_SIZE \
          ( GXV_STATETABLE_HEADER_SIZE + 2 )


  static void
  gxv_mort_subtable_type1_substitutionTable_load( FT_Bytes       table,
                                                  FT_Bytes       limit,
                                                  GXV_Validator  valid )
  {
    FT_Bytes  p = table;

    GXV_mort_subtable_type1_StateOptRecData  optdata =
      (GXV_mort_subtable_type1_StateOptRecData)valid->statetable.optdata;


    GXV_LIMIT_CHECK( 2 );
    optdata->substitutionTable = FT_NEXT_USHORT( p );
  }


  static void
  gxv_mort_subtable_type1_subtable_setup( FT_UShort      table_size,
                                          FT_UShort      classTable,
                                          FT_UShort      stateArray,
                                          FT_UShort      entryTable,
                                          FT_UShort*     classTable_length_p,
                                          FT_UShort*     stateArray_length_p,
                                          FT_UShort*     entryTable_length_p,
                                          GXV_Validator  valid )
  {
    FT_UShort  o[4];
    FT_UShort  *l[4];
    FT_UShort  buff[5];

    GXV_mort_subtable_type1_StateOptRecData  optdata =
      (GXV_mort_subtable_type1_StateOptRecData)valid->statetable.optdata;


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    o[3] = optdata->substitutionTable;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;
    l[3] = &( optdata->substitutionTable_length );

    gxv_set_length_by_ushort_offset( o, l, buff, 4, table_size, valid );
  }


  static void
  gxv_mort_subtable_type1_offset_to_subst_validate(
    FT_Short          wordOffset,
    const FT_String*  tag,
    FT_Byte           state,
    GXV_Validator     valid )
  {
    FT_UShort  substTable;
    FT_UShort  substTable_limit;
    FT_UShort  min_gid;
    FT_UShort  max_gid;

    FT_UNUSED( tag );
    FT_UNUSED( state );


    substTable =
      ((GXV_mort_subtable_type1_StateOptRec *)
       (valid->statetable.optdata))->substitutionTable;
    substTable_limit =
      (FT_UShort)( substTable +
                   ((GXV_mort_subtable_type1_StateOptRec *)
                    (valid->statetable.optdata))->substitutionTable_length );

    min_gid = (FT_UShort)( ( substTable       - wordOffset * 2 ) / 2 );
    max_gid = (FT_UShort)( ( substTable_limit - wordOffset * 2 ) / 2 );
    max_gid = (FT_UShort)( FT_MAX( max_gid, valid->face->num_glyphs ) );

    /* XXX: check range? */

    /* TODO: min_gid & max_gid comparison with ClassTable contents */
  }


  static void
  gxv_mort_subtable_type1_entry_validate(
    FT_Byte                         state,
    FT_UShort                       flags,
    GXV_StateTable_GlyphOffsetCPtr  glyphOffset_p,
    FT_Bytes                        table,
    FT_Bytes                        limit,
    GXV_Validator                   valid )
  {
    FT_UShort  setMark;
    FT_UShort  dontAdvance;
    FT_UShort  reserved;
    FT_Short   markOffset;
    FT_Short   currentOffset;

    FT_UNUSED( table );
    FT_UNUSED( limit );


    setMark       = (FT_UShort)(   flags >> 15            );
    dontAdvance   = (FT_UShort)( ( flags >> 14 ) & 1      );
    reserved      = (FT_Short)(    flags         & 0x3FFF );

    markOffset    = (FT_Short)( glyphOffset_p->ul >> 16 );
    currentOffset = (FT_Short)( glyphOffset_p->ul       );

    if ( 0 < reserved )
    {
      GXV_TRACE(( " non-zero bits found in reserved range\n" ));
      if ( valid->root->level >= FT_VALIDATE_PARANOID )
        FT_INVALID_DATA;
    }

    gxv_mort_subtable_type1_offset_to_subst_validate( markOffset,
                                                      "markOffset",
                                                      state,
                                                      valid );

    gxv_mort_subtable_type1_offset_to_subst_validate( currentOffset,
                                                      "currentOffset",
                                                      state,
                                                      valid );
  }


  static void
  gxv_mort_subtable_type1_substTable_validate( FT_Bytes       table,
                                               FT_Bytes       limit,
                                               GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  num_gids = (FT_UShort)(
                 ((GXV_mort_subtable_type1_StateOptRec *)
                  (valid->statetable.optdata))->substitutionTable_length / 2 );
    FT_UShort  i;


    GXV_NAME_ENTER( "validating contents of substitutionTable" );
    for ( i = 0; i < num_gids ; i ++ )
    {
      FT_UShort  dst_gid;


      GXV_LIMIT_CHECK( 2 );
      dst_gid = FT_NEXT_USHORT( p );

      if ( dst_gid >= 0xFFFFU )
        continue;

      if ( dst_gid > valid->face->num_glyphs )
      {
        GXV_TRACE(( "substTable include too large gid[%d]=%d >"
                    " max defined gid #%d\n",
                    i, dst_gid, valid->face->num_glyphs ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;
      }
    }

    GXV_EXIT;
  }


  /*
   * subtable for Contextual glyph substitution is a modified StateTable.
   * In addition to classTable, stateArray, and entryTable, the field
   * `substitutionTable' is added.
   */
  FT_LOCAL_DEF( void )
  gxv_mort_subtable_type1_validate( FT_Bytes       table,
                                    FT_Bytes       limit,
                                    GXV_Validator  valid )
  {
    FT_Bytes  p = table;

    GXV_mort_subtable_type1_StateOptRec  st_rec;


    GXV_NAME_ENTER( "mort chain subtable type1 (Contextual Glyph Subst)" );

    GXV_LIMIT_CHECK( GXV_MORT_SUBTABLE_TYPE1_HEADER_SIZE );

    valid->statetable.optdata =
      &st_rec;
    valid->statetable.optdata_load_func =
      gxv_mort_subtable_type1_substitutionTable_load;
    valid->statetable.subtable_setup_func =
      gxv_mort_subtable_type1_subtable_setup;
    valid->statetable.entry_glyphoffset_fmt =
      GXV_GLYPHOFFSET_ULONG;
    valid->statetable.entry_validate_func =

      gxv_mort_subtable_type1_entry_validate;
    gxv_StateTable_validate( p, limit, valid );

    gxv_mort_subtable_type1_substTable_validate(
      table + st_rec.substitutionTable,
      table + st_rec.substitutionTable + st_rec.substitutionTable_length,
      valid );

    GXV_EXIT;
  }


/* END */
