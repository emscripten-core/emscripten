/* poppler-page.cc: glib wrapper for poppler
 * Copyright (C) 2005, Red Hat, Inc.
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

#include "config.h"
#include <math.h>

#ifndef __GI_SCANNER__
#include <goo/GooList.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Outline.h>
#include <ErrorCodes.h>
#include <UnicodeMap.h>
#include <GfxState.h>
#include <PageTransition.h>
#endif

#include "poppler.h"
#include "poppler-private.h"

/**
 * SECTION:poppler-page
 * @short_description: Information about a page in a document
 * @title: PopplerPage
 */

enum
{
  PROP_0,
  PROP_LABEL
};

typedef struct _PopplerPageClass PopplerPageClass;
struct _PopplerPageClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerPage, poppler_page, G_TYPE_OBJECT)

PopplerPage *
_poppler_page_new (PopplerDocument *document, Page *page, int index)
{
  PopplerPage *poppler_page;

  g_return_val_if_fail (POPPLER_IS_DOCUMENT (document), NULL);

  poppler_page = (PopplerPage *) g_object_new (POPPLER_TYPE_PAGE, NULL, NULL);
  poppler_page->document = (PopplerDocument *) g_object_ref (document);
  poppler_page->page = page;
  poppler_page->index = index;

  return poppler_page;
}

static void
poppler_page_finalize (GObject *object)
{
  PopplerPage *page = POPPLER_PAGE (object);

  g_object_unref (page->document);
  page->document = NULL;

  if (page->annots != NULL)
    delete page->annots;
  if (page->text != NULL) 
    page->text->decRefCnt();
  /* page->page is owned by the document */
}

/**
 * poppler_page_get_size:
 * @page: A #PopplerPage
 * @width: (out) (allow-none): return location for the width of @page
 * @height: (out) (allow-none): return location for the height of @page
 * 
 * Gets the size of @page at the current scale and rotation.
 **/
void
poppler_page_get_size (PopplerPage *page,
		       double      *width,
		       double      *height)
{
  double page_width, page_height;
  int rotate;

  g_return_if_fail (POPPLER_IS_PAGE (page));

  rotate = page->page->getRotate ();
  if (rotate == 90 || rotate == 270) {
    page_height = page->page->getCropWidth ();
    page_width = page->page->getCropHeight ();
  } else {
    page_width = page->page->getCropWidth ();
    page_height = page->page->getCropHeight ();
  }

  if (width != NULL)
    *width = page_width;
  if (height != NULL)
    *height = page_height;
}

/**
 * poppler_page_get_index:
 * @page: a #PopplerPage
 * 
 * Returns the index of @page
 * 
 * Return value: index value of @page
 **/
int
poppler_page_get_index (PopplerPage *page)
{
  g_return_val_if_fail (POPPLER_IS_PAGE (page), 0);

  return page->index;
}

/**
 * poppler_page_get_label:
 * @page: a #PopplerPage
 *
 * Returns the label of @page. Note that page labels
 * and page indices might not coincide.
 *
 * Return value: a new allocated string containing the label of @page,
 *               or %NULL if @page doesn't have a label
 *
 * Since: 0.16
 **/
gchar *
poppler_page_get_label (PopplerPage *page)
{
  GooString label;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  page->document->doc->getCatalog ()->indexToLabel (page->index, &label);
  return _poppler_goo_string_to_utf8 (&label);
}

/**
 * poppler_page_get_duration:
 * @page: a #PopplerPage
 *
 * Returns the duration of @page
 *
 * Return value: duration in seconds of @page or -1.
 **/
double
poppler_page_get_duration (PopplerPage *page)
{
  g_return_val_if_fail (POPPLER_IS_PAGE (page), -1);

  return page->page->getDuration ();
}

/**
 * poppler_page_get_transition:
 * @page: a #PopplerPage
 *
 * Returns the transition effect of @page
 *
 * Return value: a #PopplerPageTransition or NULL.
 **/
PopplerPageTransition *
poppler_page_get_transition (PopplerPage *page)
{
  PageTransition *trans;
  PopplerPageTransition *transition;
  Object obj;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  trans = new PageTransition (page->page->getTrans (&obj));
  obj.free ();

  if (!trans->isOk ()) {
    delete trans;
    return NULL;
  }

  transition = poppler_page_transition_new ();

  switch (trans->getType ())
    {
      case transitionReplace:
        transition->type = POPPLER_PAGE_TRANSITION_REPLACE;
	break;
      case transitionSplit:
	transition->type = POPPLER_PAGE_TRANSITION_SPLIT;
	break;
      case transitionBlinds:
        transition->type = POPPLER_PAGE_TRANSITION_BLINDS;
	break;
      case transitionBox:
        transition->type = POPPLER_PAGE_TRANSITION_BOX;
	break;
      case transitionWipe:
        transition->type = POPPLER_PAGE_TRANSITION_WIPE;
	break;
      case transitionDissolve:
        transition->type = POPPLER_PAGE_TRANSITION_DISSOLVE;
	break;
      case transitionGlitter:
        transition->type = POPPLER_PAGE_TRANSITION_GLITTER;
	break;
      case transitionFly:
        transition->type = POPPLER_PAGE_TRANSITION_FLY;
	break;
      case transitionPush:
        transition->type = POPPLER_PAGE_TRANSITION_PUSH;
	break;
      case transitionCover:
        transition->type = POPPLER_PAGE_TRANSITION_COVER;
	break;
      case transitionUncover:
        transition->type = POPPLER_PAGE_TRANSITION_UNCOVER;
        break;
      case transitionFade:
        transition->type = POPPLER_PAGE_TRANSITION_FADE;
	break;
      default:
        g_assert_not_reached ();
    }

  transition->alignment = (trans->getAlignment() == transitionHorizontal) ?
	  POPPLER_PAGE_TRANSITION_HORIZONTAL :
	  POPPLER_PAGE_TRANSITION_VERTICAL;

  transition->direction = (trans->getDirection() == transitionInward) ?
	  POPPLER_PAGE_TRANSITION_INWARD :
	  POPPLER_PAGE_TRANSITION_OUTWARD;

  transition->duration = trans->getDuration();
  transition->angle = trans->getAngle();
  transition->scale = trans->getScale();
  transition->rectangular = trans->isRectangular();
  
  delete trans;
  
  return transition;
}

static TextPage *
poppler_page_get_text_page (PopplerPage *page)
{
  if (page->text == NULL) {
    TextOutputDev *text_dev;
    Gfx           *gfx;

    text_dev = new TextOutputDev (NULL, gTrue, gFalse, gFalse);
    gfx = page->page->createGfx(text_dev,
				72.0, 72.0, 0,
				gFalse, /* useMediaBox */
				gTrue, /* Crop */
				-1, -1, -1, -1,
				gFalse, /* printing */
				page->document->doc->getCatalog (),
				NULL, NULL, NULL, NULL);
    page->page->display(gfx);
    text_dev->endPage();

    page->text = text_dev->takeText();
    delete gfx;
    delete text_dev;
  }

  return page->text;
}

#ifdef POPPLER_WITH_GDK
static void
copy_cairo_surface_to_pixbuf (cairo_surface_t *surface,
			      GdkPixbuf       *pixbuf)
{
  int cairo_width, cairo_height, cairo_rowstride;
  unsigned char *pixbuf_data, *dst, *cairo_data;
  int pixbuf_rowstride, pixbuf_n_channels;
  unsigned int *src;
  int x, y;

  cairo_width = cairo_image_surface_get_width (surface);
  cairo_height = cairo_image_surface_get_height (surface);
  cairo_rowstride = cairo_image_surface_get_stride (surface);
  cairo_data = cairo_image_surface_get_data (surface);

  pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
  pixbuf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixbuf_n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  if (cairo_width > gdk_pixbuf_get_width (pixbuf))
    cairo_width = gdk_pixbuf_get_width (pixbuf);
  if (cairo_height > gdk_pixbuf_get_height (pixbuf))
    cairo_height = gdk_pixbuf_get_height (pixbuf);
  for (y = 0; y < cairo_height; y++)
    {
      src = (unsigned int *) (cairo_data + y * cairo_rowstride);
      dst = pixbuf_data + y * pixbuf_rowstride;
      for (x = 0; x < cairo_width; x++) 
	{
	  dst[0] = (*src >> 16) & 0xff;
	  dst[1] = (*src >> 8) & 0xff; 
	  dst[2] = (*src >> 0) & 0xff;
	  if (pixbuf_n_channels == 4)
	      dst[3] = (*src >> 24) & 0xff;
	  dst += pixbuf_n_channels;
	  src++;
	}
    }
}	
#endif /* POPPLER_WITH_GDK */

static gboolean
annot_is_markup (Annot *annot)
{
  switch (annot->getType())
    {
      case Annot::typeLink:
      case Annot::typePopup:
      case Annot::typeMovie:
      case Annot::typeScreen:
      case Annot::typePrinterMark:
      case Annot::typeTrapNet:
      case Annot::typeWatermark:
      case Annot::type3D:
      case Annot::typeWidget:
        return FALSE;
      default:
        return TRUE;
    }
}

static GBool
poppler_print_annot_cb (Annot *annot, void *user_data)
{
  PopplerPrintFlags user_print_flags = (PopplerPrintFlags)GPOINTER_TO_INT (user_data);

  if (annot->getFlags () & Annot::flagHidden)
    return gFalse;

  if (user_print_flags & POPPLER_PRINT_STAMP_ANNOTS_ONLY) {
    return (annot->getType() == Annot::typeStamp) ?
            (annot->getFlags () & Annot::flagPrint) :
            (annot->getType() == Annot::typeWidget);
  }

  if (user_print_flags & POPPLER_PRINT_MARKUP_ANNOTS) {
    return annot_is_markup (annot) ?
            (annot->getFlags () & Annot::flagPrint) :
            (annot->getType() == Annot::typeWidget);
  }

  /* Print document only, form fields are always printed */
  return (annot->getType() == Annot::typeWidget);
}

static void
_poppler_page_render (PopplerPage      *page,
		      cairo_t          *cairo,
		      GBool             printing,
                      PopplerPrintFlags print_flags)
{
  CairoOutputDev *output_dev;

  g_return_if_fail (POPPLER_IS_PAGE (page));

  output_dev = page->document->output_dev;
  output_dev->setCairo (cairo);
  output_dev->setPrinting (printing);

  if (!printing)
    output_dev->setTextPage (page->text);

  /* NOTE: instead of passing -1 we should/could use cairo_clip_extents()
   * to get a bounding box */
  cairo_save (cairo);
  page->page->displaySlice(output_dev,
			   72.0, 72.0, 0,
			   gFalse, /* useMediaBox */
			   gTrue, /* Crop */
			   -1, -1,
			   -1, -1,
			   printing,
			   page->document->doc->getCatalog (),
			   NULL, NULL,
			   printing ? poppler_print_annot_cb : NULL,
                           printing ? GINT_TO_POINTER ((gint)print_flags) : NULL);
  cairo_restore (cairo);

  output_dev->setCairo (NULL);
  output_dev->setTextPage (NULL);
}

/**
 * poppler_page_render:
 * @page: the page to render from
 * @cairo: cairo context to render to
 *
 * Render the page to the given cairo context. This function
 * is for rendering a page that will be displayed. If you want
 * to render a page that will be printed use
 * poppler_page_render_for_printing() instead
 **/
void
poppler_page_render (PopplerPage *page,
		     cairo_t *cairo)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));

  if (!page->text)
    page->text = new TextPage(gFalse);

  _poppler_page_render (page, cairo, gFalse, (PopplerPrintFlags)0);
}

/**
 * poppler_page_render_for_printing_with_options:
 * @page: the page to render from
 * @cairo: cairo context to render to
 * @options: print options
 *
 * Render the page to the given cairo context for printing
 * with the specified options
 *
 * Since: 0.16
 **/
void
poppler_page_render_for_printing_with_options (PopplerPage      *page,
                                               cairo_t          *cairo,
                                               PopplerPrintFlags options)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));

  _poppler_page_render (page, cairo, gTrue, options);
}

/**
 * poppler_page_render_for_printing:
 * @page: the page to render from
 * @cairo: cairo context to render to
 *
 * Render the page to the given cairo context for printing.
 **/
void
poppler_page_render_for_printing (PopplerPage *page,
				  cairo_t *cairo)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));

  _poppler_page_render (page, cairo, gTrue, POPPLER_PRINT_ALL);
}

static cairo_surface_t *
create_surface_from_thumbnail_data (guchar *data,
				    gint    width,
				    gint    height,
				    gint    rowstride)
{
  guchar *cairo_pixels;
  gint cairo_stride;
  cairo_surface_t *surface;
  int j;

  surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
  if (cairo_surface_status (surface))
    return NULL;

  cairo_pixels = cairo_image_surface_get_data (surface);
  cairo_stride = cairo_image_surface_get_stride (surface);

  for (j = height; j; j--) {
    guchar *p = data;
    guchar *q = cairo_pixels;
    guchar *end = p + 3 * width;

    while (p < end) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
      q[0] = p[2];
      q[1] = p[1];
      q[2] = p[0];
#else
      q[1] = p[0];
      q[2] = p[1];
      q[3] = p[2];
#endif
      p += 3;
      q += 4;
    }

    data += rowstride;
    cairo_pixels += cairo_stride;
  }

  return surface;
}
				    

/**
 * poppler_page_get_thumbnail:
 * @page: the #PopperPage to get the thumbnail for
 * 
 * Get the embedded thumbnail for the specified page.  If the document
 * doesn't have an embedded thumbnail for the page, this function
 * returns %NULL.
 * 
 * Return value: the tumbnail as a cairo_surface_t or %NULL if the document
 * doesn't have a thumbnail for this page.
 **/
cairo_surface_t *
poppler_page_get_thumbnail (PopplerPage *page)
{
  unsigned char *data;
  int width, height, rowstride;
  cairo_surface_t *surface;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  if (!page->page->loadThumb (&data, &width, &height, &rowstride))
    return NULL;

  surface = create_surface_from_thumbnail_data (data, width, height, rowstride);
  gfree (data);
  
  return surface;
}

/**
 * poppler_page_render_selection:
 * @page: the #PopplerPage for which to render selection
 * @cairo: cairo context to render to
 * @selection: start and end point of selection as a rectangle
 * @old_selection: previous selection
 * @style: a #PopplerSelectionStyle
 * @glyph_color: color to use for drawing glyphs
 * @background_color: color to use for the selection background
 *
 * Render the selection specified by @selection for @page to
 * the given cairo context.  The selection will be rendered, using
 * @glyph_color for the glyphs and @background_color for the selection
 * background.
 *
 * If non-NULL, @old_selection specifies the selection that is already
 * rendered to @cairo, in which case this function will (some day)
 * only render the changed part of the selection.
 **/
void
poppler_page_render_selection (PopplerPage           *page,
			       cairo_t               *cairo,
			       PopplerRectangle      *selection,
			       PopplerRectangle      *old_selection,
			       PopplerSelectionStyle  style, 
			       PopplerColor          *glyph_color,
			       PopplerColor          *background_color)
{
  CairoOutputDev *output_dev;
  TextPage *text;
  SelectionStyle selection_style = selectionStyleGlyph;
  PDFRectangle pdf_selection(selection->x1, selection->y1,
			     selection->x2, selection->y2);

  GfxColor gfx_background_color = {
      {
	  background_color->red,
	  background_color->green,
	  background_color->blue
      }
  };
  GfxColor gfx_glyph_color = {
      {
	  glyph_color->red,
	  glyph_color->green,
	  glyph_color->blue
      }
  };

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

  output_dev = page->document->output_dev;
  output_dev->setCairo (cairo);

  text = poppler_page_get_text_page (page);
  text->drawSelection (output_dev, 1.0, 0,
		       &pdf_selection, selection_style,
		       &gfx_glyph_color, &gfx_background_color);

  output_dev->setCairo (NULL);
}

#ifdef POPPLER_WITH_GDK
static void
_poppler_page_render_to_pixbuf (PopplerPage *page,
				int src_x, int src_y,
				int src_width, int src_height,
				double scale,
				int rotation,
				GBool printing,
				GdkPixbuf *pixbuf)
{
  cairo_t *cr;
  cairo_surface_t *surface;

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					src_width, src_height);
  cr = cairo_create (surface);
  cairo_save (cr);
  switch (rotation) {
  case 90:
	  cairo_translate (cr, src_x + src_width, -src_y);
	  break;
  case 180:
	  cairo_translate (cr, src_x + src_width, src_y + src_height);
	  break;
  case 270:
	  cairo_translate (cr, -src_x, src_y + src_height);
	  break;
  default:
	  cairo_translate (cr, -src_x, -src_y);
  }

  if (scale != 1.0)
	  cairo_scale (cr, scale, scale);

  if (rotation != 0)
	  cairo_rotate (cr, rotation * G_PI / 180.0);

  if (printing)
	  poppler_page_render_for_printing (page, cr);
  else
	  poppler_page_render (page, cr);
  cairo_restore (cr);

  cairo_set_operator (cr, CAIRO_OPERATOR_DEST_OVER);
  cairo_set_source_rgb (cr, 1., 1., 1.);
  cairo_paint (cr);

  cairo_destroy (cr);

  copy_cairo_surface_to_pixbuf (surface, pixbuf);
  cairo_surface_destroy (surface);
}

/**
 * poppler_page_render_to_pixbuf:
 * @page: the page to render from
 * @src_x: x coordinate of upper left corner  
 * @src_y: y coordinate of upper left corner  
 * @src_width: width of rectangle to render  
 * @src_height: height of rectangle to render
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render into
 *
 * First scale the document to match the specified pixels per point,
 * then render the rectangle given by the upper left corner at
 * (src_x, src_y) and src_width and src_height.
 * This function is for rendering a page that will be displayed.
 * If you want to render a page that will be printed use
 * poppler_page_render_to_pixbuf_for_printing() instead
 *
 * Deprecated: 0.16
 **/
void
poppler_page_render_to_pixbuf (PopplerPage *page,
			       int src_x, int src_y,
			       int src_width, int src_height,
			       double scale,
			       int rotation,
			       GdkPixbuf *pixbuf)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (scale > 0.0);
  g_return_if_fail (pixbuf != NULL);

  _poppler_page_render_to_pixbuf (page, src_x, src_y,
				  src_width, src_height,
				  scale, rotation,
				  gFalse,
				  pixbuf);
}

/**
 * poppler_page_render_to_pixbuf_for_printing:
 * @page: the page to render from
 * @src_x: x coordinate of upper left corner  
 * @src_y: y coordinate of upper left corner  
 * @src_width: width of rectangle to render  
 * @src_height: height of rectangle to render
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render into
 *
 * First scale the document to match the specified pixels per point,
 * then render the rectangle given by the upper left corner at
 * (src_x, src_y) and src_width and src_height.
 * This function is for rendering a page that will be printed.
 *
 * Deprecated: 0.16
 **/
void
poppler_page_render_to_pixbuf_for_printing (PopplerPage *page,
					    int src_x, int src_y,
					    int src_width, int src_height,
					    double scale,
					    int rotation,
					    GdkPixbuf *pixbuf)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (scale > 0.0);
  g_return_if_fail (pixbuf != NULL);

  _poppler_page_render_to_pixbuf (page, src_x, src_y,
				  src_width, src_height,
				  scale, rotation,
				  gTrue,
				  pixbuf);
}

/**
 * poppler_page_get_thumbnail_pixbuf:
 * @page: the #PopperPage to get the thumbnail for
 * 
 * Get the embedded thumbnail for the specified page.  If the document
 * doesn't have an embedded thumbnail for the page, this function
 * returns %NULL.
 * 
 * Return value: the tumbnail as a #GdkPixbuf or %NULL if the document
 * doesn't have a thumbnail for this page.
 *
 * Deprecated: 0.16
 **/
GdkPixbuf *
poppler_page_get_thumbnail_pixbuf (PopplerPage *page)
{
  unsigned char *data;
  int width, height, rowstride;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  if (!page->page->loadThumb (&data, &width, &height, &rowstride))
    return NULL;

  return gdk_pixbuf_new_from_data (data, GDK_COLORSPACE_RGB,
				   FALSE, 8, width, height, rowstride,
				   (GdkPixbufDestroyNotify)gfree, NULL);
}

/**
 * poppler_page_render_selection_to_pixbuf:
 * @page: the #PopplerPage for which to render selection
 * @scale: scale specified as pixels per point
 * @rotation: rotate the document by the specified degree
 * @pixbuf: pixbuf to render to
 * @selection: start and end point of selection as a rectangle
 * @old_selection: previous selection
 * @style: a #PopplerSelectionStyle
 * @glyph_color: color to use for drawing glyphs
 * @background_color: color to use for the selection background
 * 
 * Render the selection specified by @selection for @page into
 * @pixbuf.  The selection will be rendered at @scale, using
 * @glyph_color for the glyphs and @background_color for the selection
 * background.
 *
 * If non-NULL, @old_selection specifies the selection that is already
 * rendered in @pixbuf, in which case this function will (some day)
 * only render the changed part of the selection.
 *
 * Deprecated: 0.16
 **/
void
poppler_page_render_selection_to_pixbuf (PopplerPage           *page,
                                         gdouble                scale,
                                         int                    rotation,
                                         GdkPixbuf             *pixbuf,
                                         PopplerRectangle      *selection,
                                         PopplerRectangle      *old_selection,
					 PopplerSelectionStyle  style,
                                         GdkColor              *glyph_color,
                                         GdkColor              *background_color)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  double width, height;
  int cairo_width, cairo_height, rotate;
  PopplerColor poppler_background_color;
  PopplerColor poppler_glyph_color;

  poppler_background_color.red = background_color->red;
  poppler_background_color.green = background_color->green;
  poppler_background_color.blue = background_color->blue;
  poppler_glyph_color.red = glyph_color->red;
  poppler_glyph_color.green = glyph_color->green;
  poppler_glyph_color.blue = glyph_color->blue;

  rotate = rotation + page->page->getRotate ();
  if (rotate == 90 || rotate == 270) {
    height = page->page->getCropWidth ();
    width = page->page->getCropHeight ();
  } else {
    width = page->page->getCropWidth ();
    height = page->page->getCropHeight ();
  }

  cairo_width = (int) ceil(width * scale);
  cairo_height = (int) ceil(height * scale);

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					cairo_width, cairo_height);
  cr = cairo_create (surface);
  cairo_set_source_rgba (cr, 0, 0, 0, 0);
  cairo_paint (cr);

  switch (rotate) {
  case 90:
	  cairo_translate (cr, cairo_width, 0);
	  break;
  case 180:
	  cairo_translate (cr, cairo_width, cairo_height);
	  break;
  case 270:
	  cairo_translate (cr, 0, cairo_height);
	  break;
  default:
	  cairo_translate (cr, 0, 0);
  }
  if (scale != 1.0)
	  cairo_scale (cr, scale, scale);

  if (rotate != 0)
	  cairo_rotate (cr, rotation * G_PI / 180.0);

  poppler_page_render_selection (page, cr, selection, old_selection, style,
				 &poppler_glyph_color, &poppler_background_color);

  cairo_destroy (cr);

  copy_cairo_surface_to_pixbuf (surface, pixbuf);
  cairo_surface_destroy (surface);
}

#endif /* POPPLER_WITH_GDK */

/**
 * poppler_page_get_thumbnail_size:
 * @page: A #PopplerPage
 * @width: (out) return location for width
 * @height: (out) return location for height
 *
 * Returns %TRUE if @page has a thumbnail associated with it.  It also
 * fills in @width and @height with the width and height of the
 * thumbnail.  The values of width and height are not changed if no
 * appropriate thumbnail exists.
 *
 * Return value: %TRUE, if @page has a thumbnail associated with it.
 **/
gboolean
poppler_page_get_thumbnail_size (PopplerPage *page,
				 int         *width,
				 int         *height)
{
  Object thumb;
  Dict *dict;
  gboolean retval = FALSE;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);
  g_return_val_if_fail (width != NULL, FALSE);
  g_return_val_if_fail (height != NULL, FALSE);

  page->page->getThumb (&thumb);
  if (!thumb.isStream ())
    {
      thumb.free ();
      return FALSE;
    }

  dict = thumb.streamGetDict();

  /* Theoretically, this could succeed and you would still fail when
   * loading the thumb */
  if (dict->lookupInt ("Width", "W", width)  &&
      dict->lookupInt ("Height", "H", height))
    retval = TRUE;

  thumb.free ();

  return retval;
}

/**
 * poppler_page_get_selection_region:
 * @page: a #PopplerPage
 * @scale: scale specified as pixels per point
 * @style: a #PopplerSelectionStyle
 * @selection: start and end point of selection as a rectangle
 * 
 * Returns a region containing the area that would be rendered by
 * poppler_page_render_selection() or 
 * poppler_page_render_selection_to_pixbuf() as a #GList of
 * #PopplerRectangle. The returned list must be freed with
 * poppler_page_selection_region_free().
 * 
 * Return value: (element-type PopplerRectangle) (transfer full): a #GList of #PopplerRectangle
 *
 * Deprecated: 0.16: Use poppler_page_get_selected_region() instead.
 **/
GList *
poppler_page_get_selection_region (PopplerPage           *page,
				   gdouble                scale,
				   PopplerSelectionStyle  style,
				   PopplerRectangle      *selection)
{
  PDFRectangle poppler_selection;
  TextPage *text;
  SelectionStyle selection_style = selectionStyleGlyph;
  GooList *list;
  GList *region = NULL;
  int i;

  poppler_selection.x1 = selection->x1;
  poppler_selection.y1 = selection->y1;
  poppler_selection.x2 = selection->x2;
  poppler_selection.y2 = selection->y2;

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

  text = poppler_page_get_text_page (page);
  list = text->getSelectionRegion(&poppler_selection,
				  selection_style, scale);

  for (i = 0; i < list->getLength(); i++) {
    PDFRectangle *selection_rect = (PDFRectangle *) list->get(i);
    PopplerRectangle *rect;

    rect = poppler_rectangle_new ();
    
    rect->x1 = selection_rect->x1;
    rect->y1 = selection_rect->y1;
    rect->x2 = selection_rect->x2;
    rect->y2 = selection_rect->y2;
    
    region = g_list_prepend (region, rect);
    
    delete selection_rect;
  }

  delete list;

  return g_list_reverse (region);
}

/**
 * poppler_page_selection_region_free:
 * @region: a #GList of #PopplerRectangle
 *
 * Frees @region
 *
 * Deprecated: 0.16
 */
void
poppler_page_selection_region_free (GList *region)
{
  if (G_UNLIKELY (!region))
    return;

  g_list_foreach (region, (GFunc)poppler_rectangle_free, NULL);
  g_list_free (region);
}

/**
 * poppler_page_get_selected_region:
 * @page: a #PopplerPage
 * @scale: scale specified as pixels per point
 * @style: a #PopplerSelectionStyle
 * @selection: start and end point of selection as a rectangle
 *
 * Returns a region containing the area that would be rendered by
 * poppler_page_render_selection() or
 * poppler_page_render_selection_to_pixbuf().
 * The returned region must be freed with cairo_region_destroy()
 *
 * Return value: (transfer full): a cairo_region_t
 *
 * Since: 0.16
 **/
cairo_region_t *
poppler_page_get_selected_region (PopplerPage           *page,
                                  gdouble                scale,
                                  PopplerSelectionStyle  style,
                                  PopplerRectangle      *selection)
{
  PDFRectangle poppler_selection;
  TextPage *text;
  SelectionStyle selection_style = selectionStyleGlyph;
  GooList *list;
  cairo_region_t *region;
  int i;

  poppler_selection.x1 = selection->x1;
  poppler_selection.y1 = selection->y1;
  poppler_selection.x2 = selection->x2;
  poppler_selection.y2 = selection->y2;

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

  text = poppler_page_get_text_page (page);
  list = text->getSelectionRegion(&poppler_selection,
				  selection_style, 1.0);

  region = cairo_region_create ();

  for (i = 0; i < list->getLength(); i++) {
    PDFRectangle *selection_rect = (PDFRectangle *) list->get(i);
    cairo_rectangle_int_t rect;

    rect.x = (gint) ((selection_rect->x1 * scale) + 0.5);
    rect.y = (gint) ((selection_rect->y1 * scale) + 0.5);
    rect.width = (gint) (((selection_rect->x2 - selection_rect->x1) * scale) + 0.5);
    rect.height = (gint) (((selection_rect->y2 - selection_rect->y1) * scale) + 0.5);
    cairo_region_union_rectangle (region, &rect);

    delete selection_rect;
  }

  delete list;

  return region;
}

/**
 * poppler_page_get_selected_text:
 * @page: a #PopplerPage
 * @style: a #PopplerSelectionStyle
 * @selection: the #PopplerRectangle including the text
 *
 * Retrieves the contents of the specified @selection as text.
 *
 * Return value: a pointer to the contents of the @selection
 *               as a string
 * Since: 0.16
 **/
char *
poppler_page_get_selected_text (PopplerPage          *page,
				PopplerSelectionStyle style,
				PopplerRectangle     *selection)
{
  GooString *sel_text;
  char *result;
  TextPage *text;
  SelectionStyle selection_style = selectionStyleGlyph;
  PDFRectangle pdf_selection;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);
  g_return_val_if_fail (selection != NULL, NULL);

  pdf_selection.x1 = selection->x1;
  pdf_selection.y1 = selection->y1;
  pdf_selection.x2 = selection->x2;
  pdf_selection.y2 = selection->y2;

  switch (style)
    {
      case POPPLER_SELECTION_GLYPH:
        selection_style = selectionStyleGlyph;
	break;
      case POPPLER_SELECTION_WORD:
        selection_style = selectionStyleWord;
	break;
      case POPPLER_SELECTION_LINE:
        selection_style = selectionStyleLine;
	break;
    }

  text = poppler_page_get_text_page (page);
  sel_text = text->getSelectionText (&pdf_selection, selection_style);
  result = g_strdup (sel_text->getCString ());
  delete sel_text;

  return result;
}

/**
 * poppler_page_get_text:
 * @page: a #PopplerPage
 *
 * Retrieves the text of @page.
 *
 * Return value: a pointer to the text of the @page
 *               as a string
 * Since: 0.16
 **/
char *
poppler_page_get_text (PopplerPage *page)
{
  PopplerRectangle rectangle = {0, 0, 0, 0};

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  poppler_page_get_size (page, &rectangle.x2, &rectangle.y2);

  return poppler_page_get_selected_text (page, POPPLER_SELECTION_GLYPH, &rectangle);
}

/**
 * poppler_page_find_text:
 * @page: a #PopplerPage
 * @text: the text to search for (UTF-8 encoded)
 * 
 * A #GList of rectangles for each occurance of the text on the page.
 * The coordinates are in PDF points.
 * 
 * Return value: (element-type PopplerRectangle) (transfer full): a #GList of #PopplerRectangle,
 **/
GList *
poppler_page_find_text (PopplerPage *page,
			const char  *text)
{
  PopplerRectangle *match;
  GList *matches;
  double xMin, yMin, xMax, yMax;
  gunichar *ucs4;
  glong ucs4_len;
  double height;
  TextPage *text_dev;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);
  g_return_val_if_fail (text != NULL, NULL);

  text_dev = poppler_page_get_text_page (page);

  ucs4 = g_utf8_to_ucs4_fast (text, -1, &ucs4_len);
  poppler_page_get_size (page, NULL, &height);
  
  matches = NULL;
  xMin = 0;
  yMin = 0;

  while (text_dev->findText (ucs4, ucs4_len,
			     gFalse, gTrue, // startAtTop, stopAtBottom
			     gFalse, gFalse, // startAtLast, stopAtLast
			     gFalse, gFalse, // caseSensitive, backwards
			     &xMin, &yMin, &xMax, &yMax))
    {
      match = poppler_rectangle_new ();
      match->x1 = xMin;
      match->y1 = height - yMax;
      match->x2 = xMax;
      match->y2 = height - yMin;
      matches = g_list_prepend (matches, match);
    }

  g_free (ucs4);

  return g_list_reverse (matches);
}

static CairoImageOutputDev *
poppler_page_get_image_output_dev (PopplerPage *page,
				   GBool (*imgDrawDeviceCbk)(int img_id, void *data),
				   void *imgDrawCbkData)
{
  CairoImageOutputDev *image_dev;
  Gfx *gfx;
  
  image_dev = new CairoImageOutputDev ();

  if (imgDrawDeviceCbk) {
    image_dev->setImageDrawDecideCbk (imgDrawDeviceCbk,
				      imgDrawCbkData);
  }

  gfx = page->page->createGfx(image_dev,
			      72.0, 72.0, 0,
			      gFalse, /* useMediaBox */
			      gTrue, /* Crop */
			      -1, -1, -1, -1,
			      gFalse, /* printing */
			      page->document->doc->getCatalog (),
			      NULL, NULL, NULL, NULL);
  page->page->display(gfx);
  delete gfx;

  return image_dev;
}

/**
 * poppler_page_get_image_mapping:
 * @page: A #PopplerPage
 *
 * Returns a list of #PopplerImageMapping items that map from a
 * location on @page to an image of the page. This list must be freed
 * with poppler_page_free_image_mapping() when done.
 *
 * Return value: (element-type PopplerImageMapping) (transfer full): A #GList of #PopplerImageMapping
 **/
GList *
poppler_page_get_image_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  CairoImageOutputDev *out;
  gint i;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  out = poppler_page_get_image_output_dev (page, NULL, NULL);

  for (i = 0; i < out->getNumImages (); i++) {
    PopplerImageMapping *mapping;
    CairoImage *image;

    image = out->getImage (i);

    /* Create the mapping */
    mapping = poppler_image_mapping_new ();

    image->getRect (&(mapping->area.x1), &(mapping->area.y1),
		    &(mapping->area.x2), &(mapping->area.y2));
    mapping->image_id = i;
    
    mapping->area.x1 -= page->page->getCropBox()->x1;
    mapping->area.x2 -= page->page->getCropBox()->x1;
    mapping->area.y1 -= page->page->getCropBox()->y1;
    mapping->area.y2 -= page->page->getCropBox()->y1;

    map_list = g_list_prepend (map_list, mapping);
  }

  delete out;

  return map_list;	
}

static GBool
image_draw_decide_cb (int image_id, void *data)
{
  return (image_id == GPOINTER_TO_INT (data));
}

/**
 * poppler_page_get_image:
 * @page: A #PopplerPage
 * @image_id: The image identificator
 *
 * Returns a cairo surface for the image of the @page
 *
 * Return value: A cairo surface for the image
 **/
cairo_surface_t *
poppler_page_get_image (PopplerPage *page,
			gint         image_id)
{
  CairoImageOutputDev *out;
  cairo_surface_t *image;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  out = poppler_page_get_image_output_dev (page,
					   image_draw_decide_cb,
					   GINT_TO_POINTER (image_id));

  if (image_id >= out->getNumImages ()) {
    delete out;
    
    return NULL;
  }

  image = out->getImage (image_id)->getImage ();
  if (!image) {
    delete out;

    return NULL;
  }

  cairo_surface_reference (image);
  delete out;
  
  return image;
}

/**
 * poppler_page_free_image_mapping:
 * @list: A list of #PopplerImageMapping<!-- -->s
 *
 * Frees a list of #PopplerImageMapping<!-- -->s allocated by
 * poppler_page_get_image_mapping().
 **/
void
poppler_page_free_image_mapping (GList *list)
{
  if (G_UNLIKELY (list == NULL))
    return;

  g_list_foreach (list, (GFunc)poppler_image_mapping_free, NULL);
  g_list_free (list);
}

/**
 * poppler_page_render_to_ps:
 * @page: a #PopplerPage
 * @ps_file: the PopplerPSFile to render to
 * 
 * Render the page on a postscript file
 * 
 **/
void
poppler_page_render_to_ps (PopplerPage   *page,
			   PopplerPSFile *ps_file)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (ps_file != NULL);

  if (!ps_file->out)
    ps_file->out = new PSOutputDev (ps_file->filename,
                                    ps_file->document->doc,
                                    ps_file->document->doc->getXRef(),
                                    ps_file->document->doc->getCatalog(),
                                    NULL,
                                    ps_file->first_page, ps_file->last_page,
                                    psModePS, (int)ps_file->paper_width,
                                    (int)ps_file->paper_height, ps_file->duplex,
                                    0, 0, 0, 0, gFalse, gFalse);


  ps_file->document->doc->displayPage (ps_file->out, page->index + 1, 72.0, 72.0,
				       0, gFalse, gTrue, gFalse);
}

static void
poppler_page_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
  PopplerPage *page = POPPLER_PAGE (object);

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_take_string (value, poppler_page_get_label (page));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
poppler_page_class_init (PopplerPageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_page_finalize;
  gobject_class->get_property = poppler_page_get_property;

  /**
   * PopplerPage:label:
   *
   * The label of the page or %NULL. See also poppler_page_get_label()
   */
  g_object_class_install_property (G_OBJECT_CLASS (klass),
				   PROP_LABEL,
				   g_param_spec_string ("label",
							"Page Label",
							"The label of the page",
							NULL,
							G_PARAM_READABLE));
}

static void
poppler_page_init (PopplerPage *page)
{
}

/**
 * poppler_page_get_link_mapping:
 * @page: A #PopplerPage
 * 
 * Returns a list of #PopplerLinkMapping items that map from a
 * location on @page to a #PopplerAction.  This list must be freed
 * with poppler_page_free_link_mapping() when done.
 * 
 * Return value: (element-type PopplerLinkMapping) (transfer full): A #GList of #PopplerLinkMapping
 **/
GList *
poppler_page_get_link_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  gint i;
  Links *links;
  Object obj;
  double width, height;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);
  
  links = new Links (page->page->getAnnots (&obj),
		     page->document->doc->getCatalog ()->getBaseURI ());
  obj.free ();
  
  if (links == NULL)
    return NULL;
  
  poppler_page_get_size (page, &width, &height);
  
  for (i = 0; i < links->getNumLinks (); i++)
    {
      PopplerLinkMapping *mapping;
      PopplerRectangle rect;
      LinkAction *link_action;
      Link *link;
      
      link = links->getLink (i);
      link_action = link->getAction ();
      
      /* Create the mapping */
      mapping = poppler_link_mapping_new ();
      mapping->action = _poppler_action_new (page->document, link_action, NULL);

      link->getRect (&rect.x1, &rect.y1, &rect.x2, &rect.y2);

      rect.x1 -= page->page->getCropBox()->x1;
      rect.x2 -= page->page->getCropBox()->x1;
      rect.y1 -= page->page->getCropBox()->y1;
      rect.y2 -= page->page->getCropBox()->y1;
      
      switch (page->page->getRotate ())
        {
        case 90:
	  mapping->area.x1 = rect.y1;
	  mapping->area.y1 = height - rect.x2;
	  mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
	  mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
	  
	  break;
	case 180:
	  mapping->area.x1 = width - rect.x2;
	  mapping->area.y1 = height - rect.y2;
	  mapping->area.x2 = mapping->area.x1 + (rect.x2 - rect.x1);
	  mapping->area.y2 = mapping->area.y1 + (rect.y2 - rect.y1);
	  
	  break;
	case 270:
	  mapping->area.x1 = width - rect.y2;
	  mapping->area.y1 = rect.x1;
	  mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
	  mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
	  
	  break;
	default:
	  mapping->area.x1 = rect.x1;
	  mapping->area.y1 = rect.y1;
	  mapping->area.x2 = rect.x2;
	  mapping->area.y2 = rect.y2;
	}

      map_list = g_list_prepend (map_list, mapping);
    }
  
  delete links;
  
  return map_list;
}

/**
 * poppler_page_free_link_mapping:
 * @list: A list of #PopplerLinkMapping<!-- -->s
 * 
 * Frees a list of #PopplerLinkMapping<!-- -->s allocated by
 * poppler_page_get_link_mapping().  It also frees the #PopplerAction<!-- -->s
 * that each mapping contains, so if you want to keep them around, you need to
 * copy them with poppler_action_copy().
 **/
void
poppler_page_free_link_mapping (GList *list)
{
  if (G_UNLIKELY (list == NULL))
    return;

  g_list_foreach (list, (GFunc)poppler_link_mapping_free, NULL);
  g_list_free (list);
}

/**
 * poppler_page_get_form_field_mapping:
 * @page: A #PopplerPage
 *
 * Returns a list of #PopplerFormFieldMapping items that map from a
 * location on @page to a form field.  This list must be freed
 * with poppler_page_free_form_field_mapping() when done.
 *
 * Return value: (element-type PopplerFormFieldMapping) (transfer full): A #GList of #PopplerFormFieldMapping
 **/
GList *
poppler_page_get_form_field_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  FormPageWidgets *forms;
  gint i;
  
  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  forms = page->page->getPageWidgets ();
  if (forms == NULL)
    return NULL;
  
  for (i = 0; i < forms->getNumWidgets (); i++) {
    PopplerFormFieldMapping *mapping;
    FormWidget *field;

    mapping = poppler_form_field_mapping_new ();
    
    field = forms->getWidget (i);

    mapping->field = _poppler_form_field_new (page->document, field);
    field->getRect (&(mapping->area.x1), &(mapping->area.y1),
		    &(mapping->area.x2), &(mapping->area.y2));

    mapping->area.x1 -= page->page->getCropBox()->x1;
    mapping->area.x2 -= page->page->getCropBox()->x1;
    mapping->area.y1 -= page->page->getCropBox()->y1;
    mapping->area.y2 -= page->page->getCropBox()->y1;
    
    map_list = g_list_prepend (map_list, mapping);
  }
  
  return map_list;
}

/**
 * poppler_page_free_form_field_mapping:
 * @list: A list of #PopplerFormFieldMapping<!-- -->s
 *
 * Frees a list of #PopplerFormFieldMapping<!-- -->s allocated by
 * poppler_page_get_form_field_mapping().
 **/
void
poppler_page_free_form_field_mapping (GList *list)
{
  if (G_UNLIKELY (list == NULL))
    return;

  g_list_foreach (list, (GFunc) poppler_form_field_mapping_free, NULL);
  g_list_free (list);
}

/**
 * poppler_page_get_annot_mapping:
 * @page: A #PopplerPage
 *
 * Returns a list of #PopplerAnnotMapping items that map from a location on
 * @page to a #PopplerAnnot.  This list must be freed with
 * poppler_page_free_annot_mapping() when done.
 *
 * Return value: (element-type PopplerAnnotMapping) (transfer full): A #GList of #PopplerAnnotMapping
 **/
GList *
poppler_page_get_annot_mapping (PopplerPage *page)
{
  GList *map_list = NULL;
  double width, height;
  gint i;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), NULL);

  if (!page->annots)
    page->annots = page->page->getAnnots (page->document->doc->getCatalog ());
  
  if (!page->annots)
    return NULL;

  poppler_page_get_size (page, &width, &height);

  for (i = 0; i < page->annots->getNumAnnots (); i++) {
    PopplerAnnotMapping *mapping;
    PopplerRectangle rect;
    Annot *annot;
    PDFRectangle *annot_rect;
    gint rotation = 0;

    annot = page->annots->getAnnot (i);

    /* Create the mapping */
    mapping = poppler_annot_mapping_new ();

    switch (annot->getType ())
      {
      case Annot::typeText:
        mapping->annot = _poppler_annot_text_new (annot);
	break;
      case Annot::typeFreeText:
        mapping->annot = _poppler_annot_free_text_new (annot);
	break;
      case Annot::typeFileAttachment:
        mapping->annot = _poppler_annot_file_attachment_new (annot);
	break;
      case Annot::typeMovie:
        mapping->annot = _poppler_annot_movie_new (annot);
	break;
      case Annot::typeScreen:
        mapping->annot = _poppler_annot_screen_new (annot);
	break;
      default:
        mapping->annot = _poppler_annot_new (annot);
	break;
      }

    annot_rect = annot->getRect ();
    rect.x1 = annot_rect->x1 - page->page->getCropBox()->x1;
    rect.y1 = annot_rect->y1 - page->page->getCropBox()->y1;
    rect.x2 = annot_rect->x2 - page->page->getCropBox()->x1;
    rect.y2 = annot_rect->y2 - page->page->getCropBox()->y1;

    if (! (annot->getFlags () & Annot::flagNoRotate))
      rotation = page->page->getRotate ();

    switch (rotation)
      {
      case 90:
        mapping->area.x1 = rect.y1;
        mapping->area.y1 = height - rect.x2;
        mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
        mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
        break;
      case 180:
        mapping->area.x1 = width - rect.x2;
        mapping->area.y1 = height - rect.y2;
        mapping->area.x2 = mapping->area.x1 + (rect.x2 - rect.x1);
        mapping->area.y2 = mapping->area.y1 + (rect.y2 - rect.y1);
        break;
      case 270:
        mapping->area.x1 = width - rect.y2;
        mapping->area.y1 = rect.x1;
        mapping->area.x2 = mapping->area.x1 + (rect.y2 - rect.y1);
        mapping->area.y2 = mapping->area.y1 + (rect.x2 - rect.x1);
        break;
      default:
        mapping->area.x1 = rect.x1;
        mapping->area.y1 = rect.y1;
        mapping->area.x2 = rect.x2;
        mapping->area.y2 = rect.y2;
      }

    map_list = g_list_prepend (map_list, mapping);
  }

  return g_list_reverse (map_list);
}

/**
 * poppler_page_free_annot_mapping:
 * @list: A list of #PopplerAnnotMapping<!-- -->s
 *
 * Frees a list of #PopplerAnnotMapping<!-- -->s allocated by
 * poppler_page_get_annot_mapping().  It also frees the #PopplerAnnot<!-- -->s
 * that each mapping contains, so if you want to keep them around, you need to
 * copy them with poppler_annot_copy().
 **/
void
poppler_page_free_annot_mapping (GList *list)
{
  if (G_UNLIKELY (!list))
    return;

  g_list_foreach (list, (GFunc)poppler_annot_mapping_free, NULL);
  g_list_free (list);
}

/**
 * poppler_page_add_annot:
 * @page: a #PopplerPage
 * @annot: a #PopplerAnnot to add
 *
 * Adds annotation @annot to @page.
 *
 * Since: 0.16
 */
void
poppler_page_add_annot (PopplerPage  *page,
			PopplerAnnot *annot)
{
  g_return_if_fail (POPPLER_IS_PAGE (page));
  g_return_if_fail (POPPLER_IS_ANNOT (annot));

  page->page->addAnnot (annot->annot);
}

/* PopplerRectangle type */

POPPLER_DEFINE_BOXED_TYPE (PopplerRectangle, poppler_rectangle,
			   poppler_rectangle_copy,
			   poppler_rectangle_free)

/**
 * poppler_rectangle_new:
 *
 * Creates a new #PopplerRectangle
 *
 * Returns: a new #PopplerRectangle, use poppler_rectangle_free() to free it
 */
PopplerRectangle *
poppler_rectangle_new (void)
{
  return g_slice_new0 (PopplerRectangle);
}

/**
 * poppler_rectangle_copy:
 * @rectangle: a #PopplerRectangle to copy
 *
 * Creates a copy of @rectangle
 *
 * Returns: a new allocated copy of @rectangle
 */
PopplerRectangle *
poppler_rectangle_copy (PopplerRectangle *rectangle)
{
  g_return_val_if_fail (rectangle != NULL, NULL);

  return g_slice_dup (PopplerRectangle, rectangle);
}

/**
 * poppler_rectangle_free:
 * @rectangle: a #PopplerRectangle
 *
 * Frees the given #PopplerRectangle
 */
void
poppler_rectangle_free (PopplerRectangle *rectangle)
{
  g_slice_free (PopplerRectangle, rectangle);
}

/* PopplerColor type */
POPPLER_DEFINE_BOXED_TYPE (PopplerColor, poppler_color, poppler_color_copy, poppler_color_free)

/**
 * poppler_color_new:
 *
 * Creates a new #PopplerColor
 *
 * Returns: a new #PopplerColor, use poppler_color_free() to free it
 */
PopplerColor *
poppler_color_new (void)
{
  return (PopplerColor *) g_new0 (PopplerColor, 1);
}

/**
 * poppler_color_copy:
 * @color: a #PopplerColor to copy
 *
 * Creates a copy of @color
 *
 * Returns: a new allocated copy of @color
 */
PopplerColor *
poppler_color_copy (PopplerColor *color)
{
  PopplerColor *new_color;

  new_color = g_new (PopplerColor, 1);
  *new_color = *color;

  return new_color;
}

/**
 * poppler_color_free:
 * @color: a #PopplerColor
 *
 * Frees the given #PopplerColor
 */
void
poppler_color_free (PopplerColor *color)
{
  g_free (color);
}

/* PopplerLinkMapping type */
POPPLER_DEFINE_BOXED_TYPE (PopplerLinkMapping, poppler_link_mapping,
			   poppler_link_mapping_copy,
			   poppler_link_mapping_free)

/**
 * poppler_link_mapping_new:
 *
 * Creates a new #PopplerLinkMapping
 *
 * Returns: a new #PopplerLinkMapping, use poppler_link_mapping_free() to free it
 */
PopplerLinkMapping *
poppler_link_mapping_new (void)
{
  return g_slice_new0 (PopplerLinkMapping);
}

/**
 * poppler_link_mapping_copy:
 * @mapping: a #PopplerLinkMapping to copy
 *
 * Creates a copy of @mapping
 *
 * Returns: a new allocated copy of @mapping
 */
PopplerLinkMapping *
poppler_link_mapping_copy (PopplerLinkMapping *mapping)
{
  PopplerLinkMapping *new_mapping;

  new_mapping = g_slice_dup (PopplerLinkMapping, mapping);

  if (new_mapping->action)
    new_mapping->action = poppler_action_copy (new_mapping->action);

  return new_mapping;
}

/**
 * poppler_link_mapping_free:
 * @mapping: a #PopplerLinkMapping
 *
 * Frees the given #PopplerLinkMapping
 */
void
poppler_link_mapping_free (PopplerLinkMapping *mapping)
{
  if (G_UNLIKELY (!mapping))
    return;

  if (mapping->action)
    poppler_action_free (mapping->action);

  g_slice_free (PopplerLinkMapping, mapping);
}

/* Poppler Image mapping type */
POPPLER_DEFINE_BOXED_TYPE (PopplerImageMapping, poppler_image_mapping,
			   poppler_image_mapping_copy,
			   poppler_image_mapping_free)

/**
 * poppler_image_mapping_new:
 *
 * Creates a new #PopplerImageMapping
 *
 * Returns: a new #PopplerImageMapping, use poppler_image_mapping_free() to free it
 */
PopplerImageMapping *
poppler_image_mapping_new (void)
{
  return g_slice_new0 (PopplerImageMapping);
}

/**
 * poppler_image_mapping_copy:
 * @mapping: a #PopplerImageMapping to copy
 *
 * Creates a copy of @mapping
 *
 * Returns: a new allocated copy of @mapping
 */
PopplerImageMapping *
poppler_image_mapping_copy (PopplerImageMapping *mapping)
{
  return g_slice_dup (PopplerImageMapping, mapping);
}

/**
 * poppler_image_mapping_free:
 * @mapping: a #PopplerImageMapping
 *
 * Frees the given #PopplerImageMapping
 */
void
poppler_image_mapping_free (PopplerImageMapping *mapping)
{
  g_slice_free (PopplerImageMapping, mapping);
}

/* Page Transition */
POPPLER_DEFINE_BOXED_TYPE (PopplerPageTransition, poppler_page_transition,
			   poppler_page_transition_copy,
			   poppler_page_transition_free)

/**
 * poppler_page_transition_new:
 *
 * Creates a new #PopplerPageTransition
 *
 * Returns: a new #PopplerPageTransition, use poppler_page_transition_free() to free it
 */
PopplerPageTransition *
poppler_page_transition_new (void)
{
  return (PopplerPageTransition *) g_new0 (PopplerPageTransition, 1);
}

/**
 * poppler_page_transition_copy:
 * @transition: a #PopplerPageTransition to copy
 *
 * Creates a copy of @transition
 *
 * Returns: a new allocated copy of @transition
 */
PopplerPageTransition *
poppler_page_transition_copy (PopplerPageTransition *transition)
{
  PopplerPageTransition *new_transition;

  new_transition = poppler_page_transition_new ();
  *new_transition = *transition;
  
  return new_transition;
}

/**
 * poppler_page_transition_free:
 * @transition: a #PopplerPageTransition
 *
 * Frees the given #PopplerPageTransition
 */
void
poppler_page_transition_free (PopplerPageTransition *transition)
{
  g_free (transition);
}

/* Form Field Mapping Type */
POPPLER_DEFINE_BOXED_TYPE (PopplerFormFieldMapping, poppler_form_field_mapping,
			   poppler_form_field_mapping_copy,
			   poppler_form_field_mapping_free)

/**
 * poppler_form_field_mapping_new:
 *
 * Creates a new #PopplerFormFieldMapping
 *
 * Returns: a new #PopplerFormFieldMapping, use poppler_form_field_mapping_free() to free it
 */
PopplerFormFieldMapping *
poppler_form_field_mapping_new (void)
{
  return g_slice_new0 (PopplerFormFieldMapping);
}

/**
 * poppler_form_field_mapping_copy:
 * @mapping: a #PopplerFormFieldMapping to copy
 *
 * Creates a copy of @mapping
 *
 * Returns: a new allocated copy of @mapping
 */
PopplerFormFieldMapping *
poppler_form_field_mapping_copy (PopplerFormFieldMapping *mapping)
{
  PopplerFormFieldMapping *new_mapping;

  new_mapping = g_slice_dup (PopplerFormFieldMapping, mapping);

  if (mapping->field)
	  new_mapping->field = (PopplerFormField *)g_object_ref (mapping->field);
	    
  return new_mapping;
}

/**
 * poppler_form_field_mapping_free:
 * @mapping: a #PopplerFormFieldMapping
 *
 * Frees the given #PopplerFormFieldMapping
 */
void
poppler_form_field_mapping_free (PopplerFormFieldMapping *mapping)
{
  if (G_UNLIKELY (!mapping))
    return;

  if (mapping->field)
    g_object_unref (mapping->field);

  g_slice_free (PopplerFormFieldMapping, mapping);
}

/* PopplerAnnot Mapping Type */
POPPLER_DEFINE_BOXED_TYPE (PopplerAnnotMapping, poppler_annot_mapping,
			   poppler_annot_mapping_copy,
			   poppler_annot_mapping_free)

/**
 * poppler_annot_mapping_new:
 *
 * Creates a new #PopplerAnnotMapping
 *
 * Returns: a new #PopplerAnnotMapping, use poppler_annot_mapping_free() to free it
 */
PopplerAnnotMapping *
poppler_annot_mapping_new (void)
{
  return g_slice_new0 (PopplerAnnotMapping);
}

/**
 * poppler_annot_mapping_copy:
 * @mapping: a #PopplerAnnotMapping to copy
 *
 * Creates a copy of @mapping
 *
 * Returns: a new allocated copy of @mapping
 */
PopplerAnnotMapping *
poppler_annot_mapping_copy (PopplerAnnotMapping *mapping)
{
  PopplerAnnotMapping *new_mapping;

  new_mapping = g_slice_dup (PopplerAnnotMapping, mapping);

  if (mapping->annot)
    new_mapping->annot = (PopplerAnnot *) g_object_ref (mapping->annot);

  return new_mapping;
}

/**
 * poppler_annot_mapping_free:
 * @mapping: a #PopplerAnnotMapping
 *
 * Frees the given #PopplerAnnotMapping
 */
void
poppler_annot_mapping_free (PopplerAnnotMapping *mapping)
{
  if (G_UNLIKELY (!mapping))
    return;

  if (mapping->annot)
    g_object_unref (mapping->annot);

  g_slice_free (PopplerAnnotMapping, mapping);
}

/**
 * poppler_page_get_crop_box:
 * @page: a #PopplerPage
 * @rect: (out): a #PopplerRectangle to fill
 *
 * Retrurns the crop box of @page
 */
void
poppler_page_get_crop_box (PopplerPage *page, PopplerRectangle *rect)
{
  PDFRectangle* cropBox = page->page->getCropBox ();
  
  rect->x1 = cropBox->x1;
  rect->x2 = cropBox->x2;
  rect->y1 = cropBox->y1;
  rect->y2 = cropBox->y2;
}

/**
 * poppler_page_get_text_layout:
 * @page: A #PopplerPage
 * @rectangles: (out) (array length=n_rectangles) (transfer container): return location for an array of #PopplerRectangle
 * @n_rectangles: (out) length of returned array
 *
 * Obtains the layout of the text as a list of #PopplerRectangle
 * This array must be freed with g_free () when done.
 *
 * The position in the array represents an offset in the text returned by
 * poppler_page_get_text()
 *
 * Return value: %TRUE if the page contains text, %FALSE otherwise
 *
 * Since: 0.16
 **/
gboolean
poppler_page_get_text_layout (PopplerPage       *page,
                              PopplerRectangle **rectangles,
                              guint             *n_rectangles)
{
  TextPage *text;
  TextWordList *wordlist;
  TextWord *word, *nextword;
  PopplerRectangle *rect;
  int i, j, offset = 0;
  gdouble x1, y1, x2, y2;
  gdouble x3, y3, x4, y4;

  g_return_val_if_fail (POPPLER_IS_PAGE (page), FALSE);

  *n_rectangles = 0;

  text = poppler_page_get_text_page (page);
  wordlist = text->makeWordList (gFalse);

  if (wordlist->getLength () <= 0)
    {
      delete wordlist;
      return FALSE;
    }

  // Getting the array size
  for (i = 0; i < wordlist->getLength (); i++)
    {
      word = wordlist->get (i);
      *n_rectangles += word->getLength () + 1;
    }

  *rectangles = g_new (PopplerRectangle, *n_rectangles);

  // Calculating each char position
  for (i = 0; i < wordlist->getLength (); i++)
    {
      word = wordlist->get (i);
      for (j = 0; j < word->getLength (); j++)
        {
          rect = *rectangles + offset;
	  word->getCharBBox (j,
			     &(rect->x1),
			     &(rect->y1),
			     &(rect->x2),
			     &(rect->y2));
	  offset++;
	}

      // adding spaces and break lines
      rect = *rectangles + offset;
      word->getBBox (&x1, &y1, &x2, &y2);

      nextword = word->getNext ();
      if (nextword)
        {
	  nextword->getBBox (&x3, &y3, &x4, &y4);
	  // space is from one word to other and with the same height as
	  // first word.
	  rect->x1 = x2;
	  rect->y1 = y1;
	  rect->x2 = x3;
	  rect->y2 = y2;
	}
      else
        {
	  // end of line
	  rect->x1 = x2;
	  rect->y1 = y2;
	  rect->x2 = x2;
	  rect->y2 = y2;
	}
      offset++;
    }

  delete wordlist;

  return TRUE;
}
