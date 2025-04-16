//========================================================================
//
// CairoFontEngine.h
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006, 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2006, 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2008 Adrian Johnson <ajohnson@redneon.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef CAIROFONTENGINE_H
#define CAIROFONTENGINE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include <cairo-ft.h>

#include "GfxFont.h"
#include "Catalog.h"

class CairoFontEngine;

class CairoFont {
public:
  CairoFont(Ref ref,
	    cairo_font_face_t *face,
	    Gushort *codeToGID,
	    Guint codeToGIDLen,
	    GBool substitute,
	    GBool printing);
  virtual ~CairoFont();

  virtual GBool matches(Ref &other, GBool printing);
  cairo_font_face_t *getFontFace(void);
  unsigned long getGlyph(CharCode code, Unicode *u, int uLen);
  double getSubstitutionCorrection(GfxFont *gfxFont);

  GBool isSubstitute() { return substitute; }
protected:
  Ref ref;
  cairo_font_face_t *cairo_font_face;

  Gushort *codeToGID;
  Guint codeToGIDLen;

  GBool substitute;
  GBool printing;
};

//------------------------------------------------------------------------

class CairoFreeTypeFont : public CairoFont {
public:
  static CairoFreeTypeFont *create(GfxFont *gfxFont, XRef *xref, FT_Library lib, GBool useCIDs);
  virtual ~CairoFreeTypeFont();

private:
  CairoFreeTypeFont(Ref ref, cairo_font_face_t *cairo_font_face,
	    Gushort *codeToGID, Guint codeToGIDLen, GBool substitute);
};

//------------------------------------------------------------------------

class CairoType3Font : public CairoFont {
public:
  static CairoType3Font *create(GfxFont *gfxFont, XRef *xref,
				Catalog *catalog, CairoFontEngine *fontEngine,
				GBool printing);
  virtual ~CairoType3Font();

  virtual GBool matches(Ref &other, GBool printing);

private:
  CairoType3Font(Ref ref, XRef *xref, Catalog *catalog,
		 cairo_font_face_t *cairo_font_face,
		 Gushort *codeToGID, Guint codeToGIDLen,
		 GBool printing);
  XRef *xref;
  Catalog *catalog;
};

//------------------------------------------------------------------------

#define cairoFontCacheSize 64

//------------------------------------------------------------------------
// CairoFontEngine
//------------------------------------------------------------------------

class CairoFontEngine {
public:

  // Create a font engine.
  CairoFontEngine(FT_Library libA);
  ~CairoFontEngine();

  CairoFont *getFont(GfxFont *gfxFont, XRef *xref, Catalog *catalog, GBool printing);

private:
  CairoFont *fontCache[cairoFontCacheSize];
  FT_Library lib;
  GBool useCIDs;
};

#endif
