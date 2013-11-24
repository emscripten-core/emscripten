#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <goo/gmem.h>
#include <splash/SplashTypes.h>
#include <splash/SplashBitmap.h>
#include "Object.h"
#include "SplashOutputDev.h"
#include "GfxState.h"

#include <gdk/gdk.h>

#include "PDFDoc.h"
#include "GlobalParams.h"
#include "ErrorCodes.h"
#include <poppler.h>
#include <poppler-private.h>
#include <gtk/gtk.h>
#include <math.h>

static int page = 0;
static gboolean cairo_output = FALSE;
static gboolean splash_output = FALSE;
static const char **file_arguments = NULL;
static const GOptionEntry options[] = {
  { "cairo", 'c', 0, G_OPTION_ARG_NONE, &cairo_output, "Cairo Output Device", NULL},
  { "splash", 's', 0, G_OPTION_ARG_NONE, &splash_output, "Splash Output Device", NULL},
  { "page", 'p', 0, G_OPTION_ARG_INT, &page, "Page number", "PAGE" },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &file_arguments, NULL, "PDF-FILES…" },
  { NULL }
};

static GList *view_list = NULL;

//------------------------------------------------------------------------

#define xOutMaxRGBCube 6	// max size of RGB color cube

//------------------------------------------------------------------------
// GDKSplashOutputDev
//------------------------------------------------------------------------

class GDKSplashOutputDev: public SplashOutputDev {
public:

  GDKSplashOutputDev(GdkScreen *screen,
                     void (*redrawCbkA)(void *data),
                     void *redrawCbkDataA, SplashColor sc);
  
  virtual ~GDKSplashOutputDev();

  //----- initialization and control

  // End a page.
  virtual void endPage();

  // Dump page contents to display.
  virtual void dump();

  //----- update text state
  virtual void updateFont(GfxState *state);

  //----- special access

  // Clear out the document (used when displaying an empty window).
  void clear();

  // Copy the rectangle (srcX, srcY, width, height) to (destX, destY)
  // in destDC.
  void redraw(int srcX, int srcY,
              cairo_t *cr,
	      int destX, int destY,
	      int width, int height);

private:

  int incrementalUpdate;
  void (*redrawCbk)(void *data);
  void *redrawCbkData;
};

typedef struct
{
  PopplerDocument    *doc;
  GtkWidget          *drawing_area;
  GtkWidget          *spin_button;
  cairo_surface_t    *surface;
  GDKSplashOutputDev *out;
} View;

//------------------------------------------------------------------------
// Constants and macros
//------------------------------------------------------------------------

#define xoutRound(x) ((int)(x + 0.5))

//------------------------------------------------------------------------
// GDKSplashOutputDev
//------------------------------------------------------------------------

GDKSplashOutputDev::GDKSplashOutputDev(GdkScreen *screen,
                                       void (*redrawCbkA)(void *data),
                                       void *redrawCbkDataA, SplashColor sc):
  SplashOutputDev(splashModeRGB8, 4, gFalse, sc),
  incrementalUpdate (1)
{
  redrawCbk = redrawCbkA;
  redrawCbkData = redrawCbkDataA;
}

GDKSplashOutputDev::~GDKSplashOutputDev() {
}

void GDKSplashOutputDev::clear() {
  startDoc(NULL);
  startPage(0, NULL, NULL);
}

void GDKSplashOutputDev::endPage() {
  SplashOutputDev::endPage();
  if (!incrementalUpdate) {
    (*redrawCbk)(redrawCbkData);
  }
}

void GDKSplashOutputDev::dump() {
  if (incrementalUpdate && redrawCbk) {
    (*redrawCbk)(redrawCbkData);
  }
}

void GDKSplashOutputDev::updateFont(GfxState *state) {
  SplashOutputDev::updateFont(state);
}

void GDKSplashOutputDev::redraw(int srcX, int srcY,
                                cairo_t *cr,
                                int destX, int destY,
                                int width, int height) {
  GdkPixbuf *pixbuf;
  int gdk_rowstride;

  gdk_rowstride = getBitmap()->getRowSize();
  pixbuf = gdk_pixbuf_new_from_data (getBitmap()->getDataPtr() + srcY * gdk_rowstride + srcX * 3,
                                     GDK_COLORSPACE_RGB, FALSE, 8,
                                     width, height, gdk_rowstride,
                                     NULL, NULL);

  gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
  cairo_paint (cr);

  g_object_unref (pixbuf);
}

static gboolean
drawing_area_draw (GtkWidget *drawing_area,
                   cairo_t   *cr,
                   View      *view)
{
  GdkRectangle document;
  GdkRectangle clip;
  GdkRectangle draw;

  document.x = 0;
  document.y = 0;
  if (cairo_output) {
    document.width = cairo_image_surface_get_width (view->surface);
    document.height = cairo_image_surface_get_height (view->surface);
  } else {
    document.width = view->out->getBitmapWidth();
    document.height = view->out->getBitmapHeight();
  }

  if (!gdk_cairo_get_clip_rectangle (cr, &clip))
    return FALSE;

  if (!gdk_rectangle_intersect (&document, &clip, &draw))
    return FALSE;

  if (cairo_output) {
    cairo_set_source_surface (cr, view->surface, 0, 0);
    cairo_paint (cr);
  } else {
    view->out->redraw (draw.x, draw.y,
                       cr,
                       draw.x, draw.y,
                       draw.width, draw.height);
  }

  return TRUE;
}

static void
view_set_page (View *view, int page)
{
  int w, h;

  if (cairo_output) {
    cairo_t     *cr;
    double       width, height;
    PopplerPage *poppler_page;

    poppler_page = poppler_document_get_page (view->doc, page);
    poppler_page_get_size (poppler_page, &width, &height);
    w = (int) ceil(width);
    h = (int) ceil(height);

    if (view->surface)
      cairo_surface_destroy (view->surface);
    view->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);

    cr = cairo_create (view->surface);
    poppler_page_render (poppler_page, cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_DEST_OVER);
    cairo_set_source_rgb (cr, 1., 1., 1.);
    cairo_paint (cr);

    cairo_destroy (cr);
    g_object_unref (poppler_page);
  } else {
    view->doc->doc->displayPage (view->out, page + 1, 72, 72, 0, gFalse, gTrue, gTrue);
    w = view->out->getBitmapWidth();
    h = view->out->getBitmapHeight();
  }

  gtk_widget_set_size_request (view->drawing_area, w, h);
  gtk_widget_queue_draw (view->drawing_area);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (view->spin_button), page);
}

static void
redraw_callback (void *data)
{
  View *view = (View*) data;

  gtk_widget_queue_draw (view->drawing_area);
}

static void
view_free (View *view)
{
  if (G_UNLIKELY (!view))
    return;

  g_object_unref (view->doc);
  delete view->out;
  cairo_surface_destroy (view->surface);
  g_slice_free (View, view);
}

static void
destroy_window_callback (GtkWindow *window, View *view)
{
  view_list = g_list_remove (view_list, view);
  view_free (view);

  if (!view_list)
    gtk_main_quit ();
}

static void
page_changed_callback (GtkSpinButton *button, View *view)
{
  int page;

  page = gtk_spin_button_get_value_as_int (button);
  view_set_page (view, page);
}

static View *
view_new (PopplerDocument *doc)
{
  View *view;
  GtkWidget *window;
  GtkWidget *sw;
  GtkWidget *vbox, *hbox;
  guint n_pages;
  PopplerPage *page;

  view = g_slice_new0 (View);

  view->doc = doc;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy",
                    G_CALLBACK (destroy_window_callback),
                    view);

  page = poppler_document_get_page (doc, 0);
  if (page) {
    double width, height;

    poppler_page_get_size (page, &width, &height);
    gtk_window_set_default_size (GTK_WINDOW (window), (gint)width, (gint)height);
    g_object_unref (page);
  }

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);

  view->drawing_area = gtk_drawing_area_new ();
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
#if GTK_CHECK_VERSION(3, 7, 8)
  gtk_container_add(GTK_CONTAINER(sw), view->drawing_area);
#else
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw),
                                         view->drawing_area);
#endif
  gtk_widget_show (view->drawing_area);

  gtk_box_pack_end (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  gtk_widget_show (sw);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);

  n_pages = poppler_document_get_n_pages (doc);
  view->spin_button = gtk_spin_button_new_with_range  (0, n_pages - 1, 1);
  g_signal_connect (view->spin_button, "value-changed",
                    G_CALLBACK (page_changed_callback), view);

  gtk_box_pack_end (GTK_BOX (hbox), view->spin_button, FALSE, TRUE, 0);
  gtk_widget_show (view->spin_button);

  gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show (hbox);

  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  gtk_widget_show (window);


  if (!cairo_output) {
    SplashColor sc = { 255, 255, 255};

    view->out = new GDKSplashOutputDev (gtk_widget_get_screen (window),
                                        redraw_callback, (void*) view, sc);
    view->out->startDoc(view->doc->doc);
  }

  g_signal_connect (view->drawing_area,
                    "draw",
                    G_CALLBACK (drawing_area_draw),
                    view);

  return view;
}

int
main (int argc, char *argv [])
{
  GOptionContext *ctx;

  if (argc == 1) {
    char *basename = g_path_get_basename (argv[0]);

    g_printerr ("usage: %s PDF-FILES…\n", basename);
    g_free (basename);

    return -1;
  }

  ctx = g_option_context_new (NULL);
  g_option_context_add_main_entries (ctx, options, "main");
  g_option_context_parse (ctx, &argc, &argv, NULL);
  g_option_context_free (ctx);

  gtk_init (&argc, &argv);

  globalParams = new GlobalParams();

  for (int i = 0; file_arguments[i]; i++) {
    View            *view;
    GFile           *file;
    PopplerDocument *doc;
    GError          *error = NULL;

    file = g_file_new_for_commandline_arg (file_arguments[i]);
    doc = poppler_document_new_from_gfile (file, NULL, NULL, &error);
    if (!doc) {
      gchar *uri;

      uri = g_file_get_uri (file);
      g_printerr ("Error opening document %s: %s\n", uri, error->message);
      g_error_free (error);
      g_free (uri);
      g_object_unref (file);

      continue;
    }
    g_object_unref (file);

    view = view_new (doc);
    view_list = g_list_prepend (view_list, view);
    view_set_page (view, CLAMP (page, 0, poppler_document_get_n_pages (doc) - 1));
  }

  gtk_main ();

  delete globalParams;

  return 0;
}
