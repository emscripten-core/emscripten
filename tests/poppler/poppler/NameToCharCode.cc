//========================================================================
//
// NameToCharCode.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <string.h>
#include "goo/gmem.h"
#include "NameToCharCode.h"

//------------------------------------------------------------------------

struct NameToCharCodeEntry {
  char *name;
  CharCode c;
};

//------------------------------------------------------------------------

NameToCharCode::NameToCharCode() {
  int i;

  size = 31;
  len = 0;
  tab = (NameToCharCodeEntry *)gmallocn(size, sizeof(NameToCharCodeEntry));
  for (i = 0; i < size; ++i) {
    tab[i].name = NULL;
  }
}

NameToCharCode::~NameToCharCode() {
  int i;

  for (i = 0; i < size; ++i) {
    if (tab[i].name) {
      gfree(tab[i].name);
    }
  }
  gfree(tab);
}

void NameToCharCode::add(char *name, CharCode c) {
  NameToCharCodeEntry *oldTab;
  int h, i, oldSize;

  // expand the table if necessary
  if (len >= size / 2) {
    oldSize = size;
    oldTab = tab;
    size = 2*size + 1;
    tab = (NameToCharCodeEntry *)gmallocn(size, sizeof(NameToCharCodeEntry));
    for (h = 0; h < size; ++h) {
      tab[h].name = NULL;
    }
    for (i = 0; i < oldSize; ++i) {
      if (oldTab[i].name) {
	h = hash(oldTab[i].name);
	while (tab[h].name) {
	  if (++h == size) {
	    h = 0;
	  }
	}
	tab[h] = oldTab[i];
      }
    }
    gfree(oldTab);
  }

  // add the new name
  h = hash(name);
  while (tab[h].name && strcmp(tab[h].name, name)) {
    if (++h == size) {
      h = 0;
    }
  }
  if (!tab[h].name) {
    tab[h].name = copyString(name);
  }
  tab[h].c = c;

  ++len;
}

CharCode NameToCharCode::lookup(char *name) {
  int h;

  h = hash(name);
  while (tab[h].name) {
    if (!strcmp(tab[h].name, name)) {
      return tab[h].c;
    }
    if (++h == size) {
      h = 0;
    }
  }
  return 0;
}

int NameToCharCode::hash(char *name) {
  char *p;
  unsigned int h;

  h = 0;
  for (p = name; *p; ++p) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}
