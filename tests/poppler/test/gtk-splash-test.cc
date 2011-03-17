//========================================================================
//
// GDKSplashOutputDev.cc
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc. (GDK port)
//
//========================================================================

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
#include <gtk/gtk.h>

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
              GdkDrawable *drawable,
	      int destX, int destY,
	      int width, int height);

private:

  int incrementalUpdate;
  void (*redrawCbk)(void *data);
  void *redrawCbkData;
};

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
  startPage(0, NULL);
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
                                GdkDrawable *drawable,
                                int destX, int destY,
                                int width, int height) {
  GdkGC *gc;
  int gdk_rowstride;

  gdk_rowstride = getBitmap()->getRowSize();
  gc = gdk_gc_new (drawable);
  
  gdk_draw_rgb_image (drawable, gc,
                      destX, destY,
                      width, height,
                      GDK_RGB_DITHER_NORMAL,
                      getBitmap()->getDataPtr() + srcY * gdk_rowstride + srcX * 3,
                      gdk_rowstride);

  g_object_unref (gc);
}


typedef struct
{
  GtkWidget *window;
  GtkWidget *sw;
  GtkWidget *drawing_area;
  GDKSplashOutputDev *out;
  PDFDoc *doc;
} View;

static void
drawing_area_expose (GtkWidget      *drawing_area,
                     GdkEventExpose *event,
                     void           *data)
{
  View *v = (View*) data;
  GdkRectangle document;
  GdkRectangle draw;

  gdk_window_clear (drawing_area->window);
  
  document.x = 0;
  document.y = 0;
  document.width = v->out->getBitmapWidth();
  document.height = v->out->getBitmapHeight();

  if (gdk_rectangle_intersect (&document, &event->area, &draw))
    {
      v->out->redraw (draw.x, draw.y,
                      drawing_area->window,
                      draw.x, draw.y,
                      draw.width, draw.height);
    }
}

static int
view_load (View       *v,
           const char *filename)
{
  PDFDoc *newDoc;
  int err;
  GooString *filename_g;
  int w, h;

  filename_g = new GooString (filename);

  // open the PDF file
  newDoc = new PDFDoc(filename_g, 0, 0);

  delete filename_g;
  
  if (!newDoc->isOk())
    {
      err = newDoc->getErrorCode();
      delete newDoc;
      return err;
    }

  if (v->doc)
    delete v->doc;
  v->doc = newDoc;
  
  v->out->startDoc(v->doc->getXRef());

  v->doc->displayPage (v->out, 1, 72, 72, 0, gFalse, gTrue, gTrue);
  
  w = v->out->getBitmapWidth();
  h = v->out->getBitmapHeight();
  
  gtk_widget_set_size_request (v->drawing_area, w, h);

  return errNone;
}

static void
view_show (View *v)
{
  gtk_widget_show (v->window);
}

static void
redraw_callback (void *data)
{
  View *v = (View*) data;

  gtk_widget_queue_draw (v->drawing_area);
}

static View*
view_new (void)
{
  View *v;
  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *sw;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  drawing_area = gtk_drawing_area_new ();

  sw = gtk_scrolled_window_new (NULL, NULL);

  gtk_container_add (GTK_CONTAINER (window), sw);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), drawing_area);

  gtk_widget_show_all (sw);

  v = g_new0 (View, 1);

  v->window = window;
  v->drawing_area = drawing_area;
  v->sw = sw;
  SplashColor sc;
  sc[0] = 255;
  sc[1] = 255;
  sc[2] = 255;
  v->out = new GDKSplashOutputDev (gtk_widget_get_screen (window),
                                   redraw_callback, (void*) v, sc);
  v->doc = 0;

  g_signal_connect (drawing_area,
                    "expose_event",
                    G_CALLBACK (drawing_area_expose),
                    (void*) v);
  
  return v;
}

int
main (int argc, char *argv [])
{
  View *v;
  int i;
  
  gtk_init (&argc, &argv);
  
  globalParams = new GlobalParams();
  
  if (argc == 1)
    {
      fprintf (stderr, "usage: %s PDF-FILES...\n", argv[0]);
      return -1;
    }
      

  i = 1;
  while (i < argc)
    {
      int err;
      
      v = view_new ();

      err = view_load (v, argv[i]);

      if (err != errNone)
        g_printerr ("Error loading document!\n");
      
      view_show (v);

      ++i;
    }
  
  gtk_main ();
  
  delete globalParams;
  
  return 0;
}
