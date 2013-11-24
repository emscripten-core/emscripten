//========================================================================
//
// Error.cc
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
// Copyright (C) 2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2012 Marek Kasik <mkasik@redhat.com>
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include "GooString.h"
#include "GlobalParams.h"
#include "Error.h"

static const char *errorCategoryNames[] = {
  "Syntax Warning",
  "Syntax Error",
  "Config Error",
  "Command Line Error",
  "I/O Error",
  "Permission Error",
  "Unimplemented Feature",
  "Internal Error"
};

static void (*errorCbk)(void *data, ErrorCategory category,
			Goffset pos, char *msg) = NULL;
static void *errorCbkData = NULL;

void setErrorCallback(void (*cbk)(void *data, ErrorCategory category,
				  Goffset pos, char *msg),
		      void *data) {
  errorCbk = cbk;
  errorCbkData = data;
}

void CDECL error(ErrorCategory category, Goffset pos, const char *msg, ...) {
  va_list args;
  GooString *s, *sanitized;

  // NB: this can be called before the globalParams object is created
  if (!errorCbk && globalParams && globalParams->getErrQuiet()) {
    return;
  }
  va_start(args, msg);
  s = GooString::formatv(msg, args);
  va_end(args);

  sanitized = new GooString ();
  for (int i = 0; i < s->getLength(); ++i) {
    const char c = s->getChar(i);
    if (c < (char)0x20 || c >= (char)0x7f) {
      sanitized->appendf("<{0:02x}>", c & 0xff);
    } else {
      sanitized->append(c);
    }
  }

  if (errorCbk) {
    (*errorCbk)(errorCbkData, category, pos, sanitized->getCString());
  } else {
    if (pos >= 0) {
      fprintf(stderr, "%s (%lld): %s\n",
	      errorCategoryNames[category], (long long)pos, sanitized->getCString());
    } else {
      fprintf(stderr, "%s: %s\n",
	      errorCategoryNames[category], sanitized->getCString());
    }
    fflush(stderr);
  }
  delete s;
  delete sanitized;
}
