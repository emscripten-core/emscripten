/***************************************************************************/
/*                                                                         */
/*  cfftypes.h                                                             */
/*                                                                         */
/*    Basic OpenType/CFF type definitions and interface (specification     */
/*    only).                                                               */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2006, 2007, 2008, 2010 by             */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __CFFTYPES_H__
#define __CFFTYPES_H__


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_INTERNAL_SERVICE_H
#include FT_SERVICE_POSTSCRIPT_CMAPS_H
#include FT_INTERNAL_POSTSCRIPT_HINTS_H


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    CFF_IndexRec                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a CFF Index table.                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    stream      :: The source input stream.                            */
  /*                                                                       */
  /*    start       :: The position of the first index byte in the         */
  /*                   input stream.                                       */
  /*                                                                       */
  /*    count       :: The number of elements in the index.                */
  /*                                                                       */
  /*    off_size    :: The size in bytes of object offsets in index.       */
  /*                                                                       */
  /*    data_offset :: The position of first data byte in the index's      */
  /*                   bytes.                                              */
  /*                                                                       */
  /*    data_size   :: The size of the data table in this index.           */
  /*                                                                       */
  /*    offsets     :: A table of element offsets in the index.  Must be   */
  /*                   loaded explicitly.                                  */
  /*                                                                       */
  /*    bytes       :: If the index is loaded in memory, its bytes.        */
  /*                                                                       */
  typedef struct  CFF_IndexRec_
  {
    FT_Stream  stream;
    FT_ULong   start;
    FT_UInt    count;
    FT_Byte    off_size;
    FT_ULong   data_offset;
    FT_ULong   data_size;

    FT_ULong*  offsets;
    FT_Byte*   bytes;

  } CFF_IndexRec, *CFF_Index;


  typedef struct  CFF_EncodingRec_
  {
    FT_UInt     format;
    FT_ULong    offset;

    FT_UInt     count;
    FT_UShort   sids [256];  /* avoid dynamic allocations */
    FT_UShort   codes[256];

  } CFF_EncodingRec, *CFF_Encoding;


  typedef struct  CFF_CharsetRec_
  {

    FT_UInt     format;
    FT_ULong    offset;

    FT_UShort*  sids;
    FT_UShort*  cids;       /* the inverse mapping of `sids'; only needed */
                            /* for CID-keyed fonts                        */
    FT_UInt     max_cid;
    FT_UInt     num_glyphs;

  } CFF_CharsetRec, *CFF_Charset;


  typedef struct  CFF_FontRecDictRec_
  {
    FT_UInt    version;
    FT_UInt    notice;
    FT_UInt    copyright;
    FT_UInt    full_name;
    FT_UInt    family_name;
    FT_UInt    weight;
    FT_Bool    is_fixed_pitch;
    FT_Fixed   italic_angle;
    FT_Fixed   underline_position;
    FT_Fixed   underline_thickness;
    FT_Int     paint_type;
    FT_Int     charstring_type;
    FT_Matrix  font_matrix;
    FT_ULong   units_per_em;  /* temporarily used as scaling value also */
    FT_Vector  font_offset;
    FT_ULong   unique_id;
    FT_BBox    font_bbox;
    FT_Pos     stroke_width;
    FT_ULong   charset_offset;
    FT_ULong   encoding_offset;
    FT_ULong   charstrings_offset;
    FT_ULong   private_offset;
    FT_ULong   private_size;
    FT_Long    synthetic_base;
    FT_UInt    embedded_postscript;

    /* these should only be used for the top-level font dictionary */
    FT_UInt    cid_registry;
    FT_UInt    cid_ordering;
    FT_Long    cid_supplement;

    FT_Long    cid_font_version;
    FT_Long    cid_font_revision;
    FT_Long    cid_font_type;
    FT_ULong   cid_count;
    FT_ULong   cid_uid_base;
    FT_ULong   cid_fd_array_offset;
    FT_ULong   cid_fd_select_offset;
    FT_UInt    cid_font_name;

  } CFF_FontRecDictRec, *CFF_FontRecDict;


  typedef struct  CFF_PrivateRec_
  {
    FT_Byte   num_blue_values;
    FT_Byte   num_other_blues;
    FT_Byte   num_family_blues;
    FT_Byte   num_family_other_blues;

    FT_Pos    blue_values[14];
    FT_Pos    other_blues[10];
    FT_Pos    family_blues[14];
    FT_Pos    family_other_blues[10];

    FT_Fixed  blue_scale;
    FT_Pos    blue_shift;
    FT_Pos    blue_fuzz;
    FT_Pos    standard_width;
    FT_Pos    standard_height;

    FT_Byte   num_snap_widths;
    FT_Byte   num_snap_heights;
    FT_Pos    snap_widths[13];
    FT_Pos    snap_heights[13];
    FT_Bool   force_bold;
    FT_Fixed  force_bold_threshold;
    FT_Int    lenIV;
    FT_Int    language_group;
    FT_Fixed  expansion_factor;
    FT_Long   initial_random_seed;
    FT_ULong  local_subrs_offset;
    FT_Pos    default_width;
    FT_Pos    nominal_width;

  } CFF_PrivateRec, *CFF_Private;


  typedef struct  CFF_FDSelectRec_
  {
    FT_Byte   format;
    FT_UInt   range_count;

    /* that's the table, taken from the file `as is' */
    FT_Byte*  data;
    FT_UInt   data_size;

    /* small cache for format 3 only */
    FT_UInt   cache_first;
    FT_UInt   cache_count;
    FT_Byte   cache_fd;

  } CFF_FDSelectRec, *CFF_FDSelect;


  /* A SubFont packs a font dict and a private dict together.  They are */
  /* needed to support CID-keyed CFF fonts.                             */
  typedef struct  CFF_SubFontRec_
  {
    CFF_FontRecDictRec  font_dict;
    CFF_PrivateRec      private_dict;

    CFF_IndexRec        local_subrs_index;
    FT_Byte**           local_subrs; /* array of pointers into Local Subrs INDEX data */

  } CFF_SubFontRec, *CFF_SubFont;


  /* maximum number of sub-fonts in a CID-keyed file */
#define CFF_MAX_CID_FONTS  32


  typedef struct  CFF_FontRec_
  {
    FT_Stream        stream;
    FT_Memory        memory;
    FT_UInt          num_faces;
    FT_UInt          num_glyphs;

    FT_Byte          version_major;
    FT_Byte          version_minor;
    FT_Byte          header_size;
    FT_Byte          absolute_offsize;


    CFF_IndexRec     name_index;
    CFF_IndexRec     top_dict_index;
    CFF_IndexRec     global_subrs_index;

    CFF_EncodingRec  encoding;
    CFF_CharsetRec   charset;

    CFF_IndexRec     charstrings_index;
    CFF_IndexRec     font_dict_index;
    CFF_IndexRec     private_index;
    CFF_IndexRec     local_subrs_index;

    FT_String*       font_name;

    /* array of pointers into Global Subrs INDEX data */
    FT_Byte**        global_subrs;

    /* array of pointers into String INDEX data stored at string_pool */
    FT_UInt          num_strings;
    FT_Byte**        strings;
    FT_Byte*         string_pool;

    CFF_SubFontRec   top_font;
    FT_UInt          num_subfonts;
    CFF_SubFont      subfonts[CFF_MAX_CID_FONTS];

    CFF_FDSelectRec  fd_select;

    /* interface to PostScript hinter */
    PSHinter_Service  pshinter;

    /* interface to Postscript Names service */
    FT_Service_PsCMaps  psnames;

    /* since version 2.3.0 */
    PS_FontInfoRec*  font_info;   /* font info dictionary */

    /* since version 2.3.6 */
    FT_String*       registry;
    FT_String*       ordering;
      
  } CFF_FontRec, *CFF_Font;


FT_END_HEADER

#endif /* __CFFTYPES_H__ */


/* END */
