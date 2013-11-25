//========================================================================
//
// CairoOutputDev.cc
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
// Copyright (C) 2005-2008 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2009, 2012 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Nickolay V. Shmyrev <nshmyrev@yandex.ru>
// Copyright (C) 2006-2011, 2013 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2008 Carl Worth <cworth@cworth.org>
// Copyright (C) 2008-2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008, 2009 Chris Wilson <chris@chris-wilson.co.uk>
// Copyright (C) 2008, 2012 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2009, 2010 David Benjamin <davidben@mit.edu>
// Copyright (C) 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Patrick Pfeifer <p2000@mailinator.com>
// Copyright (C) 2012 Jason Crain <jason@aquaticape.us>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <string.h>
#include <math.h>
#include <assert.h>
#include <cairo.h>

#include "goo/gfile.h"
#include "goo/gtypes_p.h"
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include "Gfx.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Page.h"
#include "Link.h"
#include "FontEncodingTables.h"
#include "PDFDocEncoding.h"
#include <fofi/FoFiTrueType.h>
#include <splash/SplashBitmap.h>
#include "CairoOutputDev.h"
#include "CairoFontEngine.h"
#include "CairoRescaleBox.h"
#include "UnicodeMap.h"
//------------------------------------------------------------------------

// #define LOG_CAIRO

#ifdef LOG_CAIRO
#define LOG(x) (x)
#else
#define LOG(x)
#endif

static inline void printMatrix(cairo_matrix_t *matrix){
	printf("%f %f, %f %f (%f %f)\n", matrix->xx, matrix->yx,
			matrix->xy, matrix->yy,
			matrix->x0, matrix->y0);
}


#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))


//------------------------------------------------------------------------
// CairoImage
//------------------------------------------------------------------------

CairoImage::CairoImage (double x1, double y1, double x2, double y2) {
  this->image = NULL;
  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
}

CairoImage::~CairoImage () {
  if (image)
    cairo_surface_destroy (image);
}

void CairoImage::setImage (cairo_surface_t *image) {
  if (this->image)
    cairo_surface_destroy (this->image);
  this->image = cairo_surface_reference (image);
}

//------------------------------------------------------------------------
// CairoOutputDev
//------------------------------------------------------------------------

// We cannot tie the lifetime of an FT_Library object to that of
// CairoOutputDev, since any FT_Faces created with it may end up with a
// reference by Cairo which can be held long after the CairoOutputDev is
// deleted.  The simplest way to avoid problems is to never tear down the
// FT_Library instance; to avoid leaks, just use a single global instance
// initialized the first time it is needed.
FT_Library CairoOutputDev::ft_lib;
GBool CairoOutputDev::ft_lib_initialized = gFalse;

CairoOutputDev::CairoOutputDev() {
  doc = NULL;

  if (!ft_lib_initialized) {
    FT_Init_FreeType(&ft_lib);
    ft_lib_initialized = gTrue;
  }

  fontEngine = NULL;
  fontEngine_owner = gFalse;
  glyphs = NULL;
  fill_pattern = NULL;
  fill_color.r = fill_color.g = fill_color.b = 0;
  stroke_pattern = NULL;
  stroke_color.r = stroke_color.g = stroke_color.b = 0;
  stroke_opacity = 1.0;
  fill_opacity = 1.0;
  textClipPath = NULL;
  strokePathClip = NULL;
  cairo = NULL;
  currentFont = NULL;
  prescaleImages = gTrue;
  printing = gTrue;
  use_show_text_glyphs = gFalse;
  inUncoloredPattern = gFalse;
  inType3Char = gFalse;
  t3_glyph_has_bbox = gFalse;

  groupColorSpaceStack = NULL;
  maskStack = NULL;
  group = NULL;
  mask = NULL;
  shape = NULL;
  cairo_shape = NULL;
  knockoutCount = 0;

  text = NULL;
  actualText = NULL;

  // the SA parameter supposedly defaults to false, but Acrobat
  // apparently hardwires it to true
  stroke_adjust = globalParams->getStrokeAdjust();
  align_stroke_coords = gFalse;
  adjusted_stroke_width = gFalse;
  xref = NULL;
}

CairoOutputDev::~CairoOutputDev() {
  if (fontEngine_owner && fontEngine) {
    delete fontEngine;
  }

  if (cairo)
    cairo_destroy (cairo);
  cairo_pattern_destroy (stroke_pattern);
  cairo_pattern_destroy (fill_pattern);
  if (group)
    cairo_pattern_destroy (group);
  if (mask)
    cairo_pattern_destroy (mask);
  if (shape)
    cairo_pattern_destroy (shape);
  if (text) 
    text->decRefCnt();
  if (actualText)
    delete actualText;  
}

void CairoOutputDev::setCairo(cairo_t *cairo)
{
  if (this->cairo != NULL) {
    cairo_status_t status = cairo_status (this->cairo);
    if (status) {
      error(errInternal, -1, "cairo context error: {0:s}\n", cairo_status_to_string(status));
    }
    cairo_destroy (this->cairo);
    assert(!cairo_shape);
  }
  if (cairo != NULL) {
    this->cairo = cairo_reference (cairo);
	/* save the initial matrix so that we can use it for type3 fonts. */
	//XXX: is this sufficient? could we miss changes to the matrix somehow?
	cairo_get_matrix(cairo, &orig_matrix);
  } else {
    this->cairo = NULL;
    this->cairo_shape = NULL;
  }
}

void CairoOutputDev::setTextPage(TextPage *text)
{
  if (this->text) 
    this->text->decRefCnt();
  if (actualText)
    delete actualText;
  if (text) {
    this->text = text;
    this->text->incRefCnt();
    actualText = new ActualText(text);
  } else {
    this->text = NULL;
    actualText = NULL;
  }
}

void CairoOutputDev::startDoc(PDFDoc *docA,
			      CairoFontEngine *parentFontEngine) {
  doc = docA;
  if (parentFontEngine) {
    fontEngine = parentFontEngine;
  } else {
    if (fontEngine) {
      delete fontEngine;
    }
    fontEngine = new CairoFontEngine(ft_lib);
    fontEngine_owner = gTrue;
  }
  xref = doc->getXRef();
}

void CairoOutputDev::startPage(int pageNum, GfxState *state, XRef *xrefA) {
  /* set up some per page defaults */
  cairo_pattern_destroy(fill_pattern);
  cairo_pattern_destroy(stroke_pattern);

  fill_pattern = cairo_pattern_create_rgb(0., 0., 0.);
  fill_color.r = fill_color.g = fill_color.b = 0;
  stroke_pattern = cairo_pattern_reference(fill_pattern);
  stroke_color.r = stroke_color.g = stroke_color.b = 0;

  if (text)
    text->startPage(state);
  if (xrefA != NULL) {
    xref = xrefA;
  }
}

void CairoOutputDev::endPage() {
  if (text) {
    text->endPage();
    text->coalesce(gTrue, 0, gFalse);
  }
}

void CairoOutputDev::saveState(GfxState *state) {
  LOG(printf ("save\n"));
  cairo_save (cairo);
  if (cairo_shape)
      cairo_save (cairo_shape);

  MaskStack *ms = new MaskStack;
  ms->mask = cairo_pattern_reference(mask);
  ms->mask_matrix = mask_matrix;
  ms->next = maskStack;
  maskStack = ms;
}

void CairoOutputDev::restoreState(GfxState *state) {
  LOG(printf ("restore\n"));
  cairo_restore (cairo);
  if (cairo_shape)
      cairo_restore (cairo_shape);

  /* These aren't restored by cairo_restore() since we keep them in
   * the output device. */
  updateFillColor(state);
  updateStrokeColor(state);
  updateFillOpacity(state);
  updateStrokeOpacity(state);
  updateBlendMode(state);

  MaskStack* ms = maskStack;
  if (ms) {
    if (mask)
      cairo_pattern_destroy(mask);
    mask = ms->mask;
    mask_matrix = ms->mask_matrix;
    maskStack = ms->next;
    delete ms;
  }
}

void CairoOutputDev::updateAll(GfxState *state) {
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
  updateFlatness(state);
  updateMiterLimit(state);
  updateFillColor(state);
  updateStrokeColor(state);
  updateFillOpacity(state);
  updateStrokeOpacity(state);
  updateBlendMode(state);
  needFontUpdate = gTrue;
  if (text)
    text->updateFont(state);
}

void CairoOutputDev::setDefaultCTM(double *ctm) {
  cairo_matrix_t matrix;
  matrix.xx = ctm[0];
  matrix.yx = ctm[1];
  matrix.xy = ctm[2];
  matrix.yy = ctm[3];
  matrix.x0 = ctm[4];
  matrix.y0 = ctm[5];

  cairo_transform (cairo, &matrix);
  if (cairo_shape)
      cairo_transform (cairo_shape, &matrix);

  OutputDev::setDefaultCTM(ctm);
}

void CairoOutputDev::updateCTM(GfxState *state, double m11, double m12,
				double m21, double m22,
				double m31, double m32) {
  cairo_matrix_t matrix, invert_matrix;
  matrix.xx = m11;
  matrix.yx = m12;
  matrix.xy = m21;
  matrix.yy = m22;
  matrix.x0 = m31;
  matrix.y0 = m32;

  /* Make sure the matrix is invertible before setting it.
   * cairo will blow up if we give it a matrix that's not
   * invertible, so we need to check before passing it
   * to cairo_transform. Ignoring it is likely to give better
   * results than not rendering anything at all. See #14398
   *
   * Ideally, we could do the cairo_transform
   * and then check if anything went wrong and fix it then
   * instead of having to invert the matrix. */
  invert_matrix = matrix;
  if (cairo_matrix_invert(&invert_matrix)) {
    error(errSyntaxWarning, -1, "matrix not invertible\n");
    return;
  }

  cairo_transform (cairo, &matrix);
  if (cairo_shape)
    cairo_transform (cairo_shape, &matrix);
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
}

void CairoOutputDev::updateLineDash(GfxState *state) {
  double *dashPattern;
  int dashLength;
  double dashStart;

  state->getLineDash(&dashPattern, &dashLength, &dashStart);
  cairo_set_dash (cairo, dashPattern, dashLength, dashStart);
  if (cairo_shape)
    cairo_set_dash (cairo_shape, dashPattern, dashLength, dashStart);
}

void CairoOutputDev::updateFlatness(GfxState *state) {
  // cairo_set_tolerance (cairo, state->getFlatness());
}

void CairoOutputDev::updateLineJoin(GfxState *state) {
  switch (state->getLineJoin()) {
  case 0:
    cairo_set_line_join (cairo, CAIRO_LINE_JOIN_MITER);
    break;
  case 1:
    cairo_set_line_join (cairo, CAIRO_LINE_JOIN_ROUND);
    break;
  case 2:
    cairo_set_line_join (cairo, CAIRO_LINE_JOIN_BEVEL);
    break;
  }
  if (cairo_shape)
    cairo_set_line_join (cairo_shape, cairo_get_line_join(cairo));
}

void CairoOutputDev::updateLineCap(GfxState *state) {
  switch (state->getLineCap()) {
  case 0:
    cairo_set_line_cap (cairo, CAIRO_LINE_CAP_BUTT);
    break;
  case 1:
    cairo_set_line_cap (cairo, CAIRO_LINE_CAP_ROUND);
    break;
  case 2:
    cairo_set_line_cap (cairo, CAIRO_LINE_CAP_SQUARE);
    break;
  }
  if (cairo_shape)
    cairo_set_line_cap (cairo_shape, cairo_get_line_cap(cairo));
}

void CairoOutputDev::updateMiterLimit(GfxState *state) {
  cairo_set_miter_limit (cairo, state->getMiterLimit());
  if (cairo_shape)
    cairo_set_miter_limit (cairo_shape, state->getMiterLimit());
}

void CairoOutputDev::updateLineWidth(GfxState *state) {
  LOG(printf ("line width: %f\n", state->getLineWidth()));
  adjusted_stroke_width = gFalse;
  double width = state->getLineWidth();
  if (stroke_adjust && !printing) {
    double x, y;
    x = y = width;

    /* find out line width in device units */
    cairo_user_to_device_distance(cairo, &x, &y);
    if (fabs(x) <= 1.0 && fabs(y) <= 1.0) {
      /* adjust width to at least one device pixel */
      x = y = 1.0;
      cairo_device_to_user_distance(cairo, &x, &y);
      width = MIN(fabs(x),fabs(y));
      adjusted_stroke_width = gTrue;
    }
  } else if (width == 0.0) {
    /* Cairo does not support 0 line width == 1 device pixel. Find out
     * how big pixels (device unit) are in the x and y
     * directions. Choose the smaller of the two as our line width.
     */
    double x = 1.0, y = 1.0;
    if (printing) {
      // assume printer pixel size is 1/600 inch
      x = 72.0/600;
      y = 72.0/600;
    }
    cairo_device_to_user_distance(cairo, &x, &y);
    width = MIN(fabs(x),fabs(y));
  }
  cairo_set_line_width (cairo, width);
  if (cairo_shape)
    cairo_set_line_width (cairo_shape, cairo_get_line_width (cairo));
}

void CairoOutputDev::updateFillColor(GfxState *state) {
  GfxRGB color = fill_color;

  if (inUncoloredPattern)
    return;

  state->getFillRGB(&fill_color);
  if (cairo_pattern_get_type (fill_pattern) != CAIRO_PATTERN_TYPE_SOLID ||
      color.r != fill_color.r ||
      color.g != fill_color.g ||
      color.b != fill_color.b)
  {
    cairo_pattern_destroy(fill_pattern);
    fill_pattern = cairo_pattern_create_rgba(colToDbl(fill_color.r),
					     colToDbl(fill_color.g),
					     colToDbl(fill_color.b),
					     fill_opacity);

    LOG(printf ("fill color: %d %d %d\n",
		fill_color.r, fill_color.g, fill_color.b));
  }
}

void CairoOutputDev::updateStrokeColor(GfxState *state) {
  GfxRGB color = stroke_color;

  if (inUncoloredPattern)
    return;

  state->getStrokeRGB(&stroke_color);
  if (cairo_pattern_get_type (fill_pattern) != CAIRO_PATTERN_TYPE_SOLID ||
      color.r != stroke_color.r ||
      color.g != stroke_color.g ||
      color.b != stroke_color.b)
  {
    cairo_pattern_destroy(stroke_pattern);
    stroke_pattern = cairo_pattern_create_rgba(colToDbl(stroke_color.r),
					       colToDbl(stroke_color.g),
					       colToDbl(stroke_color.b),
					       stroke_opacity);

    LOG(printf ("stroke color: %d %d %d\n",
		stroke_color.r, stroke_color.g, stroke_color.b));
  }
}

void CairoOutputDev::updateFillOpacity(GfxState *state) {
  double opacity = fill_opacity;

  if (inUncoloredPattern)
    return;

  fill_opacity = state->getFillOpacity();
  if (opacity != fill_opacity) {
    cairo_pattern_destroy(fill_pattern);
    fill_pattern = cairo_pattern_create_rgba(colToDbl(fill_color.r),
					     colToDbl(fill_color.g),
					     colToDbl(fill_color.b),
					     fill_opacity);

    LOG(printf ("fill opacity: %f\n", fill_opacity));
  }
}

void CairoOutputDev::updateStrokeOpacity(GfxState *state) {
  double opacity = stroke_opacity;

  if (inUncoloredPattern)
    return;

  stroke_opacity = state->getStrokeOpacity();
  if (opacity != stroke_opacity) {
    cairo_pattern_destroy(stroke_pattern);
    stroke_pattern = cairo_pattern_create_rgba(colToDbl(stroke_color.r),
					       colToDbl(stroke_color.g),
					       colToDbl(stroke_color.b),
					       stroke_opacity);

    LOG(printf ("stroke opacity: %f\n", stroke_opacity));
  }
}

void CairoOutputDev::updateFillColorStop(GfxState *state, double offset) {
  if (inUncoloredPattern)
    return;

  state->getFillRGB(&fill_color);

  cairo_pattern_add_color_stop_rgba(fill_pattern, offset,
				    colToDbl(fill_color.r),
				    colToDbl(fill_color.g),
				    colToDbl(fill_color.b),
				    fill_opacity);
  LOG(printf ("fill color stop: %f (%d, %d, %d)\n",
	      offset, fill_color.r, fill_color.g, fill_color.b));
}

void CairoOutputDev::updateBlendMode(GfxState *state) {
  switch (state->getBlendMode()) {
  default:
  case gfxBlendNormal:
    cairo_set_operator (cairo, CAIRO_OPERATOR_OVER);
    break;
  case gfxBlendMultiply:
    cairo_set_operator (cairo, CAIRO_OPERATOR_MULTIPLY);
    break;
  case gfxBlendScreen:
    cairo_set_operator (cairo, CAIRO_OPERATOR_SCREEN);
    break;
  case gfxBlendOverlay:
    cairo_set_operator (cairo, CAIRO_OPERATOR_OVERLAY);
    break;
  case gfxBlendDarken:
    cairo_set_operator (cairo, CAIRO_OPERATOR_DARKEN);
    break;
  case gfxBlendLighten:
    cairo_set_operator (cairo, CAIRO_OPERATOR_LIGHTEN);
    break;
  case gfxBlendColorDodge:
    cairo_set_operator (cairo, CAIRO_OPERATOR_COLOR_DODGE);
    break;
  case gfxBlendColorBurn:
    cairo_set_operator (cairo, CAIRO_OPERATOR_COLOR_BURN);
    break;
  case gfxBlendHardLight:
    cairo_set_operator (cairo, CAIRO_OPERATOR_HARD_LIGHT);
    break;
  case gfxBlendSoftLight:
    cairo_set_operator (cairo, CAIRO_OPERATOR_SOFT_LIGHT);
    break;
  case gfxBlendDifference:
    cairo_set_operator (cairo, CAIRO_OPERATOR_DIFFERENCE);
    break;
  case gfxBlendExclusion:
    cairo_set_operator (cairo, CAIRO_OPERATOR_EXCLUSION);
    break;
  case gfxBlendHue:
    cairo_set_operator (cairo, CAIRO_OPERATOR_HSL_HUE);
    break;
  case gfxBlendSaturation:
    cairo_set_operator (cairo, CAIRO_OPERATOR_HSL_SATURATION);
    break;
  case gfxBlendColor:
    cairo_set_operator (cairo, CAIRO_OPERATOR_HSL_COLOR);
    break;
  case gfxBlendLuminosity:
    cairo_set_operator (cairo, CAIRO_OPERATOR_HSL_LUMINOSITY);
    break;
  }
  LOG(printf ("blend mode: %d\n", (int)state->getBlendMode()));
}

void CairoOutputDev::updateFont(GfxState *state) {
  cairo_font_face_t *font_face;
  cairo_matrix_t matrix, invert_matrix;

  LOG(printf ("updateFont() font=%s\n", state->getFont()->getName()->getCString()));

  needFontUpdate = gFalse;

  //FIXME: use cairo font engine?
  if (text)
    text->updateFont(state);
  
  currentFont = fontEngine->getFont (state->getFont(), doc, printing, xref);

  if (!currentFont)
    return;

  font_face = currentFont->getFontFace();
  cairo_set_font_face (cairo, font_face);

  use_show_text_glyphs = state->getFont()->hasToUnicodeCMap() &&
    cairo_surface_has_show_text_glyphs (cairo_get_target (cairo));
 
  double fontSize = state->getFontSize();
  double *m = state->getTextMat();
  /* NOTE: adjusting by a constant is hack. The correct solution
   * is probably to use user-fonts and compute the scale on a per
   * glyph basis instead of for the entire font */
  double w = currentFont->getSubstitutionCorrection(state->getFont());
  matrix.xx = m[0] * fontSize * state->getHorizScaling() * w;
  matrix.yx = m[1] * fontSize * state->getHorizScaling() * w;
  matrix.xy = -m[2] * fontSize;
  matrix.yy = -m[3] * fontSize;
  matrix.x0 = 0;
  matrix.y0 = 0;

  LOG(printf ("font matrix: %f %f %f %f\n", matrix.xx, matrix.yx, matrix.xy, matrix.yy));

 /* Make sure the font matrix is invertible before setting it.  cairo
  * will blow up if we give it a matrix that's not invertible, so we
  * need to check before passing it to cairo_set_font_matrix. Ignoring it
  * is likely to give better results than not rendering anything at
  * all. See #18254.
  */
  invert_matrix = matrix;
  if (cairo_matrix_invert(&invert_matrix)) {
    error(errSyntaxWarning, -1, "font matrix not invertible\n");
    return;
  }

  cairo_set_font_matrix (cairo, &matrix);
}

/* Tolerance in pixels for checking if strokes are horizontal or vertical
 * lines in device space */
#define STROKE_COORD_TOLERANCE 0.5

/* Align stroke coordinate i if the point is the start or end of a
 * horizontal or vertical line */
void CairoOutputDev::alignStrokeCoords(GfxSubpath *subpath, int i, double *x, double *y)
{
  double x1, y1, x2, y2;
  GBool align = gFalse;

  x1 = subpath->getX(i);
  y1 = subpath->getY(i);
  cairo_user_to_device (cairo, &x1, &y1);

  // Does the current coord and prev coord form a horiz or vert line?
  if (i > 0 && !subpath->getCurve(i - 1)) {
    x2 = subpath->getX(i - 1);
    y2 = subpath->getY(i - 1);
    cairo_user_to_device (cairo, &x2, &y2);
    if (fabs(x2 - x1) < STROKE_COORD_TOLERANCE || fabs(y2 - y1) < STROKE_COORD_TOLERANCE)
      align = gTrue;
  }

  // Does the current coord and next coord form a horiz or vert line?
  if (i < subpath->getNumPoints() - 1 && !subpath->getCurve(i + 1)) {
    x2 = subpath->getX(i + 1);
    y2 = subpath->getY(i + 1);
    cairo_user_to_device (cairo, &x2, &y2);
    if (fabs(x2 - x1) < STROKE_COORD_TOLERANCE || fabs(y2 - y1) < STROKE_COORD_TOLERANCE)
      align = gTrue;
  }

  *x = subpath->getX(i);
  *y = subpath->getY(i);
  if (align) {
    /* see http://www.cairographics.org/FAQ/#sharp_lines */
    cairo_user_to_device (cairo, x, y);
    *x = floor(*x) + 0.5;
    *y = floor(*y) + 0.5;
    cairo_device_to_user (cairo, x, y);
  }
}

#undef STROKE_COORD_TOLERANCE

void CairoOutputDev::doPath(cairo_t *cairo, GfxState *state, GfxPath *path) {
  GfxSubpath *subpath;
  int i, j;
  double x, y;
  cairo_new_path (cairo);
  for (i = 0; i < path->getNumSubpaths(); ++i) {
    subpath = path->getSubpath(i);
    if (subpath->getNumPoints() > 0) {
      if (align_stroke_coords) {
        alignStrokeCoords(subpath, 0, &x, &y);
      } else {
        x = subpath->getX(0);
        y = subpath->getY(0);
      }
      cairo_move_to (cairo, x, y);
      j = 1;
      while (j < subpath->getNumPoints()) {
	if (subpath->getCurve(j)) {
	  if (align_stroke_coords) {
            alignStrokeCoords(subpath, j + 2, &x, &y);
          } else {
            x = subpath->getX(j+2);
            y = subpath->getY(j+2);
          }
	  cairo_curve_to( cairo,
			  subpath->getX(j), subpath->getY(j),
			  subpath->getX(j+1), subpath->getY(j+1),
			  x, y);

	  j += 3;
	} else {
	  if (align_stroke_coords) {
            alignStrokeCoords(subpath, j, &x, &y);
          } else {
            x = subpath->getX(j);
            y = subpath->getY(j);
          }
          cairo_line_to (cairo, x, y);
	  ++j;
	}
      }
      if (subpath->isClosed()) {
	LOG (printf ("close\n"));
	cairo_close_path (cairo);
      }
    }
  }
}

void CairoOutputDev::stroke(GfxState *state) {
  if (inType3Char) {
      GfxGray gray;
      state->getFillGray(&gray);
      if (colToDbl(gray) > 0.5)
	  return;
  }

  if (adjusted_stroke_width)
    align_stroke_coords = gTrue;
  doPath (cairo, state, state->getPath());
  align_stroke_coords = gFalse;
  cairo_set_source (cairo, stroke_pattern);
  LOG(printf ("stroke\n"));
  cairo_stroke (cairo);
  if (cairo_shape) {
    doPath (cairo_shape, state, state->getPath());
    cairo_stroke (cairo_shape);
  }
}

void CairoOutputDev::fill(GfxState *state) {
  if (inType3Char) {
      GfxGray gray;
      state->getFillGray(&gray);
      if (colToDbl(gray) > 0.5)
	  return;
  }

  doPath (cairo, state, state->getPath());
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_WINDING);
  cairo_set_source (cairo, fill_pattern);
  LOG(printf ("fill\n"));
  //XXX: how do we get the path
  if (mask) {
    cairo_save (cairo);
    cairo_clip (cairo);
    cairo_set_matrix (cairo, &mask_matrix);
    cairo_mask (cairo, mask);
    cairo_restore (cairo);
  } else if (strokePathClip) {
    fillToStrokePathClip(state);
  } else {
    cairo_fill (cairo);
  }
  if (cairo_shape) {
    cairo_set_fill_rule (cairo_shape, CAIRO_FILL_RULE_WINDING);
    doPath (cairo_shape, state, state->getPath());
    cairo_fill (cairo_shape);
  }
}

void CairoOutputDev::eoFill(GfxState *state) {
  doPath (cairo, state, state->getPath());
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_EVEN_ODD);
  cairo_set_source (cairo, fill_pattern);
  LOG(printf ("fill-eo\n"));
  cairo_fill (cairo);

  if (cairo_shape) {
    cairo_set_fill_rule (cairo_shape, CAIRO_FILL_RULE_EVEN_ODD);
    doPath (cairo_shape, state, state->getPath());
    cairo_fill (cairo_shape);
  }

}

GBool CairoOutputDev::tilingPatternFill(GfxState *state, Gfx *gfxA, Catalog *cat, Object *str,
					double *pmat, int paintType, int /*tilingType*/, Dict *resDict,
					double *mat, double *bbox,
					int x0, int y0, int x1, int y1,
					double xStep, double yStep)
{
  PDFRectangle box;
  Gfx *gfx;
  cairo_pattern_t *pattern;
  cairo_surface_t *surface;
  cairo_matrix_t matrix;
  cairo_t *old_cairo;
  double xMin, yMin, xMax, yMax;
  double width, height;
  int surface_width, surface_height;
  StrokePathClip *strokePathTmp;

  width = bbox[2] - bbox[0];
  height = bbox[3] - bbox[1];

  if (xStep != width || yStep != height)
    return gFalse;
  /* TODO: implement the other cases here too */

  surface_width = (int) ceil (width);
  surface_height = (int) ceil (height);

  surface = cairo_surface_create_similar (cairo_get_target (cairo),
					  CAIRO_CONTENT_COLOR_ALPHA,
					  surface_width, surface_height);
  if (cairo_surface_status (surface))
    return gFalse;

  old_cairo = cairo;
  cairo = cairo_create (surface);
  cairo_surface_destroy (surface);

  box.x1 = bbox[0]; box.y1 = bbox[1];
  box.x2 = bbox[2]; box.y2 = bbox[3];
  strokePathTmp = strokePathClip;
  strokePathClip = NULL;
  gfx = new Gfx(doc, this, resDict, &box, NULL, NULL, NULL, gfxA->getXRef());
  if (paintType == 2)
    inUncoloredPattern = gTrue;
  gfx->display(str);
  if (paintType == 2)
    inUncoloredPattern = gFalse;
  delete gfx;
  strokePathClip = strokePathTmp;

  pattern = cairo_pattern_create_for_surface (cairo_get_target (cairo));
  cairo_destroy (cairo);
  cairo = old_cairo;
  if (cairo_pattern_status (pattern))
    return gFalse;

  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
  cairo_rectangle (cairo, xMin, yMin, xMax - xMin, yMax - yMin);

  cairo_matrix_init_scale (&matrix, surface_width / width, surface_height / height);
  cairo_pattern_set_matrix (pattern, &matrix);

  cairo_matrix_init (&matrix, mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  cairo_transform (cairo, &matrix);
  cairo_set_source (cairo, pattern);
  cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
  if (strokePathClip) {
    fillToStrokePathClip(state);
  } else {
    cairo_fill (cairo);
  }

  cairo_pattern_destroy (pattern);

  return gTrue;
}

GBool CairoOutputDev::axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax) {
  double x0, y0, x1, y1;
  double dx, dy;

  shading->getCoords(&x0, &y0, &x1, &y1);
  dx = x1 - x0;
  dy = y1 - y0;

  cairo_pattern_destroy(fill_pattern);
  fill_pattern = cairo_pattern_create_linear (x0 + tMin * dx, y0 + tMin * dy,
					      x0 + tMax * dx, y0 + tMax * dy);
  if (!shading->getExtend0() && !shading->getExtend1())
    cairo_pattern_set_extend (fill_pattern, CAIRO_EXTEND_NONE);
  else
    cairo_pattern_set_extend (fill_pattern, CAIRO_EXTEND_PAD);

  LOG (printf ("axial-sh\n"));

  // TODO: use the actual stops in the shading in the case
  // of linear interpolation (Type 2 Exponential functions with N=1)
  return gFalse;
}

GBool CairoOutputDev::axialShadedSupportExtend(GfxState *state, GfxAxialShading *shading)
{
  return (shading->getExtend0() == shading->getExtend1());
}

GBool CairoOutputDev::radialShadedFill(GfxState *state, GfxRadialShading *shading, double sMin, double sMax) {
  double x0, y0, r0, x1, y1, r1;
  double dx, dy, dr;

  shading->getCoords(&x0, &y0, &r0, &x1, &y1, &r1);
  dx = x1 - x0;
  dy = y1 - y0;
  dr = r1 - r0;
  cairo_pattern_destroy(fill_pattern);
  fill_pattern = cairo_pattern_create_radial (x0 + sMin * dx,
					      y0 + sMin * dy,
					      r0 + sMin * dr,
					      x0 + sMax * dx,
					      y0 + sMax * dy,
					      r0 + sMax * dr);
  if (shading->getExtend0() && shading->getExtend1())
    cairo_pattern_set_extend (fill_pattern, CAIRO_EXTEND_PAD);
  else
    cairo_pattern_set_extend (fill_pattern, CAIRO_EXTEND_NONE);

  LOG (printf ("radial-sh\n"));

  return gFalse;
}

GBool CairoOutputDev::radialShadedSupportExtend(GfxState *state, GfxRadialShading *shading)
{
  return (shading->getExtend0() == shading->getExtend1());
}

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
GBool CairoOutputDev::gouraudTriangleShadedFill(GfxState *state, GfxGouraudTriangleShading *shading)
{
  double x0, y0, x1, y1, x2, y2;
  GfxColor color[3];
  int i, j;
  GfxRGB rgb;

  cairo_pattern_destroy(fill_pattern);
  fill_pattern = cairo_pattern_create_mesh ();

  for (i = 0; i < shading->getNTriangles(); i++) {
    if (shading->isParameterized()) {
      double color0, color1, color2;
      shading->getTriangle(i, &x0, &y0, &color0,
                              &x1, &y1, &color1,
                              &x2, &y2, &color2);
      shading->getParameterizedColor(color0, &color[0]);
      shading->getParameterizedColor(color1, &color[1]);
      shading->getParameterizedColor(color2, &color[2]);
    } else {
      shading->getTriangle(i,
                           &x0, &y0, &color[0],
                           &x1, &y1, &color[1],
                           &x2, &y2, &color[2]);

    }

    cairo_mesh_pattern_begin_patch (fill_pattern);

    cairo_mesh_pattern_move_to (fill_pattern, x0, y0);
    cairo_mesh_pattern_line_to (fill_pattern, x1, y1);
    cairo_mesh_pattern_line_to (fill_pattern, x2, y2);

    for (j = 0; j < 3; j++) {
	shading->getColorSpace()->getRGB(&color[j], &rgb);
	cairo_mesh_pattern_set_corner_color_rgb (fill_pattern, j,
						 colToDbl(rgb.r),
						 colToDbl(rgb.g),
						 colToDbl(rgb.b));
    }

    cairo_mesh_pattern_end_patch (fill_pattern);
  }

  double xMin, yMin, xMax, yMax;
  // get the clip region bbox
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
  state->moveTo(xMin, yMin);
  state->lineTo(xMin, yMax);
  state->lineTo(xMax, yMax);
  state->lineTo(xMax, yMin);
  state->closePath();
  fill(state);
  state->clearPath();

  return gTrue;
}

GBool CairoOutputDev::patchMeshShadedFill(GfxState *state, GfxPatchMeshShading *shading)
{
  int i, j, k;

  cairo_pattern_destroy(fill_pattern);
  fill_pattern = cairo_pattern_create_mesh ();

  for (i = 0; i < shading->getNPatches(); i++) {
    GfxPatch *patch = shading->getPatch(i);
    GfxColor color;
    GfxRGB rgb;

    cairo_mesh_pattern_begin_patch (fill_pattern);

    cairo_mesh_pattern_move_to (fill_pattern, patch->x[0][0], patch->y[0][0]);
    cairo_mesh_pattern_curve_to (fill_pattern,
			    patch->x[0][1], patch->y[0][1],
			    patch->x[0][2], patch->y[0][2],
			    patch->x[0][3], patch->y[0][3]);

    cairo_mesh_pattern_curve_to (fill_pattern,
			    patch->x[1][3], patch->y[1][3],
			    patch->x[2][3], patch->y[2][3],
			    patch->x[3][3], patch->y[3][3]);

    cairo_mesh_pattern_curve_to (fill_pattern,
			    patch->x[3][2], patch->y[3][2],
			    patch->x[3][1], patch->y[3][1],
			    patch->x[3][0], patch->y[3][0]);

    cairo_mesh_pattern_curve_to (fill_pattern,
			    patch->x[2][0], patch->y[2][0],
			    patch->x[1][0], patch->y[1][0],
			    patch->x[0][0], patch->y[0][0]);

    cairo_mesh_pattern_set_control_point (fill_pattern, 0, patch->x[1][1], patch->y[1][1]);
    cairo_mesh_pattern_set_control_point (fill_pattern, 1, patch->x[1][2], patch->y[1][2]);
    cairo_mesh_pattern_set_control_point (fill_pattern, 2, patch->x[2][2], patch->y[2][2]);
    cairo_mesh_pattern_set_control_point (fill_pattern, 3, patch->x[2][1], patch->y[2][1]);

    for (j = 0; j < 4; j++) {
      int u, v;

      switch (j) {
	case 0:
	  u = 0; v = 0;
	  break;
	case 1:
	  u = 0; v = 1;
	  break;
	case 2:
	  u = 1; v = 1;
	  break;
	case 3:
	  u = 1; v = 0;
	  break;
      }

      if (shading->isParameterized()) {
	shading->getParameterizedColor (patch->color[u][v].c[0], &color);
      } else {
	for (k = 0; k < shading->getColorSpace()->getNComps(); k++) {
          // simply cast to the desired type; that's all what is needed.
	  color.c[k] = GfxColorComp (patch->color[u][v].c[k]);
	}
      }

      shading->getColorSpace()->getRGB(&color, &rgb);
      cairo_mesh_pattern_set_corner_color_rgb (fill_pattern, j,
					       colToDbl(rgb.r),
					       colToDbl(rgb.g),
					       colToDbl(rgb.b));
    }
    cairo_mesh_pattern_end_patch (fill_pattern);
  }

  double xMin, yMin, xMax, yMax;
  // get the clip region bbox
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
  state->moveTo(xMin, yMin);
  state->lineTo(xMin, yMax);
  state->lineTo(xMax, yMax);
  state->lineTo(xMax, yMin);
  state->closePath();
  fill(state);
  state->clearPath();

  return gTrue;
}
#endif /* CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0) */

void CairoOutputDev::clip(GfxState *state) {
  doPath (cairo, state, state->getPath());
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_WINDING);
  cairo_clip (cairo);
  LOG (printf ("clip\n"));
  if (cairo_shape) {
    doPath (cairo_shape, state, state->getPath());
    cairo_set_fill_rule (cairo_shape, CAIRO_FILL_RULE_WINDING);
    cairo_clip (cairo_shape);
  }
}

void CairoOutputDev::eoClip(GfxState *state) {
  doPath (cairo, state, state->getPath());
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_EVEN_ODD);
  cairo_clip (cairo);
  LOG (printf ("clip-eo\n"));
  if (cairo_shape) {
    doPath (cairo_shape, state, state->getPath());
    cairo_set_fill_rule (cairo_shape, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_clip (cairo_shape);
  }

}

void CairoOutputDev::clipToStrokePath(GfxState *state) {
  LOG(printf("clip-to-stroke-path\n"));
  strokePathClip = (StrokePathClip*)gmalloc (sizeof(*strokePathClip));
  strokePathClip->path = state->getPath()->copy();
  cairo_get_matrix (cairo, &strokePathClip->ctm);
  strokePathClip->line_width = cairo_get_line_width (cairo);
  strokePathClip->dash_count = cairo_get_dash_count (cairo);
  if (strokePathClip->dash_count) {
    strokePathClip->dashes = (double*) gmallocn (sizeof(double), strokePathClip->dash_count);
    cairo_get_dash (cairo, strokePathClip->dashes, &strokePathClip->dash_offset);
  } else {
    strokePathClip->dashes = NULL;
  }
  strokePathClip->cap = cairo_get_line_cap (cairo);
  strokePathClip->join = cairo_get_line_join (cairo);
  strokePathClip->miter = cairo_get_miter_limit (cairo);
}

void CairoOutputDev::fillToStrokePathClip(GfxState *state) {
  cairo_save (cairo);

  cairo_set_matrix (cairo, &strokePathClip->ctm);
  cairo_set_line_width (cairo, strokePathClip->line_width);
  strokePathClip->dash_count = cairo_get_dash_count (cairo);
  cairo_set_dash (cairo, strokePathClip->dashes, strokePathClip->dash_count, strokePathClip->dash_offset);
  cairo_set_line_cap (cairo, strokePathClip->cap);
  cairo_set_line_join (cairo, strokePathClip->join);
  cairo_set_miter_limit (cairo, strokePathClip->miter);
  doPath (cairo, state, strokePathClip->path);
  cairo_stroke (cairo);

  cairo_restore (cairo);

  delete strokePathClip->path;
  if (strokePathClip->dashes)
    gfree (strokePathClip->dashes);
  gfree (strokePathClip);
  strokePathClip = NULL;
}

void CairoOutputDev::beginString(GfxState *state, GooString *s)
{
  int len = s->getLength();

  if (needFontUpdate)
    updateFont(state);

  if (!currentFont)
    return;

  glyphs = (cairo_glyph_t *) gmallocn (len, sizeof (cairo_glyph_t));
  glyphCount = 0;
  if (use_show_text_glyphs) {
    clusters = (cairo_text_cluster_t *) gmallocn (len, sizeof (cairo_text_cluster_t));
    clusterCount = 0;
    utf8Max = len*2; // start with twice the number of glyphs. we will realloc if we need more.
    utf8 = (char *) gmalloc (utf8Max);
    utf8Count = 0;
  }
}

void CairoOutputDev::drawChar(GfxState *state, double x, double y,
			      double dx, double dy,
			      double originX, double originY,
			      CharCode code, int nBytes, Unicode *u, int uLen)
{
  if (currentFont) {
    glyphs[glyphCount].index = currentFont->getGlyph (code, u, uLen);
    glyphs[glyphCount].x = x - originX;
    glyphs[glyphCount].y = y - originY;
    glyphCount++;
    if (use_show_text_glyphs) {
      GooString enc("UTF-8");
      UnicodeMap *utf8Map = globalParams->getUnicodeMap(&enc);
      if (utf8Max - utf8Count < uLen*6) {
        // utf8 encoded characters can be up to 6 bytes
	if (utf8Max > uLen*6)
	  utf8Max *= 2;
	else
	  utf8Max += 2*uLen*6;
	utf8 = (char *) grealloc (utf8, utf8Max);
      }
      clusters[clusterCount].num_bytes = 0;
      for (int i = 0; i < uLen; i++) {
	int size = utf8Map->mapUnicode(u[i], utf8 + utf8Count, utf8Max - utf8Count);
	utf8Count += size;
	clusters[clusterCount].num_bytes += size;
      }
      clusters[clusterCount].num_glyphs = 1;
      clusterCount++;
    }
  }

  if (!text)
    return;
  actualText->addChar (state, x, y, dx, dy, code, nBytes, u, uLen);
}

void CairoOutputDev::endString(GfxState *state)
{
  int render;

  if (!currentFont)
    return;

  // endString can be called without a corresponding beginString. If this
  // happens glyphs will be null so don't draw anything, just return.
  // XXX: OutputDevs should probably not have to deal with this...
  if (!glyphs)
    return;

  // ignore empty strings and invisible text -- this is used by
  // Acrobat Capture
  render = state->getRender();
  if (render == 3 || glyphCount == 0) {
    gfree(glyphs);
    glyphs = NULL;
    return;
  }

  if (!(render & 1)) {
    LOG (printf ("fill string\n"));
    cairo_set_source (cairo, fill_pattern);
    if (use_show_text_glyphs)
      cairo_show_text_glyphs (cairo, utf8, utf8Count, glyphs, glyphCount, clusters, clusterCount, (cairo_text_cluster_flags_t)0);
    else
        cairo_show_glyphs (cairo, glyphs, glyphCount);
    if (cairo_shape)
      cairo_show_glyphs (cairo_shape, glyphs, glyphCount);
  }

  // stroke
  if ((render & 3) == 1 || (render & 3) == 2) {
    LOG (printf ("stroke string\n"));
    cairo_set_source (cairo, stroke_pattern);
    cairo_glyph_path (cairo, glyphs, glyphCount);
    cairo_stroke (cairo);
    if (cairo_shape) {
      cairo_glyph_path (cairo_shape, glyphs, glyphCount);
      cairo_stroke (cairo_shape);
    }
  }

  // clip
  if ((render & 4)) {
    LOG (printf ("clip string\n"));
    // append the glyph path to textClipPath.

    // set textClipPath as the currentPath
    if (textClipPath) {
      cairo_append_path (cairo, textClipPath);
      if (cairo_shape) {
	cairo_append_path (cairo_shape, textClipPath);
      }
      cairo_path_destroy (textClipPath);
    }
    
    // append the glyph path
    cairo_glyph_path (cairo, glyphs, glyphCount);
   
    // move the path back into textClipPath 
    // and clear the current path
    textClipPath = cairo_copy_path (cairo);
    cairo_new_path (cairo);
    if (cairo_shape) {
      cairo_new_path (cairo_shape);
    }
  }

  gfree (glyphs);
  glyphs = NULL;
  if (use_show_text_glyphs) {
    gfree (clusters);
    clusters = NULL;
    gfree (utf8);
    utf8 = NULL;
  }
}


GBool CairoOutputDev::beginType3Char(GfxState *state, double x, double y,
				      double dx, double dy,
				      CharCode code, Unicode *u, int uLen) {

  cairo_save (cairo);
  double *ctm;
  cairo_matrix_t matrix;

  ctm = state->getCTM();
  matrix.xx = ctm[0];
  matrix.yx = ctm[1];
  matrix.xy = ctm[2];
  matrix.yy = ctm[3];
  matrix.x0 = ctm[4];
  matrix.y0 = ctm[5];
  /* Restore the original matrix and then transform to matrix needed for the
   * type3 font. This is ugly but seems to work. Perhaps there is a better way to do it?*/
  cairo_set_matrix(cairo, &orig_matrix);
  cairo_transform(cairo, &matrix);
  if (cairo_shape) {
    cairo_save (cairo_shape);
    cairo_set_matrix(cairo_shape, &orig_matrix);
    cairo_transform(cairo_shape, &matrix);
  }
  cairo_pattern_destroy(stroke_pattern);
  cairo_pattern_reference(fill_pattern);
  stroke_pattern = fill_pattern;
  return gFalse;
}

void CairoOutputDev::endType3Char(GfxState *state) {
  cairo_restore (cairo);
  if (cairo_shape) {
    cairo_restore (cairo_shape);
  }
}

void CairoOutputDev::type3D0(GfxState *state, double wx, double wy) {
  t3_glyph_wx = wx;
  t3_glyph_wy = wy;
}

void CairoOutputDev::type3D1(GfxState *state, double wx, double wy,
			     double llx, double lly, double urx, double ury) {
  t3_glyph_wx = wx;
  t3_glyph_wy = wy;
  t3_glyph_bbox[0] = llx;
  t3_glyph_bbox[1] = lly;
  t3_glyph_bbox[2] = urx;
  t3_glyph_bbox[3] = ury;
  t3_glyph_has_bbox = gTrue;
}

void CairoOutputDev::beginTextObject(GfxState *state) {
}

void CairoOutputDev::endTextObject(GfxState *state) {
  if (textClipPath) {
    // clip the accumulated text path
    cairo_append_path (cairo, textClipPath);
    cairo_clip (cairo);
    if (cairo_shape) {
      cairo_append_path (cairo_shape, textClipPath);
      cairo_clip (cairo_shape);
    }
    cairo_path_destroy (textClipPath);
    textClipPath = NULL;
  }
}

void CairoOutputDev::beginActualText(GfxState *state, GooString *text)
{
  if (this->text)
    actualText->begin(state, text);
}

void CairoOutputDev::endActualText(GfxState *state)
{
  if (text)
    actualText->end(state);
}

static inline int splashRound(SplashCoord x) {
  return (int)floor(x + 0.5);
}

static inline int splashCeil(SplashCoord x) {
  return (int)ceil(x);
}

static inline int splashFloor(SplashCoord x) {
  return (int)floor(x);
}

static
cairo_surface_t *cairo_surface_create_similar_clip (cairo_t *cairo, cairo_content_t content)
{
  double x1, y1, x2, y2;
  int width, height;
  cairo_clip_extents (cairo, &x1, &y1, &x2, &y2);
  cairo_matrix_t matrix;
  cairo_get_matrix (cairo, &matrix);
  //cairo_matrix_transform_point(&matrix, &x1, &y1);
  //cairo_matrix_transform_point(&matrix, &x2, &y2);*/
  cairo_user_to_device(cairo, &x1, &y1);
  cairo_user_to_device(cairo, &x2, &y2);
  width = splashCeil(x2) - splashFloor(x1);
  //XXX: negative matrix
  ////height = splashCeil(y2) - splashFloor(y1);
  height = splashFloor(y1) - splashCeil(y2);
  cairo_surface_t *target = cairo_get_target (cairo);
  cairo_surface_t *result;

  result = cairo_surface_create_similar (target, content, width, height);
  double x_offset, y_offset;
    cairo_surface_get_device_offset(target, &x_offset, &y_offset);
    cairo_surface_set_device_offset(result, x_offset, y_offset);

 
  return result;
}



void CairoOutputDev::beginTransparencyGroup(GfxState * /*state*/, double * /*bbox*/,
                                      GfxColorSpace * blendingColorSpace,
                                      GBool /*isolated*/, GBool knockout,
				      GBool forSoftMask) {
  /* push color space */
  ColorSpaceStack* css = new ColorSpaceStack;
  css->cs = blendingColorSpace;
  css->knockout = knockout;
  cairo_get_matrix(cairo, &css->group_matrix);
  css->next = groupColorSpaceStack;
  groupColorSpaceStack = css;

  LOG(printf ("begin transparency group. knockout: %s\n", knockout ? "yes":"no"));

  if (knockout) {
    knockoutCount++;
    if (!cairo_shape) {
      /* create a surface for tracking the shape */
      cairo_surface_t *cairo_shape_surface = cairo_surface_create_similar_clip (cairo, CAIRO_CONTENT_ALPHA);
      cairo_shape = cairo_create (cairo_shape_surface);
      cairo_surface_destroy (cairo_shape_surface);

      /* the color doesn't matter as long as it is opaque */
      cairo_set_source_rgb (cairo_shape, 0, 0, 0);
      cairo_matrix_t matrix;
      cairo_get_matrix (cairo, &matrix);
      //printMatrix(&matrix);
      cairo_set_matrix (cairo_shape, &matrix);
    } else {
      cairo_reference (cairo_shape);
    }
  }
  if (groupColorSpaceStack->next && groupColorSpaceStack->next->knockout) {
    /* we need to track the shape */
    cairo_push_group (cairo_shape);
  }
  if (0 && forSoftMask)
    cairo_push_group_with_content (cairo, CAIRO_CONTENT_ALPHA);
  else
    cairo_push_group (cairo);

  /* push_group has an implicit cairo_save() */
  if (knockout) {
    /*XXX: let's hope this matches the semantics needed */
    cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  } else {
    cairo_set_operator(cairo, CAIRO_OPERATOR_OVER);
  }
}

void CairoOutputDev::endTransparencyGroup(GfxState * /*state*/) {
  if (group)
    cairo_pattern_destroy(group);
  group = cairo_pop_group (cairo);

  LOG(printf ("end transparency group\n"));

  if (groupColorSpaceStack->next && groupColorSpaceStack->next->knockout) {
    if (shape)
      cairo_pattern_destroy(shape);
    shape = cairo_pop_group (cairo_shape);
  }
}

void CairoOutputDev::paintTransparencyGroup(GfxState * /*state*/, double * /*bbox*/) {
  LOG(printf ("paint transparency group\n"));

  cairo_save (cairo);
  cairo_set_matrix (cairo, &groupColorSpaceStack->group_matrix);
  cairo_set_source (cairo, group);

  if (!mask) {
    //XXX: deal with mask && shape case
    if (shape) {
      cairo_save (cairo);

      /* OPERATOR_SOURCE w/ a mask is defined as (src IN mask) ADD (dest OUT mask)
       * however our source has already been clipped to mask so we only need to
       * do ADD and OUT */

      /* clear the shape mask */
      cairo_set_source (cairo, shape);
      cairo_set_operator (cairo, CAIRO_OPERATOR_DEST_OUT);
      cairo_paint (cairo);

      cairo_set_operator (cairo, CAIRO_OPERATOR_ADD);
      cairo_set_source (cairo, group);
      cairo_paint (cairo);

      cairo_restore (cairo);

      cairo_pattern_destroy (shape);
      shape = NULL;
    } else {
      cairo_paint_with_alpha (cairo, fill_opacity);
    }
    cairo_status_t status = cairo_status(cairo);
    if (status)
      printf("BAD status: %s\n", cairo_status_to_string(status));
  } else {
    if (fill_opacity < 1.0) {
      cairo_push_group(cairo);
    }
    cairo_save(cairo);
    cairo_set_matrix(cairo, &mask_matrix);
    cairo_mask(cairo, mask);
    cairo_restore(cairo);
    if (fill_opacity < 1.0) {
      cairo_pop_group_to_source(cairo);
      cairo_paint_with_alpha (cairo, fill_opacity);
    }
    cairo_pattern_destroy(mask);
    mask = NULL;
  }

  popTransparencyGroup();
  cairo_restore(cairo);
}

static int luminocity(uint32_t x)
{
  int r = (x >> 16) & 0xff;
  int g = (x >>  8) & 0xff;
  int b = (x >>  0) & 0xff;
  // an arbitrary integer approximation of .3*r + .59*g + .11*b
  int y = (r*19661+g*38666+b*7209 + 32829)>>16;
  return y;
}


/* XXX: do we need to deal with shape here? */
void CairoOutputDev::setSoftMask(GfxState * state, double * bbox, GBool alpha,
                                 Function * transferFunc, GfxColor * backdropColor) {
  cairo_pattern_destroy(mask);

  LOG(printf ("set softMask\n"));

  if (!alpha || transferFunc) {
    /* We need to mask according to the luminocity of the group.
     * So we paint the group to an image surface convert it to a luminocity map
     * and then use that as the mask. */

    /* Get clip extents in device space */
    double x1, y1, x2, y2, x_min, y_min, x_max, y_max;
    cairo_clip_extents(cairo, &x1, &y1, &x2, &y2);
    cairo_user_to_device(cairo, &x1, &y1);
    cairo_user_to_device(cairo, &x2, &y2);
    x_min = MIN(x1, x2);
    y_min = MIN(y1, y2);
    x_max = MAX(x1, x2);
    y_max = MAX(y1, y2);
    cairo_clip_extents(cairo, &x1, &y1, &x2, &y2);
    cairo_user_to_device(cairo, &x1, &y2);
    cairo_user_to_device(cairo, &x2, &y1);
    x_min = MIN(x_min,MIN(x1, x2));
    y_min = MIN(y_min,MIN(y1, y2));
    x_max = MAX(x_max,MAX(x1, x2));
    y_max = MAX(y_max,MAX(y1, y2));

    int width = (int)(ceil(x_max) - floor(x_min));
    int height = (int)(ceil(y_max) - floor(y_min));

    /* Get group device offset */
    double x_offset, y_offset;
    if (cairo_get_group_target(cairo) == cairo_get_target(cairo)) {
      cairo_surface_get_device_offset(cairo_get_group_target(cairo), &x_offset, &y_offset);
    } else {
      cairo_surface_t *pats;
      cairo_pattern_get_surface(group, &pats);
      cairo_surface_get_device_offset(pats, &x_offset, &y_offset);
    }

    /* Adjust extents by group offset */
    x_min += x_offset;
    y_min += y_offset;

    cairo_surface_t *source = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *maskCtx = cairo_create(source);

    //XXX: hopefully this uses the correct color space */
    if (!alpha) {
      GfxRGB backdropColorRGB;
      groupColorSpaceStack->cs->getRGB(backdropColor, &backdropColorRGB);
      /* paint the backdrop */
      cairo_set_source_rgb(maskCtx,
                           colToDbl(backdropColorRGB.r),
                           colToDbl(backdropColorRGB.g),
                           colToDbl(backdropColorRGB.b));
    }
    cairo_paint(maskCtx);

    /* Copy source ctm to mask ctm and translate origin so that the
     * mask appears it the same location on the source surface.  */
    cairo_matrix_t mat, tmat;
    cairo_matrix_init_translate(&tmat, -x_min, -y_min);
    cairo_get_matrix(cairo, &mat);
    cairo_matrix_multiply(&mat, &mat, &tmat);
    cairo_set_matrix(maskCtx, &mat);

    /* make the device offset of the new mask match that of the group */
    cairo_surface_set_device_offset(source, x_offset, y_offset);

    /* paint the group */
    cairo_set_source(maskCtx, group);
    cairo_paint(maskCtx);

    /* XXX status = cairo_status(maskCtx); */
    cairo_destroy(maskCtx);

    /* convert to a luminocity map */
    uint32_t *source_data = (uint32_t*)cairo_image_surface_get_data(source);
    /* get stride in units of 32 bits */
    int stride = cairo_image_surface_get_stride(source)/4;
    for (int y=0; y<height; y++) {
      for (int x=0; x<width; x++) {
	int lum = alpha ? fill_opacity : luminocity(source_data[y*stride + x]);
        if (transferFunc) {
          double lum_in, lum_out;
          lum_in = lum/256.0;
          transferFunc->transform(&lum_in, &lum_out);
          lum = (int)(lum_out * 255.0 + 0.5);
        }
        source_data[y*stride + x] = lum << 24;
      }
    }
    cairo_surface_mark_dirty (source);

    /* setup the new mask pattern */
    mask = cairo_pattern_create_for_surface(source);
    cairo_get_matrix(cairo, &mask_matrix);

    if (cairo_get_group_target(cairo) == cairo_get_target(cairo)) {
      cairo_pattern_set_matrix(mask, &mat);
    } else {
      cairo_matrix_t patMatrix;
      cairo_pattern_get_matrix(group, &patMatrix);
      /* Apply x_min, y_min offset to it appears in the same location as source. */
      cairo_matrix_multiply(&patMatrix, &patMatrix, &tmat);
      cairo_pattern_set_matrix(mask, &patMatrix);
    }

    cairo_surface_destroy(source);
  } else if (alpha) {
    mask = cairo_pattern_reference(group);
    cairo_get_matrix(cairo, &mask_matrix);
  }

  popTransparencyGroup();
}

void CairoOutputDev::popTransparencyGroup() {
  /* pop color space */
  ColorSpaceStack *css = groupColorSpaceStack;
  if (css->knockout) {
    knockoutCount--;
    if (!knockoutCount) {
      /* we don't need to track the shape anymore because
       * we are not above any knockout groups */
      cairo_destroy(cairo_shape);
      cairo_shape = NULL;
    }
  }
  groupColorSpaceStack = css->next;
  delete css;
}


void CairoOutputDev::clearSoftMask(GfxState * /*state*/) {
  if (mask)
    cairo_pattern_destroy(mask);
  mask = NULL;
}

/* Taken from cairo/doc/tutorial/src/singular.c */
static void
get_singular_values (const cairo_matrix_t *matrix,
		     double               *major,
		     double               *minor)
{
	double xx = matrix->xx, xy = matrix->xy;
	double yx = matrix->yx, yy = matrix->yy;

	double a = xx*xx+yx*yx;
	double b = xy*xy+yy*yy;
	double k = xx*xy+yx*yy;

	double f = (a+b) * .5;
	double g = (a-b) * .5;
	double delta = sqrt (g*g + k*k);

	if (major)
		*major = sqrt (f + delta);
	if (minor)
		*minor = sqrt (f - delta);
}

void CairoOutputDev::getScaledSize(const cairo_matrix_t *matrix,
                                   int                   orig_width,
				   int                   orig_height,
				   int                  *scaledWidth,
				   int                  *scaledHeight)
{
  double xScale;
  double yScale;
  if (orig_width > orig_height)
    get_singular_values (matrix, &xScale, &yScale);
  else
    get_singular_values (matrix, &yScale, &xScale);

  int tx, tx2, ty, ty2; /* the integer co-oridinates of the resulting image */
  if (xScale >= 0) {
    tx = splashRound(matrix->x0 - 0.01);
    tx2 = splashRound(matrix->x0 + xScale + 0.01) - 1;
  } else {
    tx = splashRound(matrix->x0 + 0.01) - 1;
    tx2 = splashRound(matrix->x0 + xScale - 0.01);
  }
  *scaledWidth = abs(tx2 - tx) + 1;
  //scaledWidth = splashRound(fabs(xScale));
  if (*scaledWidth == 0) {
    // technically, this should draw nothing, but it generally seems
    // better to draw a one-pixel-wide stripe rather than throwing it
    // away
    *scaledWidth = 1;
  }
  if (yScale >= 0) {
    ty = splashFloor(matrix->y0 + 0.01);
    ty2 = splashCeil(matrix->y0 + yScale - 0.01);
  } else {
    ty = splashCeil(matrix->y0 - 0.01);
    ty2 = splashFloor(matrix->y0 + yScale + 0.01);
  }
  *scaledHeight = abs(ty2 - ty);
  if (*scaledHeight == 0) {
    *scaledHeight = 1;
  }
}

cairo_filter_t
CairoOutputDev::getFilterForSurface(cairo_surface_t *image,
				    GBool interpolate)
{
  if (interpolate)
    return CAIRO_FILTER_BILINEAR;

  int orig_width = cairo_image_surface_get_width (image);
  int orig_height = cairo_image_surface_get_height (image);
  if (orig_width == 0 || orig_height == 0)
	  return CAIRO_FILTER_NEAREST;

  /* When printing, don't change the interpolation. */
  if (printing)
    return CAIRO_FILTER_NEAREST;

  cairo_matrix_t matrix;
  cairo_get_matrix(cairo, &matrix);
  int scaled_width, scaled_height;
  getScaledSize (&matrix, orig_width, orig_height, &scaled_width, &scaled_height);

  /* When scale factor is >= 400% we don't interpolate. See bugs #25268, #9860 */
  if (scaled_width / orig_width >= 4 || scaled_height / orig_height >= 4)
	  return CAIRO_FILTER_NEAREST;

  return CAIRO_FILTER_BILINEAR;
}

void CairoOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				   int width, int height, GBool invert,
				   GBool interpolate, GBool inlineImg) {

  /* FIXME: Doesn't the image mask support any colorspace? */
  cairo_set_source (cairo, fill_pattern);

  /* work around a cairo bug when scaling 1x1 surfaces */
  if (width == 1 && height == 1) {
    ImageStream *imgStr;
    Guchar pix;
    int invert_bit;

    imgStr = new ImageStream(str, width, 1, 1);
    imgStr->reset();
    imgStr->getPixel(&pix);
    imgStr->close();
    delete imgStr;

    invert_bit = invert ? 1 : 0;
    if (pix ^ invert_bit)
      return;

    cairo_save (cairo);
    cairo_rectangle (cairo, 0., 0., width, height);
    cairo_fill (cairo);
    cairo_restore (cairo);
    if (cairo_shape) {
      cairo_save (cairo_shape);
      cairo_rectangle (cairo_shape, 0., 0., width, height);
      cairo_fill (cairo_shape);
      cairo_restore (cairo_shape);
    }
    return;
  }

  /* shape is 1.0 for painted areas, 0.0 for unpainted ones */

  cairo_matrix_t matrix;
  cairo_get_matrix (cairo, &matrix);
  //XXX: it is possible that we should only do sub pixel positioning if 
  // we are rendering fonts */
  if (!printing && prescaleImages
      /* not rotated */
      && matrix.xy == 0 && matrix.yx == 0
      /* axes not flipped / not 180 deg rotated */
      && matrix.xx > 0 && (upsideDown() ? -1 : 1) * matrix.yy > 0) {
    drawImageMaskPrescaled(state, ref, str, width, height, invert, interpolate, inlineImg);
  } else {
    drawImageMaskRegular(state, ref, str, width, height, invert, interpolate, inlineImg);
  }

}

void CairoOutputDev::setSoftMaskFromImageMask(GfxState *state, Object *ref, Stream *str,
				   int width, int height, GBool invert,
				   GBool inlineImg, double *baseMatrix) {

  /* FIXME: Doesn't the image mask support any colorspace? */
  cairo_set_source (cairo, fill_pattern);

  /* work around a cairo bug when scaling 1x1 surfaces */
  if (width == 1 && height == 1) {
    ImageStream *imgStr;
    Guchar pix;
    int invert_bit;

    imgStr = new ImageStream(str, width, 1, 1);
    imgStr->reset();
    imgStr->getPixel(&pix);
    imgStr->close();
    delete imgStr;

    invert_bit = invert ? 1 : 0;
    if (pix ^ invert_bit)
      return;

    cairo_save (cairo);
    cairo_rectangle (cairo, 0., 0., width, height);
    cairo_fill (cairo);
    cairo_restore (cairo);
    if (cairo_shape) {
      cairo_save (cairo_shape);
      cairo_rectangle (cairo_shape, 0., 0., width, height);
      cairo_fill (cairo_shape);
      cairo_restore (cairo_shape);
    }
    return;
  }

  cairo_push_group_with_content (cairo, CAIRO_CONTENT_ALPHA);

  /* shape is 1.0 for painted areas, 0.0 for unpainted ones */

  cairo_matrix_t matrix;
  cairo_get_matrix (cairo, &matrix);
  //XXX: it is possible that we should only do sub pixel positioning if 
  // we are rendering fonts */
  if (!printing && prescaleImages && matrix.xy == 0.0 && matrix.yx == 0.0) {
    drawImageMaskPrescaled(state, ref, str, width, height, invert, gFalse, inlineImg);
  } else {
    drawImageMaskRegular(state, ref, str, width, height, invert, gFalse, inlineImg);
  }

  if (state->getFillColorSpace()->getMode() == csPattern) {
    cairo_set_source_rgb (cairo, 1, 1, 1);
    cairo_set_matrix (cairo, &mask_matrix);
    cairo_mask (cairo, mask);
  }

  if (mask)
    cairo_pattern_destroy (mask);
  mask = cairo_pop_group (cairo);

  saveState(state);
  double bbox[4] = {0,0,1,1}; // dummy
  beginTransparencyGroup(state, bbox, state->getFillColorSpace(),
                         gTrue, gFalse, gFalse);
}

void CairoOutputDev::unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix) {
  double bbox[4] = {0,0,1,1}; // dummy

  endTransparencyGroup(state);
  restoreState(state);
  paintTransparencyGroup(state, bbox);
  clearSoftMask(state);
}

void CairoOutputDev::drawImageMaskRegular(GfxState *state, Object *ref, Stream *str,
					  int width, int height, GBool invert,
					  GBool interpolate, GBool inlineImg) {
  unsigned char *buffer;
  unsigned char *dest;
  cairo_surface_t *image;
  cairo_pattern_t *pattern;
  int x, y, i, bit;
  ImageStream *imgStr;
  Guchar *pix;
  cairo_matrix_t matrix;
  int invert_bit;
  int row_stride;
  cairo_filter_t filter;

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width, 1, 1);
  imgStr->reset();

  image = cairo_image_surface_create (CAIRO_FORMAT_A1, width, height);
  if (cairo_surface_status (image))
    goto cleanup;

  buffer = cairo_image_surface_get_data (image);
  row_stride = cairo_image_surface_get_stride (image);

  invert_bit = invert ? 1 : 0;

  for (y = 0; y < height; y++) {
    pix = imgStr->getLine();
    dest = buffer + y * row_stride;
    i = 0;
    bit = 0;
    for (x = 0; x < width; x++) {
      if (bit == 0)
	dest[i] = 0;
      if (!(pix[x] ^ invert_bit)) {
#ifdef WORDS_BIGENDIAN
	dest[i] |= (1 << (7 - bit));
#else
	dest[i] |= (1 << bit);
#endif
      }
      bit++;
      if (bit > 7) {
	bit = 0;
	i++;
      }
    }
  }

  filter = getFilterForSurface (image, interpolate);

  cairo_surface_mark_dirty (image);
  pattern = cairo_pattern_create_for_surface (image);
  cairo_surface_destroy (image);
  if (cairo_pattern_status (pattern))
    goto cleanup;

  LOG (printf ("drawImageMask %dx%d\n", width, height));

  cairo_pattern_set_filter (pattern, filter);

  if (!printing)
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

  cairo_matrix_init_translate (&matrix, 0, height);
  cairo_matrix_scale (&matrix, width, -height);
  cairo_pattern_set_matrix (pattern, &matrix);
  if (cairo_pattern_status (pattern)) {
    cairo_pattern_destroy (pattern);
    goto cleanup;
  }

  if (state->getFillColorSpace()->getMode() == csPattern) {
    mask = cairo_pattern_reference (pattern);
    cairo_get_matrix (cairo, &mask_matrix);
  } else if (!printing) {
    cairo_save (cairo);
    cairo_rectangle (cairo, 0., 0., 1., 1.);
    cairo_clip (cairo);
    cairo_mask (cairo, pattern);
    cairo_restore (cairo);
  } else {
    cairo_mask (cairo, pattern);
  }

  if (cairo_shape) {
    cairo_save (cairo_shape);
    cairo_set_source (cairo_shape, pattern);
    if (!printing) {
      cairo_rectangle (cairo_shape, 0., 0., 1., 1.);
      cairo_fill (cairo_shape);
    } else {
      cairo_mask (cairo_shape, pattern);
    }
    cairo_restore (cairo_shape);
  }

  cairo_pattern_destroy (pattern);

cleanup:
  imgStr->close();
  delete imgStr;
}


void CairoOutputDev::drawImageMaskPrescaled(GfxState *state, Object *ref, Stream *str,
					    int width, int height, GBool invert,
					    GBool interpolate, GBool inlineImg) {
  unsigned char *buffer;
  cairo_surface_t *image;
  cairo_pattern_t *pattern;
  ImageStream *imgStr;
  Guchar *pix;
  cairo_matrix_t matrix;
  int invert_bit;
  int row_stride;

  /* cairo does a very poor job of scaling down images so we scale them ourselves */

  LOG (printf ("drawImageMaskPrescaled %dx%d\n", width, height));

  /* this scaling code is adopted from the splash image scaling code */
  cairo_get_matrix(cairo, &matrix);
#if 0
  printf("[%f %f], [%f %f], %f %f\n", matrix.xx, matrix.xy, matrix.yx, matrix.yy, matrix.x0, matrix.y0);
#endif
  /* this whole computation should be factored out */
  double xScale = matrix.xx;
  double yScale = matrix.yy;
  int tx, tx2, ty, ty2; /* the integer co-oridinates of the resulting image */
  int scaledHeight;
  int scaledWidth;
  if (xScale >= 0) {
    tx = splashRound(matrix.x0 - 0.01);
    tx2 = splashRound(matrix.x0 + xScale + 0.01) - 1;
  } else {
    tx = splashRound(matrix.x0 + 0.01) - 1;
    tx2 = splashRound(matrix.x0 + xScale - 0.01);
  }
  scaledWidth = abs(tx2 - tx) + 1;
  //scaledWidth = splashRound(fabs(xScale));
  if (scaledWidth == 0) {
    // technically, this should draw nothing, but it generally seems
    // better to draw a one-pixel-wide stripe rather than throwing it
    // away
    scaledWidth = 1;
  }
  if (yScale >= 0) {
    ty = splashFloor(matrix.y0 + 0.01);
    ty2 = splashCeil(matrix.y0 + yScale - 0.01);
  } else {
    ty = splashCeil(matrix.y0 - 0.01);
    ty2 = splashFloor(matrix.y0 + yScale + 0.01);
  }
  scaledHeight = abs(ty2 - ty);
  if (scaledHeight == 0) {
    scaledHeight = 1;
  }
#if 0
  printf("xscale: %g, yscale: %g\n", xScale, yScale);
  printf("width: %d, height: %d\n", width, height);
  printf("scaledWidth: %d, scaledHeight: %d\n", scaledWidth, scaledHeight);
#endif

  /* compute the required padding */
  /* Padding is used to preserve the aspect ratio.
     We compute total_pad to make (height+total_pad)/scaledHeight as close to height/yScale as possible */
  int head_pad = 0;
  int tail_pad = 0;
  int total_pad = splashRound(height*(scaledHeight/fabs(yScale)) - height);

  /* compute the two pieces of padding */
  if (total_pad > 0) {
    //XXX: i'm not positive fabs() is correct
    float tail_error = fabs(matrix.y0 - ty);
    float head_error = fabs(ty2 - (matrix.y0 + yScale));
    float tail_fraction = tail_error/(tail_error + head_error);
    tail_pad = splashRound(total_pad*tail_fraction);
    head_pad = total_pad - tail_pad;
  } else {
    tail_pad = 0;
    head_pad = 0;
  }
  int origHeight = height;
  height += tail_pad;
  height += head_pad;
#if 0
  printf("head_pad: %d tail_pad: %d\n", head_pad, tail_pad);
  printf("origHeight: %d height: %d\n", origHeight, height);
  printf("ty: %d, ty2: %d\n", ty, ty2);
#endif

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width, 1, 1);
  imgStr->reset();

  invert_bit = invert ? 1 : 0;

  image = cairo_image_surface_create (CAIRO_FORMAT_A8, scaledWidth, scaledHeight);
  if (cairo_surface_status (image)) {
    imgStr->close();
    delete imgStr;
    return;
  }

  buffer = cairo_image_surface_get_data (image);
  row_stride = cairo_image_surface_get_stride (image);

  int yp = height / scaledHeight;
  int yq = height % scaledHeight;
  int xp = width / scaledWidth;
  int xq = width % scaledWidth;
  int yt = 0;
  int origHeight_c = origHeight;
  /* use MIN() because yp might be > origHeight because of padding */
  unsigned char *pixBuf = (unsigned char *)malloc(MIN(yp+1, origHeight)*width);
  int lastYStep = 1;
  int total = 0;
  for (int y = 0; y < scaledHeight; y++) {
    // y scale Bresenham
    int yStep = yp;
    yt += yq;

    if (yt >= scaledHeight) {
      yt -= scaledHeight;
      ++yStep;
    }

    // read row (s) from image ignoring the padding as appropriate
    {
      int n = (yp > 0) ? yStep : lastYStep;
      total += n;
      if (n > 0) {
	unsigned char *p = pixBuf;
	int head_pad_count = head_pad;
	int origHeight_count = origHeight;
	int tail_pad_count = tail_pad;
	for (int i=0; i<n; i++) {
	  // get row
	  if (head_pad_count) {
	    head_pad_count--;
	  } else if (origHeight_count) {
	    pix = imgStr->getLine();
	    for (int j=0; j<width; j++) {
	      if (pix[j] ^ invert_bit)
		p[j] = 0;
	      else
		p[j] = 255;
	    }
	    origHeight_count--;
	    p += width;
	  } else if (tail_pad_count) {
	    tail_pad_count--;
	  } else {
	    printf("%d %d\n", n, total);
	    assert(0 && "over run\n");
	  }
	}
      }
    }

    lastYStep = yStep;
    int k1 = y;

    int xt = 0;
    int xSrc = 0;
    int x1 = k1;
    int n = yStep > 0 ? yStep : 1;
    int origN = n;

    /* compute the size of padding and pixels that will be used for this row */
    int head_pad_size = MIN(n, head_pad);
    n -= head_pad_size;
    head_pad -= MIN(head_pad_size, yStep);

    int pix_size = MIN(n, origHeight);
    n -= pix_size;
    origHeight -= MIN(pix_size, yStep);

    int tail_pad_size = MIN(n, tail_pad);
    n -= tail_pad_size;
    tail_pad -= MIN(tail_pad_size, yStep);
    if (n != 0) {
      printf("n = %d (%d %d %d)\n", n, head_pad_size, pix_size, tail_pad_size);
      assert(n == 0);
    }

    for (int x = 0; x < scaledWidth; ++x) {
      int xStep = xp;
      xt += xq;
      if (xt >= scaledWidth) {
	xt -= scaledWidth;
	++xStep;
      }
      int m = xStep > 0 ? xStep : 1;
      float pixAcc0 = 0;
      /* could m * head_pad_size * tail_pad_size  overflow? */
      if (invert_bit) {
	pixAcc0 += m * head_pad_size * tail_pad_size * 255;
      } else {
	pixAcc0 += m * head_pad_size * tail_pad_size * 0;
      }
      /* Accumulate all of the source pixels for the destination pixel */
      for (int i = 0; i < pix_size; ++i) {
	for (int j = 0; j< m; ++j) {
	  if (xSrc + i*width + j > MIN(yp + 1, origHeight_c)*width) {
	    printf("%d > %d (%d %d %d %d) (%d %d %d)\n", xSrc + i*width + j, MIN(yp + 1, origHeight_c)*width, xSrc, i , width, j, yp, origHeight_c, width);
	    printf("%d %d %d\n", head_pad_size, pix_size, tail_pad_size);
	    assert(0 && "bad access\n");
	  }
	  pixAcc0 += pixBuf[xSrc + i*width + j];
	}
      }
      buffer[y * row_stride + x] = splashFloor(pixAcc0 / (origN*m));
      xSrc += xStep;
      x1 += 1;
    }

  }
  free(pixBuf);

  cairo_surface_mark_dirty (image);
  pattern = cairo_pattern_create_for_surface (image);
  cairo_surface_destroy (image);
  if (cairo_pattern_status (pattern)) {
    imgStr->close();
    delete imgStr;
    return;
  }

  /* we should actually be using CAIRO_FILTER_NEAREST here. However,
   * cairo doesn't yet do minifaction filtering causing scaled down
   * images with CAIRO_FILTER_NEAREST to look really bad */
  cairo_pattern_set_filter (pattern,
			    interpolate ? CAIRO_FILTER_BEST : CAIRO_FILTER_FAST);
  cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

  if (state->getFillColorSpace()->getMode() == csPattern) {
    cairo_matrix_init_translate (&matrix, 0, scaledHeight);
    cairo_matrix_scale (&matrix, scaledWidth, -scaledHeight);
    cairo_pattern_set_matrix (pattern, &matrix);
    if (cairo_pattern_status (pattern)) {
      cairo_pattern_destroy (pattern);
      imgStr->close();
      delete imgStr;
      return;
    }

    mask = cairo_pattern_reference (pattern);
    cairo_get_matrix (cairo, &mask_matrix);
  } else {
    cairo_save (cairo);

    /* modify our current transformation so that the prescaled image
     * goes where it is supposed to */
    cairo_get_matrix(cairo, &matrix);
    cairo_scale(cairo, 1.0/matrix.xx, 1.0/matrix.yy);
    // get integer co-ords
    cairo_translate (cairo, tx - matrix.x0, ty2 - matrix.y0);
    if (yScale > 0)
      cairo_scale(cairo, 1, -1);

    cairo_rectangle (cairo, 0., 0., scaledWidth, scaledHeight);
    cairo_clip (cairo);
    cairo_mask (cairo, pattern);

    //cairo_get_matrix(cairo, &matrix);
    //printf("mask at: [%f %f], [%f %f], %f %f\n\n", matrix.xx, matrix.xy, matrix.yx, matrix.yy, matrix.x0, matrix.y0);
    cairo_restore(cairo);
  }

  if (cairo_shape) {
    cairo_save (cairo_shape);

    /* modify our current transformation so that the prescaled image
     * goes where it is supposed to */
    cairo_get_matrix(cairo_shape, &matrix);
    cairo_scale(cairo_shape, 1.0/matrix.xx, 1.0/matrix.yy);
    // get integer co-ords
    cairo_translate (cairo_shape, tx - matrix.x0, ty2 - matrix.y0);
    if (yScale > 0)
      cairo_scale(cairo_shape, 1, -1);

    cairo_rectangle (cairo_shape, 0., 0., scaledWidth, scaledHeight);
    cairo_fill (cairo_shape);

    cairo_restore(cairo_shape);
  }

  cairo_pattern_destroy (pattern);

  imgStr->close();
  delete imgStr;
}

void CairoOutputDev::drawMaskedImage(GfxState *state, Object *ref,
				     Stream *str, int width, int height,
				     GfxImageColorMap *colorMap,
				     GBool interpolate,
				     Stream *maskStr, int maskWidth,
				     int maskHeight, GBool maskInvert,
				     GBool maskInterpolate)
{
  ImageStream *maskImgStr, *imgStr;
  int row_stride;
  unsigned char *maskBuffer, *buffer;
  unsigned char *maskDest;
  unsigned int *dest;
  cairo_surface_t *maskImage, *image;
  cairo_pattern_t *maskPattern, *pattern;
  cairo_matrix_t matrix;
  cairo_matrix_t maskMatrix;
  Guchar *pix;
  int x, y;
  int invert_bit;
  cairo_filter_t filter;
  cairo_filter_t maskFilter;

  maskImgStr = new ImageStream(maskStr, maskWidth, 1, 1);
  maskImgStr->reset();

  maskImage = cairo_image_surface_create (CAIRO_FORMAT_A8, maskWidth, maskHeight);
  if (cairo_surface_status (maskImage)) {
    maskImgStr->close();
    delete maskImgStr;
    return;
  }

  maskBuffer = cairo_image_surface_get_data (maskImage);
  row_stride = cairo_image_surface_get_stride (maskImage);

  invert_bit = maskInvert ? 1 : 0;

  for (y = 0; y < maskHeight; y++) {
    pix = maskImgStr->getLine();
    maskDest = maskBuffer + y * row_stride;
    for (x = 0; x < maskWidth; x++) {
      if (pix[x] ^ invert_bit)
	*maskDest++ = 0;
      else
	*maskDest++ = 255;
    }
  }

  maskImgStr->close();
  delete maskImgStr;

  maskFilter = getFilterForSurface (maskImage, maskInterpolate);

  cairo_surface_mark_dirty (maskImage);
  maskPattern = cairo_pattern_create_for_surface (maskImage);
  cairo_surface_destroy (maskImage);
  if (cairo_pattern_status (maskPattern))
    return;

#if 0
  /* ICCBased color space doesn't do any color correction
   * so check its underlying color space as well */
  int is_identity_transform;
  is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
		  (colorMap->getColorSpace()->getMode() == csICCBased && 
		   ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB);
#endif

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width,
			   colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();

  image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
  if (cairo_surface_status (image))
    goto cleanup;

  buffer = cairo_image_surface_get_data (image);
  row_stride = cairo_image_surface_get_stride (image);
  for (y = 0; y < height; y++) {
    dest = (unsigned int *) (buffer + y * row_stride);
    pix = imgStr->getLine();
    colorMap->getRGBLine (pix, dest, width);
  }

  filter = getFilterForSurface (image, interpolate);

  cairo_surface_mark_dirty (image);
  pattern = cairo_pattern_create_for_surface (image);
  cairo_surface_destroy (image);
  if (cairo_pattern_status (pattern))
    goto cleanup;

  LOG (printf ("drawMaskedImage %dx%d\n", width, height));

  cairo_pattern_set_filter (pattern, filter);
  cairo_pattern_set_filter (maskPattern, maskFilter);

  if (!printing) {
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
    cairo_pattern_set_extend (maskPattern, CAIRO_EXTEND_PAD);
  }

  cairo_matrix_init_translate (&matrix, 0, height);
  cairo_matrix_scale (&matrix, width, -height);
  cairo_pattern_set_matrix (pattern, &matrix);
  if (cairo_pattern_status (pattern)) {
    cairo_pattern_destroy (pattern);
    cairo_pattern_destroy (maskPattern);
    goto cleanup;
  }

  cairo_matrix_init_translate (&maskMatrix, 0, maskHeight);
  cairo_matrix_scale (&maskMatrix, maskWidth, -maskHeight);
  cairo_pattern_set_matrix (maskPattern, &maskMatrix);
  if (cairo_pattern_status (maskPattern)) {
    cairo_pattern_destroy (maskPattern);
    cairo_pattern_destroy (pattern);
    goto cleanup;
  }

  if (!printing) {
    cairo_save (cairo);
    cairo_set_source (cairo, pattern);
    cairo_rectangle (cairo, 0., 0., 1., 1.);
    cairo_clip (cairo);
    cairo_mask (cairo, maskPattern);
    cairo_restore (cairo);
  } else {
    cairo_set_source (cairo, pattern);
    cairo_mask (cairo, maskPattern);
  }

  if (cairo_shape) {
    cairo_save (cairo_shape);
    cairo_set_source (cairo_shape, pattern);
    if (!printing) {
      cairo_rectangle (cairo_shape, 0., 0., 1., 1.);
      cairo_fill (cairo_shape);
    } else {
      cairo_mask (cairo_shape, pattern);
    }
    cairo_restore (cairo_shape);
  }

  cairo_pattern_destroy (maskPattern);
  cairo_pattern_destroy (pattern);

cleanup:
  imgStr->close();
  delete imgStr;
}


//XXX: is this affect by AIS(alpha is shape)?
void CairoOutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
					 int width, int height,
					 GfxImageColorMap *colorMap,
					 GBool interpolate,
					 Stream *maskStr,
					 int maskWidth, int maskHeight,
					 GfxImageColorMap *maskColorMap,
					 GBool maskInterpolate)
{
  ImageStream *maskImgStr, *imgStr;
  int row_stride;
  unsigned char *maskBuffer, *buffer;
  unsigned char *maskDest;
  unsigned int *dest;
  cairo_surface_t *maskImage, *image;
  cairo_pattern_t *maskPattern, *pattern;
  cairo_matrix_t maskMatrix, matrix;
  Guchar *pix;
  int y;
  cairo_filter_t filter;
  cairo_filter_t maskFilter;

  maskImgStr = new ImageStream(maskStr, maskWidth,
			       maskColorMap->getNumPixelComps(),
			       maskColorMap->getBits());
  maskImgStr->reset();

  maskImage = cairo_image_surface_create (CAIRO_FORMAT_A8, maskWidth, maskHeight);
  if (cairo_surface_status (maskImage)) {
    maskImgStr->close();
    delete maskImgStr;
    return;
  }

  maskBuffer = cairo_image_surface_get_data (maskImage);
  row_stride = cairo_image_surface_get_stride (maskImage);
  for (y = 0; y < maskHeight; y++) {
    maskDest = (unsigned char *) (maskBuffer + y * row_stride);
    pix = maskImgStr->getLine();
    maskColorMap->getGrayLine (pix, maskDest, maskWidth);
  }

  maskImgStr->close();
  delete maskImgStr;

  maskFilter = getFilterForSurface (maskImage, maskInterpolate);

  cairo_surface_mark_dirty (maskImage);
  maskPattern = cairo_pattern_create_for_surface (maskImage);
  cairo_surface_destroy (maskImage);
  if (cairo_pattern_status (maskPattern))
    return;

#if 0
  /* ICCBased color space doesn't do any color correction
   * so check its underlying color space as well */
  int is_identity_transform;
  is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
		  (colorMap->getColorSpace()->getMode() == csICCBased &&
		   ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB);
#endif

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width,
			   colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();

  image = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
  if (cairo_surface_status (image))
    goto cleanup;

  buffer = cairo_image_surface_get_data (image);
  row_stride = cairo_image_surface_get_stride (image);
  for (y = 0; y < height; y++) {
    dest = (unsigned int *) (buffer + y * row_stride);
    pix = imgStr->getLine();
    colorMap->getRGBLine (pix, dest, width);
  }

  filter = getFilterForSurface (image, interpolate);

  cairo_surface_mark_dirty (image);

  setMimeData(str, ref, image);

  pattern = cairo_pattern_create_for_surface (image);
  cairo_surface_destroy (image);
  if (cairo_pattern_status (pattern))
    goto cleanup;

  LOG (printf ("drawSoftMaskedImage %dx%d\n", width, height));

  cairo_pattern_set_filter (pattern, filter);
  cairo_pattern_set_filter (maskPattern, maskFilter);

  if (!printing) {
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
    cairo_pattern_set_extend (maskPattern, CAIRO_EXTEND_PAD);
  }

  cairo_matrix_init_translate (&matrix, 0, height);
  cairo_matrix_scale (&matrix, width, -height);
  cairo_pattern_set_matrix (pattern, &matrix);
  if (cairo_pattern_status (pattern)) {
    cairo_pattern_destroy (pattern);
    cairo_pattern_destroy (maskPattern);
    goto cleanup;
  }

  cairo_matrix_init_translate (&maskMatrix, 0, maskHeight);
  cairo_matrix_scale (&maskMatrix, maskWidth, -maskHeight);
  cairo_pattern_set_matrix (maskPattern, &maskMatrix);
  if (cairo_pattern_status (maskPattern)) {
    cairo_pattern_destroy (maskPattern);
    cairo_pattern_destroy (pattern);
    goto cleanup;
  }

  if (fill_opacity != 1.0)
    cairo_push_group (cairo);
  else
    cairo_save (cairo);

  cairo_set_source (cairo, pattern);
  if (!printing) {
    cairo_rectangle (cairo, 0., 0., 1., 1.);
    cairo_clip (cairo);
  }
  cairo_mask (cairo, maskPattern);

  if (fill_opacity != 1.0) {
    cairo_pop_group_to_source (cairo);
    cairo_save (cairo);
    if (!printing) {
      cairo_rectangle (cairo, 0., 0., 1., 1.);
      cairo_clip (cairo);
    }
    cairo_paint_with_alpha (cairo, fill_opacity);
  }
  cairo_restore (cairo);

  if (cairo_shape) {
    cairo_save (cairo_shape);
    cairo_set_source (cairo_shape, pattern);
    if (!printing) {
      cairo_rectangle (cairo_shape, 0., 0., 1., 1.);
      cairo_fill (cairo_shape);
    } else {
      cairo_mask (cairo_shape, pattern);
    }
    cairo_restore (cairo_shape);
  }

  cairo_pattern_destroy (maskPattern);
  cairo_pattern_destroy (pattern);

cleanup:
  imgStr->close();
  delete imgStr;
}

GBool CairoOutputDev::getStreamData (Stream *str, char **buffer, int *length)
{
  int len, i;
  char *strBuffer;

  len = 0;
  str->close();
  str->reset();
  while (str->getChar() != EOF) len++;
  if (len == 0)
    return gFalse;

  strBuffer = (char *)gmalloc (len);

  str->close();
  str->reset();
  for (i = 0; i < len; ++i)
    strBuffer[i] = str->getChar();

  *buffer = strBuffer;
  *length = len;

  return gTrue;
}

void CairoOutputDev::setMimeData(Stream *str, Object *ref, cairo_surface_t *image)
{
  char *strBuffer;
  int len;
  Object obj;

  if (!printing || !(str->getKind() == strDCT || str->getKind() == strJPX))
    return;

  // colorspace in stream dict may be different from colorspace in jpx
  // data
  if (str->getKind() == strJPX) {
    GBool hasColorSpace = !str->getDict()->lookup("ColorSpace", &obj)->isNull();
    obj.free();
    if (hasColorSpace)
      return;
  }

  if (getStreamData (str->getNextStream(), &strBuffer, &len)) {
    cairo_status_t st;

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 11, 2)
    if (ref && ref->isRef()) {
      Ref imgRef = ref->getRef();
      GooString *surfaceId = new GooString("poppler-surface-");
      surfaceId->appendf("{0:d}-{1:d}", imgRef.gen, imgRef.num);
      char *idBuffer = copyString(surfaceId->getCString());
      st = cairo_surface_set_mime_data (image, CAIRO_MIME_TYPE_UNIQUE_ID,
                                        (const unsigned char *)idBuffer,
                                        surfaceId->getLength(),
                                        gfree, idBuffer);
      if (st)
        gfree(idBuffer);
      delete surfaceId;
    }
#endif

    st = cairo_surface_set_mime_data (image,
				      str->getKind() == strDCT ?
				      CAIRO_MIME_TYPE_JPEG : CAIRO_MIME_TYPE_JP2,
				      (const unsigned char *)strBuffer, len,
				      gfree, strBuffer);
    if (st)
      gfree (strBuffer);
  }
}

class RescaleDrawImage : public CairoRescaleBox {
private:
  ImageStream *imgStr;
  GfxRGB *lookup;
  int width;
  GfxImageColorMap *colorMap;
  int *maskColors;
  int current_row;

public:
  cairo_surface_t *getSourceImage(Stream *str,
                                  int widthA, int height,
                                  int scaledWidth, int scaledHeight,
                                  GBool printing,
                                  GfxImageColorMap *colorMapA,
                                  int *maskColorsA) {
    cairo_surface_t *image = NULL;
    int i;

    lookup = NULL;
    colorMap = colorMapA;
    maskColors = maskColorsA;
    width = widthA;
    current_row = -1;

    /* TODO: Do we want to cache these? */
    imgStr = new ImageStream(str, width,
                             colorMap->getNumPixelComps(),
                             colorMap->getBits());
    imgStr->reset();

#if 0
    /* ICCBased color space doesn't do any color correction
     * so check its underlying color space as well */
    int is_identity_transform;
    is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
      (colorMap->getColorSpace()->getMode() == csICCBased &&
       ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB);
#endif

    // special case for one-channel (monochrome/gray/separation) images:
    // build a lookup table here
    if (colorMap->getNumPixelComps() == 1) {
      int n;
      Guchar pix;

      n = 1 << colorMap->getBits();
      lookup = (GfxRGB *)gmallocn(n, sizeof(GfxRGB));
      for (i = 0; i < n; ++i) {
        pix = (Guchar)i;

        colorMap->getRGB(&pix, &lookup[i]);
      }
    }

    if (printing || scaledWidth >= width || scaledHeight >= height) {
      // No downscaling. Create cairo image containing the source image data.
      unsigned char *buffer;
      int stride;

      image = cairo_image_surface_create (maskColors ?
                                          CAIRO_FORMAT_ARGB32 :
                                          CAIRO_FORMAT_RGB24,
                                          width, height);
      if (cairo_surface_status (image))
        goto cleanup;

      buffer = cairo_image_surface_get_data (image);
      stride = cairo_image_surface_get_stride (image);
      for (int y = 0; y < height; y++) {
        uint32_t *dest = (uint32_t *) (buffer + y * stride);
        getRow(y, dest);
      }
    } else {
      // // Downscaling required. Create cairo image the size of the
      // rescaled image and // downscale the source image data into
      // the cairo image. downScaleImage() will call getRow() to read
      // source image data from the image stream. This avoids having
      // to create an image the size of the source image which may
      // exceed cairo's 32676x32767 image size limit (and also saves a
      // lot of memory).
      image = cairo_image_surface_create (maskColors ?
                                          CAIRO_FORMAT_ARGB32 :
                                          CAIRO_FORMAT_RGB24,
                                          scaledWidth, scaledHeight);
      if (cairo_surface_status (image))
        goto cleanup;

      downScaleImage(width, height,
                     scaledWidth, scaledHeight,
                     0, 0, scaledWidth, scaledHeight,
                     image);
    }
    cairo_surface_mark_dirty (image);

  cleanup:
    gfree(lookup);
    imgStr->close();
    delete imgStr;
    return image;
  }

  void getRow(int row_num, uint32_t *row_data) {
    int i;
    Guchar *pix;

    if (row_num <= current_row)
      return;

    while (current_row  < row_num) {
      pix = imgStr->getLine();
      current_row++;
    }

    if (lookup) {
      Guchar *p = pix;
      GfxRGB rgb;

      for (i = 0; i < width; i++) {
        rgb = lookup[*p];
        row_data[i] =
          ((int) colToByte(rgb.r) << 16) |
          ((int) colToByte(rgb.g) << 8) |
          ((int) colToByte(rgb.b) << 0);
        p++;
      }
    } else {
      colorMap->getRGBLine (pix, row_data, width);
    }

    if (maskColors) {
      for (int x = 0; x < width; x++) {
        bool is_opaque = false;
        for (int i = 0; i < colorMap->getNumPixelComps(); ++i) {
          if (pix[i] < maskColors[2*i] ||
              pix[i] > maskColors[2*i+1]) {
            is_opaque = true;
            break;
          }
        }
        if (is_opaque)
          *row_data |= 0xff000000;
        else
          *row_data = 0;
        row_data++;
        pix += colorMap->getNumPixelComps();
      }
    }
  }

};

void CairoOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			       int widthA, int heightA,
			       GfxImageColorMap *colorMap,
			       GBool interpolate,
			       int *maskColors, GBool inlineImg)
{
  cairo_surface_t *image;
  cairo_pattern_t *pattern, *maskPattern;
  cairo_matrix_t matrix;
  int width, height;
  int scaledWidth, scaledHeight;
  cairo_filter_t filter = CAIRO_FILTER_BILINEAR;
  RescaleDrawImage rescale;

  LOG (printf ("drawImage %dx%d\n", widthA, heightA));

  cairo_get_matrix(cairo, &matrix);
  getScaledSize (&matrix, widthA, heightA, &scaledWidth, &scaledHeight);
  image = rescale.getSourceImage(str, widthA, heightA, scaledWidth, scaledHeight, printing, colorMap, maskColors);
  if (!image)
    return;

  width = cairo_image_surface_get_width (image);
  height = cairo_image_surface_get_height (image);
  if (width == widthA && height == heightA)
    filter = getFilterForSurface (image, interpolate);

  if (!inlineImg) /* don't read stream twice if it is an inline image */
    setMimeData(str, ref, image);

  pattern = cairo_pattern_create_for_surface (image);
  cairo_surface_destroy (image);
  if (cairo_pattern_status (pattern))
    return;

  cairo_pattern_set_filter (pattern, filter);

  if (!printing)
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

  cairo_matrix_init_translate (&matrix, 0, height);
  cairo_matrix_scale (&matrix, width, -height);
  cairo_pattern_set_matrix (pattern, &matrix);
  if (cairo_pattern_status (pattern)) {
    cairo_pattern_destroy (pattern);
    return;
  }

  if (!mask && fill_opacity != 1.0) {
    maskPattern = cairo_pattern_create_rgba (1., 1., 1., fill_opacity);
  } else if (mask) {
    maskPattern = cairo_pattern_reference (mask);
  } else {
    maskPattern = NULL;
  }

  cairo_save (cairo);
  cairo_set_source (cairo, pattern);
  if (!printing)
    cairo_rectangle (cairo, 0., 0., 1., 1.);
  if (maskPattern) {
    if (!printing)
      cairo_clip (cairo);
    if (mask)
      cairo_set_matrix (cairo, &mask_matrix);
    cairo_mask (cairo, maskPattern);
  } else {
    if (printing)
      cairo_paint (cairo);
    else
      cairo_fill (cairo);
  }
  cairo_restore (cairo);

  cairo_pattern_destroy (maskPattern);

  if (cairo_shape) {
    cairo_save (cairo_shape);
    cairo_set_source (cairo_shape, pattern);
    if (printing) {
      cairo_paint (cairo_shape);
    } else {
      cairo_rectangle (cairo_shape, 0., 0., 1., 1.);
      cairo_fill (cairo_shape);
    }
    cairo_restore (cairo_shape);
  }

  cairo_pattern_destroy (pattern);
}


//------------------------------------------------------------------------
// ImageOutputDev
//------------------------------------------------------------------------

CairoImageOutputDev::CairoImageOutputDev()
{
  images = NULL;
  numImages = 0;
  size = 0;
  imgDrawCbk = NULL;
  imgDrawCbkData = NULL;
}

CairoImageOutputDev::~CairoImageOutputDev()
{
  int i;

  for (i = 0; i < numImages; i++)
    delete images[i];
  gfree (images);
}

void CairoImageOutputDev::saveImage(CairoImage *image)
{ 
  if (numImages >= size) {
	  size += 16;
	  images = (CairoImage **) greallocn (images, size, sizeof (CairoImage *));
  }
  images[numImages++] = image;
}

void CairoImageOutputDev::getBBox(GfxState *state, int width, int height,
                                  double *x1, double *y1, double *x2, double *y2)
{
  double *ctm = state->getCTM();
  cairo_matrix_t matrix;
  cairo_matrix_init(&matrix,
                    ctm[0], ctm[1],
                    -ctm[2], -ctm[3],
                    ctm[2] + ctm[4], ctm[3] + ctm[5]);

  int scaledWidth, scaledHeight;
  getScaledSize (&matrix, width, height, &scaledWidth, &scaledHeight);

  if (matrix.xx >= 0) {
    *x1 = matrix.x0;
  } else {
    *x1 = matrix.x0 - scaledWidth;
  }
  *x2 = *x1 + scaledWidth;

  if (matrix.yy >= 0) {
    *y1 = matrix.y0;
  } else {
    *y1 = matrix.y0 - scaledHeight;
  }
  *y2 = *y1 + scaledHeight;
}

void CairoImageOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
					int width, int height, GBool invert,
					GBool interpolate, GBool inlineImg)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  double x1, y1, x2, y2;
  CairoImage *image;

  getBBox(state, width, height, &x1, &y1, &x2, &y2);

  image = new CairoImage (x1, y1, x2, y2);
  saveImage (image);

  if (imgDrawCbk && imgDrawCbk (numImages - 1, imgDrawCbkData)) {
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    setCairo (cr);
    cairo_translate (cr, 0, height);
    cairo_scale (cr, width, -height);

    CairoOutputDev::drawImageMask(state, ref, str, width, height, invert, interpolate, inlineImg);
    image->setImage (surface);

    setCairo (NULL);
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
  }
}

void CairoImageOutputDev::setSoftMaskFromImageMask(GfxState *state, Object *ref, Stream *str,
                                                   int width, int height, GBool invert,
                                                   GBool inlineImg, double *baseMatrix)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  double x1, y1, x2, y2;
  CairoImage *image;

  getBBox(state, width, height, &x1, &y1, &x2, &y2);

  image = new CairoImage (x1, y1, x2, y2);
  saveImage (image);

  if (imgDrawCbk && imgDrawCbk (numImages - 1, imgDrawCbkData)) {
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    setCairo (cr);
    cairo_translate (cr, 0, height);
    cairo_scale (cr, width, -height);

    CairoOutputDev::drawImageMask(state, ref, str, width, height, invert, inlineImg, gFalse);
    if (state->getFillColorSpace()->getMode() == csPattern) {
      cairo_mask (cairo, mask);
    }
    image->setImage (surface);

    setCairo (NULL);
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
  }
}

void CairoImageOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
				    int width, int height, GfxImageColorMap *colorMap,
				    GBool interpolate, int *maskColors, GBool inlineImg)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  double x1, y1, x2, y2;
  CairoImage *image;

  getBBox(state, width, height, &x1, &y1, &x2, &y2);

  image = new CairoImage (x1, y1, x2, y2);
  saveImage (image);

  if (imgDrawCbk && imgDrawCbk (numImages - 1, imgDrawCbkData)) {
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    setCairo (cr);
    cairo_translate (cr, 0, height);
    cairo_scale (cr, width, -height);
    
    CairoOutputDev::drawImage(state, ref, str, width, height, colorMap, interpolate, maskColors, inlineImg);
    image->setImage (surface);
    
    setCairo (NULL);
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
  }
}

void CairoImageOutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
					      int width, int height,
					      GfxImageColorMap *colorMap,
					      GBool interpolate,
					      Stream *maskStr,
					      int maskWidth, int maskHeight,
					      GfxImageColorMap *maskColorMap,
					      GBool maskInterpolate)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  double x1, y1, x2, y2;
  CairoImage *image;

  getBBox(state, width, height, &x1, &y1, &x2, &y2);

  image = new CairoImage (x1, y1, x2, y2);
  saveImage (image);

  if (imgDrawCbk && imgDrawCbk (numImages - 1, imgDrawCbkData)) {
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    setCairo (cr);
    cairo_translate (cr, 0, height);
    cairo_scale (cr, width, -height);
    
    CairoOutputDev::drawSoftMaskedImage(state, ref, str, width, height, colorMap, interpolate,
					maskStr, maskWidth, maskHeight, maskColorMap, maskInterpolate);
    image->setImage (surface);
    
    setCairo (NULL);
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
  }
}

void CairoImageOutputDev::drawMaskedImage(GfxState *state, Object *ref, Stream *str,
					  int width, int height,
					  GfxImageColorMap *colorMap,
					  GBool interpolate,
					  Stream *maskStr,
					  int maskWidth, int maskHeight,
					  GBool maskInvert, GBool maskInterpolate)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  double x1, y1, x2, y2;
  CairoImage *image;

  getBBox(state, width, height, &x1, &y1, &x2, &y2);

  image = new CairoImage (x1, y1, x2, y2);
  saveImage (image);

  if (imgDrawCbk && imgDrawCbk (numImages - 1, imgDrawCbkData)) {
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create (surface);
    setCairo (cr);
    cairo_translate (cr, 0, height);
    cairo_scale (cr, width, -height);
    
    CairoOutputDev::drawMaskedImage(state, ref, str, width, height, colorMap, interpolate,
				    maskStr, maskWidth, maskHeight, maskInvert, maskInterpolate);
    image->setImage (surface);
    
    setCairo (NULL);
    cairo_surface_destroy (surface);
    cairo_destroy (cr);
  }
}
