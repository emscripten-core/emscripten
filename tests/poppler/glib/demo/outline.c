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

#include "outline.h"
#include "utils.h"

enum {
	OUTLINE_TITLE_COLUMN,
	OUTLINE_ACTION_TYPE_COLUMN,
	OUTLINE_EXPAND_COLUMN,
	OUTLINE_ACTION_COLUMN,
	N_COLUMNS
};

static void
build_tree (PopplerDocument  *document,
	    GtkTreeModel     *model,
	    GtkTreeIter      *parent,
	    PopplerIndexIter *iter)
{

	do {
		GtkTreeIter       tree_iter;
		PopplerIndexIter *child;
		PopplerAction    *action;
		gboolean          expand;
		gchar            *markup;
		GEnumValue       *enum_value;

		action = poppler_index_iter_get_action (iter);
		expand = poppler_index_iter_is_open (iter);

		if (!action)
			continue;

		markup = g_markup_escape_text (action->any.title, -1);
		enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_ACTION_TYPE), action->type);
		
		if (action->type == POPPLER_ACTION_GOTO_DEST &&
		    action->goto_dest.dest->type == POPPLER_DEST_NAMED) {
			/* TODO */
		}			

		gtk_tree_store_append (GTK_TREE_STORE (model), &tree_iter, parent);
		gtk_tree_store_set (GTK_TREE_STORE (model), &tree_iter,
				    OUTLINE_TITLE_COLUMN, markup,
				    OUTLINE_ACTION_TYPE_COLUMN, enum_value->value_name,
				    OUTLINE_EXPAND_COLUMN, expand,
				    OUTLINE_ACTION_COLUMN, action,
				    -1);
		g_object_weak_ref (G_OBJECT (model),
				   (GWeakNotify)poppler_action_free,
				   action);

		g_free (markup);

		child = poppler_index_iter_get_child (iter);
		if (child)
			build_tree (document, model, &tree_iter, child);
		poppler_index_iter_free (child);
	} while (poppler_index_iter_next (iter));
}

GtkTreeModel *
pgd_outline_create_model (PopplerDocument *document)
{
	GtkTreeModel     *model;
	PopplerIndexIter *iter;

	iter = poppler_index_iter_new (document);
	if (iter) {
		model = GTK_TREE_MODEL (
			gtk_tree_store_new (N_COLUMNS,
					    G_TYPE_STRING, G_TYPE_STRING, 
					    G_TYPE_BOOLEAN, G_TYPE_POINTER));
		build_tree (document, model, NULL, iter);
		poppler_index_iter_free (iter);
	} else {
		GtkTreeIter tree_iter;
		gchar      *markup;
		
		model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
		gtk_list_store_append (GTK_LIST_STORE (model), &tree_iter);
		markup = g_strdup_printf ("<span size=\"larger\" style=\"italic\">%s</span>",
					  "The document doesn't contain outline");
		gtk_list_store_set (GTK_LIST_STORE (model), &tree_iter,
				    0, markup, -1);
		g_free (markup);
	}

	return model;
}

static void
expand_open_links (GtkTreeView  *tree_view,
		   GtkTreeModel *model,
		   GtkTreeIter  *parent)
{
	GtkTreeIter iter;
	gboolean    expand;

	if (gtk_tree_model_iter_children (model, &iter, parent)) {
		do {
			gtk_tree_model_get (model, &iter,
					    OUTLINE_EXPAND_COLUMN, &expand,
					    -1);
			if (expand) {
				GtkTreePath *path;

				path = gtk_tree_model_get_path (model, &iter);
				gtk_tree_view_expand_row (tree_view, path, FALSE);
				gtk_tree_path_free (path);
			}

			expand_open_links (tree_view, model, &iter);
		} while (gtk_tree_model_iter_next (model, &iter));
	}
}

static void
pgd_outline_selection_changed (GtkTreeSelection *treeselection,
			       GtkWidget        *action_view)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;
	
	if (gtk_tree_selection_get_selected (treeselection, &model, &iter)) {
		PopplerAction *action;

		gtk_tree_model_get (model, &iter,
				    OUTLINE_ACTION_COLUMN, &action,
				    -1);
		pgd_action_view_set_action (action_view, action);
	}
}

GtkWidget *
pgd_outline_create_widget (PopplerDocument *document)
{
	GtkWidget        *swindow;
	GtkWidget        *treeview;
	GtkTreeModel     *model;
	GtkCellRenderer  *renderer;
	GtkTreeSelection *selection;
	GtkWidget        *hpaned, *action;

	hpaned = gtk_hpaned_new ();

	action = pgd_action_view_new (document);
	
	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	model = pgd_outline_create_model (document);
	treeview = gtk_tree_view_new_with_model (model);
	g_object_unref (model);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     0, "Title",
						     renderer,
						     "markup", OUTLINE_TITLE_COLUMN,
						     NULL);
	g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	g_object_set (G_OBJECT (gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 0)),
		      "expand", TRUE, NULL);

	if (GTK_IS_TREE_STORE (model)) {
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
							     1, "Action Type",
							     renderer,
							     "text", OUTLINE_ACTION_TYPE_COLUMN,
							     NULL);

		expand_open_links (GTK_TREE_VIEW (treeview), model, NULL);

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
		g_signal_connect (G_OBJECT (selection), "changed",
				  G_CALLBACK (pgd_outline_selection_changed),
				  (gpointer)action);
	} else {
		gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)),
					     GTK_SELECTION_NONE);
	}

	gtk_container_add (GTK_CONTAINER (swindow), treeview);
	gtk_widget_show (treeview);

	gtk_paned_add1 (GTK_PANED (hpaned), swindow);
	gtk_widget_show (swindow);

	gtk_paned_add2 (GTK_PANED (hpaned), action);
	gtk_widget_show (action);

	gtk_paned_set_position (GTK_PANED (hpaned), 300);

	return hpaned;
}
