/* poppler.h: glib interface to poppler
 * Copyright (C) 2004, Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __POPPLER_GLIB_H__
#define __POPPLER_GLIB_H__

#include <glib-object.h>

G_BEGIN_DECLS

GQuark poppler_error_quark (void);

#define POPPLER_ERROR poppler_error_quark ()

/**
 * PopplerError:
 * @POPPLER_ERROR_INVALID: Generic error when a document opration fails
 * @POPPLER_ERROR_ENCRYPTED: Document is encrypted
 * @POPPLER_ERROR_OPEN_FILE: File could not be opened for writing when saving document
 * @POPPLER_ERROR_BAD_CATALOG: Failed to read the document catalog
 * @POPPLER_ERROR_DAMAGED: Document is damaged
 *
 * Error codes returned by #PopplerDocument
 */
typedef enum
{
  POPPLER_ERROR_INVALID,
  POPPLER_ERROR_ENCRYPTED,
  POPPLER_ERROR_OPEN_FILE,
  POPPLER_ERROR_BAD_CATALOG,
  POPPLER_ERROR_DAMAGED
} PopplerError;

typedef enum
{
  POPPLER_ORIENTATION_PORTRAIT,
  POPPLER_ORIENTATION_LANDSCAPE,
  POPPLER_ORIENTATION_UPSIDEDOWN,
  POPPLER_ORIENTATION_SEASCAPE
} PopplerOrientation;

/**
 * PopplerPageTransitionType:
 * @POPPLER_PAGE_TRANSITION_REPLACE: the new page replace the old one
 * @POPPLER_PAGE_TRANSITION_SPLIT: two lines sweep across the screen, revealing the new page
 * @POPPLER_PAGE_TRANSITION_BLINDS: multiple lines, evenly spaced across the screen, synchronously
 * sweep in the same direction to reveal the new page
 * @POPPLER_PAGE_TRANSITION_BOX: a rectangular box sweeps inward from the edges of the page or
 * outward from the center revealing the new page
 * @POPPLER_PAGE_TRANSITION_WIPE: a single line sweeps across the screen from one edge to the other
 * revealing the new page
 * @POPPLER_PAGE_TRANSITION_DISSOLVE: the old page dissolves gradually to reveal the new one
 * @POPPLER_PAGE_TRANSITION_GLITTER: similar to #POPPLER_PAGE_TRANSITION_DISSOLVE, except that the effect
 * sweeps across the page in a wide band moving from one side of the screen to the other
 * @POPPLER_PAGE_TRANSITION_FLY: changes are flown out or in to or from a location that is offscreen
 * @POPPLER_PAGE_TRANSITION_PUSH: the old page slides off the screen while the new page slides in
 * @POPPLER_PAGE_TRANSITION_COVER: the new page slides on to the screen covering the old page
 * @POPPLER_PAGE_TRANSITION_UNCOVER: the old page slides off the screen uncovering the new page
 * @POPPLER_PAGE_TRANSITION_FADE: the new page gradually becomes visible through the old one
 *
 * Page transition types
 */
typedef enum
{
  POPPLER_PAGE_TRANSITION_REPLACE,
  POPPLER_PAGE_TRANSITION_SPLIT,
  POPPLER_PAGE_TRANSITION_BLINDS,
  POPPLER_PAGE_TRANSITION_BOX,
  POPPLER_PAGE_TRANSITION_WIPE,
  POPPLER_PAGE_TRANSITION_DISSOLVE,
  POPPLER_PAGE_TRANSITION_GLITTER,
  POPPLER_PAGE_TRANSITION_FLY,
  POPPLER_PAGE_TRANSITION_PUSH,
  POPPLER_PAGE_TRANSITION_COVER,
  POPPLER_PAGE_TRANSITION_UNCOVER,
  POPPLER_PAGE_TRANSITION_FADE
} PopplerPageTransitionType;

/**
 * PopplerPageTransitionAlignment:
 * @POPPLER_PAGE_TRANSITION_HORIZONTAL: horizontal dimension
 * @POPPLER_PAGE_TRANSITION_VERTICAL: vertical dimension
 *
 * Page transition alignment types for #POPPLER_PAGE_TRANSITION_SPLIT
 * and #POPPLER_PAGE_TRANSITION_BLINDS transition types
 */
typedef enum
{
  POPPLER_PAGE_TRANSITION_HORIZONTAL,
  POPPLER_PAGE_TRANSITION_VERTICAL
} PopplerPageTransitionAlignment;

/**
 * PopplerPageTransitionDirection:
 * @POPPLER_PAGE_TRANSITION_INWARD: inward from the edges of the page
 * @POPPLER_PAGE_TRANSITION_OUTWARD: outward from the center of the page
 *
 * Page transition direction types for #POPPLER_PAGE_TRANSITION_SPLIT,
 * #POPPLER_PAGE_TRANSITION_BOX and #POPPLER_PAGE_TRANSITION_FLY transition types
 */
typedef enum
{
  POPPLER_PAGE_TRANSITION_INWARD,
  POPPLER_PAGE_TRANSITION_OUTWARD
} PopplerPageTransitionDirection;

/**
 * PopplerSelectionStyle:
 * @POPPLER_SELECTION_GLYPH: glyph is the minimum unit for selection
 * @POPPLER_SELECTION_WORD: word is the minimum unit for selection
 * @POPPLER_SELECTION_LINE: line is the minimum unit for selection
 *
 * Selection styles
 */
typedef enum
{
  POPPLER_SELECTION_GLYPH,
  POPPLER_SELECTION_WORD,
  POPPLER_SELECTION_LINE
} PopplerSelectionStyle;

/**
 * PopplerPrintFlags:
 * @POPPLER_PRINT_DOCUMENT: print main document contents
 * @POPPLER_PRINT_MARKUP_ANNOTS: print document and markup annotations
 * @POPPLER_PRINT_STAMP_ANNOTS_ONLY: print doucment and only stamp annotations
 * @POPPLER_PRINT_ALL: print main document contents and all markup annotations
 *
 * Printing flags
 *
 * Since: 0.16
 */
typedef enum /*< flags >*/
{
  POPPLER_PRINT_DOCUMENT          = 0,
  POPPLER_PRINT_MARKUP_ANNOTS     = 1 << 0,
  POPPLER_PRINT_STAMP_ANNOTS_ONLY = 1 << 1,
  POPPLER_PRINT_ALL               = POPPLER_PRINT_MARKUP_ANNOTS
} PopplerPrintFlags;

typedef struct _PopplerDocument            PopplerDocument;
typedef struct _PopplerIndexIter           PopplerIndexIter;
typedef struct _PopplerFontsIter           PopplerFontsIter;
typedef struct _PopplerLayersIter          PopplerLayersIter;
typedef struct _PopplerRectangle           PopplerRectangle;
typedef struct _PopplerColor               PopplerColor;
typedef struct _PopplerLinkMapping         PopplerLinkMapping;
typedef struct _PopplerPageTransition      PopplerPageTransition;
typedef struct _PopplerImageMapping        PopplerImageMapping;
typedef struct _PopplerFormFieldMapping    PopplerFormFieldMapping;
typedef struct _PopplerAnnotMapping        PopplerAnnotMapping;
typedef struct _PopplerPage                PopplerPage;
typedef struct _PopplerFontInfo            PopplerFontInfo;
typedef struct _PopplerLayer               PopplerLayer;
typedef struct _PopplerPSFile              PopplerPSFile;
typedef union  _PopplerAction              PopplerAction;
typedef struct _PopplerDest                PopplerDest;
typedef struct _PopplerActionLayer         PopplerActionLayer;
typedef struct _PopplerFormField           PopplerFormField;
typedef struct _PopplerAttachment          PopplerAttachment;
typedef struct _PopplerMovie               PopplerMovie;
typedef struct _PopplerMedia               PopplerMedia;
typedef struct _PopplerAnnot               PopplerAnnot;
typedef struct _PopplerAnnotMarkup         PopplerAnnotMarkup;
typedef struct _PopplerAnnotText           PopplerAnnotText;
typedef struct _PopplerAnnotFreeText       PopplerAnnotFreeText;
typedef struct _PopplerAnnotFileAttachment PopplerAnnotFileAttachment;
typedef struct _PopplerAnnotMovie          PopplerAnnotMovie;
typedef struct _PopplerAnnotScreen         PopplerAnnotScreen;
typedef struct _PopplerAnnotCalloutLine    PopplerAnnotCalloutLine;

typedef enum
{
  POPPLER_BACKEND_UNKNOWN,
  POPPLER_BACKEND_SPLASH,
  POPPLER_BACKEND_CAIRO
} PopplerBackend;

PopplerBackend poppler_get_backend (void);
const char *   poppler_get_version (void);

G_END_DECLS

#include "poppler-features.h"
#include "poppler-document.h"
#include "poppler-page.h"
#include "poppler-layer.h"
#include "poppler-action.h"
#include "poppler-form-field.h"
#include "poppler-enums.h"
#include "poppler-attachment.h"
#include "poppler-annot.h"
#include "poppler-date.h"
#include "poppler-movie.h"
#include "poppler-media.h"

#endif /* __POPPLER_GLIB_H__ */
