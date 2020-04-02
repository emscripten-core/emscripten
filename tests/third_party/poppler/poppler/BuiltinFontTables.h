//========================================================================
//
// BuiltinFontTables.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef BUILTINFONTTABLES_H
#define BUILTINFONTTABLES_H

#include "BuiltinFont.h"

#define nBuiltinFonts      14
#define nBuiltinFontSubsts 12

extern BuiltinFont builtinFonts[nBuiltinFonts];
extern BuiltinFont *builtinFontSubst[nBuiltinFontSubsts];

extern void initBuiltinFontTables();
extern void freeBuiltinFontTables();

#endif
