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
#include <goo/GooHash.h>
#include <goo/GooTimer.h>
#include <splash/SplashTypes.h>
#include <splash/SplashBitmap.h>
#include "Object.h"
#include "ProfileData.h"
#include "GfxState.h"

#include <gdk/gdk.h>
#include "CairoOutputDev.h"

#include "PDFDoc.h"
#include "GlobalParams.h"
#include "ErrorCodes.h"
#include <gtk/gtk.h>


// Mapping
#include "pdf-operators.c"

enum {
  OP_STRING,
  OP_COUNT,
  OP_TOTAL,
  OP_MIN,
  OP_MAX,
  N_COLUMNS
};

class PdfInspector {
public:

  PdfInspector(void);

  void set_file_name (const char *file_name);
  void load (const char *file_name);
  void run (void);
  void error_dialog (const char *error_message);
  void analyze_page (int page);

private:
  static void on_file_activated (GtkWidget *widget, PdfInspector *inspector);
  static void on_selection_changed (GtkTreeSelection *selection, PdfInspector *inspector);
  static void on_analyze_clicked (GtkWidget *widget, PdfInspector *inspector);

  GtkBuilder* builder;
  GtkTreeModel *model;
  PDFDoc *doc;
  CairoOutputDev *output;
};



PdfInspector::PdfInspector(void)
{
  GtkWidget *widget;
  GError* error = NULL;
  
  builder = gtk_builder_new ();

  if (!gtk_builder_add_from_file (builder, "./pdf-inspector.ui", &error))
  {
    g_warning ("Couldn't load builder file: %s", error->message);
    g_error_free (error);
  }

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "pdf_file_chooser_button"));
  g_signal_connect (widget, "selection-changed", G_CALLBACK (on_file_activated), this);

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "analyze_button"));
  g_signal_connect (widget, "clicked", G_CALLBACK (on_analyze_clicked), this);

  // setup the TreeView 
  widget = GTK_WIDGET (gtk_builder_get_object (builder, "pdf_tree_view"));
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)),
		    "changed", G_CALLBACK (on_selection_changed), this);
  model = (GtkTreeModel *)gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_INT,
					      G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (widget), model);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (widget),
					       0, "Operation", 
					       gtk_cell_renderer_text_new (),
					       "text", OP_STRING,
					       NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (widget),
					       1, "Count", 
					       gtk_cell_renderer_text_new (),
					       "text", OP_COUNT,
					       NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (widget),
					       2, "Elapsed", 
					       gtk_cell_renderer_text_new (),
					       "text", OP_TOTAL,
					       NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (widget),
					       3, "Min", 
					       gtk_cell_renderer_text_new (),
					       "text", OP_MIN,
					       NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (widget),
					       4, "Max", 
					       gtk_cell_renderer_text_new (),
					       "text", OP_MAX,
					       NULL);

  for (int i = 0; i < N_COLUMNS; i++)
    {
      GtkTreeViewColumn *column;
      
      column = gtk_tree_view_get_column (GTK_TREE_VIEW (widget), i);
      gtk_tree_view_column_set_sort_column_id (column, i);
    }
  doc = NULL;
  output = new CairoOutputDev();
  output->setPrinting (gFalse);

  // set up initial widgets
  load (NULL);
}
    
void
PdfInspector::set_file_name(const char *file_name)
{
  GtkWidget *widget;

  widget = GTK_WIDGET (gtk_builder_get_object (builder, "pdf_file_chooser_button"));
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (widget), file_name);
}

void
PdfInspector::on_file_activated (GtkWidget *widget, PdfInspector *inspector)
{
  gchar *file_name;

  file_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
  if (file_name)
    inspector->load (file_name);

  g_free (file_name);
}

void
PdfInspector::on_selection_changed (GtkTreeSelection *selection, PdfInspector *inspector)
{
  GtkWidget *label;
  size_t i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *op = NULL;

  label = GTK_WIDGET (gtk_builder_get_object (inspector->builder, "description_label"));
  gtk_label_set_markup (GTK_LABEL (label), "<i>No Description</i>");

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter,
			  OP_STRING, &op,
			  -1);

    }

  if (op == NULL)
    return;

  for (i = 0; i < G_N_ELEMENTS (op_mapping); i++)
    {

      if (!strcmp (op, op_mapping[i].op))
	{
	  gchar *text;
	  text = g_strdup_printf ("<i>%s</i>", op_mapping[i].description);
	  gtk_label_set_markup (GTK_LABEL (label), text);
	  g_free (text);
	  break;
	}
    }

  g_free (op);  
}

void
PdfInspector::on_analyze_clicked (GtkWidget *widget, PdfInspector *inspector)
{
  GtkWidget *spin;
  int page;

  spin = GTK_WIDGET (gtk_builder_get_object (inspector->builder, "pdf_spin"));

  page = (int) gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin));

  inspector->analyze_page (page);

}

void
PdfInspector::analyze_page (int page)
{
  GooHashIter *iter;
  GooHash *hash;
  GooString *key;
  void *p;
  GtkWidget *label;
  char *text;
  cairo_t *cr;
  cairo_surface_t *surface;

  label = GTK_WIDGET (gtk_builder_get_object (builder, "pdf_total_label"));

  output->startProfile ();
  gtk_list_store_clear (GTK_LIST_STORE (model));

  GooTimer timer;
  surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
					doc->getPageCropWidth(page + 1),
					doc->getPageCropHeight(page + 1));
  cr = cairo_create (surface);
  cairo_surface_destroy (surface);
  output->setCairo (cr);
  cairo_destroy (cr);
  doc->displayPage (output, page + 1, 72, 72, 0, gFalse, gTrue, gFalse);
  output->setCairo (NULL);

  // Total time;
  text = g_strdup_printf ("%g", timer.getElapsed ());
  gtk_label_set_text (GTK_LABEL (label), text);
  g_free (text);

  // Individual times;
  hash = output->endProfile ();
  hash->startIter(&iter);
  while (hash->getNext(&iter, &key, &p))
    {
      GtkTreeIter tree_iter;
      ProfileData *data_p = (ProfileData *) p;

      gtk_list_store_append (GTK_LIST_STORE (model), &tree_iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &tree_iter,
			  OP_STRING, key->getCString(),
			  OP_COUNT, data_p->getCount (),
			  OP_TOTAL, data_p->getTotal (),
			  OP_MIN, data_p->getMin (),
			  OP_MAX, data_p->getMax (),
			  -1);
    }
  hash->killIter(&iter);
  deleteGooHash (hash, ProfileData);
}
 
void
PdfInspector::load(const char *file_name)
{
  GtkWidget *spin;
  GtkWidget *button;
  GtkWidget *label;

  // kill the old PDF file
  if (doc != NULL)
    {
      delete doc;
      doc = NULL;
    }

  // load the new file
  if (file_name)
    {
      GooString *filename_g;

      filename_g = new GooString (file_name);
      doc = new PDFDoc(filename_g, 0, 0);
      delete filename_g;
    }
  
  if (doc && !doc->isOk())
    {
      this->error_dialog ("Failed to load file.");
      delete doc;
      doc = NULL;
    }

  spin = GTK_WIDGET (gtk_builder_get_object (builder, "pdf_spin"));
  button = GTK_WIDGET (gtk_builder_get_object (builder, "analyze_button"));
  label = GTK_WIDGET (gtk_builder_get_object (builder, "pdf_total_label"));
  gtk_label_set_text (GTK_LABEL (label), "");

  if (doc)
    {
      gtk_widget_set_sensitive (spin, TRUE);
      gtk_widget_set_sensitive (button, TRUE);
      gtk_widget_set_sensitive (label, TRUE);
      gtk_spin_button_set_range (GTK_SPIN_BUTTON (spin), 0, doc->getNumPages()-1);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 0);

      output->startDoc (doc->getXRef(), doc->getCatalog());
    }
  else
    {      
      gtk_widget_set_sensitive (spin, FALSE);
      gtk_widget_set_sensitive (button, FALSE);
      gtk_widget_set_sensitive (label, FALSE);
    }
}

void
PdfInspector::error_dialog (const char *error_message)
{
  g_warning ("%s", error_message);
}

void
PdfInspector::run()
{
  GtkWidget *dialog;

  dialog = GTK_WIDGET (gtk_builder_get_object (builder, "pdf_dialog"));

  gtk_dialog_run (GTK_DIALOG (dialog));
}



int
main (int argc, char *argv [])
{
  const char *file_name = NULL;
  PdfInspector *inspector;
  
  gtk_init (&argc, &argv);
  
  globalParams = new GlobalParams();
  globalParams->setProfileCommands (true);
  globalParams->setPrintCommands (true);

  if (argc == 2)
    file_name = argv[1];
  else if (argc > 2)
    {
      fprintf (stderr, "usage: %s [PDF-FILE]\n", argv[0]);
      return -1;
    }

  inspector = new PdfInspector ();

  if (file_name)
    inspector->set_file_name (file_name);

  inspector->run ();

  delete inspector;
  delete globalParams;
  
  return 0;
}


