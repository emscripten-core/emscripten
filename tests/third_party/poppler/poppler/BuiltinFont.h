//========================================================================
//
// BuiltinFont.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef BUILTINFONT_H
#define BUILTINFONT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"

struct BuiltinFont;
class BuiltinFontWidths;

//------------------------------------------------------------------------

struct BuiltinFont {
  char *name;
  char **defaultBaseEnc;
  short ascent;
  short descent;
  short bbox[4];
  BuiltinFontWidths *widths;
};

//------------------------------------------------------------------------

struct BuiltinFontWidth {
  char *name;
  Gushort width;
  BuiltinFontWidth *next;
};

class BuiltinFontWidths {
public:

  BuiltinFontWidths(BuiltinFontWidth *widths, int sizeA);
  ~BuiltinFontWidths();
  GBool getWidth(char *name, Gushort *width);

private:

  int hash(char *name);

  BuiltinFontWidth **tab;
  int size;
};

#endif
