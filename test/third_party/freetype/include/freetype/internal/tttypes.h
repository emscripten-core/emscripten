/***************************************************************************/
/*                                                                         */
/*  tttypes.h                                                              */
/*                                                                         */
/*    Basic SFNT/TrueType type definitions and interface (specification    */
/*    only).                                                               */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2004, 2005, 2006, 2007, 2008 by             */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTTYPES_H__
#define __TTTYPES_H__


#include <ft2build.h>
#include FT_TRUETYPE_TABLES_H
#include FT_INTERNAL_OBJECTS_H

#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
#include FT_MULTIPLE_MASTERS_H
#endif


FT_BEGIN_HEADER


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***             REQUIRED TRUETYPE/OPENTYPE TABLES DEFINITIONS         ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TTC_HeaderRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    TrueType collection header.  This table contains the offsets of    */
  /*    the font headers of each distinct TrueType face in the file.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    tag     :: Must be `ttc ' to indicate a TrueType collection.       */
  /*                                                                       */
  /*    version :: The version number.                                     */
  /*                                                                       */
  /*    count   :: The number of faces in the collection.  The             */
  /*               specification says this should be an unsigned long, but */
  /*               we use a signed long since we need the value -1 for     */
  /*               specific purposes.                                      */
  /*                                                                       */
  /*    offsets :: The offsets of the font headers, one per face.          */
  /*                                                                       */
  typedef struct  TTC_HeaderRec_
  {
    FT_ULong   tag;
    FT_Fixed   version;
    FT_Long    count;
    FT_ULong*  offsets;

  } TTC_HeaderRec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    SFNT_HeaderRec                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    SFNT file format header.                                           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    format_tag     :: The font format tag.                             */
  /*                                                                       */
  /*    num_tables     :: The number of tables in file.                    */
  /*                                                                       */
  /*    search_range   :: Must be `16 * (max power of 2 <= num_tables)'.   */
  /*                                                                       */
  /*    entry_selector :: Must be log2 of `search_range / 16'.             */
  /*                                                                       */
  /*    range_shift    :: Must be `num_tables * 16 - search_range'.        */
  /*                                                                       */
  typedef struct  SFNT_HeaderRec_
  {
    FT_ULong   format_tag;
    FT_UShort  num_tables;
    FT_UShort  search_range;
    FT_UShort  entry_selector;
    FT_UShort  range_shift;

    FT_ULong   offset;  /* not in file */

  } SFNT_HeaderRec, *SFNT_Header;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_TableRec                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure describes a given table of a TrueType font.         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    Tag      :: A four-bytes tag describing the table.                 */
  /*                                                                       */
  /*    CheckSum :: The table checksum.  This value can be ignored.        */
  /*                                                                       */
  /*    Offset   :: The offset of the table from the start of the TrueType */
  /*                font in its resource.                                  */
  /*                                                                       */
  /*    Length   :: The table length (in bytes).                           */
  /*                                                                       */
  typedef struct  TT_TableRec_
  {
    FT_ULong  Tag;        /*        table type */
    FT_ULong  CheckSum;   /*    table checksum */
    FT_ULong  Offset;     /* table file offset */
    FT_ULong  Length;     /*      table length */

  } TT_TableRec, *TT_Table;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_LongMetricsRec                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling the long metrics of the `hmtx' and `vmtx'     */
  /*    TrueType tables.  The values are expressed in font units.          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    advance :: The advance width or height for the glyph.              */
  /*                                                                       */
  /*    bearing :: The left-side or top-side bearing for the glyph.        */
  /*                                                                       */
  typedef struct  TT_LongMetricsRec_
  {
    FT_UShort  advance;
    FT_Short   bearing;

  } TT_LongMetricsRec, *TT_LongMetrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_ShortMetrics                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple type to model the short metrics of the `hmtx' and `vmtx'  */
  /*    tables.                                                            */
  /*                                                                       */
  typedef FT_Short  TT_ShortMetrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_NameEntryRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling TrueType name records.  Name records are used */
  /*    to store important strings like family name, style name,           */
  /*    copyright, etc. in _localized_ versions (i.e., language, encoding, */
  /*    etc).                                                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    platformID   :: The ID of the name's encoding platform.            */
  /*                                                                       */
  /*    encodingID   :: The platform-specific ID for the name's encoding.  */
  /*                                                                       */
  /*    languageID   :: The platform-specific ID for the name's language.  */
  /*                                                                       */
  /*    nameID       :: The ID specifying what kind of name this is.       */
  /*                                                                       */
  /*    stringLength :: The length of the string in bytes.                 */
  /*                                                                       */
  /*    stringOffset :: The offset to the string in the `name' table.      */
  /*                                                                       */
  /*    string       :: A pointer to the string's bytes.  Note that these  */
  /*                    are usually UTF-16 encoded characters.             */
  /*                                                                       */
  typedef struct  TT_NameEntryRec_
  {
    FT_UShort  platformID;
    FT_UShort  encodingID;
    FT_UShort  languageID;
    FT_UShort  nameID;
    FT_UShort  stringLength;
    FT_ULong   stringOffset;

    /* this last field is not defined in the spec */
    /* but used by the FreeType engine            */

    FT_Byte*   string;

  } TT_NameEntryRec, *TT_NameEntry;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_NameTableRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling the TrueType name table.                      */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    format         :: The format of the name table.                    */
  /*                                                                       */
  /*    numNameRecords :: The number of names in table.                    */
  /*                                                                       */
  /*    storageOffset  :: The offset of the name table in the `name'       */
  /*                      TrueType table.                                  */
  /*                                                                       */
  /*    names          :: An array of name records.                        */
  /*                                                                       */
  /*    stream         :: the file's input stream.                         */
  /*                                                                       */
  typedef struct  TT_NameTableRec_
  {
    FT_UShort         format;
    FT_UInt           numNameRecords;
    FT_UInt           storageOffset;
    TT_NameEntryRec*  names;
    FT_Stream         stream;

  } TT_NameTableRec, *TT_NameTable;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***             OPTIONAL TRUETYPE/OPENTYPE TABLES DEFINITIONS         ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_GaspRangeRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A tiny structure used to model a gasp range according to the       */
  /*    TrueType specification.                                            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    maxPPEM  :: The maximum ppem value to which `gaspFlag' applies.    */
  /*                                                                       */
  /*    gaspFlag :: A flag describing the grid-fitting and anti-aliasing   */
  /*                modes to be used.                                      */
  /*                                                                       */
  typedef struct  TT_GaspRangeRec_
  {
    FT_UShort  maxPPEM;
    FT_UShort  gaspFlag;

  } TT_GaspRangeRec, *TT_GaspRange;


#define TT_GASP_GRIDFIT  0x01
#define TT_GASP_DOGRAY   0x02


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_GaspRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure modeling the TrueType `gasp' table used to specify     */
  /*    grid-fitting and anti-aliasing behaviour.                          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    version    :: The version number.                                  */
  /*                                                                       */
  /*    numRanges  :: The number of gasp ranges in table.                  */
  /*                                                                       */
  /*    gaspRanges :: An array of gasp ranges.                             */
  /*                                                                       */
  typedef struct  TT_Gasp_
  {
    FT_UShort     version;
    FT_UShort     numRanges;
    TT_GaspRange  gaspRanges;

  } TT_GaspRec;


#ifdef FT_CONFIG_OPTION_OLD_INTERNALS

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_HdmxEntryRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A small structure used to model the pre-computed widths of a given */
  /*    size.  They are found in the `hdmx' table.                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    ppem      :: The pixels per EM value at which these metrics apply. */
  /*                                                                       */
  /*    max_width :: The maximum advance width for this metric.            */
  /*                                                                       */
  /*    widths    :: An array of widths.  Note: These are 8-bit bytes.     */
  /*                                                                       */
  typedef struct  TT_HdmxEntryRec_
  {
    FT_Byte   ppem;
    FT_Byte   max_width;
    FT_Byte*  widths;

  } TT_HdmxEntryRec, *TT_HdmxEntry;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_HdmxRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model the `hdmx' table, which contains         */
  /*    pre-computed widths for a set of given sizes/dimensions.           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    version     :: The version number.                                 */
  /*                                                                       */
  /*    num_records :: The number of hdmx records.                         */
  /*                                                                       */
  /*    records     :: An array of hdmx records.                           */
  /*                                                                       */
  typedef struct  TT_HdmxRec_
  {
    FT_UShort     version;
    FT_Short      num_records;
    TT_HdmxEntry  records;

  } TT_HdmxRec, *TT_Hdmx;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Kern0_PairRec                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a kerning pair for the kerning table     */
  /*    format 0.  The engine now loads this table if it finds one in the  */
  /*    font file.                                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    left  :: The index of the left glyph in pair.                      */
  /*                                                                       */
  /*    right :: The index of the right glyph in pair.                     */
  /*                                                                       */
  /*    value :: The kerning distance.  A positive value spaces the        */
  /*             glyphs, a negative one makes them closer.                 */
  /*                                                                       */
  typedef struct  TT_Kern0_PairRec_
  {
    FT_UShort  left;   /* index of left  glyph in pair */
    FT_UShort  right;  /* index of right glyph in pair */
    FT_FWord   value;  /* kerning value                */

  } TT_Kern0_PairRec, *TT_Kern0_Pair;

#endif /* FT_CONFIG_OPTION_OLD_INTERNALS */


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                    EMBEDDED BITMAPS SUPPORT                       ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_MetricsRec                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold the big metrics of a given glyph bitmap   */
  /*    in a TrueType or OpenType font.  These are usually found in the    */
  /*    `EBDT' (Microsoft) or `bloc' (Apple) table.                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    height       :: The glyph height in pixels.                        */
  /*                                                                       */
  /*    width        :: The glyph width in pixels.                         */
  /*                                                                       */
  /*    horiBearingX :: The horizontal left bearing.                       */
  /*                                                                       */
  /*    horiBearingY :: The horizontal top bearing.                        */
  /*                                                                       */
  /*    horiAdvance  :: The horizontal advance.                            */
  /*                                                                       */
  /*    vertBearingX :: The vertical left bearing.                         */
  /*                                                                       */
  /*    vertBearingY :: The vertical top bearing.                          */
  /*                                                                       */
  /*    vertAdvance  :: The vertical advance.                              */
  /*                                                                       */
  typedef struct  TT_SBit_MetricsRec_
  {
    FT_Byte  height;
    FT_Byte  width;

    FT_Char  horiBearingX;
    FT_Char  horiBearingY;
    FT_Byte  horiAdvance;

    FT_Char  vertBearingX;
    FT_Char  vertBearingY;
    FT_Byte  vertAdvance;

  } TT_SBit_MetricsRec, *TT_SBit_Metrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_SmallMetricsRec                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold the small metrics of a given glyph bitmap */
  /*    in a TrueType or OpenType font.  These are usually found in the    */
  /*    `EBDT' (Microsoft) or the `bdat' (Apple) table.                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    height   :: The glyph height in pixels.                            */
  /*                                                                       */
  /*    width    :: The glyph width in pixels.                             */
  /*                                                                       */
  /*    bearingX :: The left-side bearing.                                 */
  /*                                                                       */
  /*    bearingY :: The top-side bearing.                                  */
  /*                                                                       */
  /*    advance  :: The advance width or height.                           */
  /*                                                                       */
  typedef struct  TT_SBit_Small_Metrics_
  {
    FT_Byte  height;
    FT_Byte  width;

    FT_Char  bearingX;
    FT_Char  bearingY;
    FT_Byte  advance;

  } TT_SBit_SmallMetricsRec, *TT_SBit_SmallMetrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_LineMetricsRec                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to describe the text line metrics of a given      */
  /*    bitmap strike, for either a horizontal or vertical layout.         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    ascender                :: The ascender in pixels.                 */
  /*                                                                       */
  /*    descender               :: The descender in pixels.                */
  /*                                                                       */
  /*    max_width               :: The maximum glyph width in pixels.      */
  /*                                                                       */
  /*    caret_slope_enumerator  :: Rise of the caret slope, typically set  */
  /*                               to 1 for non-italic fonts.              */
  /*                                                                       */
  /*    caret_slope_denominator :: Rise of the caret slope, typically set  */
  /*                               to 0 for non-italic fonts.              */
  /*                                                                       */
  /*    caret_offset            :: Offset in pixels to move the caret for  */
  /*                               proper positioning.                     */
  /*                                                                       */
  /*    min_origin_SB           :: Minimum of horiBearingX (resp.          */
  /*                               vertBearingY).                          */
  /*    min_advance_SB          :: Minimum of                              */
  /*                                                                       */
  /*                                 horizontal advance -                  */
  /*                                   ( horiBearingX + width )            */
  /*                                                                       */
  /*                               resp.                                   */
  /*                                                                       */
  /*                                 vertical advance -                    */
  /*                                   ( vertBearingY + height )           */
  /*                                                                       */
  /*    max_before_BL           :: Maximum of horiBearingY (resp.          */
  /*                               vertBearingY).                          */
  /*                                                                       */
  /*    min_after_BL            :: Minimum of                              */
  /*                                                                       */
  /*                                 horiBearingY - height                 */
  /*                                                                       */
  /*                               resp.                                   */
  /*                                                                       */
  /*                                 vertBearingX - width                  */
  /*                                                                       */
  /*    pads                    :: Unused (to make the size of the record  */
  /*                               a multiple of 32 bits.                  */
  /*                                                                       */
  typedef struct  TT_SBit_LineMetricsRec_
  {
    FT_Char  ascender;
    FT_Char  descender;
    FT_Byte  max_width;
    FT_Char  caret_slope_numerator;
    FT_Char  caret_slope_denominator;
    FT_Char  caret_offset;
    FT_Char  min_origin_SB;
    FT_Char  min_advance_SB;
    FT_Char  max_before_BL;
    FT_Char  min_after_BL;
    FT_Char  pads[2];

  } TT_SBit_LineMetricsRec, *TT_SBit_LineMetrics;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_RangeRec                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A TrueType/OpenType subIndexTable as defined in the `EBLC'         */
  /*    (Microsoft) or `bloc' (Apple) tables.                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    first_glyph   :: The first glyph index in the range.               */
  /*                                                                       */
  /*    last_glyph    :: The last glyph index in the range.                */
  /*                                                                       */
  /*    index_format  :: The format of index table.  Valid values are 1    */
  /*                     to 5.                                             */
  /*                                                                       */
  /*    image_format  :: The format of `EBDT' image data.                  */
  /*                                                                       */
  /*    image_offset  :: The offset to image data in `EBDT'.               */
  /*                                                                       */
  /*    image_size    :: For index formats 2 and 5.  This is the size in   */
  /*                     bytes of each glyph bitmap.                       */
  /*                                                                       */
  /*    big_metrics   :: For index formats 2 and 5.  This is the big       */
  /*                     metrics for each glyph bitmap.                    */
  /*                                                                       */
  /*    num_glyphs    :: For index formats 4 and 5.  This is the number of */
  /*                     glyphs in the code array.                         */
  /*                                                                       */
  /*    glyph_offsets :: For index formats 1 and 3.                        */
  /*                                                                       */
  /*    glyph_codes   :: For index formats 4 and 5.                        */
  /*                                                                       */
  /*    table_offset  :: The offset of the index table in the `EBLC'       */
  /*                     table.  Only used during strike loading.          */
  /*                                                                       */
  typedef struct  TT_SBit_RangeRec_
  {
    FT_UShort           first_glyph;
    FT_UShort           last_glyph;

    FT_UShort           index_format;
    FT_UShort           image_format;
    FT_ULong            image_offset;

    FT_ULong            image_size;
    TT_SBit_MetricsRec  metrics;
    FT_ULong            num_glyphs;

    FT_ULong*           glyph_offsets;
    FT_UShort*          glyph_codes;

    FT_ULong            table_offset;

  } TT_SBit_RangeRec, *TT_SBit_Range;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_StrikeRec                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used describe a given bitmap strike in the `EBLC'      */
  /*    (Microsoft) or `bloc' (Apple) tables.                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*   num_index_ranges :: The number of index ranges.                     */
  /*                                                                       */
  /*   index_ranges     :: An array of glyph index ranges.                 */
  /*                                                                       */
  /*   color_ref        :: Unused.  `color_ref' is put in for future       */
  /*                       enhancements, but these fields are already      */
  /*                       in use by other platforms (e.g. Newton).        */
  /*                       For details, please see                         */
  /*                                                                       */
  /*                         http://fonts.apple.com/                       */
  /*                                TTRefMan/RM06/Chap6bloc.html           */
  /*                                                                       */
  /*   hori             :: The line metrics for horizontal layouts.        */
  /*                                                                       */
  /*   vert             :: The line metrics for vertical layouts.          */
  /*                                                                       */
  /*   start_glyph      :: The lowest glyph index for this strike.         */
  /*                                                                       */
  /*   end_glyph        :: The highest glyph index for this strike.        */
  /*                                                                       */
  /*   x_ppem           :: The number of horizontal pixels per EM.         */
  /*                                                                       */
  /*   y_ppem           :: The number of vertical pixels per EM.           */
  /*                                                                       */
  /*   bit_depth        :: The bit depth.  Valid values are 1, 2, 4,       */
  /*                       and 8.                                          */
  /*                                                                       */
  /*   flags            :: Is this a vertical or horizontal strike?  For   */
  /*                       details, please see                             */
  /*                                                                       */
  /*                         http://fonts.apple.com/                       */
  /*                                TTRefMan/RM06/Chap6bloc.html           */
  /*                                                                       */
  typedef struct  TT_SBit_StrikeRec_
  {
    FT_Int                  num_ranges;
    TT_SBit_Range           sbit_ranges;
    FT_ULong                ranges_offset;

    FT_ULong                color_ref;

    TT_SBit_LineMetricsRec  hori;
    TT_SBit_LineMetricsRec  vert;

    FT_UShort               start_glyph;
    FT_UShort               end_glyph;

    FT_Byte                 x_ppem;
    FT_Byte                 y_ppem;

    FT_Byte                 bit_depth;
    FT_Char                 flags;

  } TT_SBit_StrikeRec, *TT_SBit_Strike;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_ComponentRec                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure to describe a compound sbit element.            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    glyph_code :: The element's glyph index.                           */
  /*                                                                       */
  /*    x_offset   :: The element's left bearing.                          */
  /*                                                                       */
  /*    y_offset   :: The element's top bearing.                           */
  /*                                                                       */
  typedef struct  TT_SBit_ComponentRec_
  {
    FT_UShort  glyph_code;
    FT_Char    x_offset;
    FT_Char    y_offset;

  } TT_SBit_ComponentRec, *TT_SBit_Component;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_SBit_ScaleRec                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used describe a given bitmap scaling table, as defined */
  /*    in the `EBSC' table.                                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    hori              :: The horizontal line metrics.                  */
  /*                                                                       */
  /*    vert              :: The vertical line metrics.                    */
  /*                                                                       */
  /*    x_ppem            :: The number of horizontal pixels per EM.       */
  /*                                                                       */
  /*    y_ppem            :: The number of vertical pixels per EM.         */
  /*                                                                       */
  /*    x_ppem_substitute :: Substitution x_ppem value.                    */
  /*                                                                       */
  /*    y_ppem_substitute :: Substitution y_ppem value.                    */
  /*                                                                       */
  typedef struct  TT_SBit_ScaleRec_
  {
    TT_SBit_LineMetricsRec  hori;
    TT_SBit_LineMetricsRec  vert;

    FT_Byte                 x_ppem;
    FT_Byte                 y_ppem;

    FT_Byte                 x_ppem_substitute;
    FT_Byte                 y_ppem_substitute;

  } TT_SBit_ScaleRec, *TT_SBit_Scale;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                  POSTSCRIPT GLYPH NAMES SUPPORT                   ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Post_20Rec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Postscript names sub-table, format 2.0.  Stores the PS name of     */
  /*    each glyph in the font face.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_glyphs    :: The number of named glyphs in the table.          */
  /*                                                                       */
  /*    num_names     :: The number of PS names stored in the table.       */
  /*                                                                       */
  /*    glyph_indices :: The indices of the glyphs in the names arrays.    */
  /*                                                                       */
  /*    glyph_names   :: The PS names not in Mac Encoding.                 */
  /*                                                                       */
  typedef struct  TT_Post_20Rec_
  {
    FT_UShort   num_glyphs;
    FT_UShort   num_names;
    FT_UShort*  glyph_indices;
    FT_Char**   glyph_names;

  } TT_Post_20Rec, *TT_Post_20;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Post_25Rec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Postscript names sub-table, format 2.5.  Stores the PS name of     */
  /*    each glyph in the font face.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_glyphs :: The number of glyphs in the table.                   */
  /*                                                                       */
  /*    offsets    :: An array of signed offsets in a normal Mac           */
  /*                  Postscript name encoding.                            */
  /*                                                                       */
  typedef struct  TT_Post_25_
  {
    FT_UShort  num_glyphs;
    FT_Char*   offsets;

  } TT_Post_25Rec, *TT_Post_25;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Post_NamesRec                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Postscript names table, either format 2.0 or 2.5.                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    loaded    :: A flag to indicate whether the PS names are loaded.   */
  /*                                                                       */
  /*    format_20 :: The sub-table used for format 2.0.                    */
  /*                                                                       */
  /*    format_25 :: The sub-table used for format 2.5.                    */
  /*                                                                       */
  typedef struct  TT_Post_NamesRec_
  {
    FT_Bool  loaded;

    union
    {
      TT_Post_20Rec  format_20;
      TT_Post_25Rec  format_25;

    } names;

  } TT_Post_NamesRec, *TT_Post_Names;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                    GX VARIATION TABLE SUPPORT                     ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
  typedef struct GX_BlendRec_  *GX_Blend;
#endif

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***              EMBEDDED BDF PROPERTIES TABLE SUPPORT                ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*
   * These types are used to support a `BDF ' table that isn't part of the
   * official TrueType specification.  It is mainly used in SFNT-based
   * bitmap fonts that were generated from a set of BDF fonts.
   *
   * The format of the table is as follows.
   *
   *   USHORT   version      `BDF ' table version number, should be 0x0001.
   *   USHORT   strikeCount  Number of strikes (bitmap sizes) in this table.
   *   ULONG    stringTable  Offset (from start of BDF table) to string
   *                         table.
   *
   * This is followed by an array of `strikeCount' descriptors, having the
   * following format.
   *
   *   USHORT   ppem         Vertical pixels per EM for this strike.
   *   USHORT   numItems     Number of items for this strike (properties and
   *                         atoms).  Maximum is 255.
   *
   * This array in turn is followed by `strikeCount' value sets.  Each
   * `value set' is an array of `numItems' items with the following format.
   *
   *   ULONG    item_name    Offset in string table to item name.
   *   USHORT   item_type    The item type.  Possible values are
   *                            0 => string (e.g., COMMENT)
   *                            1 => atom   (e.g., FONT or even SIZE)
   *                            2 => int32
   *                            3 => uint32
   *                         0x10 => A flag to indicate a properties.  This
   *                                 is ORed with the above values.
   *   ULONG    item_value   For strings  => Offset into string table without
   *                                         the corresponding double quotes.
   *                         For atoms    => Offset into string table.
   *                         For integers => Direct value.
   *
   * All strings in the string table consist of bytes and are
   * zero-terminated.
   *
   */

#ifdef TT_CONFIG_OPTION_BDF

  typedef struct  TT_BDFRec_
  {
    FT_Byte*   table;
    FT_Byte*   table_end;
    FT_Byte*   strings;
    FT_ULong   strings_size;
    FT_UInt    num_strikes;
    FT_Bool    loaded;

  } TT_BDFRec, *TT_BDF;

#endif /* TT_CONFIG_OPTION_BDF */

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                  ORIGINAL TT_FACE CLASS DEFINITION                ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This structure/class is defined here because it is common to the      */
  /* following formats: TTF, OpenType-TT, and OpenType-CFF.                */
  /*                                                                       */
  /* Note, however, that the classes TT_Size and TT_GlyphSlot are not      */
  /* shared between font drivers, and are thus defined in `ttobjs.h'.      */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    TT_Face                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a TrueType face/font object.  A TT_Face encapsulates   */
  /*    the resolution and scaling independent parts of a TrueType font    */
  /*    resource.                                                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The TT_Face structure is also used as a `parent class' for the     */
  /*    OpenType-CFF class (T2_Face).                                      */
  /*                                                                       */
  typedef struct TT_FaceRec_*  TT_Face;


  /* a function type used for the truetype bytecode interpreter hooks */
  typedef FT_Error
  (*TT_Interpreter)( void*  exec_context );

  /* forward declaration */
  typedef struct TT_LoaderRec_*  TT_Loader;


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Loader_GotoTableFunc                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Seeks a stream to the start of a given TrueType table.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*                                                                       */
  /*    tag    :: A 4-byte tag used to name the table.                     */
  /*                                                                       */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    length :: The length of the table in bytes.  Set to 0 if not       */
  /*              needed.                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream cursor must be at the font file's origin.               */
  /*                                                                       */
  typedef FT_Error
  (*TT_Loader_GotoTableFunc)( TT_Face    face,
                              FT_ULong   tag,
                              FT_Stream  stream,
                              FT_ULong*  length );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Loader_StartGlyphFunc                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Seeks a stream to the start of a given glyph element, and opens a  */
  /*    frame for it.                                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    loader      :: The current TrueType glyph loader object.           */
  /*                                                                       */
  /*    glyph index :: The index of the glyph to access.                   */
  /*                                                                       */
  /*    offset      :: The offset of the glyph according to the            */
  /*                   `locations' table.                                  */
  /*                                                                       */
  /*    byte_count  :: The size of the frame in bytes.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function is normally equivalent to FT_STREAM_SEEK(offset)     */
  /*    followed by FT_FRAME_ENTER(byte_count) with the loader's stream,   */
  /*    but alternative formats (e.g. compressed ones) might use something */
  /*    different.                                                         */
  /*                                                                       */
  typedef FT_Error
  (*TT_Loader_StartGlyphFunc)( TT_Loader  loader,
                               FT_UInt    glyph_index,
                               FT_ULong   offset,
                               FT_UInt    byte_count );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Loader_ReadGlyphFunc                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Reads one glyph element (its header, a simple glyph, or a          */
  /*    composite) from the loader's current stream frame.                 */
  /*                                                                       */
  /* <Input>                                                               */
  /*    loader :: The current TrueType glyph loader object.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0 means success.                             */
  /*                                                                       */
  typedef FT_Error
  (*TT_Loader_ReadGlyphFunc)( TT_Loader  loader );


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    TT_Loader_EndGlyphFunc                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Closes the current loader stream frame for the glyph.              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    loader :: The current TrueType glyph loader object.                */
  /*                                                                       */
  typedef void
  (*TT_Loader_EndGlyphFunc)( TT_Loader  loader );


  /*************************************************************************/
  /*                                                                       */
  /*                         TrueType Face Type                            */
  /*                                                                       */
  /* <Struct>                                                              */
  /*    TT_Face                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The TrueType face class.  These objects model the resolution and   */
  /*    point-size independent data found in a TrueType font file.         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root                 :: The base FT_Face structure, managed by the */
  /*                            base layer.                                */
  /*                                                                       */
  /*    ttc_header           :: The TrueType collection header, used when  */
  /*                            the file is a `ttc' rather than a `ttf'.   */
  /*                            For ordinary font files, the field         */
  /*                            `ttc_header.count' is set to 0.            */
  /*                                                                       */
  /*    format_tag           :: The font format tag.                       */
  /*                                                                       */
  /*    num_tables           :: The number of TrueType tables in this font */
  /*                            file.                                      */
  /*                                                                       */
  /*    dir_tables           :: The directory of TrueType tables for this  */
  /*                            font file.                                 */
  /*                                                                       */
  /*    header               :: The font's font header (`head' table).     */
  /*                            Read on font opening.                      */
  /*                                                                       */
  /*    horizontal           :: The font's horizontal header (`hhea'       */
  /*                            table).  This field also contains the      */
  /*                            associated horizontal metrics table        */
  /*                            (`hmtx').                                  */
  /*                                                                       */
  /*    max_profile          :: The font's maximum profile table.  Read on */
  /*                            font opening.  Note that some maximum      */
  /*                            values cannot be taken directly from this  */
  /*                            table.  We thus define additional fields   */
  /*                            below to hold the computed maxima.         */
  /*                                                                       */
  /*    vertical_info        :: A boolean which is set when the font file  */
  /*                            contains vertical metrics.  If not, the    */
  /*                            value of the `vertical' field is           */
  /*                            undefined.                                 */
  /*                                                                       */
  /*    vertical             :: The font's vertical header (`vhea' table). */
  /*                            This field also contains the associated    */
  /*                            vertical metrics table (`vmtx'), if found. */
  /*                            IMPORTANT: The contents of this field is   */
  /*                            undefined if the `verticalInfo' field is   */
  /*                            unset.                                     */
  /*                                                                       */
  /*    num_names            :: The number of name records within this     */
  /*                            TrueType font.                             */
  /*                                                                       */
  /*    name_table           :: The table of name records (`name').        */
  /*                                                                       */
  /*    os2                  :: The font's OS/2 table (`OS/2').            */
  /*                                                                       */
  /*    postscript           :: The font's PostScript table (`post'        */
  /*                            table).  The PostScript glyph names are    */
  /*                            not loaded by the driver on face opening.  */
  /*                            See the `ttpost' module for more details.  */
  /*                                                                       */
  /*    cmap_table           :: Address of the face's `cmap' SFNT table    */
  /*                            in memory (it's an extracted frame).       */
  /*                                                                       */
  /*    cmap_size            :: The size in bytes of the `cmap_table'      */
  /*                            described above.                           */
  /*                                                                       */
  /*    goto_table           :: A function called by each TrueType table   */
  /*                            loader to position a stream's cursor to    */
  /*                            the start of a given table according to    */
  /*                            its tag.  It defaults to TT_Goto_Face but  */
  /*                            can be different for strange formats (e.g. */
  /*                            Type 42).                                  */
  /*                                                                       */
  /*    access_glyph_frame   :: A function used to access the frame of a   */
  /*                            given glyph within the face's font file.   */
  /*                                                                       */
  /*    forget_glyph_frame   :: A function used to forget the frame of a   */
  /*                            given glyph when all data has been loaded. */
  /*                                                                       */
  /*    read_glyph_header    :: A function used to read a glyph header.    */
  /*                            It must be called between an `access' and  */
  /*                            `forget'.                                  */
  /*                                                                       */
  /*    read_simple_glyph    :: A function used to read a simple glyph.    */
  /*                            It must be called after the header was     */
  /*                            read, and before the `forget'.             */
  /*                                                                       */
  /*    read_composite_glyph :: A function used to read a composite glyph. */
  /*                            It must be called after the header was     */
  /*                            read, and before the `forget'.             */
  /*                                                                       */
  /*    sfnt                 :: A pointer to the SFNT service.             */
  /*                                                                       */
  /*    psnames              :: A pointer to the PostScript names service. */
  /*                                                                       */
  /*    hdmx                 :: The face's horizontal device metrics       */
  /*                            (`hdmx' table).  This table is optional in */
  /*                            TrueType/OpenType fonts.                   */
  /*                                                                       */
  /*    gasp                 :: The grid-fitting and scaling properties    */
  /*                            table (`gasp').  This table is optional in */
  /*                            TrueType/OpenType fonts.                   */
  /*                                                                       */
  /*    pclt                 :: The `pclt' SFNT table.                     */
  /*                                                                       */
  /*    num_sbit_strikes     :: The number of sbit strikes, i.e., bitmap   */
  /*                            sizes, embedded in this font.              */
  /*                                                                       */
  /*    sbit_strikes         :: An array of sbit strikes embedded in this  */
  /*                            font.  This table is optional in a         */
  /*                            TrueType/OpenType font.                    */
  /*                                                                       */
  /*    num_sbit_scales      :: The number of sbit scales for this font.   */
  /*                                                                       */
  /*    sbit_scales          :: Array of sbit scales embedded in this      */
  /*                            font.  This table is optional in a         */
  /*                            TrueType/OpenType font.                    */
  /*                                                                       */
  /*    postscript_names     :: A table used to store the Postscript names */
  /*                            of  the glyphs for this font.  See the     */
  /*                            file  `ttconfig.h' for comments on the     */
  /*                            TT_CONFIG_OPTION_POSTSCRIPT_NAMES option.  */
  /*                                                                       */
  /*    num_locations        :: The number of glyph locations in this      */
  /*                            TrueType file.  This should be             */
  /*                            identical to the number of glyphs.         */
  /*                            Ignored for Type 2 fonts.                  */
  /*                                                                       */
  /*    glyph_locations      :: An array of longs.  These are offsets to   */
  /*                            glyph data within the `glyf' table.        */
  /*                            Ignored for Type 2 font faces.             */
  /*                                                                       */
  /*    glyf_len             :: The length of the `glyf' table.  Needed    */
  /*                            for malformed `loca' tables.               */
  /*                                                                       */
  /*    font_program_size    :: Size in bytecodes of the face's font       */
  /*                            program.  0 if none defined.  Ignored for  */
  /*                            Type 2 fonts.                              */
  /*                                                                       */
  /*    font_program         :: The face's font program (bytecode stream)  */
  /*                            executed at load time, also used during    */
  /*                            glyph rendering.  Comes from the `fpgm'    */
  /*                            table.  Ignored for Type 2 font fonts.     */
  /*                                                                       */
  /*    cvt_program_size     :: The size in bytecodes of the face's cvt    */
  /*                            program.  Ignored for Type 2 fonts.        */
  /*                                                                       */
  /*    cvt_program          :: The face's cvt program (bytecode stream)   */
  /*                            executed each time an instance/size is     */
  /*                            changed/reset.  Comes from the `prep'      */
  /*                            table.  Ignored for Type 2 fonts.          */
  /*                                                                       */
  /*    cvt_size             :: Size of the control value table (in        */
  /*                            entries).   Ignored for Type 2 fonts.      */
  /*                                                                       */
  /*    cvt                  :: The face's original control value table.   */
  /*                            Coordinates are expressed in unscaled font */
  /*                            units.  Comes from the `cvt ' table.       */
  /*                            Ignored for Type 2 fonts.                  */
  /*                                                                       */
  /*    num_kern_pairs       :: The number of kerning pairs present in the */
  /*                            font file.  The engine only loads the      */
  /*                            first horizontal format 0 kern table it    */
  /*                            finds in the font file.  Ignored for       */
  /*                            Type 2 fonts.                              */
  /*                                                                       */
  /*    kern_table_index     :: The index of the kerning table in the font */
  /*                            kerning directory.  Ignored for Type 2     */
  /*                            fonts.                                     */
  /*                                                                       */
  /*    interpreter          :: A pointer to the TrueType bytecode         */
  /*                            interpreters field is also used to hook    */
  /*                            the debugger in `ttdebug'.                 */
  /*                                                                       */
  /*    unpatented_hinting   :: If true, use only unpatented methods in    */
  /*                            the bytecode interpreter.                  */
  /*                                                                       */
  /*    doblend              :: A boolean which is set if the font should  */
  /*                            be blended (this is for GX var).           */
  /*                                                                       */
  /*    blend                :: Contains the data needed to control GX     */
  /*                            variation tables (rather like Multiple     */
  /*                            Master data).                              */
  /*                                                                       */
  /*    extra                :: Reserved for third-party font drivers.     */
  /*                                                                       */
  /*    postscript_name      :: The PS name of the font.  Used by the      */
  /*                            postscript name service.                   */
  /*                                                                       */
  typedef struct  TT_FaceRec_
  {
    FT_FaceRec            root;

    TTC_HeaderRec         ttc_header;

    FT_ULong              format_tag;
    FT_UShort             num_tables;
    TT_Table              dir_tables;

    TT_Header             header;       /* TrueType header table          */
    TT_HoriHeader         horizontal;   /* TrueType horizontal header     */

    TT_MaxProfile         max_profile;
#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
    FT_ULong              max_components;  /* stubbed to 0 */
#endif

    FT_Bool               vertical_info;
    TT_VertHeader         vertical;     /* TT Vertical header, if present */

    FT_UShort             num_names;    /* number of name records  */
    TT_NameTableRec       name_table;   /* name table              */

    TT_OS2                os2;          /* TrueType OS/2 table            */
    TT_Postscript         postscript;   /* TrueType Postscript table      */

    FT_Byte*              cmap_table;   /* extracted `cmap' table */
    FT_ULong              cmap_size;

    TT_Loader_GotoTableFunc   goto_table;

    TT_Loader_StartGlyphFunc  access_glyph_frame;
    TT_Loader_EndGlyphFunc    forget_glyph_frame;
    TT_Loader_ReadGlyphFunc   read_glyph_header;
    TT_Loader_ReadGlyphFunc   read_simple_glyph;
    TT_Loader_ReadGlyphFunc   read_composite_glyph;

    /* a typeless pointer to the SFNT_Interface table used to load */
    /* the basic TrueType tables in the face object                */
    void*                 sfnt;

    /* a typeless pointer to the FT_Service_PsCMapsRec table used to */
    /* handle glyph names <-> unicode & Mac values                   */
    void*                 psnames;


    /***********************************************************************/
    /*                                                                     */
    /* Optional TrueType/OpenType tables                                   */
    /*                                                                     */
    /***********************************************************************/

    /* horizontal device metrics */
#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
    TT_HdmxRec            hdmx;
#endif

    /* grid-fitting and scaling table */
    TT_GaspRec            gasp;                 /* the `gasp' table */

    /* PCL 5 table */
    TT_PCLT               pclt;

    /* embedded bitmaps support */
#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
    FT_ULong              num_sbit_strikes;
    TT_SBit_Strike        sbit_strikes;
#endif

    FT_ULong              num_sbit_scales;
    TT_SBit_Scale         sbit_scales;

    /* postscript names table */
    TT_Post_NamesRec      postscript_names;


    /***********************************************************************/
    /*                                                                     */
    /* TrueType-specific fields (ignored by the OTF-Type2 driver)          */
    /*                                                                     */
    /***********************************************************************/

    /* the glyph locations */
#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
    FT_UShort             num_locations_stub;
    FT_Long*              glyph_locations_stub;
#endif

    /* the font program, if any */
    FT_ULong              font_program_size;
    FT_Byte*              font_program;

    /* the cvt program, if any */
    FT_ULong              cvt_program_size;
    FT_Byte*              cvt_program;

    /* the original, unscaled, control value table */
    FT_ULong              cvt_size;
    FT_Short*             cvt;

#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
    /* the format 0 kerning table, if any */
    FT_Int                num_kern_pairs;
    FT_Int                kern_table_index;
    TT_Kern0_Pair         kern_pairs;
#endif

    /* A pointer to the bytecode interpreter to use.  This is also */
    /* used to hook the debugger for the `ttdebug' utility.        */
    TT_Interpreter        interpreter;

#ifdef TT_CONFIG_OPTION_UNPATENTED_HINTING
    /* Use unpatented hinting only. */
    FT_Bool               unpatented_hinting;
#endif

    /***********************************************************************/
    /*                                                                     */
    /* Other tables or fields. This is used by derivative formats like     */
    /* OpenType.                                                           */
    /*                                                                     */
    /***********************************************************************/

    FT_Generic            extra;

    const char*           postscript_name;

    /* since version 2.1.8, but was originally placed after */
    /* `glyph_locations_stub'                               */
    FT_ULong              glyf_len;

    /* since version 2.1.8, but was originally placed before `extra' */
#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
    FT_Bool               doblend;
    GX_Blend              blend;
#endif

    /* since version 2.2 */

    FT_Byte*              horz_metrics;
    FT_ULong              horz_metrics_size;

    FT_Byte*              vert_metrics;
    FT_ULong              vert_metrics_size;

    FT_ULong              num_locations; /* in broken TTF, gid > 0xFFFF */ 
    FT_Byte*              glyph_locations;

    FT_Byte*              hdmx_table;
    FT_ULong              hdmx_table_size;
    FT_UInt               hdmx_record_count;
    FT_ULong              hdmx_record_size;
    FT_Byte*              hdmx_record_sizes;

    FT_Byte*              sbit_table;
    FT_ULong              sbit_table_size;
    FT_UInt               sbit_num_strikes;

    FT_Byte*              kern_table;
    FT_ULong              kern_table_size;
    FT_UInt               num_kern_tables;
    FT_UInt32             kern_avail_bits;
    FT_UInt32             kern_order_bits;

#ifdef TT_CONFIG_OPTION_BDF
    TT_BDFRec             bdf;
#endif /* TT_CONFIG_OPTION_BDF */

    /* since 2.3.0 */
    FT_ULong              horz_metrics_offset;
    FT_ULong              vert_metrics_offset;

  } TT_FaceRec;


  /*************************************************************************/
  /*                                                                       */
  /*  <Struct>                                                             */
  /*     TT_GlyphZoneRec                                                   */
  /*                                                                       */
  /*  <Description>                                                        */
  /*     A glyph zone is used to load, scale and hint glyph outline        */
  /*     coordinates.                                                      */
  /*                                                                       */
  /*  <Fields>                                                             */
  /*     memory       :: A handle to the memory manager.                   */
  /*                                                                       */
  /*     max_points   :: The maximal size in points of the zone.           */
  /*                                                                       */
  /*     max_contours :: Max size in links contours of the zone.           */
  /*                                                                       */
  /*     n_points     :: The current number of points in the zone.         */
  /*                                                                       */
  /*     n_contours   :: The current number of contours in the zone.       */
  /*                                                                       */
  /*     org          :: The original glyph coordinates (font              */
  /*                     units/scaled).                                    */
  /*                                                                       */
  /*     cur          :: The current glyph coordinates (scaled/hinted).    */
  /*                                                                       */
  /*     tags         :: The point control tags.                           */
  /*                                                                       */
  /*     contours     :: The contours end points.                          */
  /*                                                                       */
  /*     first_point  :: Offset of the current subglyph's first point.     */
  /*                                                                       */
  typedef struct  TT_GlyphZoneRec_
  {
    FT_Memory   memory;
    FT_UShort   max_points;
    FT_UShort   max_contours;
    FT_UShort   n_points;    /* number of points in zone    */
    FT_Short    n_contours;  /* number of contours          */

    FT_Vector*  org;         /* original point coordinates  */
    FT_Vector*  cur;         /* current point coordinates   */
    FT_Vector*  orus;        /* original (unscaled) point coordinates */

    FT_Byte*    tags;        /* current touch flags         */
    FT_UShort*  contours;    /* contour end points          */

    FT_UShort   first_point; /* offset of first (#0) point  */

  } TT_GlyphZoneRec, *TT_GlyphZone;


  /* handle to execution context */
  typedef struct TT_ExecContextRec_*  TT_ExecContext;

  /* glyph loader structure */
  typedef struct  TT_LoaderRec_
  {
    FT_Face          face;
    FT_Size          size;
    FT_GlyphSlot     glyph;
    FT_GlyphLoader   gloader;

    FT_ULong         load_flags;
    FT_UInt          glyph_index;

    FT_Stream        stream;
    FT_Int           byte_len;

    FT_Short         n_contours;
    FT_BBox          bbox;
    FT_Int           left_bearing;
    FT_Int           advance;
    FT_Int           linear;
    FT_Bool          linear_def;
    FT_Bool          preserve_pps;
    FT_Vector        pp1;
    FT_Vector        pp2;

    FT_ULong         glyf_offset;

    /* the zone where we load our glyphs */
    TT_GlyphZoneRec  base;
    TT_GlyphZoneRec  zone;

    TT_ExecContext   exec;
    FT_Byte*         instructions;
    FT_ULong         ins_pos;

    /* for possible extensibility in other formats */
    void*            other;

    /* since version 2.1.8 */
    FT_Int           top_bearing;
    FT_Int           vadvance;
    FT_Vector        pp3;
    FT_Vector        pp4;

    /* since version 2.2.1 */
    FT_Byte*         cursor;
    FT_Byte*         limit;

  } TT_LoaderRec;


FT_END_HEADER

#endif /* __TTTYPES_H__ */


/* END */
