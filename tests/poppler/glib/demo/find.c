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
        PAGE_COLUMN,
        PAGE_RECT,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;

	GtkWidget       *treeview;
        GtkWidget       *darea;
	GtkWidget       *entry;
	GtkWidget       *progress;

        PopplerFindFlags options;
	gint             n_pages;
	gint             page_index;

	guint            idle_id;

        cairo_surface_t *surface;
        gint             selected_page;
        GdkRectangle     selected_match;
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

        if (demo->surface) {
                cairo_surface_destroy (demo->surface);
                demo->surface = NULL;
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
	PopplerPage  *page;
	GList        *matches;
	GTimer       *timer;
        GtkTreeModel *model;

	page = poppler_document_get_page (demo->doc, demo->page_index);
	if (!page) {
		demo->page_index++;
		return demo->page_index < demo->n_pages;
	}

        model = gtk_tree_view_get_model (GTK_TREE_VIEW (demo->treeview));
	timer = g_timer_new ();
	matches = poppler_page_find_text_with_options (page, gtk_entry_get_text (GTK_ENTRY (demo->entry)), demo->options);
	g_timer_stop (timer);
	if (matches) {
		GtkTreeIter iter;
		gchar      *str;
		GList      *l;
                gdouble     height;
		gint        n_match = 0;

		str = g_strdup_printf ("%d matches found on page %d in %.4f seconds",
				       g_list_length (matches), demo->page_index + 1,
				       g_timer_elapsed (timer, NULL));
		
		gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
				    TITLE_COLUMN, str,
				    VISIBLE_COLUMN, FALSE,
                                    PAGE_COLUMN, demo->page_index,
				    -1);
		g_free (str);

                poppler_page_get_size (page, NULL, &height);

		for (l = matches; l && l->data; l = g_list_next (l)) {
			PopplerRectangle *rect = (PopplerRectangle *)l->data;
			GtkTreeIter       iter_child;
			gchar            *x1, *y1, *x2, *y2;
                        gdouble           tmp;

			str = g_strdup_printf ("Match %d", ++n_match);
			x1 = g_strdup_printf ("%.2f", rect->x1);
			y1 = g_strdup_printf ("%.2f", rect->y1);
			x2 = g_strdup_printf ("%.2f", rect->x2);
			y2 = g_strdup_printf ("%.2f", rect->y2);

                        tmp = rect->y1;
                        rect->y1 = height - rect->y2;
                        rect->y2 = height - tmp;

			gtk_tree_store_append (GTK_TREE_STORE (model), &iter_child, &iter);
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter_child,
					    TITLE_COLUMN, str,
					    X1_COLUMN, x1,
					    Y1_COLUMN, y1,
					    X2_COLUMN, x2,
					    Y2_COLUMN, y2,
					    VISIBLE_COLUMN, TRUE,
                                            PAGE_COLUMN, demo->page_index,
                                            PAGE_RECT, rect,
					    -1);
			g_free (str);
			g_free (x1);
			g_free (y1);
			g_free (x2);
			g_free (y2);
                        g_object_weak_ref (G_OBJECT (model),
                                           (GWeakNotify)poppler_rectangle_free,
                                           rect);
		}
		g_list_free (matches);
	}

	g_timer_destroy (timer);
	g_object_unref (page);

	demo->page_index++;
	pgd_find_update_progress (demo, demo->page_index);

	return demo->page_index < demo->n_pages;
}

static cairo_surface_t *
pgd_find_render_page (PgdFindDemo *demo)
{
        cairo_t *cr;
        PopplerPage *page;
        gdouble width, height;
        cairo_surface_t *surface = NULL;

        page = poppler_document_get_page (demo->doc, demo->selected_page);
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
pgd_find_viewer_drawing_area_draw (GtkWidget   *area,
                                   cairo_t     *cr,
                                   PgdFindDemo *demo)
{
        if (demo->selected_page == -1)
                return FALSE;

        if (!demo->surface) {
                demo->surface = pgd_find_render_page (demo);
                if (!demo->surface)
                        return FALSE;
        }

        cairo_set_source_surface (cr, demo->surface, 0, 0);
        cairo_paint (cr);

        if (demo->selected_match.width > 0 && demo->selected_match.height > 0) {
                cairo_set_source_rgb (cr, 1., 1., 0.);
                cairo_set_operator (cr, CAIRO_OPERATOR_MULTIPLY);
                gdk_cairo_rectangle (cr, &demo->selected_match);
                cairo_fill (cr);
        }

        return TRUE;
}

static gboolean
pgd_find_viewer_redraw (PgdFindDemo *demo)
{
        cairo_surface_destroy (demo->surface);
        demo->surface = NULL;

        gtk_widget_queue_draw (demo->darea);

        return FALSE;
}

static void
pgd_find_viewer_queue_redraw (PgdFindDemo *demo)
{
        g_idle_add ((GSourceFunc)pgd_find_viewer_redraw, demo);
}

static GtkTreeModel *
pgd_find_create_model ()
{
        return GTK_TREE_MODEL (gtk_tree_store_new (N_COLUMNS,
                                                   G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_STRING, G_TYPE_STRING,
                                                   G_TYPE_BOOLEAN, G_TYPE_UINT,
                                                   G_TYPE_POINTER));
}

static void
pgd_find_button_clicked (GtkButton   *button,
			 PgdFindDemo *demo)
{
        GtkTreeModel *model;

        /* Delete the model and create a new one instead of
         * just clearing it to make sure rectangle are free.
         * This is a workaround because GtkTreeModel doesn't
         * support boxed types and we have to store rectangles
         * as pointers that are freed when the model is deleted.
         */
        model = pgd_find_create_model ();
        gtk_tree_view_set_model (GTK_TREE_VIEW (demo->treeview), model);
        g_object_unref (model);

        demo->selected_page = -1;
        pgd_find_viewer_queue_redraw (demo);

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

static void
pgd_find_selection_changed (GtkTreeSelection *treeselection,
                            PgdFindDemo      *demo)
{
        GtkTreeModel *model;
        GtkTreeIter   iter;

        if (gtk_tree_selection_get_selected (treeselection, &model, &iter)) {
                guint page_index;
                PopplerRectangle *rect;

                gtk_tree_model_get (model, &iter,
                                    PAGE_COLUMN, &page_index,
                                    PAGE_RECT, &rect,
                                    -1);

                if (rect) {
                        demo->selected_match.x = rect->x1;
                        demo->selected_match.y = rect->y1;
                        demo->selected_match.width = rect->x2 - rect->x1;
                        demo->selected_match.height = rect->y2 - rect->y1;
                } else {
                        demo->selected_match.width = 0;
                        demo->selected_match.height = 0;
                }

                if (page_index != demo->selected_page) {
                        demo->selected_page = page_index;
                        pgd_find_viewer_queue_redraw (demo);
                } else {
                        gtk_widget_queue_draw (demo->darea);
                }
        }
}

static void
pgd_find_case_sensitive_toggled (GtkToggleButton *togglebutton,
                                 PgdFindDemo     *demo)
{
        if (gtk_toggle_button_get_active (togglebutton))
                demo->options |= POPPLER_FIND_CASE_SENSITIVE;
        else
                demo->options &= ~POPPLER_FIND_CASE_SENSITIVE;
}

static void
pgd_find_backwards_toggled (GtkToggleButton *togglebutton,
                            PgdFindDemo     *demo)
{
        if (gtk_toggle_button_get_active (togglebutton))
                demo->options |= POPPLER_FIND_BACKWARDS;
        else
                demo->options &= ~POPPLER_FIND_BACKWARDS;
}

static void
pgd_find_whole_words_toggled (GtkToggleButton *togglebutton,
                              PgdFindDemo     *demo)
{
        if (gtk_toggle_button_get_active (togglebutton))
                demo->options |= POPPLER_FIND_WHOLE_WORDS_ONLY;
        else
                demo->options &= ~POPPLER_FIND_WHOLE_WORDS_ONLY;
}

GtkWidget *
pgd_find_create_widget (PopplerDocument *document)
{
	PgdFindDemo      *demo;
	GtkWidget        *vbox, *hbox;
	GtkWidget        *button;
	GtkWidget        *swindow;
        GtkWidget        *checkbutton;
        GtkTreeModel     *model;
	GtkWidget        *treeview;
	GtkCellRenderer  *renderer;
        GtkWidget        *hpaned;
        GtkTreeSelection *selection;

	demo = g_new0 (PgdFindDemo, 1);

	demo->doc = g_object_ref (document);

	demo->n_pages = poppler_document_get_n_pages (document);
        demo->selected_page = -1;
        demo->options = POPPLER_FIND_DEFAULT;

        hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
        gtk_paned_set_position (GTK_PANED (hpaned), 300);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

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

        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

        checkbutton = gtk_check_button_new_with_label ("Case sensitive");
        g_signal_connect (checkbutton, "toggled",
                          G_CALLBACK (pgd_find_case_sensitive_toggled),
                          demo);
        gtk_box_pack_start (GTK_BOX (hbox), checkbutton, FALSE, FALSE, 0);
        gtk_widget_show (checkbutton);

        checkbutton = gtk_check_button_new_with_label ("Backwards");
        g_signal_connect (checkbutton, "toggled",
                          G_CALLBACK (pgd_find_backwards_toggled),
                          demo);
        gtk_box_pack_start (GTK_BOX (hbox), checkbutton, FALSE, FALSE, 0);
        gtk_widget_show (checkbutton);

        checkbutton = gtk_check_button_new_with_label ("Whole words only");
        g_signal_connect (checkbutton, "toggled",
                          G_CALLBACK (pgd_find_whole_words_toggled),
                          demo);
        gtk_box_pack_start (GTK_BOX (hbox), checkbutton, FALSE, FALSE, 0);
        gtk_widget_show (checkbutton);

        gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
        gtk_widget_show (hbox);

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	model = pgd_find_create_model ();
	treeview = gtk_tree_view_new_with_model (model);
        g_object_unref (model);
        demo->treeview = treeview;
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
        g_signal_connect (selection, "changed",
                          G_CALLBACK (pgd_find_selection_changed),
                          demo);

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

        gtk_paned_add1 (GTK_PANED (hpaned), swindow);
        gtk_widget_show (swindow);

        demo->darea = gtk_drawing_area_new ();
        g_signal_connect (demo->darea, "draw",
                          G_CALLBACK (pgd_find_viewer_drawing_area_draw),
                          demo);

        swindow = gtk_scrolled_window_new (NULL, NULL);
#if GTK_CHECK_VERSION(3, 7, 8)
        gtk_container_add(GTK_CONTAINER(swindow), demo->darea);
#else
        gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (swindow), demo->darea);
#endif
        gtk_widget_show (demo->darea);

        gtk_paned_add2 (GTK_PANED (hpaned), swindow);
        gtk_widget_show (swindow);

        gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
        gtk_widget_show (hpaned);

	g_object_weak_ref (G_OBJECT (vbox),
			   (GWeakNotify)pgd_find_free,
			   (gpointer)demo);

	return vbox;
}
