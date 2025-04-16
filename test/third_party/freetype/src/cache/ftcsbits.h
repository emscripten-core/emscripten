/***************************************************************************/
/*                                                                         */
/*  ftcsbits.h                                                             */
/*                                                                         */
/*    A small-bitmap cache (specification).                                */
/*                                                                         */
/*  Copyright 2000-2001, 2002, 2003, 2006 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCSBITS_H__
#define __FTCSBITS_H__


#include <ft2build.h>
#include FT_CACHE_H
#include "ftcglyph.h"


FT_BEGIN_HEADER

#define FTC_SBIT_ITEMS_PER_NODE  16

  typedef struct  FTC_SNodeRec_
  {
    FTC_GNodeRec  gnode;
    FT_UInt       count;
    FTC_SBitRec   sbits[FTC_SBIT_ITEMS_PER_NODE];

  } FTC_SNodeRec, *FTC_SNode;


#define FTC_SNODE( x )         ( (FTC_SNode)( x ) )
#define FTC_SNODE_GINDEX( x )  FTC_GNODE( x )->gindex
#define FTC_SNODE_FAMILY( x )  FTC_GNODE( x )->family

  typedef FT_UInt
  (*FTC_SFamily_GetCountFunc)( FTC_Family   family,
                               FTC_Manager  manager );

  typedef FT_Error
  (*FTC_SFamily_LoadGlyphFunc)( FTC_Family   family,
                                FT_UInt      gindex,
                                FTC_Manager  manager,
                                FT_Face     *aface );

  typedef struct  FTC_SFamilyClassRec_
  {
    FTC_MruListClassRec        clazz;
    FTC_SFamily_GetCountFunc   family_get_count;
    FTC_SFamily_LoadGlyphFunc  family_load_glyph;

  } FTC_SFamilyClassRec;

  typedef const FTC_SFamilyClassRec*  FTC_SFamilyClass;

#define FTC_SFAMILY_CLASS( x )  ((FTC_SFamilyClass)(x))

#define FTC_CACHE__SFAMILY_CLASS( x )  \
          FTC_SFAMILY_CLASS( FTC_CACHE__GCACHE_CLASS( x )->family_class )


  FT_LOCAL( void )
  FTC_SNode_Free( FTC_SNode  snode,
                  FTC_Cache  cache );

  FT_LOCAL( FT_Error )
  FTC_SNode_New( FTC_SNode   *psnode,
                 FTC_GQuery   gquery,
                 FTC_Cache    cache );

#if 0
  FT_LOCAL( FT_ULong )
  FTC_SNode_Weight( FTC_SNode  inode );
#endif


  FT_LOCAL( FT_Bool )
  FTC_SNode_Compare( FTC_SNode   snode,
                     FTC_GQuery  gquery,
                     FTC_Cache   cache );

  /* */

FT_END_HEADER

#endif /* __FTCSBITS_H__ */


/* END */
