/*
 * Copyright (C) 2007 Carlos Garcia Campos  <carlosgc@gnome.org>
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

#include <gtk/gtk.h>

#include "fonts.h"

enum {
	FONTS_NAME_COLUMN,
	FONTS_DETAILS_COLUMN,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;
	
	GtkWidget       *treeview;
	GtkWidget       *progress;

	guint            idle_id;
} PgdFontsDemo;

static void
pgd_fonts_free (PgdFontsDemo *demo)
{
	if (!demo)
		return;

	if (demo->idle_id > 0) {
		g_source_remove (demo->idle_id);
		demo->idle_id = 0;
	}
	
	if (demo->doc) {
		g_object_unref (demo->doc);
		demo->doc = NULL;
	}

	g_free (demo);
}

static void
pdg_fonts_cell_data_func (GtkTreeViewColumn *col,
			  GtkCellRenderer   *renderer,
			  GtkTreeModel      *model,
			  GtkTreeIter       *iter,
			  gpointer           user_data)
{
	char *name;
	char *details;
	char *markup;

	gtk_tree_model_get (model, iter,
			    FONTS_NAME_COLUMN, &name,
			    FONTS_DETAILS_COLUMN, &details,
			    -1);

	if (details) {
		markup = g_strdup_printf ("<b><big>%s</big></b>\n<small>%s</small>",
					  name, details);
	} else {
		markup = g_strdup_printf ("<b><big>%s</big></b>", name);
	}

	g_object_set (renderer, "markup", markup, NULL);

	g_free (markup);
	g_free (details);
	g_free (name);
}

static const gchar *
font_type_to_string (PopplerFontType type)
{
	switch (type) {
	case POPPLER_FONT_TYPE_TYPE1:
		return "Type 1";
	case POPPLER_FONT_TYPE_TYPE1C:
		return "Type 1C";
	case POPPLER_FONT_TYPE_TYPE3:
		return "Type 3";
	case POPPLER_FONT_TYPE_TRUETYPE:
		return "TrueType";
	case POPPLER_FONT_TYPE_CID_TYPE0:
		return "Type 1 (CID)";
	case POPPLER_FONT_TYPE_CID_TYPE0C:
		return "Type 1C (CID)";
	case POPPLER_FONT_TYPE_CID_TYPE2:
		return "TrueType (CID)";
	default:
		return "Unknown font type";
	}
}

static void
pgd_fonts_update_progress (PgdFontsDemo *demo,
			   gint          n_pages,
			   gint          scanned)
{
	gchar *str;

	str = g_strdup_printf ("Scanning fonts (%d%%)",
			       MIN (scanned * 100 / n_pages, 100));
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (demo->progress), str);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (demo->progress),
				       MIN ((gdouble)scanned / n_pages, 1.0));
	g_free (str);
}

static gboolean
pgd_fonts_fill_model (PgdFontsDemo *demo)
{
	GtkTreeModel     *model;
	PopplerFontInfo  *font_info;
	PopplerFontsIter *fonts_iter;
	gint              n_pages, scanned = 0;

	n_pages = poppler_document_get_n_pages (demo->doc);
	
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (demo->treeview));
	g_object_ref (model);

	gtk_list_store_clear (GTK_LIST_STORE (model));

	font_info = poppler_font_info_new (demo->doc);

	while (poppler_font_info_scan (font_info, 20, &fonts_iter)) {
		pgd_fonts_update_progress (demo, n_pages, scanned);
		
		while (gtk_events_pending ())
			gtk_main_iteration ();

		scanned += 20;
		
		if (!fonts_iter)
			continue;
		
		do {
			GtkTreeIter iter;
			const gchar *name;
			const gchar *type;
			const gchar *embedded;
			const gchar *filename;
			gchar       *details;

			name = poppler_fonts_iter_get_name (fonts_iter);
			if (!name)
				name = "No name";

			type = font_type_to_string (poppler_fonts_iter_get_font_type (fonts_iter));
			
			if (poppler_fonts_iter_is_embedded (fonts_iter)) {
				if (poppler_fonts_iter_is_subset (fonts_iter))
					embedded = "Embedded subset";
				else
					embedded = "Embedded";
			} else {
				embedded = "Not embedded";
			}

			filename = poppler_fonts_iter_get_file_name (fonts_iter);

			if (filename)
				details = g_markup_printf_escaped ("%s\n%s (%s)", type, embedded, filename);
			else
				details = g_markup_printf_escaped ("%s\n%s", type, embedded);

			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter,
					    FONTS_NAME_COLUMN, name,
					    FONTS_DETAILS_COLUMN, details,
					    -1);
			
			g_free (details);
		} while (poppler_fonts_iter_next (fonts_iter));
		poppler_fonts_iter_free (fonts_iter);
	}

	pgd_fonts_update_progress (demo, n_pages, scanned);
	
	g_object_unref (font_info);
	g_object_unref (model);

	return FALSE;
}

static void
pgd_fonts_scan_button_clicked (GtkButton    *button,
			       PgdFontsDemo *demo)
{
	demo->idle_id = g_idle_add ((GSourceFunc)pgd_fonts_fill_model, demo);
}

GtkWidget *
pgd_fonts_create_widget (PopplerDocument *document)
{
	PgdFontsDemo      *demo;
	GtkWidget         *vbox;
	GtkListStore      *model;
	GtkCellRenderer   *renderer;
	GtkTreeViewColumn *column;
	GtkWidget         *swindow;
	GtkWidget         *hbox, *button;

	demo = g_new0 (PgdFontsDemo, 1);

	demo->doc = g_object_ref (document);
	
	vbox = gtk_vbox_new (FALSE, 12);

	hbox = gtk_hbox_new (FALSE, 6);
	
	demo->progress = gtk_progress_bar_new ();
	gtk_progress_bar_set_ellipsize (GTK_PROGRESS_BAR (demo->progress),
					PANGO_ELLIPSIZE_END);
	gtk_box_pack_start (GTK_BOX (hbox), demo->progress, TRUE, TRUE, 0);
	gtk_widget_show (demo->progress);

	button = gtk_button_new_with_label ("Scan");
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_fonts_scan_button_clicked),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 6);
	gtk_widget_show (hbox);
	
	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	model = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	demo->treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (demo->treeview), FALSE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (demo->treeview), TRUE);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (demo->treeview)),
				     GTK_SELECTION_NONE);
	g_object_unref (model);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (demo->treeview), column);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN (column), renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func (column, renderer,
						 pdg_fonts_cell_data_func,
						 NULL, NULL);

	gtk_container_add (GTK_CONTAINER (swindow), demo->treeview);
	gtk_widget_show (demo->treeview);

	gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
	gtk_widget_show (swindow);

	g_object_weak_ref (G_OBJECT (swindow),
			   (GWeakNotify)pgd_fonts_free,
			   (gpointer)demo);

	return vbox;
}
