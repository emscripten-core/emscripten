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

#include "config.h"

#include <gtk/gtk.h>

#include "layers.h"

enum {
	LAYERS_TITLE_COLUMN,
	LAYERS_VISIBILITY_COLUMN,
	LAYERS_ENABLE_COLUMN,
	LAYERS_SHOWTOGGLE_COLUMN,
	LAYERS_RB_GROUP_COLUMN,
	LAYERS_LAYER_COLUMN,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;
	guint            page;
	GtkWidget       *treeview;
	GtkWidget       *darea;

	cairo_surface_t *surface;
} PgdLayersDemo;

static void
pgd_layers_free (PgdLayersDemo *demo)
{
	if (!demo)
		return;

	if (demo->doc) {
		g_object_unref (demo->doc);
		demo->doc = NULL;
	}

	if (demo->surface) {
		cairo_surface_destroy (demo->surface);
		demo->surface = NULL;
	}

	g_free (demo);
}

static void
build_tree (PopplerDocument   *document,
	    GtkTreeModel      *model,
	    GtkTreeIter       *parent,
	    PopplerLayersIter *iter)
{

	do {
		GtkTreeIter        tree_iter;
		PopplerLayersIter *child;
		PopplerLayer      *layer;
		gboolean           visible;
		gchar             *markup;
		gint               rb_group = 0;

		layer = poppler_layers_iter_get_layer (iter);
		if (layer) {
			markup = g_markup_escape_text (poppler_layer_get_title (layer), -1);
			visible = poppler_layer_is_visible (layer);
			rb_group = poppler_layer_get_radio_button_group_id (layer);
		} else {
			gchar *title;

			title = poppler_layers_iter_get_title (iter);
			markup = g_markup_escape_text (title, -1);
			g_free (title);

			visible = FALSE;
			layer = NULL;
		}

		gtk_tree_store_append (GTK_TREE_STORE (model), &tree_iter, parent);
		gtk_tree_store_set (GTK_TREE_STORE (model), &tree_iter,
				    LAYERS_TITLE_COLUMN, markup,
				    LAYERS_VISIBILITY_COLUMN, visible,
				    LAYERS_ENABLE_COLUMN, TRUE, /* FIXME */
				    LAYERS_SHOWTOGGLE_COLUMN, (layer != NULL),
				    LAYERS_RB_GROUP_COLUMN, rb_group,
				    LAYERS_LAYER_COLUMN, layer,
				    -1);
		if (layer)
			g_object_unref (layer);
		g_free (markup);

		child = poppler_layers_iter_get_child (iter);
		if (child)
			build_tree (document, model, &tree_iter, child);
		poppler_layers_iter_free (child);
	} while (poppler_layers_iter_next (iter));
}

GtkTreeModel *
pgd_layers_create_model (PopplerDocument *document)
{
	GtkTreeModel     *model;
	PopplerLayersIter *iter;

	iter = poppler_layers_iter_new (document);
	if (iter) {
		model = GTK_TREE_MODEL (
			gtk_tree_store_new (N_COLUMNS,
					    G_TYPE_STRING, 
					    G_TYPE_BOOLEAN,
					    G_TYPE_BOOLEAN,
					    G_TYPE_BOOLEAN,
					    G_TYPE_INT,
					    G_TYPE_OBJECT));
		build_tree (document, model, NULL, iter);
		poppler_layers_iter_free (iter);
	} else {
		GtkTreeIter tree_iter;
		gchar      *markup;
		
		model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
		gtk_list_store_append (GTK_LIST_STORE (model), &tree_iter);
		markup = g_strdup_printf ("<span size=\"larger\" style=\"italic\">%s</span>",
					  "The document doesn't contain layers");
		gtk_list_store_set (GTK_LIST_STORE (model), &tree_iter,
				    0, markup, -1);
		g_free (markup);
	}

	return model;
}

static cairo_surface_t *
pgd_layers_render_page (PgdLayersDemo *demo)
{
	cairo_t *cr;
	PopplerPage *page;
	gdouble width, height;
	cairo_surface_t *surface = NULL;

	page = poppler_document_get_page (demo->doc, demo->page);
	if (!page)
		return NULL;

	poppler_page_get_size (page, &width, &height);
	gtk_widget_set_size_request (demo->darea, width, height);
	
	surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
					      width, height);
	cr = cairo_create (surface);

	cairo_save (cr);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);
	cairo_restore (cr);

	cairo_save (cr);
	poppler_page_render (page, cr);
	cairo_restore (cr);
	
	cairo_destroy (cr);
	g_object_unref (page);

	return surface;
}

static gboolean
pgd_layers_viewer_drawing_area_expose (GtkWidget      *area,
				       GdkEventExpose *event,
				       PgdLayersDemo  *demo)
{
	cairo_t *cr;
	
	if (!demo->surface) {
		demo->surface = pgd_layers_render_page (demo);
		if (!demo->surface)
			return FALSE;
	}

	gdk_window_clear (gtk_widget_get_window (area));

	cr = gdk_cairo_create (gtk_widget_get_window (area));
	cairo_set_source_surface (cr, demo->surface, 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);

	return TRUE;
}

static gboolean
pgd_layers_viewer_redraw (PgdLayersDemo *demo)
{
	cairo_surface_destroy (demo->surface);
	demo->surface = NULL;

	gtk_widget_queue_draw (demo->darea);

	return FALSE;
}

static void
pgd_layers_viewer_queue_redraw (PgdLayersDemo *demo)
{
	g_idle_add ((GSourceFunc)pgd_layers_viewer_redraw, demo);
}

static void
pgd_layers_page_selector_value_changed (GtkSpinButton *spinbutton,
					PgdLayersDemo *demo)
{
	demo->page = (gint)gtk_spin_button_get_value (spinbutton) - 1;
	pgd_layers_viewer_queue_redraw (demo);
}

static GtkWidget *
pgd_layers_create_viewer (PgdLayersDemo *demo)
{
	GtkWidget *vbox, *hbox;
	GtkWidget *label;
	GtkWidget *swindow;
	GtkWidget *page_selector;
	guint      n_pages;
	gchar     *str;

	vbox = gtk_vbox_new (FALSE, 6);

	hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Page:");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);

	n_pages = poppler_document_get_n_pages (demo->doc);
	page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
	g_signal_connect (G_OBJECT (page_selector), "value-changed",
			  G_CALLBACK (pgd_layers_page_selector_value_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (hbox), page_selector, FALSE, TRUE, 0);
	gtk_widget_show (page_selector);

	str = g_strdup_printf ("of %d", n_pages);
	label = gtk_label_new (str);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	g_free (str);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);

	demo->darea = gtk_drawing_area_new ();
	g_signal_connect (G_OBJECT (demo->darea), "expose_event",
			  G_CALLBACK (pgd_layers_viewer_drawing_area_expose),
			  (gpointer)demo);

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (swindow), demo->darea);
	gtk_widget_show (demo->darea);

	gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
	gtk_widget_show (swindow);

	return vbox;
}

static gboolean
update_kids (GtkTreeModel *model,
	     GtkTreePath  *path,
	     GtkTreeIter  *iter,
	     GtkTreeIter  *parent)
{
	if (gtk_tree_store_is_ancestor (GTK_TREE_STORE (model), parent, iter)) {
		gboolean visible;

		gtk_tree_model_get (model, parent,
				    LAYERS_VISIBILITY_COLUMN, &visible,
				    -1);
		gtk_tree_store_set (GTK_TREE_STORE (model), iter,
				    LAYERS_ENABLE_COLUMN, visible,
				    -1);
	}

	return FALSE;
}

static gboolean
clear_rb_group (GtkTreeModel *model,
		GtkTreePath  *path,
		GtkTreeIter  *iter,
		gint         *rb_group)
{
	gint group;
	
	gtk_tree_model_get (model, iter,
			    LAYERS_RB_GROUP_COLUMN, &group,
			    -1);
	
	if (group == *rb_group) {
		gtk_tree_store_set (GTK_TREE_STORE (model), iter,
				    LAYERS_VISIBILITY_COLUMN, FALSE,
				    -1);
	}
	
	return FALSE;
}

static void
pgd_layers_visibility_changed (GtkCellRendererToggle *cell,
			       gchar                 *path_str,
			       PgdLayersDemo         *demo)
{
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gboolean visible;
	PopplerLayer *layer;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (demo->treeview));

	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter,
			    LAYERS_VISIBILITY_COLUMN, &visible,
			    LAYERS_LAYER_COLUMN, &layer,
			    -1);

	visible = !visible;
	visible ? poppler_layer_show (layer) : poppler_layer_hide (layer);

	if (visible) {
		gint rb_group;
		
		rb_group = poppler_layer_get_radio_button_group_id (layer);
		if (rb_group) {
			gtk_tree_model_foreach (model,
						(GtkTreeModelForeachFunc)clear_rb_group,
						&rb_group);
		}
	}
	
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			    LAYERS_VISIBILITY_COLUMN, visible,
			    -1);
	
	if (poppler_layer_is_parent (layer)) {
		gtk_tree_model_foreach (model,
					(GtkTreeModelForeachFunc)update_kids,
					&iter);
	}

	pgd_layers_viewer_queue_redraw (demo);

	gtk_tree_path_free (path);
	g_object_unref (layer);
}

GtkWidget *
pgd_layers_create_widget (PopplerDocument *document)
{
	PgdLayersDemo    *demo;
	GtkWidget        *swindow;
	GtkWidget        *treeview;
	GtkTreeModel     *model;
	GtkCellRenderer  *renderer;
	GtkTreeSelection *selection;
	GtkWidget        *hpaned, *viewer;

	demo = g_new0 (PgdLayersDemo, 1);
	demo->doc = g_object_ref (document);
	
	hpaned = gtk_hpaned_new ();

	viewer = pgd_layers_create_viewer (demo);
	
	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	model = pgd_layers_create_model (document);
	treeview = gtk_tree_view_new_with_model (model);
	demo->treeview = treeview;
	g_object_unref (model);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     0, "Layer",
						     renderer,
						     "markup", LAYERS_TITLE_COLUMN,
						     NULL);
	g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	g_object_set (G_OBJECT (gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 0)),
		      "expand", TRUE, NULL);

	if (GTK_IS_TREE_STORE (model)) {
		renderer = gtk_cell_renderer_toggle_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
							     1, "Show/Hide",
							     renderer,
							     "active", LAYERS_VISIBILITY_COLUMN,
							     "activatable", LAYERS_ENABLE_COLUMN,
							     "visible", LAYERS_SHOWTOGGLE_COLUMN,
							     NULL);

		g_signal_connect (renderer, "toggled",
				  G_CALLBACK (pgd_layers_visibility_changed),
				  (gpointer)demo);
		gtk_tree_view_column_set_clickable (gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 1),
						    TRUE);
	}

	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
				     GTK_SELECTION_NONE);

	gtk_container_add (GTK_CONTAINER (swindow), treeview);
	gtk_widget_show (treeview);

	gtk_paned_add1 (GTK_PANED (hpaned), swindow);
	gtk_widget_show (swindow);

	gtk_paned_add2 (GTK_PANED (hpaned), viewer);
	gtk_widget_show (viewer);

	gtk_paned_set_position (GTK_PANED (hpaned), 150);

	g_object_weak_ref (G_OBJECT (hpaned),
			   (GWeakNotify)pgd_layers_free,
			   (gpointer)demo);

	return hpaned;
}
