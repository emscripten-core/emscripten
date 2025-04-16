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

#include "transitions.h"

enum {
	TRANSITIONS_PAGE_COLUMN,
	TRANSITIONS_TYPE_COLUMN,
	TRANSITIONS_ALIGNMENT_COLUMN,
	TRANSITIONS_DIRECTION_COLUMN,
	TRANSITIONS_DURATION_COLUMN,
	TRANSITIONS_ANGLE_COLUMN,
	TRANSITIONS_SCALE_COLUMN,
	TRANSITIONS_RECTANGULAR_COLUMN,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;

	GtkWidget       *treeview;
	GtkWidget       *progress;

	guint            idle_id;
} PgdTransitionsDemo;

static void
pgd_transitions_free (PgdTransitionsDemo *demo)
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

static const gchar *
transition_type_to_string (PopplerPageTransitionType type)
{
	switch (type) {
	case POPPLER_PAGE_TRANSITION_REPLACE:
		return "Replace";
	case POPPLER_PAGE_TRANSITION_SPLIT:
		return "Split";
	case POPPLER_PAGE_TRANSITION_BLINDS:
		return "Blinds";
	case POPPLER_PAGE_TRANSITION_BOX:
		return "Box";
	case POPPLER_PAGE_TRANSITION_WIPE:
		return "Wipe";
	case POPPLER_PAGE_TRANSITION_DISSOLVE:
		return "Dissolve";
	case POPPLER_PAGE_TRANSITION_GLITTER:
		return "Glitter";
	case POPPLER_PAGE_TRANSITION_FLY:
		return "Fly";
	case POPPLER_PAGE_TRANSITION_PUSH:
		return "Push";
	case POPPLER_PAGE_TRANSITION_COVER:
		return "Cover";
	case POPPLER_PAGE_TRANSITION_UNCOVER:
		return "Uncover";
	case POPPLER_PAGE_TRANSITION_FADE:
		return "Fade";
	}

	return "Unknown";
}

static const gchar *
transition_alignment_to_string (PopplerPageTransitionAlignment alignment)
{
	return alignment == POPPLER_PAGE_TRANSITION_HORIZONTAL ? "Horizontal" : "Vertical";
}

static const gchar *
transition_direction_to_string (PopplerPageTransitionDirection direction)
{
	return direction == POPPLER_PAGE_TRANSITION_INWARD ? "Inward" : "Outward";
}

static void
pgd_transitions_update_progress (PgdTransitionsDemo *demo,
				 gint                n_pages,
				 gint                scanned)
{
	gchar *str;

	str = g_strdup_printf ("Scanning transitions (%d%%)",
			       MIN (scanned * 100 / n_pages, 100));
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (demo->progress), str);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (demo->progress),
				       MIN ((gdouble)scanned / n_pages, 1.0));
	g_free (str);
}

static gboolean
pgd_transitions_fill_model (PgdTransitionsDemo *demo)
{
	GtkTreeModel *model;
	gint          i, n_pages;

	n_pages = poppler_document_get_n_pages (demo->doc);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (demo->treeview));
	g_object_ref (model);

	for (i = 0; i < n_pages; i++) {
		PopplerPage           *page;
		PopplerPageTransition *transition;

		pgd_transitions_update_progress (demo, n_pages, i);

		while (gtk_events_pending ())
			gtk_main_iteration ();

		page = poppler_document_get_page (demo->doc, i);
		if (!page)
			continue;

		transition = poppler_page_get_transition (page);
		if (transition) {
			GtkTreeIter iter;
			gchar      *npage;
			gchar      *duration;
			gchar      *angle;
			gchar      *scale;

			npage = g_strdup_printf ("%d", i + 1);
			duration = g_strdup_printf ("%d", transition->duration);
			angle = g_strdup_printf ("%d", transition->angle);
			scale = g_strdup_printf ("%.2f", transition->scale);
			
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter,
					    TRANSITIONS_PAGE_COLUMN, npage,
					    TRANSITIONS_TYPE_COLUMN,
					    transition_type_to_string (transition->type),
					    TRANSITIONS_ALIGNMENT_COLUMN,
					    transition_alignment_to_string (transition->alignment),
					    TRANSITIONS_DIRECTION_COLUMN,
					    transition_direction_to_string (transition->direction),
					    TRANSITIONS_DURATION_COLUMN, duration, 
					    TRANSITIONS_ANGLE_COLUMN, angle, 
					    TRANSITIONS_SCALE_COLUMN, scale, 
					    TRANSITIONS_RECTANGULAR_COLUMN,
					    transition->rectangular ? "Yes" : "No",
					    -1);
			g_free (npage);
			g_free (duration);
			g_free (angle);
			g_free (scale);

			poppler_page_transition_free (transition);
		}

		g_object_unref (page);
	}

	pgd_transitions_update_progress (demo, n_pages, n_pages);
	g_object_unref (model);

	return FALSE;
}

static void
pgd_transitions_scan_button_clicked (GtkButton          *button,
				     PgdTransitionsDemo *demo)
{
	if (demo->idle_id > 0)
		g_source_remove (demo->idle_id);
	
	demo->idle_id = g_idle_add ((GSourceFunc)pgd_transitions_fill_model, demo);
}

static GtkWidget *
pgd_transitions_create_list (GtkTreeModel *model)
{
	GtkWidget       *treeview;
	GtkCellRenderer *renderer;
	
	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
				     GTK_SELECTION_NONE);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     0, "Page",
						     renderer,
						     "text", TRANSITIONS_PAGE_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     1, "Type",
						     renderer,
						     "text", TRANSITIONS_TYPE_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     2, "Alignment",
						     renderer,
						     "text", TRANSITIONS_ALIGNMENT_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     3, "Direction",
						     renderer,
						     "text", TRANSITIONS_DIRECTION_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     4, "Duration",
						     renderer,
						     "text", TRANSITIONS_DURATION_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     5, "Angle",
						     renderer,
						     "text", TRANSITIONS_ANGLE_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     6, "Scale",
						     renderer,
						     "text", TRANSITIONS_SCALE_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     7, "Rectangular",
						     renderer,
						     "text", TRANSITIONS_RECTANGULAR_COLUMN,
						     NULL);
	return treeview;
}

GtkWidget *
pgd_transitions_create_widget (PopplerDocument *document)
{
	PgdTransitionsDemo *demo;
	GtkWidget          *vbox;
	GtkListStore       *model;
	GtkWidget          *swindow;
	GtkWidget          *hbox, *button;

	demo = g_new0 (PgdTransitionsDemo, 1);

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
			  G_CALLBACK (pgd_transitions_scan_button_clicked),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 6);
	gtk_widget_show (hbox);

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	model = gtk_list_store_new (N_COLUMNS,
				    G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_STRING);
	demo->treeview = pgd_transitions_create_list (GTK_TREE_MODEL (model));
	g_object_unref (model);

	gtk_container_add (GTK_CONTAINER (swindow), demo->treeview);
	gtk_widget_show (demo->treeview);

	gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
	gtk_widget_show (swindow);

	g_object_weak_ref (G_OBJECT (swindow),
			   (GWeakNotify)pgd_transitions_free,
			   (gpointer)demo);

	return vbox;
}
