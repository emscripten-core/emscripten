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
#include "GlobalParams.h"
#include "Error.h"

static void defaultErrorFunction(int pos, char *msg, va_list args)
{
  if (pos >= 0) {
    fprintf(stderr, "Error (%d): ", pos);
  } else {
    fprintf(stderr, "Error: ");
  }
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  fflush(stderr);
}

static void (*errorFunction)(int, char *, va_list args) = defaultErrorFunction;

void setErrorFunction(void (* f)(int, char *, va_list args))
{
    errorFunction = f;
}

void CDECL error(int pos, char *msg, ...) {
  va_list args;
  // NB: this can be called before the globalParams object is created
  if (globalParams && globalParams->getErrQuiet()) {
    return;
  }
  va_start(args, msg);
  (*errorFunction)(pos, msg, args);
  va_end(args);
}

void warning(char *msg, ...) {
  va_list args;
  va_start(args, msg);
  vprintf(msg, args);
  va_end(args);
}
