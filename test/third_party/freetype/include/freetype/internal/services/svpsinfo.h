/***************************************************************************/
/*                                                                         */
/*  svpsinfo.h                                                             */
/*                                                                         */
/*    The FreeType PostScript info service (specification).                */
/*                                                                         */
/*  Copyright 2003, 2004, 2009 by                                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __SVPSINFO_H__
#define __SVPSINFO_H__

#include FT_INTERNAL_SERVICE_H
#include FT_INTERNAL_TYPE1_TYPES_H


FT_BEGIN_HEADER


#define FT_SERVICE_ID_POSTSCRIPT_INFO  "postscript-info"


  typedef FT_Error
  (*PS_GetFontInfoFunc)( FT_Face          face,
                         PS_FontInfoRec*  afont_info );

  typedef FT_Error
  (*PS_GetFontExtraFunc)( FT_Face           face,
                          PS_FontExtraRec*  afont_extra );

  typedef FT_Int
  (*PS_HasGlyphNamesFunc)( FT_Face   face );

  typedef FT_Error
  (*PS_GetFontPrivateFunc)( FT_Face         face,
                            PS_PrivateRec*  afont_private );


  FT_DEFINE_SERVICE( PsInfo )
  {
    PS_GetFontInfoFunc     ps_get_font_info;
    PS_GetFontExtraFunc    ps_get_font_extra;
    PS_HasGlyphNamesFunc   ps_has_glyph_names;
    PS_GetFontPrivateFunc  ps_get_font_private;
  };

#ifndef FT_CONFIG_OPTION_PIC

#define FT_DEFINE_SERVICE_PSINFOREC(class_, get_font_info_,      \
        ps_get_font_extra_, has_glyph_names_, get_font_private_) \
  static const FT_Service_PsInfoRec class_ =                     \
  {                                                              \
    get_font_info_, ps_get_font_extra_, has_glyph_names_,        \
    get_font_private_                                            \
  };

#else /* FT_CONFIG_OPTION_PIC */ 

#define FT_DEFINE_SERVICE_PSINFOREC(class_, get_font_info_,      \
        ps_get_font_extra_, has_glyph_names_, get_font_private_) \
  void                                                           \
  FT_Init_Class_##class_( FT_Library library,                    \
                          FT_Service_PsInfoRec*  clazz)          \
  {                                                              \
    FT_UNUSED(library);                                          \
    clazz->ps_get_font_info = get_font_info_;                    \
    clazz->ps_get_font_extra = ps_get_font_extra_;               \
    clazz->ps_has_glyph_names = has_glyph_names_;                \
    clazz->ps_get_font_private = get_font_private_;              \
  } 

#endif /* FT_CONFIG_OPTION_PIC */ 

  /* */


FT_END_HEADER


#endif /* __SVPSINFO_H__ */


/* END */
