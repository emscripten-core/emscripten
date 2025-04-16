/***************************************************************************/
/*                                                                         */
/*  cidriver.c                                                             */
/*                                                                         */
/*    CID driver interface (body).                                         */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2006, 2008, 2009 by             */
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
#include "cidriver.h"
#include "cidgload.h"
#include FT_INTERNAL_DEBUG_H

#include "ciderrs.h"

#include FT_SERVICE_POSTSCRIPT_NAME_H
#include FT_SERVICE_XFREE86_NAME_H
#include FT_SERVICE_POSTSCRIPT_INFO_H
#include FT_SERVICE_CID_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_ciddriver


  /*
   *  POSTSCRIPT NAME SERVICE
   *
   */

  static const char*
  cid_get_postscript_name( CID_Face  face )
  {
    const char*  result = face->cid.cid_font_name;


    if ( result && result[0] == '/' )
      result++;

    return result;
  }


  static const FT_Service_PsFontNameRec  cid_service_ps_name =
  {
    (FT_PsName_GetFunc) cid_get_postscript_name
  };


  /*
   *  POSTSCRIPT INFO SERVICE
   *
   */

  static FT_Error
  cid_ps_get_font_info( FT_Face          face,
                        PS_FontInfoRec*  afont_info )
  {
    *afont_info = ((CID_Face)face)->cid.font_info;

    return CID_Err_Ok;
  }

  static FT_Error
  cid_ps_get_font_extra( FT_Face          face,
                        PS_FontExtraRec*  afont_extra )
  {
    *afont_extra = ((CID_Face)face)->font_extra;

    return CID_Err_Ok;
  }

  static const FT_Service_PsInfoRec  cid_service_ps_info =
  {
    (PS_GetFontInfoFunc)   cid_ps_get_font_info,
    (PS_GetFontExtraFunc)  cid_ps_get_font_extra,
    (PS_HasGlyphNamesFunc) NULL,        /* unsupported with CID fonts */
    (PS_GetFontPrivateFunc)NULL         /* unsupported                */
  };


  /*
   *  CID INFO SERVICE
   *
   */
  static FT_Error
  cid_get_ros( CID_Face      face,
               const char*  *registry,
               const char*  *ordering,
               FT_Int       *supplement )
  {
    CID_FaceInfo  cid = &face->cid;


    if ( registry )
      *registry = cid->registry;
      
    if ( ordering )
      *ordering = cid->ordering;

    if ( supplement )
      *supplement = cid->supplement;
      
    return CID_Err_Ok;
  }


  static FT_Error
  cid_get_is_cid( CID_Face  face,
                  FT_Bool  *is_cid )
  {
    FT_Error  error = CID_Err_Ok;
    FT_UNUSED( face );


    if ( is_cid )
      *is_cid = 1; /* cid driver is only used for CID keyed fonts */

    return error;
  }


  static FT_Error
  cid_get_cid_from_glyph_index( CID_Face  face,
                                FT_UInt   glyph_index,
                                FT_UInt  *cid )
  {
    FT_Error  error = CID_Err_Ok;
    FT_UNUSED( face );


    if ( cid )
      *cid = glyph_index; /* identity mapping */

    return error;
  }


  static const FT_Service_CIDRec  cid_service_cid_info =
  {
     (FT_CID_GetRegistryOrderingSupplementFunc)cid_get_ros,
     (FT_CID_GetIsInternallyCIDKeyedFunc)      cid_get_is_cid,
     (FT_CID_GetCIDFromGlyphIndexFunc)         cid_get_cid_from_glyph_index
  };


  /*
   *  SERVICE LIST
   *
   */

  static const FT_ServiceDescRec  cid_services[] =
  {
    { FT_SERVICE_ID_XF86_NAME,            FT_XF86_FORMAT_CID },
    { FT_SERVICE_ID_POSTSCRIPT_FONT_NAME, &cid_service_ps_name },
    { FT_SERVICE_ID_POSTSCRIPT_INFO,      &cid_service_ps_info },
    { FT_SERVICE_ID_CID,                  &cid_service_cid_info },
    { NULL, NULL }
  };


  FT_CALLBACK_DEF( FT_Module_Interface )
  cid_get_interface( FT_Module    module,
                     const char*  cid_interface )
  {
    FT_UNUSED( module );

    return ft_service_list_lookup( cid_services, cid_interface );
  }



  FT_CALLBACK_TABLE_DEF
  const FT_Driver_ClassRec  t1cid_driver_class =
  {
    /* first of all, the FT_Module_Class fields */
    {
      FT_MODULE_FONT_DRIVER       |
      FT_MODULE_DRIVER_SCALABLE   |
      FT_MODULE_DRIVER_HAS_HINTER,

      sizeof( FT_DriverRec ),
      "t1cid",   /* module name           */
      0x10000L,  /* version 1.0 of driver */
      0x20000L,  /* requires FreeType 2.0 */

      0,

      cid_driver_init,
      cid_driver_done,
      cid_get_interface
    },

    /* then the other font drivers fields */
    sizeof( CID_FaceRec ),
    sizeof( CID_SizeRec ),
    sizeof( CID_GlyphSlotRec ),

    cid_face_init,
    cid_face_done,

    cid_size_init,
    cid_size_done,
    cid_slot_init,
    cid_slot_done,

#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
    ft_stub_set_char_sizes,
    ft_stub_set_pixel_sizes,
#endif

    cid_slot_load_glyph,

    0,                      /* FT_Face_GetKerningFunc  */
    0,                      /* FT_Face_AttachFunc      */

    0,                      /* FT_Face_GetAdvancesFunc */

    cid_size_request,
    0                       /* FT_Size_SelectFunc      */
  };


/* END */
