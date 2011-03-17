/*
 * Copyright (C) 2008 Carlos Garcia Campos  <carlosgc@gnome.org>
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

#include "find.h"

enum {
	TITLE_COLUMN,
	X1_COLUMN,
	Y1_COLUMN,
	X2_COLUMN,
	Y2_COLUMN,

	VISIBLE_COLUMN,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;

	GtkTreeModel    *model;
	GtkWidget       *entry;
	GtkWidget       *progress;

	gint             n_pages;
	gint             page_index;
	
	guint            idle_id;
} PgdFindDemo;

static void
pgd_find_free (PgdFindDemo *demo)
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

	if (demo->model) {
		g_object_unref (demo->model);
		demo->model = NULL;
	}

	g_free (demo);
}

static void
pgd_find_update_progress (PgdFindDemo *demo,
			  gint         scanned)
{
	gchar *str;

	str = g_strdup_printf ("Searching ... (%d%%)",
			       MIN (scanned * 100 / demo->n_pages, 100));
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (demo->progress), str);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (demo->progress),
				       MIN ((gdouble)scanned / demo->n_pages, 1.0));
	g_free (str);
}

static gboolean
pgd_find_find_text (PgdFindDemo *demo)
{
	PopplerPage *page;
	GList       *matches;
	GTimer      *timer;

	page = poppler_document_get_page (demo->doc, demo->page_index);
	if (!page) {
		demo->page_index++;
		return demo->page_index < demo->n_pages;
	}

	timer = g_timer_new ();
	matches = poppler_page_find_text (page, gtk_entry_get_text (GTK_ENTRY (demo->entry)));
	g_timer_stop (timer);
	if (matches) {
		GtkTreeIter iter;
		gchar      *str;
		GList      *l;
		gint        n_match = 0;

		str = g_strdup_printf ("%d matches found on page %d in %.4f seconds",
				       g_list_length (matches), demo->page_index + 1,
				       g_timer_elapsed (timer, NULL));
		
		gtk_tree_store_append (GTK_TREE_STORE (demo->model), &iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (demo->model), &iter,
				    TITLE_COLUMN, str,
				    VISIBLE_COLUMN, FALSE,
				    -1);
		g_free (str);
		
		for (l = matches; l && l->data; l = g_list_next (l)) {
			PopplerRectangle *rect = (PopplerRectangle *)l->data;
			GtkTreeIter       iter_child;
			gchar            *x1, *y1, *x2, *y2;

			str = g_strdup_printf ("Match %d", ++n_match);
			x1 = g_strdup_printf ("%.2f", rect->x1);
			y1 = g_strdup_printf ("%.2f", rect->y1);
			x2 = g_strdup_printf ("%.2f", rect->x2);
			y2 = g_strdup_printf ("%.2f", rect->y2);
			
			gtk_tree_store_append (GTK_TREE_STORE (demo->model), &iter_child, &iter);
			gtk_tree_store_set (GTK_TREE_STORE (demo->model), &iter_child,
					    TITLE_COLUMN, str,
					    X1_COLUMN, x1,
					    Y1_COLUMN, y1,
					    X2_COLUMN, x2,
					    Y2_COLUMN, y2,
					    VISIBLE_COLUMN, TRUE,
					    -1);
			g_free (str);
			g_free (x1);
			g_free (y1);
			g_free (x2);
			g_free (y2);
			poppler_rectangle_free (rect);
		}
		g_list_free (matches);
	}

	g_timer_destroy (timer);
	g_object_unref (page);

	demo->page_index++;
	pgd_find_update_progress (demo, demo->page_index);

	return demo->page_index < demo->n_pages;
}

static void
pgd_find_button_clicked (GtkButton   *button,
			 PgdFindDemo *demo)
{
	gtk_tree_store_clear (GTK_TREE_STORE (demo->model));
	demo->page_index = 0;
	pgd_find_update_progress (demo, demo->page_index);
	if (demo->idle_id > 0)
		g_source_remove (demo->idle_id);
	demo->idle_id = g_idle_add ((GSourceFunc)pgd_find_find_text, demo);
}

static void
pgd_find_button_sensitivity_cb (GtkWidget *button,
				GtkEntry  *entry)
{
	const gchar *text;

	text = gtk_entry_get_text (entry);
	gtk_widget_set_sensitive (button, text != NULL && text[0] != '\0');
}

GtkWidget *
pgd_find_create_widget (PopplerDocument *document)
{
	PgdFindDemo     *demo;
	GtkWidget       *vbox, *hbox;
	GtkWidget       *button;
	GtkWidget       *swindow;
	GtkWidget       *treeview;
	GtkCellRenderer *renderer;

	demo = g_new0 (PgdFindDemo, 1);

	demo->doc = g_object_ref (document);

	demo->n_pages = poppler_document_get_n_pages (document);

	vbox = gtk_vbox_new (FALSE, 12);

	hbox = gtk_hbox_new (FALSE, 6);

	demo->entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), demo->entry, FALSE, TRUE, 0);
	gtk_widget_show (demo->entry);

	demo->progress = gtk_progress_bar_new ();
	gtk_progress_bar_set_ellipsize (GTK_PROGRESS_BAR (demo->progress),
					PANGO_ELLIPSIZE_END);
	gtk_box_pack_start (GTK_BOX (hbox), demo->progress, TRUE, TRUE, 0);
	gtk_widget_show (demo->progress);

	button = gtk_button_new_with_label ("Find");
	gtk_widget_set_sensitive (button, FALSE);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_find_button_clicked),
			  (gpointer)demo);
	g_signal_connect_swapped (G_OBJECT (demo->entry), "changed",
				  G_CALLBACK (pgd_find_button_sensitivity_cb),
				  (gpointer)button);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 6);
	gtk_widget_show (hbox);

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	demo->model = GTK_TREE_MODEL (
		gtk_tree_store_new (N_COLUMNS,
				    G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_BOOLEAN));
	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (demo->model));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
				     GTK_SELECTION_NONE);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     TITLE_COLUMN, "Matches",
						     renderer,
						     "text", TITLE_COLUMN,
						     NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     X1_COLUMN, "X1",
						     renderer,
						     "text", X1_COLUMN,
						     "visible", VISIBLE_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     Y1_COLUMN, "Y1",
						     renderer,
						     "text", Y1_COLUMN,
						     "visible", VISIBLE_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     X2_COLUMN, "X2",
						     renderer,
						     "text", X2_COLUMN,
						     "visible", VISIBLE_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     Y2_COLUMN, "Y2",
						     renderer,
						     "text", Y2_COLUMN,
						     "visible", VISIBLE_COLUMN,
						     NULL);
	gtk_container_add (GTK_CONTAINER (swindow), treeview);
	gtk_widget_show (treeview);

	gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
	gtk_widget_show (swindow);

	g_object_weak_ref (G_OBJECT (vbox),
			   (GWeakNotify)pgd_find_free,
			   (gpointer)demo);

	return vbox;
}
