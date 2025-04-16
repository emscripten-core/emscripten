/***************************************************************************/
/*                                                                         */
/*  sfntpic.h                                                              */
/*                                                                         */
/*    The FreeType position independent code services for sfnt module.     */
/*                                                                         */
/*  Copyright 2009 by                                                      */
/*  Oran Agra and Mickey Gabel.                                            */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __SFNTPIC_H__
#define __SFNTPIC_H__

  
FT_BEGIN_HEADER

#include FT_INTERNAL_PIC_H

 #ifndef FT_CONFIG_OPTION_PIC
#define FT_SFNT_SERVICES_GET             sfnt_services
#define FT_SFNT_SERVICE_GLYPH_DICT_GET   sfnt_service_glyph_dict
#define FT_SFNT_SERVICE_PS_NAME_GET      sfnt_service_ps_name
#define FT_TT_SERVICE_GET_CMAP_INFO_GET  tt_service_get_cmap_info
#define FT_SFNT_SERVICES_GET             sfnt_services
#define FT_TT_CMAP_CLASSES_GET           tt_cmap_classes
#define FT_SFNT_SERVICE_SFNT_TABLE_GET   sfnt_service_sfnt_table
#define FT_SFNT_SERVICE_BDF_GET          sfnt_service_bdf
#define FT_SFNT_INTERFACE_GET            sfnt_interface

#else /* FT_CONFIG_OPTION_PIC */

/* some include files required for members of sfntModulePIC */
#include FT_SERVICE_GLYPH_DICT_H
#include FT_SERVICE_POSTSCRIPT_NAME_H
#include FT_SERVICE_SFNT_H
#include FT_SERVICE_TT_CMAP_H
#ifdef TT_CONFIG_OPTION_BDF
#include "ttbdf.h"
#include FT_SERVICE_BDF_H
#endif
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_INTERNAL_SFNT_H
#include "ttcmap.h"

typedef struct sfntModulePIC_
  {
    FT_ServiceDescRec* sfnt_services;
    FT_Service_GlyphDictRec sfnt_service_glyph_dict;
    FT_Service_PsFontNameRec  sfnt_service_ps_name;
    FT_Service_TTCMapsRec  tt_service_get_cmap_info;
    TT_CMap_Class* tt_cmap_classes;
    FT_Service_SFNT_TableRec sfnt_service_sfnt_table;
#ifdef TT_CONFIG_OPTION_BDF
    FT_Service_BDFRec sfnt_service_bdf;
#endif
    SFNT_Interface sfnt_interface;
  } sfntModulePIC;

#define GET_PIC(lib)                      ((sfntModulePIC*)((lib)->pic_container.sfnt))
#define FT_SFNT_SERVICES_GET              (GET_PIC(library)->sfnt_services)
#define FT_SFNT_SERVICE_GLYPH_DICT_GET    (GET_PIC(library)->sfnt_service_glyph_dict)
#define FT_SFNT_SERVICE_PS_NAME_GET       (GET_PIC(library)->sfnt_service_ps_name)
#define FT_TT_SERVICE_GET_CMAP_INFO_GET   (GET_PIC(library)->tt_service_get_cmap_info)
#define FT_SFNT_SERVICES_GET              (GET_PIC(library)->sfnt_services)
#define FT_TT_CMAP_CLASSES_GET            (GET_PIC(library)->tt_cmap_classes)
#define FT_SFNT_SERVICE_SFNT_TABLE_GET    (GET_PIC(library)->sfnt_service_sfnt_table)
#define FT_SFNT_SERVICE_BDF_GET           (GET_PIC(library)->sfnt_service_bdf)
#define FT_SFNT_INTERFACE_GET             (GET_PIC(library)->sfnt_interface)

#endif /* FT_CONFIG_OPTION_PIC */

/* */

FT_END_HEADER

#endif /* __SFNTPIC_H__ */


/* END */
