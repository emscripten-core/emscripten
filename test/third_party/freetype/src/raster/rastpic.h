/***************************************************************************/
/*                                                                         */
/*  rastpic.h                                                              */
/*                                                                         */
/*    The FreeType position independent code services for raster module.   */
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


#ifndef __RASTPIC_H__
#define __RASTPIC_H__

  
FT_BEGIN_HEADER

#include FT_INTERNAL_PIC_H

#ifndef FT_CONFIG_OPTION_PIC
#define FT_STANDARD_RASTER_GET     ft_standard_raster

#else /* FT_CONFIG_OPTION_PIC */

  typedef struct RasterPIC_
  {
    int ref_count;
    FT_Raster_Funcs ft_standard_raster;
  } RasterPIC;

#define GET_PIC(lib)               ((RasterPIC*)((lib)->pic_container.raster))
#define FT_STANDARD_RASTER_GET     (GET_PIC(library)->ft_standard_raster)

#endif /* FT_CONFIG_OPTION_PIC */

 /* */

FT_END_HEADER

#endif /* __RASTPIC_H__ */


/* END */
