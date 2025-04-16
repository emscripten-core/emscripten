/***************************************************************************/
/*                                                                         */
/*  ftbase.c                                                               */
/*                                                                         */
/*    Single object library component (body only).                         */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009 by       */
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

#define  FT_MAKE_OPTION_SINGLE_OBJECT

#include "ftpic.c"
#include "basepic.c"
#include "ftadvanc.c"
#include "ftcalc.c"
#include "ftdbgmem.c"
#include "ftgloadr.c"
#include "ftobjs.c"
#include "ftoutln.c"
#include "ftrfork.c"
#include "ftsnames.c"
#include "ftstream.c"
#include "fttrigon.c"
#include "ftutil.c"

#if defined( FT_MACINTOSH ) && !defined ( DARWIN_NO_CARBON )
#include "ftmac.c"
#endif

/* END */
