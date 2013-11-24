//========================================================================
//
// UnicodeTypeTable.h
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Ed Catmur <ed@catmur.co.uk>
// Copyright (C) 2012 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef UNICODETYPETABLE_H
#define UNICODETYPETABLE_H

#include "goo/gtypes.h"

extern GBool unicodeTypeL(Unicode c);

extern GBool unicodeTypeR(Unicode c);

extern GBool unicodeTypeNum(Unicode c);

extern GBool unicodeTypeAlphaNum(Unicode c);

extern GBool unicodeIsAlphabeticPresentationForm(Unicode c);

extern Unicode unicodeToUpper(Unicode c);

extern Unicode *unicodeNormalizeNFKC(Unicode *in, int len, 
				     int *out_len, int **offsets);

#endif
