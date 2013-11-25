//========================================================================
//
// Error.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2005 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef ERROR_H
#define ERROR_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdarg.h>
#include "poppler-config.h"
#include "goo/gtypes.h"

enum ErrorCategory {
  errSyntaxWarning,    // PDF syntax error which can be worked around;
                       //   output will probably be correct
  errSyntaxError,      // PDF syntax error which can be worked around;
                       //   output will probably be incorrect
  errConfig,           // error in Xpdf config info (xpdfrc file, etc.)
  errCommandLine,      // error in user-supplied parameters, action not
                       //   allowed, etc. (only used by command-line tools)
  errIO,               // error in file I/O
  errNotAllowed,       // action not allowed by PDF permission bits
  errUnimplemented,    // unimplemented PDF feature - display will be
                       //   incorrect
  errInternal          // internal error - malfunction within the Xpdf code
};

extern void setErrorCallback(void (*cbk)(void *data, ErrorCategory category,
					 Goffset pos, char *msg),
			     void *data);

extern void CDECL error(ErrorCategory category, Goffset pos, const char *msg, ...);

#endif
