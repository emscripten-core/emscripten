//========================================================================
//
// SplashT1Font.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#if HAVE_T1LIB_H

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <t1lib.h>
#include "goo/gmem.h"
#include "SplashMath.h"
#include "SplashGlyphBitmap.h"
#include "SplashPath.h"
#include "SplashT1FontEngine.h"
#include "SplashT1FontFile.h"
#include "SplashT1Font.h"

//------------------------------------------------------------------------

static Guchar bitReverse[256] = {
  0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
  0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
  0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
  0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
  0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
  0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
  0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
  0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
  0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
  0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
  0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
  0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
  0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
  0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
  0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
  0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
  0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
  0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
  0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
  0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
  0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
  0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
  0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
  0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
  0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
  0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
  0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
  0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
  0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
  0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
  0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
  0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

//------------------------------------------------------------------------
// SplashT1Font
//------------------------------------------------------------------------

SplashT1Font::SplashT1Font(SplashT1FontFile *fontFileA, SplashCoord *matA,
			   SplashCoord *textMatA):
  SplashFont(fontFileA, matA, textMatA, fontFileA->engine->aa)
{
  T1_TMATRIX matrix;
  BBox bbox;
  SplashCoord bbx0, bby0, bbx1, bby1;
  int x, y;

  t1libID = T1_CopyFont(fontFileA->t1libID);
  outlineID = -1;

  // compute font size
  size = (float)splashSqrt(mat[2]*mat[2] + mat[3]*mat[3]);

  // transform the four corners of the font bounding box -- the min
  // and max values form the bounding box of the transformed font
  bbox = T1_GetFontBBox(t1libID);
  bbx0 = 0.001 * bbox.llx;
  bby0 = 0.001 * bbox.lly;
  bbx1 = 0.001 * bbox.urx;
  bby1 = 0.001 * bbox.ury;
  // some fonts are completely broken, so we fake it (with values
  // large enough that most glyphs should fit)
  if (bbx0 == 0 && bby0 == 0 && bbx1 == 0 && bby1 == 0) {
    bbx0 = bby0 = -0.5;
    bbx1 = bby1 = 1.5;
  }
  x = (int)(mat[0] * bbx0 + mat[2] * bby0);
  xMin = xMax = x;
  y = (int)(mat[1] * bbx0 + mat[3] * bby0);
  yMin = yMax = y;
  x = (int)(mat[0] * bbx0 + mat[2] * bby1);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)(mat[1] * bbx0 + mat[3] * bby1);
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  x = (int)(mat[0] * bbx1 + mat[2] * bby0);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)(mat[1] * bbx1 + mat[3] * bby0);
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  x = (int)(mat[0] * bbx1 + mat[2] * bby1);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  y = (int)(mat[1] * bbx1 + mat[3] * bby1);
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
    yMax = (int)(1.2 * size);
  }
  // Another kludge: an unusually large xMin or yMin coordinate is
  // probably wrong.
  if (xMin > 0) {
    xMin = 0;
  }
  if (yMin > 0) {
    yMin = 0;
  }
  // Another kludge: t1lib doesn't correctly handle fonts with
  // real (non-integer) bounding box coordinates.
  if (xMax - xMin > 5000) {
    xMin = 0;
    xMax = (int)size;
  }
  if (yMax - yMin > 5000) {
    yMin = 0;
    yMax = (int)(1.2 * size);
  }

  // transform the font
  matrix.cxx = (double)mat[0] / size;
  matrix.cxy = (double)mat[1] / size;
  matrix.cyx = (double)mat[2] / size;
  matrix.cyy = (double)mat[3] / size;
  T1_TransformFont(t1libID, &matrix);
}

SplashT1Font::~SplashT1Font() {
  T1_DeleteFont(t1libID);
  if (outlineID >= 0) {
    T1_DeleteFont(outlineID);
  }
}

GBool SplashT1Font::getGlyph(int c, int xFrac, int yFrac,
			     SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes) {
  return SplashFont::getGlyph(c, 0, 0, bitmap, x0, y0, clip, clipRes);
}

GBool SplashT1Font::makeGlyph(int c, int xFrac, int yFrac,
			      SplashGlyphBitmap *bitmap, int x0, int y0, SplashClip *clip, SplashClipResult *clipRes) {
  GLYPH *glyph;
  int n, i;

  if (aa) {
    glyph = T1_AASetChar(t1libID, c, size, NULL);
  } else {
    glyph = T1_SetChar(t1libID, c, size, NULL);
  }
  if (!glyph) {
    return gFalse;
  }

  bitmap->x = -glyph->metrics.leftSideBearing;
  bitmap->y = glyph->metrics.ascent;
  bitmap->w = glyph->metrics.rightSideBearing - glyph->metrics.leftSideBearing;
  bitmap->h = glyph->metrics.ascent - glyph->metrics.descent;
  bitmap->aa = aa;
  if (aa) {
    bitmap->data = (Guchar *)glyph->bits;
    bitmap->freeData = gFalse;
  } else {
    n = bitmap->h * ((bitmap->w + 7) >> 3);
    bitmap->data = (Guchar *)gmalloc(n);
    for (i = 0; i < n; ++i) {
      bitmap->data[i] = bitReverse[glyph->bits[i] & 0xff];
    }
    bitmap->freeData = gTrue;
  }

  *clipRes = clip->testRect(x0 - bitmap->x,
                            y0 - bitmap->y,
                            x0 - bitmap->x + bitmap->w - 1,
                            y0 - bitmap->y + bitmap->h - 1);

  return gTrue;
}

SplashPath *SplashT1Font::getGlyphPath(int c) {
  T1_TMATRIX matrix;
  SplashPath *path;
  T1_OUTLINE *outline;
  T1_PATHSEGMENT *seg;
  T1_BEZIERSEGMENT *bez;
  SplashCoord x, y, x1, y1;
  GBool needClose;

  if (outlineID < 0) {
    outlineID = T1_CopyFont(((SplashT1FontFile *)fontFile)->t1libID);
    outlineSize = (float)splashSqrt(textMat[2]*textMat[2] +
				    textMat[3]*textMat[3]);
    matrix.cxx = (double)textMat[0] / outlineSize;
    matrix.cxy = (double)textMat[1] / outlineSize;
    matrix.cyx = (double)textMat[2] / outlineSize;
    matrix.cyy = (double)textMat[3] / outlineSize;
    // t1lib doesn't seem to handle small sizes correctly here, so set
    // the size to 1000, and scale the resulting coordinates later
    outlineMul = (float)(outlineSize / 65536000.0);
    outlineSize = 1000;
    T1_TransformFont(outlineID, &matrix);
  }

  path = new SplashPath();
  if ((outline = T1_GetCharOutline(outlineID, c, outlineSize, NULL))) {
    x = 0;
    y = 0;
    needClose = gFalse;
    for (seg = outline; seg; seg = seg->link) {
      switch (seg->type) {
      case T1_PATHTYPE_MOVE:
	if (needClose) {
	  path->close();
	  needClose = gFalse;
	}
	x += seg->dest.x * outlineMul;
	y += seg->dest.y * outlineMul;
	path->moveTo(x, -y);
	break;
      case T1_PATHTYPE_LINE:
	x += seg->dest.x * outlineMul;
	y += seg->dest.y * outlineMul;
	path->lineTo(x, -y);
	needClose = gTrue;
	break;
      case T1_PATHTYPE_BEZIER:
	bez = (T1_BEZIERSEGMENT *)seg;
	x1 = x + (SplashCoord)(bez->dest.x * outlineMul);
	y1 = y + (SplashCoord)(bez->dest.y * outlineMul);
	path->curveTo(x + (SplashCoord)(bez->B.x * outlineMul),
		      -(y + (SplashCoord)(bez->B.y * outlineMul)),
		      x + (SplashCoord)(bez->C.x * outlineMul),
		      -(y + (SplashCoord)(bez->C.y * outlineMul)),
		      x1, -y1);
	x = x1;
	y = y1;
	needClose = gTrue;
	break;
      }
    }
    if (needClose) {
      path->close();
    }
    T1_FreeOutline(outline);
  }

  return path;
}

#endif // HAVE_T1LIB_H
