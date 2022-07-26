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

#include "links.h"
#include "utils.h"

enum {
	LINKS_ACTION_TYPE_COLUMN,
	LINKS_X1_COLUMN,
	LINKS_Y1_COLUMN,
	LINKS_X2_COLUMN,
	LINKS_Y2_COLUMN,
	LINKS_ACTION_COLUMN,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;

	GtkListStore    *model;
	GtkWidget       *action_view;
	GtkWidget       *timer_label;
	
	gint             page;
} PgdLinksDemo;

static void
pgd_links_free (PgdLinksDemo *demo)
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

static void
pgd_links_get_links (GtkWidget    *button,
		     PgdLinksDemo *demo)
{
	PopplerPage *page;
	GList       *mapping, *l;
	gint         n_links;
	GTimer      *timer;
	
	gtk_list_store_clear (demo->model);
	pgd_action_view_set_action (demo->action_view, NULL);

	page = poppler_document_get_page (demo->doc, demo->page);
	if (!page)
		return;

	timer = g_timer_new ();
	mapping = poppler_page_get_link_mapping (page);
	g_timer_stop (timer);

	n_links = g_list_length (mapping);
	if (n_links > 0) {
		gchar *str;
		
		str = g_strdup_printf ("<i>%d links found in %.4f seconds</i>",
				       n_links, g_timer_elapsed (timer, NULL));
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), str);
		g_free (str);
	} else {
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No links found</i>");
	}

	g_timer_destroy (timer);

	for (l = mapping; l; l = g_list_next (l)) {
		PopplerLinkMapping *lmapping;
		PopplerAction      *action;
		GEnumValue         *enum_value;
		GtkTreeIter         iter;
		gchar              *x1, *y1, *x2, *y2;

		lmapping = (PopplerLinkMapping *)l->data;
		action = poppler_action_copy (lmapping->action);
		enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_ACTION_TYPE), action->type);

		x1 = g_strdup_printf ("%.2f", lmapping->area.x1);
		y1 = g_strdup_printf ("%.2f", lmapping->area.y1);
		x2 = g_strdup_printf ("%.2f", lmapping->area.x2);
		y2 = g_strdup_printf ("%.2f", lmapping->area.y2);

		gtk_list_store_append (demo->model, &iter);
		gtk_list_store_set (demo->model, &iter,
				    LINKS_ACTION_TYPE_COLUMN, enum_value->value_name,
				    LINKS_X1_COLUMN, x1, 
				    LINKS_Y1_COLUMN, y1,
				    LINKS_X2_COLUMN, x2,
				    LINKS_Y2_COLUMN, y2,
				    LINKS_ACTION_COLUMN, action,
				    -1);
		g_free (x1);
		g_free (y1);
		g_free (x2);
		g_free (y2);
		
		g_object_weak_ref (G_OBJECT (demo->model),
				   (GWeakNotify)poppler_action_free,
				   action);
	}

	poppler_page_free_link_mapping (mapping);
	g_object_unref (page);
}

static void
pgd_links_page_selector_value_changed (GtkSpinButton *spinbutton,
				       PgdLinksDemo  *demo)
{
	demo->page = (gint)gtk_spin_button_get_value (spinbutton) - 1;
}

static void
pgd_links_selection_changed (GtkTreeSelection *treeselection,
			     PgdLinksDemo     *demo)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;

	if (gtk_tree_selection_get_selected (treeselection, &model, &iter)) {
		PopplerAction *action;

		gtk_tree_model_get (model, &iter,
				    LINKS_ACTION_COLUMN, &action,
				    -1);
		pgd_action_view_set_action (demo->action_view, action);
	}
}

GtkWidget *
pgd_links_create_widget (PopplerDocument *document)
{
	PgdLinksDemo     *demo;
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

	demo = g_new0 (PgdLinksDemo, 1);
	
	demo->doc = g_object_ref (document);
	
	n_pages = poppler_document_get_n_pages (document);

	vbox = gtk_vbox_new (FALSE, 12);

	hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Page:");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);

	page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
	g_signal_connect (G_OBJECT (page_selector), "value-changed",
			  G_CALLBACK (pgd_links_page_selector_value_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (hbox), page_selector, FALSE, TRUE, 0);
	gtk_widget_show (page_selector);

	str = g_strdup_printf ("of %d", n_pages);
	label = gtk_label_new (str);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	g_free (str);

	button = gtk_button_new_with_label ("Get Links");
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_links_get_links),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);

	demo->timer_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No links found</i>");
	g_object_set (G_OBJECT (demo->timer_label), "xalign", 1.0, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), demo->timer_label, FALSE, TRUE, 0);
	gtk_widget_show (demo->timer_label);

	hpaned = gtk_hpaned_new ();

	demo->action_view = pgd_action_view_new (document);

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	
	demo->model = gtk_list_store_new (N_COLUMNS,
					  G_TYPE_STRING, G_TYPE_STRING,
					  G_TYPE_STRING, G_TYPE_STRING,
					  G_TYPE_STRING, G_TYPE_POINTER);
	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (demo->model));

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     0, "Action Type",
						     renderer,
						     "text", LINKS_ACTION_TYPE_COLUMN,
						     NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     1, "X1",
						     renderer,
						     "text", LINKS_X1_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     2, "Y1",
						     renderer,
						     "text", LINKS_Y1_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     3, "X2",
						     renderer,
						     "text", LINKS_X2_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     4, "Y2",
						     renderer,
						     "text", LINKS_Y2_COLUMN,
						     NULL);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (pgd_links_selection_changed),
			  (gpointer)demo);

	gtk_container_add (GTK_CONTAINER (swindow), treeview);
	gtk_widget_show (treeview);

	gtk_paned_add1 (GTK_PANED (hpaned), swindow);
	gtk_widget_show (swindow);

	gtk_paned_add2 (GTK_PANED (hpaned), demo->action_view);
	gtk_widget_show (demo->action_view);

	gtk_paned_set_position (GTK_PANED (hpaned), 300);

	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
	gtk_widget_show (hpaned);

	g_object_weak_ref (G_OBJECT (vbox),
			   (GWeakNotify)pgd_links_free,
			   demo);
	
	return vbox;
}
