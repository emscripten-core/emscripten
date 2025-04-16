/***************************************************************************/
/*                                                                         */
/*  pspic.h                                                                */
/*                                                                         */
/*    The FreeType position independent code services for psnames module.  */
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


#ifndef __PSPIC_H__
#define __PSPIC_H__

  
FT_BEGIN_HEADER

#include FT_INTERNAL_PIC_H

#ifndef FT_CONFIG_OPTION_PIC
#define FT_PSCMAPS_SERVICES_GET     pscmaps_services
#define FT_PSCMAPS_INTERFACE_GET    pscmaps_interface

#else /* FT_CONFIG_OPTION_PIC */

#include FT_SERVICE_POSTSCRIPT_CMAPS_H

  typedef struct PSModulePIC_
  {
    FT_ServiceDescRec* pscmaps_services;
    FT_Service_PsCMapsRec pscmaps_interface;
  } PSModulePIC;

#define GET_PIC(lib)                ((PSModulePIC*)((lib)->pic_container.psnames))
#define FT_PSCMAPS_SERVICES_GET     (GET_PIC(library)->pscmaps_services)
#define FT_PSCMAPS_INTERFACE_GET    (GET_PIC(library)->pscmaps_interface)

#endif /* FT_CONFIG_OPTION_PIC */

 /* */

FT_END_HEADER

#endif /* __PSPIC_H__ */


/* END */
