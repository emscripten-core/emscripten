/***************************************************************************/
/*                                                                         */
/*  pcferror.h                                                             */
/*                                                                         */
/*    PCF error codes (specification only).                                */
/*                                                                         */
/*  Copyright 2001 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* This file is used to define the PCF error enumeration constants.      */
  /*                                                                       */
  /*************************************************************************/

#ifndef __PCFERROR_H__
#define __PCFERROR_H__

#include FT_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT_ERR_PREFIX  PCF_Err_
#define FT_ERR_BASE    FT_Mod_Err_PCF

#include FT_ERRORS_H

#endif /* __PCFERROR_H__ */


/* END */
