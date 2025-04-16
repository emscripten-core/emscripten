/***************************************************************************/
/*                                                                         */
/*  gxverror.h                                                             */
/*                                                                         */
/*    TrueTypeGX/AAT validation module error codes (specification only).   */
/*                                                                         */
/*  Copyright 2004, 2005 by suzuki toshiya, Masatake YAMATO, Red Hat K.K., */
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


  /*************************************************************************/
  /*                                                                       */
  /* This file is used to define the OpenType validation module error      */
  /* enumeration constants.                                                */
  /*                                                                       */
  /*************************************************************************/

#ifndef __GXVERROR_H__
#define __GXVERROR_H__

#include FT_MODULE_ERRORS_H

#undef __FTERRORS_H__

#define FT_ERR_PREFIX  GXV_Err_
#define FT_ERR_BASE    FT_Mod_Err_GXV

#define FT_KEEP_ERR_PREFIX

#include FT_ERRORS_H

#endif /* __GXVERROR_H__ */


/* END */
