//========================================================================
//
// BuiltinFont.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include "goo/gmem.h"
#include "FontEncodingTables.h"
#include "BuiltinFont.h"

//------------------------------------------------------------------------

BuiltinFontWidths::BuiltinFontWidths(BuiltinFontWidth *widths, int sizeA) {
  int i, h;

  size = sizeA;
  tab = (BuiltinFontWidth **)gmallocn(size, sizeof(BuiltinFontWidth *));
  for (i = 0; i < size; ++i) {
    tab[i] = NULL;
  }
  for (i = 0; i < sizeA; ++i) {
    h = hash(widths[i].name);
    widths[i].next = tab[h];
    tab[h] = &widths[i];
  }
}

BuiltinFontWidths::~BuiltinFontWidths() {
  gfree(tab);
}

GBool BuiltinFontWidths::getWidth(const char *name, Gushort *width) {
  int h;
  BuiltinFontWidth *p;

  h = hash(name);
  for (p = tab[h]; p; p = p->next) {
    if (!strcmp(p->name, name)) {
      *width = p->width;
      return gTrue;
    }
  }
  return gFalse;
}

int BuiltinFontWidths::hash(const char *name) {
  const char *p;
  unsigned int h;

  h = 0;
  for (p = name; *p; ++p) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}
