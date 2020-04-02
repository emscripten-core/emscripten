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

#include <math.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <poppler.h>

typedef struct
{
  GtkWidget *window;
  GtkWidget *sw;
  GtkWidget *drawing_area;
  GtkWidget *spin_button;
  PopplerPage *page;
  PopplerDocument *document;
  cairo_surface_t *surface;
  int *window_count;
} View;

static void
drawing_area_expose (GtkWidget      *drawing_area,
                     GdkEventExpose *event,
                     void           *data)
{
  View *v = (View*) data;
  cairo_t *cr;

  gdk_window_clear (drawing_area->window);
  cr = gdk_cairo_create (drawing_area->window);

  cairo_set_source_surface (cr, v->surface, 0, 0);
  cairo_paint (cr);
  cairo_destroy (cr);
}

static void
view_set_page (View *v, int page)
{
  int w, h;
  double width, height;
  cairo_t *cr;

  v->page = poppler_document_get_page (v->document, page);
  poppler_page_get_size (v->page, &width, &height);
  w = (int) ceil(width);
  h = (int) ceil(height);
  cairo_surface_destroy (v->surface);
  v->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
  cr = cairo_create (v->surface);
  poppler_page_render (v->page, cr);
  cairo_destroy (cr);
  gtk_widget_set_size_request (v->drawing_area, w, h);
  gtk_widget_queue_draw (v->drawing_area);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (v->spin_button), page);
}

static void
page_changed_callback (GtkSpinButton *button, View *v)
{
    int page;

    page = gtk_spin_button_get_value_as_int (button);
    view_set_page (v, page);
}

static void
destroy_window_callback (GtkWindow *window, View *v)
{
    if (--(*v->window_count) == 0)
	gtk_main_quit();
}

static View*
view_new (const char *filename, int *window_count)
{
  View *v;
  GtkWidget *window;
  GtkWidget *drawing_area;
  GtkWidget *sw;
  GtkWidget *vbox, *hbox;
  GtkWidget *spin_button;
  int n_pages;

  v = g_new0 (View, 1);

  v->document = poppler_document_new_from_file (filename, NULL, NULL);
  if (v->document == NULL)
      return NULL;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  vbox = gtk_vbox_new(FALSE, 5);

  gtk_container_add (GTK_CONTAINER (window), vbox);
  drawing_area = gtk_drawing_area_new ();

  sw = gtk_scrolled_window_new (NULL, NULL);

  gtk_box_pack_end (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw),
					 drawing_area);

  n_pages = poppler_document_get_n_pages (v->document);
  spin_button = gtk_spin_button_new_with_range  (0, n_pages - 1, 1);
  g_signal_connect (G_OBJECT (spin_button), "value-changed",
		    G_CALLBACK (page_changed_callback), v);
  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_end (GTK_BOX (hbox), spin_button, FALSE, TRUE, 0);

  gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);

  gtk_widget_show_all (window);
  gtk_widget_realize (window);

  v->window = window;
  v->drawing_area = drawing_area;
  v->sw = sw;
  v->window_count = window_count;
  v->spin_button = spin_button;

  g_signal_connect (drawing_area,
                    "expose_event",
                    G_CALLBACK (drawing_area_expose),
                    (void*) v);
  
  g_signal_connect (window,
                    "destroy",
                    G_CALLBACK (destroy_window_callback),
                    (void*) v);

  return v;
}

static int option_page = 0;
static GOptionEntry demo_options[] = {
  { "page", 0, 0, G_OPTION_ARG_INT, &option_page, "Page number", "PAGE" },
  { NULL }
};
    
int
main (int argc, char *argv [])
{
  View *v;
  int i, window_count;
  GOptionContext *ctx;

  ctx = g_option_context_new("FILENAME ...");
  g_option_context_add_main_entries(ctx, demo_options, "main");
  g_option_context_parse(ctx, &argc, &argv, NULL);
  g_option_context_free(ctx);

  gtk_init (&argc, &argv);
   
  if (argc == 1)
    {
      char *basename = g_path_get_basename (argv[0]);
      fprintf (stderr, "usage: %s PDF-FILES...\n", basename);
      g_free (basename);
      return -1;
    }
      
  window_count = 0;
  for (i = 1; i < argc; i++) {
      v = view_new (argv[i], &window_count);
      if (v == NULL) {
	  g_printerr ("Error loading %s\n", argv[i]);
	  continue;
      }
      
      view_set_page (v, option_page);
      window_count++;
  }
  
  if (window_count > 0)
    gtk_main ();
  
  return 0;
}
