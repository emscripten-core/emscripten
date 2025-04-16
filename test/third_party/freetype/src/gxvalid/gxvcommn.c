/***************************************************************************/
/*                                                                         */
/*  gxvcommn.c                                                             */
/*                                                                         */
/*    TrueTypeGX/AAT common tables validation (body).                      */
/*                                                                         */
/*  Copyright 2004, 2005, 2009, 2010                                       */
/*  by suzuki toshiya, Masatake YAMATO, Red Hat K.K.,                      */
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


#include "gxvcommn.h"


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvcommon


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       16bit offset sorter                     *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static int
  gxv_compare_ushort_offset( FT_UShort*  a,
                             FT_UShort*  b )
  {
    if ( *a < *b )
      return -1;
    else if ( *a > *b )
      return 1;
    else
      return 0;
  }


  FT_LOCAL_DEF( void )
  gxv_set_length_by_ushort_offset( FT_UShort*     offset,
                                   FT_UShort**    length,
                                   FT_UShort*     buff,
                                   FT_UInt        nmemb,
                                   FT_UShort      limit,
                                   GXV_Validator  valid )
  {
    FT_UInt  i;


    for ( i = 0; i < nmemb; i++ )
      *(length[i]) = 0;

    for ( i = 0; i < nmemb; i++ )
      buff[i] = offset[i];
    buff[nmemb] = limit;

    ft_qsort( buff, ( nmemb + 1 ), sizeof ( FT_UShort ),
              ( int(*)(const void*, const void*) )gxv_compare_ushort_offset );

    if ( buff[nmemb] > limit )
      FT_INVALID_OFFSET;

    for ( i = 0; i < nmemb; i++ )
    {
      FT_UInt  j;


      for ( j = 0; j < nmemb; j++ )
        if ( buff[j] == offset[i] )
          break;

      if ( j == nmemb )
        FT_INVALID_OFFSET;

      *(length[i]) = (FT_UShort)( buff[j + 1] - buff[j] );

      if ( 0 != offset[i] && 0 == *(length[i]) )
        FT_INVALID_OFFSET;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       32bit offset sorter                     *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static int
  gxv_compare_ulong_offset( FT_ULong*  a,
                            FT_ULong*  b )
  {
    if ( *a < *b )
      return -1;
    else if ( *a > *b )
      return 1;
    else
      return 0;
  }


  FT_LOCAL_DEF( void )
  gxv_set_length_by_ulong_offset( FT_ULong*      offset,
                                  FT_ULong**     length,
                                  FT_ULong*      buff,
                                  FT_UInt        nmemb,
                                  FT_ULong       limit,
                                  GXV_Validator  valid)
  {
    FT_UInt  i;


    for ( i = 0; i < nmemb; i++ )
      *(length[i]) = 0;

    for ( i = 0; i < nmemb; i++ )
      buff[i] = offset[i];
    buff[nmemb] = limit;

    ft_qsort( buff, ( nmemb + 1 ), sizeof ( FT_ULong ),
              ( int(*)(const void*, const void*) )gxv_compare_ulong_offset );

    if ( buff[nmemb] > limit )
      FT_INVALID_OFFSET;

    for ( i = 0; i < nmemb; i++ )
    {
      FT_UInt  j;


      for ( j = 0; j < nmemb; j++ )
        if ( buff[j] == offset[i] )
          break;

      if ( j == nmemb )
        FT_INVALID_OFFSET;

      *(length[i]) = buff[j + 1] - buff[j];

      if ( 0 != offset[i] && 0 == *(length[i]) )
        FT_INVALID_OFFSET;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****               scan value array and get min & max              *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_LOCAL_DEF( void )
  gxv_array_getlimits_byte( FT_Bytes       table,
                            FT_Bytes       limit,
                            FT_Byte*       min,
                            FT_Byte*       max,
                            GXV_Validator  valid )
  {
    FT_Bytes  p = table;


    *min = 0xFF;
    *max = 0x00;

    while ( p < limit )
    {
      FT_Byte  val;


      GXV_LIMIT_CHECK( 1 );
      val = FT_NEXT_BYTE( p );

      *min = (FT_Byte)FT_MIN( *min, val );
      *max = (FT_Byte)FT_MAX( *max, val );
    }

    valid->subtable_length = p - table;
  }


  FT_LOCAL_DEF( void )
  gxv_array_getlimits_ushort( FT_Bytes       table,
                              FT_Bytes       limit,
                              FT_UShort*     min,
                              FT_UShort*     max,
                              GXV_Validator  valid )
  {
    FT_Bytes  p = table;


    *min = 0xFFFFU;
    *max = 0x0000;

    while ( p < limit )
    {
      FT_UShort  val;


      GXV_LIMIT_CHECK( 2 );
      val = FT_NEXT_USHORT( p );

      *min = (FT_Byte)FT_MIN( *min, val );
      *max = (FT_Byte)FT_MAX( *max, val );
    }

    valid->subtable_length = p - table;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       BINSEARCHHEADER                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct  GXV_BinSrchHeader_
  {
    FT_UShort  unitSize;
    FT_UShort  nUnits;
    FT_UShort  searchRange;
    FT_UShort  entrySelector;
    FT_UShort  rangeShift;

  } GXV_BinSrchHeader;


  static void
  gxv_BinSrchHeader_check_consistency( GXV_BinSrchHeader*  binSrchHeader,
                                       GXV_Validator       valid )
  {
    FT_UShort  searchRange;
    FT_UShort  entrySelector;
    FT_UShort  rangeShift;


    if ( binSrchHeader->unitSize == 0 )
      FT_INVALID_DATA;

    if ( binSrchHeader->nUnits == 0 )
    {
      if ( binSrchHeader->searchRange   == 0 &&
           binSrchHeader->entrySelector == 0 &&
           binSrchHeader->rangeShift    == 0 )
        return;
      else
        FT_INVALID_DATA;
    }

    for ( searchRange = 1, entrySelector = 1;
          ( searchRange * 2 ) <= binSrchHeader->nUnits &&
            searchRange < 0x8000U;
          searchRange *= 2, entrySelector++ )
      ;

    entrySelector--;
    searchRange = (FT_UShort)( searchRange * binSrchHeader->unitSize );
    rangeShift  = (FT_UShort)( binSrchHeader->nUnits * binSrchHeader->unitSize
                               - searchRange );

    if ( searchRange   != binSrchHeader->searchRange   ||
         entrySelector != binSrchHeader->entrySelector ||
         rangeShift    != binSrchHeader->rangeShift    )
    {
      GXV_TRACE(( "Inconsistency found in BinSrchHeader\n" ));
      GXV_TRACE(( "originally: unitSize=%d, nUnits=%d, "
                  "searchRange=%d, entrySelector=%d, "
                  "rangeShift=%d\n",
                  binSrchHeader->unitSize, binSrchHeader->nUnits,
                  binSrchHeader->searchRange, binSrchHeader->entrySelector,
                  binSrchHeader->rangeShift ));
      GXV_TRACE(( "calculated: unitSize=%d, nUnits=%d, "
                  "searchRange=%d, entrySelector=%d, "
                  "rangeShift=%d\n",
                  binSrchHeader->unitSize, binSrchHeader->nUnits,
                  searchRange, entrySelector, rangeShift ));

      if ( valid->root->level >= FT_VALIDATE_PARANOID )
        FT_INVALID_DATA;
    }
  }


  /*
   * parser & validator of BinSrchHeader
   * which is used in LookupTable format 2, 4, 6.
   *
   * Essential parameters (unitSize, nUnits) are returned by
   * given pointer, others (searchRange, entrySelector, rangeShift)
   * can be calculated by essential parameters, so they are just
   * validated and discarded.
   *
   * However, wrong values in searchRange, entrySelector, rangeShift
   * won't cause fatal errors, because these parameters might be
   * only used in old m68k font driver in MacOS.
   *   -- suzuki toshiya <mpsuzuki@hiroshima-u.ac.jp>
   */

  FT_LOCAL_DEF( void )
  gxv_BinSrchHeader_validate( FT_Bytes       table,
                              FT_Bytes       limit,
                              FT_UShort*     unitSize_p,
                              FT_UShort*     nUnits_p,
                              GXV_Validator  valid )
  {
    FT_Bytes           p = table;
    GXV_BinSrchHeader  binSrchHeader;


    GXV_NAME_ENTER( "BinSrchHeader validate" );

    if ( *unitSize_p == 0 )
    {
      GXV_LIMIT_CHECK( 2 );
      binSrchHeader.unitSize =  FT_NEXT_USHORT( p );
    }
    else
      binSrchHeader.unitSize = *unitSize_p;

    if ( *nUnits_p == 0 )
    {
      GXV_LIMIT_CHECK( 2 );
      binSrchHeader.nUnits = FT_NEXT_USHORT( p );
    }
    else
      binSrchHeader.nUnits = *nUnits_p;

    GXV_LIMIT_CHECK( 2 + 2 + 2 );
    binSrchHeader.searchRange   = FT_NEXT_USHORT( p );
    binSrchHeader.entrySelector = FT_NEXT_USHORT( p );
    binSrchHeader.rangeShift    = FT_NEXT_USHORT( p );
    GXV_TRACE(( "nUnits %d\n", binSrchHeader.nUnits ));

    gxv_BinSrchHeader_check_consistency( &binSrchHeader, valid );

    if ( *unitSize_p == 0 )
      *unitSize_p = binSrchHeader.unitSize;

    if ( *nUnits_p == 0 )
      *nUnits_p = binSrchHeader.nUnits;

    valid->subtable_length = p - table;
    GXV_EXIT;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         LOOKUP TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define GXV_LOOKUP_VALUE_LOAD( P, SIGNSPEC )                   \
          ( P += 2, gxv_lookup_value_load( P - 2, SIGNSPEC ) )

  static GXV_LookupValueDesc
  gxv_lookup_value_load( FT_Bytes  p,
                         int       signspec )
  {
    GXV_LookupValueDesc  v;


    if ( signspec == GXV_LOOKUPVALUE_UNSIGNED )
      v.u = FT_NEXT_USHORT( p );
    else
      v.s = FT_NEXT_SHORT( p );

    return v;
  }


#define GXV_UNITSIZE_VALIDATE( FORMAT, UNITSIZE, NUNITS, CORRECTSIZE ) \
          FT_BEGIN_STMNT                                               \
            if ( UNITSIZE != CORRECTSIZE )                             \
            {                                                          \
              FT_ERROR(( "unitSize=%d differs from"                    \
                         " expected unitSize=%d"                       \
                         " in LookupTable %s\n",                       \
                          UNITSIZE, CORRECTSIZE, FORMAT ));            \
              if ( UNITSIZE != 0 && NUNITS != 0 )                      \
              {                                                        \
                FT_ERROR(( " cannot validate anymore\n" ));            \
                FT_INVALID_FORMAT;                                     \
              }                                                        \
              else                                                     \
                FT_ERROR(( " forcibly continues\n" ));                 \
            }                                                          \
          FT_END_STMNT


  /* ================= Simple Array Format 0 Lookup Table ================ */
  static void
  gxv_LookupTable_fmt0_validate( FT_Bytes       table,
                                 FT_Bytes       limit,
                                 GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  i;

    GXV_LookupValueDesc  value;


    GXV_NAME_ENTER( "LookupTable format 0" );

    GXV_LIMIT_CHECK( 2 * valid->face->num_glyphs );

    for ( i = 0; i < valid->face->num_glyphs; i++ )
    {
      GXV_LIMIT_CHECK( 2 );
      if ( p + 2 >= limit )     /* some fonts have too-short fmt0 array */
      {
        GXV_TRACE(( "too short, glyphs %d - %d are missing\n",
                    i, valid->face->num_glyphs ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;
        break;
      }

      value = GXV_LOOKUP_VALUE_LOAD( p, valid->lookupval_sign );
      valid->lookupval_func( i, &value, valid );
    }

    valid->subtable_length = p - table;
    GXV_EXIT;
  }


  /* ================= Segment Single Format 2 Loolup Table ============== */
  /*
   * Apple spec says:
   *
   *   To guarantee that a binary search terminates, you must include one or
   *   more special `end of search table' values at the end of the data to
   *   be searched.  The number of termination values that need to be
   *   included is table-specific.  The value that indicates binary search
   *   termination is 0xFFFF.
   *
   * The problem is that nUnits does not include this end-marker.  It's
   * quite difficult to discriminate whether the following 0xFFFF comes from
   * the end-marker or some next data.
   *
   *   -- suzuki toshiya <mpsuzuki@hiroshima-u.ac.jp>
   */
  static void
  gxv_LookupTable_fmt2_skip_endmarkers( FT_Bytes       table,
                                        FT_UShort      unitSize,
                                        GXV_Validator  valid )
  {
    FT_Bytes  p = table;


    while ( ( p + 4 ) < valid->root->limit )
    {
      if ( p[0] != 0xFF || p[1] != 0xFF || /* lastGlyph */
           p[2] != 0xFF || p[3] != 0xFF )  /* firstGlyph */
        break;
      p += unitSize;
    }

    valid->subtable_length = p - table;
  }


  static void
  gxv_LookupTable_fmt2_validate( FT_Bytes       table,
                                 FT_Bytes       limit,
                                 GXV_Validator  valid )
  {
    FT_Bytes             p = table;
    FT_UShort            gid;

    FT_UShort            unitSize;
    FT_UShort            nUnits;
    FT_UShort            unit;
    FT_UShort            lastGlyph;
    FT_UShort            firstGlyph;
    GXV_LookupValueDesc  value;


    GXV_NAME_ENTER( "LookupTable format 2" );

    unitSize = nUnits = 0;
    gxv_BinSrchHeader_validate( p, limit, &unitSize, &nUnits, valid );
    p += valid->subtable_length;

    GXV_UNITSIZE_VALIDATE( "format2", unitSize, nUnits, 6 );

    for ( unit = 0, gid = 0; unit < nUnits; unit++ )
    {
      GXV_LIMIT_CHECK( 2 + 2 + 2 );
      lastGlyph  = FT_NEXT_USHORT( p );
      firstGlyph = FT_NEXT_USHORT( p );
      value      = GXV_LOOKUP_VALUE_LOAD( p, valid->lookupval_sign );

      gxv_glyphid_validate( firstGlyph, valid );
      gxv_glyphid_validate( lastGlyph, valid );

      if ( lastGlyph < gid )
      {
        GXV_TRACE(( "reverse ordered segment specification:"
                    " lastGlyph[%d]=%d < lastGlyph[%d]=%d\n",
                    unit, lastGlyph, unit - 1 , gid ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;
      }

      if ( lastGlyph < firstGlyph )
      {
        GXV_TRACE(( "reverse ordered range specification at unit %d:",
                    " lastGlyph %d < firstGlyph %d ",
                    unit, lastGlyph, firstGlyph ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;

        if ( valid->root->level == FT_VALIDATE_TIGHT )
          continue;     /* ftxvalidator silently skips such an entry */

        FT_TRACE4(( "continuing with exchanged values\n" ));
        gid        = firstGlyph;
        firstGlyph = lastGlyph;
        lastGlyph  = gid;
      }

      for ( gid = firstGlyph; gid <= lastGlyph; gid++ )
        valid->lookupval_func( gid, &value, valid );
    }

    gxv_LookupTable_fmt2_skip_endmarkers( p, unitSize, valid );
    p += valid->subtable_length;

    valid->subtable_length = p - table;
    GXV_EXIT;
  }


  /* ================= Segment Array Format 4 Lookup Table =============== */
  static void
  gxv_LookupTable_fmt4_validate( FT_Bytes       table,
                                 FT_Bytes       limit,
                                 GXV_Validator  valid )
  {
    FT_Bytes             p = table;
    FT_UShort            unit;
    FT_UShort            gid;

    FT_UShort            unitSize;
    FT_UShort            nUnits;
    FT_UShort            lastGlyph;
    FT_UShort            firstGlyph;
    GXV_LookupValueDesc  base_value;
    GXV_LookupValueDesc  value;


    GXV_NAME_ENTER( "LookupTable format 4" );

    unitSize = nUnits = 0;
    gxv_BinSrchHeader_validate( p, limit, &unitSize, &nUnits, valid );
    p += valid->subtable_length;

    GXV_UNITSIZE_VALIDATE( "format4", unitSize, nUnits, 6 );

    for ( unit = 0, gid = 0; unit < nUnits; unit++ )
    {
      GXV_LIMIT_CHECK( 2 + 2 );
      lastGlyph  = FT_NEXT_USHORT( p );
      firstGlyph = FT_NEXT_USHORT( p );

      gxv_glyphid_validate( firstGlyph, valid );
      gxv_glyphid_validate( lastGlyph, valid );

      if ( lastGlyph < gid )
      {
        GXV_TRACE(( "reverse ordered segment specification:"
                    " lastGlyph[%d]=%d < lastGlyph[%d]=%d\n",
                    unit, lastGlyph, unit - 1 , gid ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;
      }

      if ( lastGlyph < firstGlyph )
      {
        GXV_TRACE(( "reverse ordered range specification at unit %d:",
                    " lastGlyph %d < firstGlyph %d ",
                    unit, lastGlyph, firstGlyph ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;

        if ( valid->root->level == FT_VALIDATE_TIGHT )
          continue; /* ftxvalidator silently skips such an entry */

        FT_TRACE4(( "continuing with exchanged values\n" ));
        gid        = firstGlyph;
        firstGlyph = lastGlyph;
        lastGlyph  = gid;
      }

      GXV_LIMIT_CHECK( 2 );
      base_value = GXV_LOOKUP_VALUE_LOAD( p, GXV_LOOKUPVALUE_UNSIGNED );

      for ( gid = firstGlyph; gid <= lastGlyph; gid++ )
      {
        value = valid->lookupfmt4_trans( (FT_UShort)( gid - firstGlyph ),
                                         &base_value,
                                         limit,
                                         valid );

        valid->lookupval_func( gid, &value, valid );
      }
    }

    gxv_LookupTable_fmt2_skip_endmarkers( p, unitSize, valid );
    p += valid->subtable_length;

    valid->subtable_length = p - table;
    GXV_EXIT;
  }


  /* ================= Segment Table Format 6 Lookup Table =============== */
  static void
  gxv_LookupTable_fmt6_skip_endmarkers( FT_Bytes       table,
                                        FT_UShort      unitSize,
                                        GXV_Validator  valid )
  {
    FT_Bytes  p = table;


    while ( p < valid->root->limit )
    {
      if ( p[0] != 0xFF || p[1] != 0xFF )
        break;
      p += unitSize;
    }

    valid->subtable_length = p - table;
  }


  static void
  gxv_LookupTable_fmt6_validate( FT_Bytes       table,
                                 FT_Bytes       limit,
                                 GXV_Validator  valid )
  {
    FT_Bytes             p = table;
    FT_UShort            unit;
    FT_UShort            prev_glyph;

    FT_UShort            unitSize;
    FT_UShort            nUnits;
    FT_UShort            glyph;
    GXV_LookupValueDesc  value;


    GXV_NAME_ENTER( "LookupTable format 6" );

    unitSize = nUnits = 0;
    gxv_BinSrchHeader_validate( p, limit, &unitSize, &nUnits, valid );
    p += valid->subtable_length;

    GXV_UNITSIZE_VALIDATE( "format6", unitSize, nUnits, 4 );

    for ( unit = 0, prev_glyph = 0; unit < nUnits; unit++ )
    {
      GXV_LIMIT_CHECK( 2 + 2 );
      glyph = FT_NEXT_USHORT( p );
      value = GXV_LOOKUP_VALUE_LOAD( p, valid->lookupval_sign );

      if ( gxv_glyphid_validate( glyph, valid ) )
        GXV_TRACE(( " endmarker found within defined range"
                    " (entry %d < nUnits=%d)\n",
                    unit, nUnits ));

      if ( prev_glyph > glyph )
      {
        GXV_TRACE(( "current gid 0x%04x < previous gid 0x%04x\n",
                    glyph, prev_glyph ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;
      }
      prev_glyph = glyph;

      valid->lookupval_func( glyph, &value, valid );
    }

    gxv_LookupTable_fmt6_skip_endmarkers( p, unitSize, valid );
    p += valid->subtable_length;

    valid->subtable_length = p - table;
    GXV_EXIT;
  }


  /* ================= Trimmed Array Format 8 Lookup Table =============== */
  static void
  gxv_LookupTable_fmt8_validate( FT_Bytes       table,
                                 FT_Bytes       limit,
                                 GXV_Validator  valid )
  {
    FT_Bytes              p = table;
    FT_UShort             i;

    GXV_LookupValueDesc   value;
    FT_UShort             firstGlyph;
    FT_UShort             glyphCount;


    GXV_NAME_ENTER( "LookupTable format 8" );

    /* firstGlyph + glyphCount */
    GXV_LIMIT_CHECK( 2 + 2 );
    firstGlyph = FT_NEXT_USHORT( p );
    glyphCount = FT_NEXT_USHORT( p );

    gxv_glyphid_validate( firstGlyph, valid );
    gxv_glyphid_validate( (FT_UShort)( firstGlyph + glyphCount ), valid );

    /* valueArray */
    for ( i = 0; i < glyphCount; i++ )
    {
      GXV_LIMIT_CHECK( 2 );
      value = GXV_LOOKUP_VALUE_LOAD( p, valid->lookupval_sign );
      valid->lookupval_func( (FT_UShort)( firstGlyph + i ), &value, valid );
    }

    valid->subtable_length = p - table;
    GXV_EXIT;
  }


  FT_LOCAL_DEF( void )
  gxv_LookupTable_validate( FT_Bytes       table,
                            FT_Bytes       limit,
                            GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  format;

    GXV_Validate_Func  fmt_funcs_table[] =
    {
      gxv_LookupTable_fmt0_validate, /* 0 */
      NULL,                          /* 1 */
      gxv_LookupTable_fmt2_validate, /* 2 */
      NULL,                          /* 3 */
      gxv_LookupTable_fmt4_validate, /* 4 */
      NULL,                          /* 5 */
      gxv_LookupTable_fmt6_validate, /* 6 */
      NULL,                          /* 7 */
      gxv_LookupTable_fmt8_validate, /* 8 */
    };

    GXV_Validate_Func  func;


    GXV_NAME_ENTER( "LookupTable" );

    /* lookuptbl_head may be used in fmt4 transit function. */
    valid->lookuptbl_head = table;

    /* format */
    GXV_LIMIT_CHECK( 2 );
    format = FT_NEXT_USHORT( p );
    GXV_TRACE(( " (format %d)\n", format ));

    if ( format > 8 )
      FT_INVALID_FORMAT;

    func = fmt_funcs_table[format];
    if ( func == NULL )
      FT_INVALID_FORMAT;

    func( p, limit, valid );
    p += valid->subtable_length;

    valid->subtable_length = p - table;

    GXV_EXIT;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                          Glyph ID                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_LOCAL_DEF( FT_Int )
  gxv_glyphid_validate( FT_UShort      gid,
                        GXV_Validator  valid )
  {
    FT_Face  face;


    if ( gid == 0xFFFFU )
    {
      GXV_EXIT;
      return 1;
    }

    face = valid->face;
    if ( face->num_glyphs < gid )
    {
      GXV_TRACE(( " gxv_glyphid_check() gid overflow: num_glyphs %d < %d\n",
                  face->num_glyphs, gid ));
      if ( valid->root->level >= FT_VALIDATE_PARANOID )
        FT_INVALID_GLYPH_ID;
    }

    return 0;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        CONTROL POINT                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_LOCAL_DEF( void )
  gxv_ctlPoint_validate( FT_UShort      gid,
                         FT_Short       ctl_point,
                         GXV_Validator  valid )
  {
    FT_Face       face;
    FT_Error      error;

    FT_GlyphSlot  glyph;
    FT_Outline    outline;
    short         n_points;


    face = valid->face;

    error = FT_Load_Glyph( face,
                           gid,
                           FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM );
    if ( error )
      FT_INVALID_GLYPH_ID;

    glyph    = face->glyph;
    outline  = glyph->outline;
    n_points = outline.n_points;


    if ( !( ctl_point < n_points ) )
      FT_INVALID_DATA;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                          SFNT NAME                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_LOCAL_DEF( void )
  gxv_sfntName_validate( FT_UShort      name_index,
                         FT_UShort      min_index,
                         FT_UShort      max_index,
                         GXV_Validator  valid )
  {
    FT_SfntName  name;
    FT_UInt      i;
    FT_UInt      nnames;


    GXV_NAME_ENTER( "sfntName" );

    if ( name_index < min_index || max_index < name_index )
      FT_INVALID_FORMAT;

    nnames = FT_Get_Sfnt_Name_Count( valid->face );
    for ( i = 0; i < nnames; i++ )
    {
      if ( FT_Get_Sfnt_Name( valid->face, i, &name ) != GXV_Err_Ok )
        continue ;

      if ( name.name_id == name_index )
        goto Out;
    }

    GXV_TRACE(( "  nameIndex = %d (UNTITLED)\n", name_index ));
    FT_INVALID_DATA;
    goto Exit;  /* make compiler happy */

  Out:
    FT_TRACE1(( "  nameIndex = %d (", name_index ));
    GXV_TRACE_HEXDUMP_SFNTNAME( name );
    FT_TRACE1(( ")\n" ));

  Exit:
    GXV_EXIT;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                          STATE TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* -------------------------- Class Table --------------------------- */

  /*
   * highestClass specifies how many classes are defined in this
   * Class Subtable.  Apple spec does not mention whether undefined
   * holes in the class (e.g.: 0-3 are predefined, 4 is unused, 5 is used)
   * are permitted.  At present, holes in a defined class are not checked.
   *   -- suzuki toshiya <mpsuzuki@hiroshima-u.ac.jp>
   */

  static void
  gxv_ClassTable_validate( FT_Bytes       table,
                           FT_UShort*     length_p,
                           FT_UShort      stateSize,
                           FT_Byte*       maxClassID_p,
                           GXV_Validator  valid )
  {
    FT_Bytes   p     = table;
    FT_Bytes   limit = table + *length_p;
    FT_UShort  firstGlyph;
    FT_UShort  nGlyphs;


    GXV_NAME_ENTER( "ClassTable" );

    *maxClassID_p = 3;  /* Classes 0, 2, and 3 are predefined */

    GXV_LIMIT_CHECK( 2 + 2 );
    firstGlyph = FT_NEXT_USHORT( p );
    nGlyphs    = FT_NEXT_USHORT( p );

    GXV_TRACE(( " (firstGlyph = %d, nGlyphs = %d)\n", firstGlyph, nGlyphs ));

    if ( !nGlyphs )
      goto Out;

    gxv_glyphid_validate( (FT_UShort)( firstGlyph + nGlyphs ), valid );

    {
      FT_Byte    nGlyphInClass[256];
      FT_Byte    classID;
      FT_UShort  i;


      ft_memset( nGlyphInClass, 0, 256 );


      for ( i = 0; i < nGlyphs; i++ )
      {
        GXV_LIMIT_CHECK( 1 );
        classID = FT_NEXT_BYTE( p );
        switch ( classID )
        {
          /* following classes should not appear in class array */
        case 0:             /* end of text */
        case 2:             /* out of bounds */
        case 3:             /* end of line */
          FT_INVALID_DATA;
          break;

        case 1:             /* out of bounds */
        default:            /* user-defined: 4 - ( stateSize - 1 ) */
          if ( classID >= stateSize )
            FT_INVALID_DATA;   /* assign glyph to undefined state */

          nGlyphInClass[classID]++;
          break;
        }
      }
      *length_p = (FT_UShort)( p - table );

      /* scan max ClassID in use */
      for ( i = 0; i < stateSize; i++ )
        if ( ( 3 < i ) && ( nGlyphInClass[i] > 0 ) )
          *maxClassID_p = (FT_Byte)i;  /* XXX: Check Range? */
    }

  Out:
    GXV_TRACE(( "Declared stateSize=0x%02x, Used maxClassID=0x%02x\n",
                stateSize, *maxClassID_p ));
    GXV_EXIT;
  }


  /* --------------------------- State Array ----------------------------- */

  static void
  gxv_StateArray_validate( FT_Bytes       table,
                           FT_UShort*     length_p,
                           FT_Byte        maxClassID,
                           FT_UShort      stateSize,
                           FT_Byte*       maxState_p,
                           FT_Byte*       maxEntry_p,
                           GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    FT_Bytes  limit = table + *length_p;
    FT_Byte   clazz;
    FT_Byte   entry;

    FT_UNUSED( stateSize ); /* for the non-debugging case */


    GXV_NAME_ENTER( "StateArray" );

    GXV_TRACE(( "parse %d bytes by stateSize=%d maxClassID=%d\n",
                (int)(*length_p), stateSize, (int)(maxClassID) ));

    /*
     * 2 states are predefined and must be described in StateArray:
     * state 0 (start of text), 1 (start of line)
     */
    GXV_LIMIT_CHECK( ( 1 + maxClassID ) * 2 );

    *maxState_p = 0;
    *maxEntry_p = 0;

    /* read if enough to read another state */
    while ( p + ( 1 + maxClassID ) <= limit )
    {
      (*maxState_p)++;
      for ( clazz = 0; clazz <= maxClassID; clazz++ )
      {
        entry = FT_NEXT_BYTE( p );
        *maxEntry_p = (FT_Byte)FT_MAX( *maxEntry_p, entry );
      }
    }
    GXV_TRACE(( "parsed: maxState=%d, maxEntry=%d\n",
                *maxState_p, *maxEntry_p ));

    *length_p = (FT_UShort)( p - table );

    GXV_EXIT;
  }


  /* --------------------------- Entry Table ----------------------------- */

  static void
  gxv_EntryTable_validate( FT_Bytes       table,
                           FT_UShort*     length_p,
                           FT_Byte        maxEntry,
                           FT_UShort      stateArray,
                           FT_UShort      stateArray_length,
                           FT_Byte        maxClassID,
                           FT_Bytes       statetable_table,
                           FT_Bytes       statetable_limit,
                           GXV_Validator  valid )
  {
    FT_Bytes  p     = table;
    FT_Bytes  limit = table + *length_p;
    FT_Byte   entry;
    FT_Byte   state;
    FT_Int    entrySize = 2 + 2 + GXV_GLYPHOFFSET_SIZE( statetable );

    GXV_XStateTable_GlyphOffsetDesc  glyphOffset;


    GXV_NAME_ENTER( "EntryTable" );

    GXV_TRACE(( "maxEntry=%d entrySize=%d\n", maxEntry, entrySize ));

    if ( ( maxEntry + 1 ) * entrySize > *length_p )
    {
      if ( valid->root->level >= FT_VALIDATE_PARANOID )
        FT_INVALID_TOO_SHORT;

      /* ftxvalidator and FontValidator both warn and continue */
      maxEntry = (FT_Byte)( *length_p / entrySize - 1 );
      GXV_TRACE(( "too large maxEntry, shrinking to %d fit EntryTable length\n",
                  maxEntry ));
    }

    for ( entry = 0; entry <= maxEntry; entry++ )
    {
      FT_UShort  newState;
      FT_UShort  flags;


      GXV_LIMIT_CHECK( 2 + 2 );
      newState = FT_NEXT_USHORT( p );
      flags    = FT_NEXT_USHORT( p );


      if ( newState < stateArray                     ||
           stateArray + stateArray_length < newState )
      {
        GXV_TRACE(( " newState offset 0x%04x is out of stateArray\n",
                    newState ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_OFFSET;
        continue;
      }

      if ( 0 != ( ( newState - stateArray ) % ( 1 + maxClassID ) ) )
      {
        GXV_TRACE(( " newState offset 0x%04x is not aligned to %d-classes\n",
                    newState,  1 + maxClassID ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_OFFSET;
        continue;
      }

      state = (FT_Byte)( ( newState - stateArray ) / ( 1 + maxClassID ) );

      switch ( GXV_GLYPHOFFSET_FMT( statetable ) )
      {
      case GXV_GLYPHOFFSET_NONE:
        glyphOffset.uc = 0;  /* make compiler happy */
        break;

      case GXV_GLYPHOFFSET_UCHAR:
        glyphOffset.uc = FT_NEXT_BYTE( p );
        break;

      case GXV_GLYPHOFFSET_CHAR:
        glyphOffset.c = FT_NEXT_CHAR( p );
        break;

      case GXV_GLYPHOFFSET_USHORT:
        glyphOffset.u = FT_NEXT_USHORT( p );
        break;

      case GXV_GLYPHOFFSET_SHORT:
        glyphOffset.s = FT_NEXT_SHORT( p );
        break;

      case GXV_GLYPHOFFSET_ULONG:
        glyphOffset.ul = FT_NEXT_ULONG( p );
        break;

      case GXV_GLYPHOFFSET_LONG:
        glyphOffset.l = FT_NEXT_LONG( p );
        break;

      default:
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_FORMAT;
        goto Exit;
      }

      if ( NULL != valid->statetable.entry_validate_func )
        valid->statetable.entry_validate_func( state,
                                               flags,
                                               &glyphOffset,
                                               statetable_table,
                                               statetable_limit,
                                               valid );
    }

  Exit:
    *length_p = (FT_UShort)( p - table );

    GXV_EXIT;
  }


  /* =========================== State Table ============================= */

  FT_LOCAL_DEF( void )
  gxv_StateTable_subtable_setup( FT_UShort      table_size,
                                 FT_UShort      classTable,
                                 FT_UShort      stateArray,
                                 FT_UShort      entryTable,
                                 FT_UShort*     classTable_length_p,
                                 FT_UShort*     stateArray_length_p,
                                 FT_UShort*     entryTable_length_p,
                                 GXV_Validator  valid )
  {
    FT_UShort   o[3];
    FT_UShort*  l[3];
    FT_UShort   buff[4];


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;

    gxv_set_length_by_ushort_offset( o, l, buff, 3, table_size, valid );
  }


  FT_LOCAL_DEF( void )
  gxv_StateTable_validate( FT_Bytes       table,
                           FT_Bytes       limit,
                           GXV_Validator  valid )
  {
    FT_UShort   stateSize;
    FT_UShort   classTable;     /* offset to Class(Sub)Table */
    FT_UShort   stateArray;     /* offset to StateArray */
    FT_UShort   entryTable;     /* offset to EntryTable */

    FT_UShort   classTable_length;
    FT_UShort   stateArray_length;
    FT_UShort   entryTable_length;
    FT_Byte     maxClassID;
    FT_Byte     maxState;
    FT_Byte     maxEntry;

    GXV_StateTable_Subtable_Setup_Func  setup_func;

    FT_Bytes    p = table;


    GXV_NAME_ENTER( "StateTable" );

    GXV_TRACE(( "StateTable header\n" ));

    GXV_LIMIT_CHECK( 2 + 2 + 2 + 2 );
    stateSize  = FT_NEXT_USHORT( p );
    classTable = FT_NEXT_USHORT( p );
    stateArray = FT_NEXT_USHORT( p );
    entryTable = FT_NEXT_USHORT( p );

    GXV_TRACE(( "stateSize=0x%04x\n", stateSize ));
    GXV_TRACE(( "offset to classTable=0x%04x\n", classTable ));
    GXV_TRACE(( "offset to stateArray=0x%04x\n", stateArray ));
    GXV_TRACE(( "offset to entryTable=0x%04x\n", entryTable ));

    if ( stateSize > 0xFF )
      FT_INVALID_DATA;

    if ( valid->statetable.optdata_load_func != NULL )
      valid->statetable.optdata_load_func( p, limit, valid );

    if ( valid->statetable.subtable_setup_func != NULL)
      setup_func = valid->statetable.subtable_setup_func;
    else
      setup_func = gxv_StateTable_subtable_setup;

    setup_func( (FT_UShort)( limit - table ),
                classTable,
                stateArray,
                entryTable,
                &classTable_length,
                &stateArray_length,
                &entryTable_length,
                valid );

    GXV_TRACE(( "StateTable Subtables\n" ));

    if ( classTable != 0 )
      gxv_ClassTable_validate( table + classTable,
                               &classTable_length,
                               stateSize,
                               &maxClassID,
                               valid );
    else
      maxClassID = (FT_Byte)( stateSize - 1 );

    if ( stateArray != 0 )
      gxv_StateArray_validate( table + stateArray,
                               &stateArray_length,
                               maxClassID,
                               stateSize,
                               &maxState,
                               &maxEntry,
                               valid );
    else
    {
      maxState = 1;     /* 0:start of text, 1:start of line are predefined */
      maxEntry = 0;
    }

    if ( maxEntry > 0 && entryTable == 0 )
      FT_INVALID_OFFSET;

    if ( entryTable != 0 )
      gxv_EntryTable_validate( table + entryTable,
                               &entryTable_length,
                               maxEntry,
                               stateArray,
                               stateArray_length,
                               maxClassID,
                               table,
                               limit,
                               valid );

    GXV_EXIT;
  }


  /* ================= eXtended State Table (for morx) =================== */

  FT_LOCAL_DEF( void )
  gxv_XStateTable_subtable_setup( FT_ULong       table_size,
                                  FT_ULong       classTable,
                                  FT_ULong       stateArray,
                                  FT_ULong       entryTable,
                                  FT_ULong*      classTable_length_p,
                                  FT_ULong*      stateArray_length_p,
                                  FT_ULong*      entryTable_length_p,
                                  GXV_Validator  valid )
  {
    FT_ULong   o[3];
    FT_ULong*  l[3];
    FT_ULong   buff[4];


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;

    gxv_set_length_by_ulong_offset( o, l, buff, 4, table_size, valid );
  }


  static void
  gxv_XClassTable_lookupval_validate( FT_UShort            glyph,
                                      GXV_LookupValueCPtr  value_p,
                                      GXV_Validator        valid )
  {
    FT_UNUSED( glyph );

    if ( value_p->u >= valid->xstatetable.nClasses )
      FT_INVALID_DATA;
    if ( value_p->u > valid->xstatetable.maxClassID )
      valid->xstatetable.maxClassID = value_p->u;
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
     ....                     |
                              |
    16bit value array         |
    +===============+         |
    |     value     | <-------+
     ....
  */
  static GXV_LookupValueDesc
  gxv_XClassTable_lookupfmt4_transit( FT_UShort            relative_gindex,
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
                          relative_gindex * sizeof ( FT_UShort ) );

    p     = valid->lookuptbl_head + offset;
    limit = lookuptbl_limit;

    GXV_LIMIT_CHECK ( 2 );
    value.u = FT_NEXT_USHORT( p );

    return value;
  }


  static void
  gxv_XStateArray_validate( FT_Bytes       table,
                            FT_ULong*      length_p,
                            FT_UShort      maxClassID,
                            FT_ULong       stateSize,
                            FT_UShort*     maxState_p,
                            FT_UShort*     maxEntry_p,
                            GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_Bytes   limit = table + *length_p;
    FT_UShort  clazz;
    FT_UShort  entry;

    FT_UNUSED( stateSize ); /* for the non-debugging case */


    GXV_NAME_ENTER( "XStateArray" );

    GXV_TRACE(( "parse % 3d bytes by stateSize=% 3d maxClassID=% 3d\n",
                (int)(*length_p), stateSize, (int)(maxClassID) ));

    /*
     * 2 states are predefined and must be described:
     * state 0 (start of text), 1 (start of line)
     */
    GXV_LIMIT_CHECK( ( 1 + maxClassID ) * 2 * 2 );

    *maxState_p = 0;
    *maxEntry_p = 0;

    /* read if enough to read another state */
    while ( p + ( ( 1 + maxClassID ) * 2 ) <= limit )
    {
      (*maxState_p)++;
      for ( clazz = 0; clazz <= maxClassID; clazz++ )
      {
        entry = FT_NEXT_USHORT( p );
        *maxEntry_p = (FT_UShort)FT_MAX( *maxEntry_p, entry );
      }
    }
    GXV_TRACE(( "parsed: maxState=%d, maxEntry=%d\n",
                *maxState_p, *maxEntry_p ));

    *length_p = p - table;

    GXV_EXIT;
  }


  static void
  gxv_XEntryTable_validate( FT_Bytes       table,
                            FT_ULong*      length_p,
                            FT_UShort      maxEntry,
                            FT_ULong       stateArray_length,
                            FT_UShort      maxClassID,
                            FT_Bytes       xstatetable_table,
                            FT_Bytes       xstatetable_limit,
                            GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_Bytes   limit = table + *length_p;
    FT_UShort  entry;
    FT_UShort  state;
    FT_Int     entrySize = 2 + 2 + GXV_GLYPHOFFSET_SIZE( xstatetable );


    GXV_NAME_ENTER( "XEntryTable" );
    GXV_TRACE(( "maxEntry=%d entrySize=%d\n", maxEntry, entrySize ));

    if ( ( p + ( maxEntry + 1 ) * entrySize ) > limit )
      FT_INVALID_TOO_SHORT;

    for (entry = 0; entry <= maxEntry ; entry++ )
    {
      FT_UShort                        newState_idx;
      FT_UShort                        flags;
      GXV_XStateTable_GlyphOffsetDesc  glyphOffset;


      GXV_LIMIT_CHECK( 2 + 2 );
      newState_idx = FT_NEXT_USHORT( p );
      flags        = FT_NEXT_USHORT( p );

      if ( stateArray_length < (FT_ULong)( newState_idx * 2 ) )
      {
        GXV_TRACE(( "  newState index 0x%04x points out of stateArray\n",
                    newState_idx ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_OFFSET;
      }

      state = (FT_UShort)( newState_idx / ( 1 + maxClassID ) );
      if ( 0 != ( newState_idx % ( 1 + maxClassID ) ) )
      {
        FT_TRACE4(( "-> new state = %d (supposed)\n"
                    "but newState index 0x%04x is not aligned to %d-classes\n",
                    state, newState_idx,  1 + maxClassID ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_OFFSET;
      }

      switch ( GXV_GLYPHOFFSET_FMT( xstatetable ) )
      {
      case GXV_GLYPHOFFSET_NONE:
        glyphOffset.uc = 0; /* make compiler happy */
        break;

      case GXV_GLYPHOFFSET_UCHAR:
        glyphOffset.uc = FT_NEXT_BYTE( p );
        break;

      case GXV_GLYPHOFFSET_CHAR:
        glyphOffset.c = FT_NEXT_CHAR( p );
        break;

      case GXV_GLYPHOFFSET_USHORT:
        glyphOffset.u = FT_NEXT_USHORT( p );
        break;

      case GXV_GLYPHOFFSET_SHORT:
        glyphOffset.s = FT_NEXT_SHORT( p );
        break;

      case GXV_GLYPHOFFSET_ULONG:
        glyphOffset.ul = FT_NEXT_ULONG( p );
        break;

      case GXV_GLYPHOFFSET_LONG:
        glyphOffset.l = FT_NEXT_LONG( p );
        break;

      default:
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_FORMAT;
        goto Exit;
      }

      if ( NULL != valid->xstatetable.entry_validate_func )
        valid->xstatetable.entry_validate_func( state,
                                                flags,
                                                &glyphOffset,
                                                xstatetable_table,
                                                xstatetable_limit,
                                                valid );
    }

  Exit:
    *length_p = p - table;

    GXV_EXIT;
  }


  FT_LOCAL_DEF( void )
  gxv_XStateTable_validate( FT_Bytes       table,
                            FT_Bytes       limit,
                            GXV_Validator  valid )
  {
    /* StateHeader members */
    FT_ULong   classTable;      /* offset to Class(Sub)Table */
    FT_ULong   stateArray;      /* offset to StateArray */
    FT_ULong   entryTable;      /* offset to EntryTable */

    FT_ULong   classTable_length;
    FT_ULong   stateArray_length;
    FT_ULong   entryTable_length;
    FT_UShort  maxState;
    FT_UShort  maxEntry;

    GXV_XStateTable_Subtable_Setup_Func  setup_func;

    FT_Bytes   p = table;


    GXV_NAME_ENTER( "XStateTable" );

    GXV_TRACE(( "XStateTable header\n" ));

    GXV_LIMIT_CHECK( 4 + 4 + 4 + 4 );
    valid->xstatetable.nClasses = FT_NEXT_ULONG( p );
    classTable = FT_NEXT_ULONG( p );
    stateArray = FT_NEXT_ULONG( p );
    entryTable = FT_NEXT_ULONG( p );

    GXV_TRACE(( "nClasses =0x%08x\n", valid->xstatetable.nClasses ));
    GXV_TRACE(( "offset to classTable=0x%08x\n", classTable ));
    GXV_TRACE(( "offset to stateArray=0x%08x\n", stateArray ));
    GXV_TRACE(( "offset to entryTable=0x%08x\n", entryTable ));

    if ( valid->xstatetable.nClasses > 0xFFFFU )
      FT_INVALID_DATA;

    GXV_TRACE(( "StateTable Subtables\n" ));

    if ( valid->xstatetable.optdata_load_func != NULL )
      valid->xstatetable.optdata_load_func( p, limit, valid );

    if ( valid->xstatetable.subtable_setup_func != NULL )
      setup_func = valid->xstatetable.subtable_setup_func;
    else
      setup_func = gxv_XStateTable_subtable_setup;

    setup_func( limit - table,
                classTable,
                stateArray,
                entryTable,
                &classTable_length,
                &stateArray_length,
                &entryTable_length,
                valid );

    if ( classTable != 0 )
    {
      valid->xstatetable.maxClassID = 0;
      valid->lookupval_sign         = GXV_LOOKUPVALUE_UNSIGNED;
      valid->lookupval_func         = gxv_XClassTable_lookupval_validate;
      valid->lookupfmt4_trans       = gxv_XClassTable_lookupfmt4_transit;
      gxv_LookupTable_validate( table + classTable,
                                table + classTable + classTable_length,
                                valid );
      if ( valid->subtable_length < classTable_length )
        classTable_length = valid->subtable_length;
    }
    else
    {
      /* XXX: check range? */
      valid->xstatetable.maxClassID =
        (FT_UShort)( valid->xstatetable.nClasses - 1 );
    }

    if ( stateArray != 0 )
      gxv_XStateArray_validate( table + stateArray,
                                &stateArray_length,
                                valid->xstatetable.maxClassID,
                                valid->xstatetable.nClasses,
                                &maxState,
                                &maxEntry,
                                valid );
    else
    {
      maxState = 1; /* 0:start of text, 1:start of line are predefined */
      maxEntry = 0;
    }

    if ( maxEntry > 0 && entryTable == 0 )
      FT_INVALID_OFFSET;

    if ( entryTable != 0 )
      gxv_XEntryTable_validate( table + entryTable,
                                &entryTable_length,
                                maxEntry,
                                stateArray_length,
                                valid->xstatetable.maxClassID,
                                table,
                                limit,
                                valid );

    GXV_EXIT;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        Table overlapping                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static int
  gxv_compare_ranges( FT_Bytes  table1_start,
                      FT_ULong  table1_length,
                      FT_Bytes  table2_start,
                      FT_ULong  table2_length )
  {
    if ( table1_start == table2_start )
    {
      if ( ( table1_length == 0 || table2_length == 0 ) )
        goto Out;
    }
    else if ( table1_start < table2_start )
    {
      if ( ( table1_start + table1_length ) <= table2_start )
        goto Out;
    }
    else if ( table1_start > table2_start )
    {
      if ( ( table1_start >= table2_start + table2_length ) )
        goto Out;
    }
    return 1;

  Out:
    return 0;
  }


  FT_LOCAL_DEF( void )
  gxv_odtect_add_range( FT_Bytes          start,
                        FT_ULong          length,
                        const FT_String*  name,
                        GXV_odtect_Range  odtect )
  {
    odtect->range[ odtect->nRanges ].start  = start;
    odtect->range[ odtect->nRanges ].length = length;
    odtect->range[ odtect->nRanges ].name   = (FT_String*)name;
    odtect->nRanges++;
  }


  FT_LOCAL_DEF( void )
  gxv_odtect_validate( GXV_odtect_Range  odtect,
                       GXV_Validator     valid )
  {
    FT_UInt  i, j;


    GXV_NAME_ENTER( "check overlap among multi ranges" );

    for ( i = 0; i < odtect->nRanges; i++ )
      for ( j = 0; j < i; j++ )
        if ( 0 != gxv_compare_ranges( odtect->range[i].start,
                                      odtect->range[i].length,
                                      odtect->range[j].start,
                                      odtect->range[j].length ) )
        {
          if ( odtect->range[i].name || odtect->range[j].name )
            GXV_TRACE(( "found overlap between range %d and range %d\n",
                        i, j ));
          else
            GXV_TRACE(( "found overlap between `%s' and `%s\'\n",
                        odtect->range[i].name,
                        odtect->range[j].name ));
          FT_INVALID_OFFSET;
        }

    GXV_EXIT;
  }


/* END */
