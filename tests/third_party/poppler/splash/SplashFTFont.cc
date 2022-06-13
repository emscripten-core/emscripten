//========================================================================
//
// SplashFTFont.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2007-2011 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Kristian Høgsberg <krh@bitplanet.net>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2010 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_SIZES_H
#include FT_GLYPH_H
#include "goo/gmem.h"
#include "SplashMath.h"
#include "SplashGlyphBitmap.h"
#include "SplashPath.h"
#include "SplashFTFontEngine.h"
#include "SplashFTFontFile.h"
#include "SplashFTFont.h"

//------------------------------------------------------------------------

static int glyphPathMoveTo(const FT_Vector *pt, void *path);
static int glyphPathLineTo(const FT_Vector *pt, void *path);
static int glyphPathConicTo(const FT_Vector *ctrl, const FT_Vector *pt,
			    void *path);
static int glyphPathCubicTo(const FT_Vector *ctrl1, const FT_Vector *ctrl2,
			    const FT_Vector *pt, void *path);

//------------------------------------------------------------------------
// SplashFTFont
//------------------------------------------------------------------------

SplashFTFont::SplashFTFont(SplashFTFontFile *fontFileA, SplashCoord *matA,
			   SplashCoord *textMatA):
  SplashFont(fontFileA, matA, textMatA, fontFileA->engine->aa), 
  enableFreeTypeHinting(fontFileA->engine->enableFreeTypeHinting)
{
  FT_Face face;
  double div;
  int x, y;

  face = fontFileA->face;
  if (FT_New_Size(face, &sizeObj)) {
    return;
  }
  face->size = sizeObj;
  size = splashSqrt(mat[2]*mat[2] + mat[3]*mat[3]);
  if ((int)size < 1) {
    size = 1;
  }
  if (FT_Set_Pixel_Sizes(face, 0, (int)size)) {
    return;
  }
  // if the textMat values are too small, FreeType's fixed point
  // arithmetic doesn't work so well
  textScale = splashSqrt(textMat[2]*textMat[2] + textMat[3]*textMat[3]) / size;

  div = face->bbox.xMax > 20000 ? 65536 : 1;

  // transform the four corners of the font bounding box -- the min
  // and max values form the bounding box of the transformed font
  x = (int)((mat[0] * face->bbox.xMin + mat[2] * face->bbox.yMin) /
	    (div * face->units_per_EM));
  xMin = xMax = x;
  y = (int)((mat[1] * face->bbox.xMin + mat[3] * face->bbox.yMin) /
	    (div * face->units_per_EM));
  yMin = yMax = y;
  x = (int)((mat[0] * face->bbox.xMin + mat[2] * face->bbox.yMax) /
	    (div * face->units_per_EM));
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)((mat[1] * face->bbox.xMin + mat[3] * face->bbox.yMax) /
	    (div * face->units_per_EM));
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  x = (int)((mat[0] * face->bbox.xMax + mat[2] * face->bbox.yMin) /
	    (div * face->units_per_EM));
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)((mat[1] * face->bbox.xMax + mat[3] * face->bbox.yMin) /
	    (div * face->units_per_EM));
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  x = (int)((mat[0] * face->bbox.xMax + mat[2] * face->bbox.yMax) /
	    (div * face->units_per_EM));
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)((mat[1] * face->bbox.xMax + mat[3] * face->bbox.yMax) /
	    (div * face->units_per_EM));
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  // This is a kludge: some buggy PDF generators embed fonts with
  // zero bounding boxes.
  if (xMax == xMin) {
    xMin = 0;
    xMax = (int)size;
  }
  if (yMax == yMin) {
    yMin = 0;
    yMax = (int)((SplashCoord)1.2 * size);
  }

  // compute the transform matrix
#if USE_FIXEDPOINT
  matrix.xx = (FT_Fixed)((mat[0] / size).getRaw());
  matrix.yx = (FT_Fixed)((mat[1] / size).getRaw());
  matrix.xy = (FT_Fixed)((mat[2] / size).getRaw());
  matrix.yy = (FT_Fixed)((mat[3] / size).getRaw());
  textMatrix.xx = (FT_Fixed)((textMat[0] / (textScale * size)).getRaw());
  textMatrix.yx = (FT_Fixed)((textMat[1] / (textScale * size)).getRaw());
  textMatrix.xy = (FT_Fixed)((textMat[2] / (textScale * size)).getRaw());
  textMatrix.yy = (FT_Fixed)((textMat[3] / (textScale * size)).getRaw());
#else
  matrix.xx = (FT_Fixed)((mat[0] / size) * 65536);
  matrix.yx = (FT_Fixed)((mat[1] / size) * 65536);
  matrix.xy = (FT_Fixed)((mat[2] / size) * 65536);
  matrix.yy = (FT_Fixed)((mat[3] / size) * 65536);
  textMatrix.xx = (FT_Fixed)((textMat[0] / (size * textScale)) * 65536);
  textMatrix.yx = (FT_Fixed)((textMat[1] / (size * textScale)) * 65536);
  textMatrix.xy = (FT_Fixed)((textMat[2] / (size * textScale)) * 65536);
  textMatrix.yy = (FT_Fixed)((textMat[3] / (size * textScale)) * 65536);
#endif
}

SplashFTFont::~SplashFTFont() {
}

GBool SplashFTFont::getGlyph(int c, int xFrac, int yFrac,
			     SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes) {
  return SplashFont::getGlyph(c, xFrac, 0, bitmap, x0, y0, clip, clipRes);
}

static FT_Int32 getFTLoadFlags(GBool aa, GBool enableFreeTypeHinting)
{
  if (aa && enableFreeTypeHinting) return FT_LOAD_NO_BITMAP;
  else if (aa && !enableFreeTypeHinting) return FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
  else if (!aa && enableFreeTypeHinting) return FT_LOAD_DEFAULT;
  else return FT_LOAD_NO_HINTING;
}

GBool SplashFTFont::makeGlyph(int c, int xFrac, int yFrac,
			      SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes) {
  SplashFTFontFile *ff;
  FT_Vector offset;
  FT_GlyphSlot slot;
  FT_UInt gid;
  int rowSize;
  Guchar *p, *q;
  int i;

  ff = (SplashFTFontFile *)fontFile;

  ff->face->size = sizeObj;
  offset.x = (FT_Pos)(int)((SplashCoord)xFrac * splashFontFractionMul * 64);
  offset.y = 0;
  FT_Set_Transform(ff->face, &matrix, &offset);
  slot = ff->face->glyph;

  if (ff->codeToGID && c < ff->codeToGIDLen) {
    gid = (FT_UInt)ff->codeToGID[c];
  } else {
    gid = (FT_UInt)c;
  }

  if (FT_Load_Glyph(ff->face, gid, getFTLoadFlags(aa, enableFreeTypeHinting))) {
    return gFalse;
  }

  // prelimirary values based on FT_Outline_Get_CBox
  // we add two pixels to each side to be in the safe side
  FT_BBox cbox;
  FT_Outline_Get_CBox(&ff->face->glyph->outline, &cbox);
  bitmap->x = -(cbox.xMin / 64) + 2;
  bitmap->y =  (cbox.yMax / 64) + 2;
  bitmap->w = ((cbox.xMax - cbox.xMin) / 64) + 4;
  bitmap->h = ((cbox.yMax - cbox.yMin) / 64) + 4;

  *clipRes = clip->testRect(x0 - bitmap->x,
                            y0 - bitmap->y,
                            x0 - bitmap->x + bitmap->w,
                            y0 - bitmap->y + bitmap->h);
  if (*clipRes == splashClipAllOutside) {
    bitmap->freeData = gFalse;
    return gTrue;
  }

  if (FT_Render_Glyph(slot, aa ? ft_render_mode_normal
		               : ft_render_mode_mono)) {
    return gFalse;
  }

  bitmap->x = -slot->bitmap_left;
  bitmap->y = slot->bitmap_top;
  bitmap->w = slot->bitmap.width;
  bitmap->h = slot->bitmap.rows;
  bitmap->aa = aa;
  if (aa) {
    rowSize = bitmap->w;
  } else {
    rowSize = (bitmap->w + 7) >> 3;
  }
  bitmap->data = (Guchar *)gmallocn_checkoverflow(rowSize, bitmap->h);
  if (!bitmap->data) {
    return gFalse;
  }
  bitmap->freeData = gTrue;
  for (i = 0, p = bitmap->data, q = slot->bitmap.buffer;
       i < bitmap->h;
       ++i, p += rowSize, q += slot->bitmap.pitch) {
    memcpy(p, q, rowSize);
  }

  return gTrue;
}

double SplashFTFont::getGlyphAdvance(int c)
{
  SplashFTFontFile *ff;
  FT_Vector offset;
  FT_UInt gid;
  FT_Matrix identityMatrix;

  ff = (SplashFTFontFile *)fontFile;

  // init the matrix
  identityMatrix.xx = 65536; // 1 in 16.16 format
  identityMatrix.xy = 0;
  identityMatrix.yx = 0;
  identityMatrix.yy = 65536; // 1 in 16.16 format

  // init the offset
  offset.x = 0;
  offset.y = 0;

  ff->face->size = sizeObj;
  FT_Set_Transform(ff->face, &identityMatrix, &offset);

  if (ff->codeToGID && c < ff->codeToGIDLen) {
    gid = (FT_UInt)ff->codeToGID[c];
  } else {
    gid = (FT_UInt)c;
  }
  if (ff->trueType && gid == 0) {
    // skip the TrueType notdef glyph
    return -1;
  }

  if (FT_Load_Glyph(ff->face, gid, getFTLoadFlags(aa, enableFreeTypeHinting))) {
    return -1;
  }

  // 64.0 is 1 in 26.6 format
  return ff->face->glyph->metrics.horiAdvance / 64.0 / size;
}

struct SplashFTFontPath {
  SplashPath *path;
  SplashCoord textScale;
  GBool needClose;
};

SplashPath *SplashFTFont::getGlyphPath(int c) {
  static FT_Outline_Funcs outlineFuncs = {
#if FREETYPE_MINOR <= 1
    (int (*)(FT_Vector *, void *))&glyphPathMoveTo,
    (int (*)(FT_Vector *, void *))&glyphPathLineTo,
    (int (*)(FT_Vector *, FT_Vector *, void *))&glyphPathConicTo,
    (int (*)(FT_Vector *, FT_Vector *, FT_Vector *, void *))&glyphPathCubicTo,
#else
    &glyphPathMoveTo,
    &glyphPathLineTo,
    &glyphPathConicTo,
    &glyphPathCubicTo,
#endif
    0, 0
  };
  SplashFTFontFile *ff;
  SplashFTFontPath path;
  FT_GlyphSlot slot;
  FT_UInt gid;
  FT_Glyph glyph;

  ff = (SplashFTFontFile *)fontFile;
  ff->face->size = sizeObj;
  FT_Set_Transform(ff->face, &textMatrix, NULL);
  slot = ff->face->glyph;
  if (ff->codeToGID && c < ff->codeToGIDLen) {
    gid = ff->codeToGID[c];
  } else {
    gid = (FT_UInt)c;
  }
  if (ff->trueType && gid == 0) {
    // skip the TrueType notdef glyph
    return NULL;
  }
  if (FT_Load_Glyph(ff->face, gid, getFTLoadFlags(aa, enableFreeTypeHinting))) {
    return NULL;
  }
  if (FT_Get_Glyph(slot, &glyph)) {
    return NULL;
  }
  path.path = new SplashPath();
  path.textScale = textScale;
  path.needClose = gFalse;
  FT_Outline_Decompose(&((FT_OutlineGlyph)glyph)->outline,
		       &outlineFuncs, &path);
  if (path.needClose) {
    path.path->close();
  }
  FT_Done_Glyph(glyph);
  return path.path;
}

static int glyphPathMoveTo(const FT_Vector *pt, void *path) {
  SplashFTFontPath *p = (SplashFTFontPath *)path;

  if (p->needClose) {
    p->path->close();
    p->needClose = gFalse;
  }
  p->path->moveTo((SplashCoord)pt->x * p->textScale / 64.0,
		  (SplashCoord)pt->y * p->textScale / 64.0);
  return 0;
}

static int glyphPathLineTo(const FT_Vector *pt, void *path) {
  SplashFTFontPath *p = (SplashFTFontPath *)path;

  p->path->lineTo((SplashCoord)pt->x * p->textScale / 64.0,
		  (SplashCoord)pt->y * p->textScale / 64.0);
  p->needClose = gTrue;
  return 0;
}

static int glyphPathConicTo(const FT_Vector *ctrl, const FT_Vector *pt,
			    void *path) {
  SplashFTFontPath *p = (SplashFTFontPath *)path;
  SplashCoord x0, y0, x1, y1, x2, y2, x3, y3, xc, yc;

  if (!p->path->getCurPt(&x0, &y0)) {
    return 0;
  }
  xc = (SplashCoord)ctrl->x * p->textScale / 64.0;
  yc = (SplashCoord)ctrl->y * p->textScale / 64.0;
  x3 = (SplashCoord)pt->x * p->textScale / 64.0;
  y3 = (SplashCoord)pt->y * p->textScale / 64.0;

  // A second-order Bezier curve is defined by two endpoints, p0 and
  // p3, and one control point, pc:
  //
  //     p(t) = (1-t)^2*p0 + t*(1-t)*pc + t^2*p3
  //
  // A third-order Bezier curve is defined by the same two endpoints,
  // p0 and p3, and two control points, p1 and p2:
  //
  //     p(t) = (1-t)^3*p0 + 3t*(1-t)^2*p1 + 3t^2*(1-t)*p2 + t^3*p3
  //
  // Applying some algebra, we can convert a second-order curve to a
  // third-order curve:
  //
  //     p1 = (1/3) * (p0 + 2pc)
  //     p2 = (1/3) * (2pc + p3)

  x1 = (SplashCoord)(1.0 / 3.0) * (x0 + (SplashCoord)2 * xc);
  y1 = (SplashCoord)(1.0 / 3.0) * (y0 + (SplashCoord)2 * yc);
  x2 = (SplashCoord)(1.0 / 3.0) * ((SplashCoord)2 * xc + x3);
  y2 = (SplashCoord)(1.0 / 3.0) * ((SplashCoord)2 * yc + y3);

  p->path->curveTo(x1, y1, x2, y2, x3, y3);
  p->needClose = gTrue;
  return 0;
}

static int glyphPathCubicTo(const FT_Vector *ctrl1, const FT_Vector *ctrl2,
			    const FT_Vector *pt, void *path) {
  SplashFTFontPath *p = (SplashFTFontPath *)path;

  p->path->curveTo((SplashCoord)ctrl1->x * p->textScale / 64.0,
		   (SplashCoord)ctrl1->y * p->textScale / 64.0,
		   (SplashCoord)ctrl2->x * p->textScale / 64.0,
		   (SplashCoord)ctrl2->y * p->textScale / 64.0,
		   (SplashCoord)pt->x * p->textScale / 64.0,
		   (SplashCoord)pt->y * p->textScale / 64.0);
  p->needClose = gTrue;
  return 0;
}

#endif // HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
