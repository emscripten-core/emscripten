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

#include "images.h"

enum {
	IMAGES_ID_COLUMN,
	IMAGES_X1_COLUMN,
	IMAGES_Y1_COLUMN,
	IMAGES_X2_COLUMN,
	IMAGES_Y2_COLUMN,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;

	GtkListStore    *model;
	GtkWidget       *timer_label;
	GtkWidget       *image_view;

	gint             page;
} PgdImagesDemo;

static void
pgd_images_free (PgdImagesDemo *demo)
{
	if (!demo)
		return;

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

static gboolean
pgd_image_view_drawing_area_expose (GtkWidget      *area,
				    GdkEventExpose *event,
				    GtkWidget      *image_view)
{
	cairo_t         *cr;
	cairo_surface_t *image;

	image = g_object_get_data (G_OBJECT (image_view), "image-surface");
	if (!image)
		return FALSE;

	gtk_widget_set_size_request (area,
				     cairo_image_surface_get_width (image),
				     cairo_image_surface_get_height (image));
	
	cr = gdk_cairo_create (gtk_widget_get_window (area));
	cairo_set_source_surface (cr, image, 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);

	return TRUE;
}

static GtkWidget *
pgd_image_view_new ()
{
	GtkWidget *swindow;
	GtkWidget *darea;

	swindow = gtk_scrolled_window_new (NULL, NULL);
	
	darea = gtk_drawing_area_new ();
	g_signal_connect (G_OBJECT (darea), "expose_event",
			  G_CALLBACK (pgd_image_view_drawing_area_expose),
			  (gpointer)swindow);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (swindow),
					       darea);
	gtk_widget_show (darea);

	return swindow;
}

static void
pgd_image_view_set_image (GtkWidget       *image_view,
			  cairo_surface_t *image)
{
	g_object_set_data_full (G_OBJECT (image_view), "image-surface",
				image,
				(GDestroyNotify)cairo_surface_destroy);
	gtk_widget_queue_draw (image_view);
}

static void
pgd_images_get_images (GtkWidget     *button,
		       PgdImagesDemo *demo)
{
	PopplerPage *page;
	GList       *mapping, *l;
	gint         n_images;
	GTimer      *timer;
	
	gtk_list_store_clear (demo->model);
	pgd_image_view_set_image (demo->image_view, NULL);

	page = poppler_document_get_page (demo->doc, demo->page);
	if (!page)
		return;

	timer = g_timer_new ();
	mapping = poppler_page_get_image_mapping (page);
	g_timer_stop (timer);

	n_images = g_list_length (mapping);
	if (n_images > 0) {
		gchar *str;
		
		str = g_strdup_printf ("<i>%d images found in %.4f seconds</i>",
				       n_images, g_timer_elapsed (timer, NULL));
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), str);
		g_free (str);
	} else {
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No images found</i>");
	}

	g_timer_destroy (timer);

	for (l = mapping; l; l = g_list_next (l)) {
		PopplerImageMapping *imapping;
		GtkTreeIter          iter;
		gchar               *x1, *y1, *x2, *y2;

		imapping = (PopplerImageMapping *)l->data;

		x1 = g_strdup_printf ("%.2f", imapping->area.x1);
		y1 = g_strdup_printf ("%.2f", imapping->area.y1);
		x2 = g_strdup_printf ("%.2f", imapping->area.x2);
		y2 = g_strdup_printf ("%.2f", imapping->area.y2);

		gtk_list_store_append (demo->model, &iter);
		gtk_list_store_set (demo->model, &iter,
				    IMAGES_ID_COLUMN, imapping->image_id,
				    IMAGES_X1_COLUMN, x1, 
				    IMAGES_Y1_COLUMN, y1,
				    IMAGES_X2_COLUMN, x2,
				    IMAGES_Y2_COLUMN, y2,
				    -1);
		g_free (x1);
		g_free (y1);
		g_free (x2);
		g_free (y2);
	}

	poppler_page_free_image_mapping (mapping);
	g_object_unref (page);
}

static void
pgd_images_page_selector_value_changed (GtkSpinButton *spinbutton,
					PgdImagesDemo *demo)
{
	demo->page = (gint)gtk_spin_button_get_value (spinbutton) - 1;
}

static void
pgd_images_selection_changed (GtkTreeSelection *treeselection,
			      PgdImagesDemo    *demo)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;

	if (gtk_tree_selection_get_selected (treeselection, &model, &iter)) {
		PopplerPage *page;
		gint         image_id;

		gtk_tree_model_get (model, &iter,
				    IMAGES_ID_COLUMN, &image_id,
				    -1);
		page = poppler_document_get_page (demo->doc, demo->page);
		pgd_image_view_set_image (demo->image_view,
					  poppler_page_get_image (page, image_id));
		g_object_unref (page);
					  
	}
}

GtkWidget *
pgd_images_create_widget (PopplerDocument *document)
{
	PgdImagesDemo    *demo;
	GtkWidget        *label;
	GtkWidget        *vbox;
	GtkWidget        *hbox, *page_selector;
	GtkWidget        *button;
	GtkWidget        *hpaned;
	GtkWidget        *swindow, *treeview;
	GtkTreeSelection *selection;
	GtkCellRenderer  *renderer;
	gchar            *str;
	gint              n_pages;

	demo = g_new0 (PgdImagesDemo, 1);
	
	demo->doc = g_object_ref (document);
	
	n_pages = poppler_document_get_n_pages (document);

	vbox = gtk_vbox_new (FALSE, 12);

	hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Page:");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);

	page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
	g_signal_connect (G_OBJECT (page_selector), "value-changed",
			  G_CALLBACK (pgd_images_page_selector_value_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (hbox), page_selector, FALSE, TRUE, 0);
	gtk_widget_show (page_selector);

	str = g_strdup_printf ("of %d", n_pages);
	label = gtk_label_new (str);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	g_free (str);

	button = gtk_button_new_with_label ("Get Images");
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_images_get_images),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);

	demo->timer_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No images found</i>");
	g_object_set (G_OBJECT (demo->timer_label), "xalign", 1.0, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), demo->timer_label, FALSE, TRUE, 0);
	gtk_widget_show (demo->timer_label);

	hpaned = gtk_hpaned_new ();

	demo->image_view = pgd_image_view_new ();

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	
	demo->model = gtk_list_store_new (N_COLUMNS, G_TYPE_INT,
					  G_TYPE_STRING, G_TYPE_STRING,
					  G_TYPE_STRING, G_TYPE_STRING);
	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (demo->model));

	renderer = gtk_cell_renderer_text_new ();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     0, "Image",
						     renderer,
						     "text", IMAGES_ID_COLUMN,
						     NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     1, "X1",
						     renderer,
						     "text", IMAGES_X1_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     2, "Y1",
						     renderer,
						     "text", IMAGES_Y1_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     3, "X2",
						     renderer,
						     "text", IMAGES_X2_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     4, "Y2",
						     renderer,
						     "text", IMAGES_Y2_COLUMN,
						     NULL);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (pgd_images_selection_changed),
			  (gpointer)demo);

	gtk_container_add (GTK_CONTAINER (swindow), treeview);
	gtk_widget_show (treeview);

	gtk_paned_add1 (GTK_PANED (hpaned), swindow);
	gtk_widget_show (swindow);

	gtk_paned_add2 (GTK_PANED (hpaned), demo->image_view);
	gtk_widget_show (demo->image_view);

	gtk_paned_set_position (GTK_PANED (hpaned), 300);

	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
	gtk_widget_show (hpaned);

	g_object_weak_ref (G_OBJECT (vbox),
			   (GWeakNotify)pgd_images_free,
			   demo);
	
	return vbox;
}
