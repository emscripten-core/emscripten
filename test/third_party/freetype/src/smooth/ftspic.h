/***************************************************************************/
/*                                                                         */
/*  ftspic.h                                                               */
/*                                                                         */
/*    The FreeType position independent code services for smooth module.   */
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


#ifndef __FTSPIC_H__
#define __FTSPIC_H__

  
FT_BEGIN_HEADER

#include FT_INTERNAL_PIC_H

#ifndef FT_CONFIG_OPTION_PIC
#define FT_GRAYS_RASTER_GET        ft_grays_raster

#else /* FT_CONFIG_OPTION_PIC */

  typedef struct SmoothPIC_
  {
    int ref_count;
    FT_Raster_Funcs ft_grays_raster;
  } SmoothPIC;

#define GET_PIC(lib)               ((SmoothPIC*)((lib)->pic_container.smooth))
#define FT_GRAYS_RASTER_GET        (GET_PIC(library)->ft_grays_raster)

#endif /* FT_CONFIG_OPTION_PIC */

 /* */

FT_END_HEADER

#endif /* __FTSPIC_H__ */


/* END */
