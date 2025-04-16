/***************************************************************************/
/*                                                                         */
/*  ftccback.h                                                             */
/*                                                                         */
/*    Callback functions of the caching sub-system (specification only).   */
/*                                                                         */
/*  Copyright 2004, 2005, 2006 by                                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FTCCBACK_H__
#define __FTCCBACK_H__

#include <ft2build.h>
#include FT_CACHE_H
#include "ftcmru.h"
#include "ftcimage.h"
#include "ftcmanag.h"
#include "ftcglyph.h"
#include "ftcsbits.h"


  FT_LOCAL( void )
  ftc_inode_free( FTC_Node   inode,
                  FTC_Cache  cache );

  FT_LOCAL( FT_Error )
  ftc_inode_new( FTC_Node   *pinode,
                 FT_Pointer  gquery,
                 FTC_Cache   cache );

  FT_LOCAL( FT_Offset )
  ftc_inode_weight( FTC_Node   inode,
                    FTC_Cache  cache );


  FT_LOCAL( void )
  ftc_snode_free( FTC_Node   snode,
                  FTC_Cache  cache );

  FT_LOCAL( FT_Error )
  ftc_snode_new( FTC_Node   *psnode,
                 FT_Pointer  gquery,
                 FTC_Cache   cache );

  FT_LOCAL( FT_Offset )
  ftc_snode_weight( FTC_Node   snode,
                    FTC_Cache  cache );

  FT_LOCAL( FT_Bool )
  ftc_snode_compare( FTC_Node    snode,
                     FT_Pointer  gquery,
                     FTC_Cache   cache );


  FT_LOCAL( FT_Bool )
  ftc_gnode_compare( FTC_Node    gnode,
                     FT_Pointer  gquery,
                     FTC_Cache   cache );


  FT_LOCAL( FT_Error )
  ftc_gcache_init( FTC_Cache  cache );

  FT_LOCAL( void )
  ftc_gcache_done( FTC_Cache  cache );


  FT_LOCAL( FT_Error )
  ftc_cache_init( FTC_Cache  cache );

  FT_LOCAL( void )
  ftc_cache_done( FTC_Cache  cache );

#ifndef FT_CONFIG_OPTION_OLD_INTERNALS
  FT_LOCAL( void )
  ftc_node_destroy( FTC_Node     node,
                    FTC_Manager  manager );
#endif

#endif /* __FTCCBACK_H__ */

/* END */
