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
#include <string.h>

#include "forms.h"
#include "utils.h"

enum {
	FORMS_FIELD_TYPE_COLUMN,
	FORMS_ID_COLUMN,
	FORMS_READ_ONLY_COLUMN,
	FORMS_X1_COLUMN,
	FORMS_Y1_COLUMN,
	FORMS_X2_COLUMN,
	FORMS_Y2_COLUMN,
	FORMS_FIELD_COLUMN,
	N_COLUMNS
};

typedef struct {
	PopplerDocument *doc;

	GtkListStore    *model;
	GtkWidget       *field_view;
	GtkWidget       *timer_label;

	gint             page;
} PgdFormsDemo;

static void
pgd_forms_free (PgdFormsDemo *demo)
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

static GtkWidget *
pgd_form_field_view_new (void)
{
	GtkWidget  *frame, *label;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Form Field Properties</b>");
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_widget_show (label);

	return frame;
}

static void
pgd_form_field_view_add_choice_items (GtkTable         *table,
				      PopplerFormField *field,
				      gint             *selected,
				      gint             *row)
{
	GtkWidget     *label;
	GtkWidget     *textview, *swindow;
	GtkTextBuffer *buffer;
	gint           i;

	label = gtk_label_new (NULL);
	g_object_set (G_OBJECT (label), "xalign", 0.0, NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Items:</b>");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, *row, *row + 1,
			  GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (label);

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	textview = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
	
	for (i = 0; i < poppler_form_field_choice_get_n_items (field); i++) {
		gchar *item;

		item = poppler_form_field_choice_get_item (field, i);
		gtk_text_buffer_insert_at_cursor (buffer, item, strlen (item));
		gtk_text_buffer_insert_at_cursor (buffer, "\n", strlen ("\n"));
		g_free (item);
		
		if (poppler_form_field_choice_is_item_selected (field, i))
			*selected = i;
	}

	gtk_container_add (GTK_CONTAINER (swindow), textview);
	gtk_widget_show (textview);
	
	gtk_table_attach (GTK_TABLE (table), swindow, 1, 2, *row, *row + 1,
			  GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (swindow); 
	
	*row += 1;
}

static void
pgd_form_field_view_set_field (GtkWidget        *field_view,
			       PopplerFormField *field)
{
	GtkWidget  *alignment;
	GtkWidget  *table;
	GEnumValue *enum_value;
	gchar      *text;
	gint        row = 0;

	alignment = gtk_bin_get_child (GTK_BIN (field_view));
	if (alignment) {
		gtk_container_remove (GTK_CONTAINER (field_view), alignment);
	}
	
	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
	gtk_container_add (GTK_CONTAINER (field_view), alignment);
	gtk_widget_show (alignment);

	if (!field)
		return;

	table = gtk_table_new (13, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);

	text = poppler_form_field_get_name (field);
	if (text) {
		pgd_table_add_property (GTK_TABLE (table), "<b>Name:</b>", text, &row);
		g_free (text);
	}
	text = poppler_form_field_get_partial_name (field);
	if (text) {
		pgd_table_add_property (GTK_TABLE (table), "<b>Partial Name:</b>", text, &row);
		g_free (text);
	}
	text = poppler_form_field_get_mapping_name (field);
	if (text) {
		pgd_table_add_property (GTK_TABLE (table), "<b>Mapping Name:</b>", text, &row);
		g_free (text);
	}

	switch (poppler_form_field_get_field_type (field)) {
	case POPPLER_FORM_FIELD_BUTTON:
		enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_FORM_BUTTON_TYPE),
					       poppler_form_field_button_get_button_type (field));
		pgd_table_add_property (GTK_TABLE (table), "<b>Button Type:</b>", enum_value->value_name, &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Button State:</b>",
					poppler_form_field_button_get_state (field) ? "Active" : "Inactive", &row);
		break;
	case POPPLER_FORM_FIELD_TEXT:
		enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_FORM_TEXT_TYPE),
					       poppler_form_field_text_get_text_type (field));
		pgd_table_add_property (GTK_TABLE (table), "<b>Text Type:</b>", enum_value->value_name, &row);

		text = poppler_form_field_text_get_text (field);
		pgd_table_add_property (GTK_TABLE (table), "<b>Contents:</b>", text, &row);
		g_free (text);

		text = g_strdup_printf ("%d", poppler_form_field_text_get_max_len (field));
		pgd_table_add_property (GTK_TABLE (table), "<b>Max Length:</b>", text, &row);
		g_free (text);

		pgd_table_add_property (GTK_TABLE (table), "<b>Do spellcheck:</b>",
					poppler_form_field_text_do_spell_check (field) ? "Yes" : "No", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Do scroll:</b>",
					poppler_form_field_text_do_scroll (field) ? "Yes" : "No", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Rich Text:</b>",
					poppler_form_field_text_is_rich_text (field) ? "Yes" : "No", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Pasword type:</b>",
					poppler_form_field_text_is_password (field) ? "Yes" : "No", &row);
		break;
	case POPPLER_FORM_FIELD_CHOICE: {
		gchar *item;
		gint   selected;
		
		enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_FORM_CHOICE_TYPE),
					       poppler_form_field_choice_get_choice_type (field));
		pgd_table_add_property (GTK_TABLE (table), "<b>Choice Type:</b>", enum_value->value_name, &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Editable:</b>",
					poppler_form_field_choice_is_editable (field) ? "Yes" : "No", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Multiple Selection:</b>",
					poppler_form_field_choice_can_select_multiple (field) ? "Yes" : "No", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Do spellcheck:</b>",
					poppler_form_field_choice_do_spell_check (field) ? "Yes" : "No", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Commit on Change:</b>",
					poppler_form_field_choice_commit_on_change (field) ? "Yes" : "No", &row);

		text = g_strdup_printf ("%d", poppler_form_field_choice_get_n_items (field));
		pgd_table_add_property (GTK_TABLE (table), "<b>Number of items:</b>", text, &row);
		g_free (text);

		pgd_form_field_view_add_choice_items (GTK_TABLE (table), field, &selected, &row);

		if (poppler_form_field_choice_get_n_items (field) > selected) {
			item = poppler_form_field_choice_get_item (field, selected);
			text = g_strdup_printf ("%d (%s)", selected, item);
			g_free (item);
			pgd_table_add_property (GTK_TABLE (table), "<b>Selected item:</b>", text, &row);
			g_free (text);
		}

		text = poppler_form_field_choice_get_text (field);
		pgd_table_add_property (GTK_TABLE (table), "<b>Contents:</b>", text, &row);
		g_free (text);
	}
		break;
	case POPPLER_FORM_FIELD_SIGNATURE:
	case POPPLER_FORM_FIELD_UNKNOWN:
		break;
	default:
		g_assert_not_reached ();
	}

	gtk_container_add (GTK_CONTAINER (alignment), table);
	gtk_widget_show (table);
}

const gchar *
get_form_field_type (PopplerFormField *field)
{
	switch (poppler_form_field_get_field_type (field)) {
	case POPPLER_FORM_FIELD_TEXT:
		return "Text";
	case POPPLER_FORM_FIELD_BUTTON:
		return "Button";
	case POPPLER_FORM_FIELD_CHOICE:
		return "Choice";
	case POPPLER_FORM_FIELD_SIGNATURE:
		return "Signature";
	case POPPLER_FORM_FIELD_UNKNOWN:
	default:
		break;
	}

	return "Unknown";
}

static void
pgd_forms_get_form_fields (GtkWidget    *button,
			   PgdFormsDemo *demo)
{
	PopplerPage *page;
	GList       *mapping, *l;
	gint         n_fields;
	GTimer      *timer;
	
	gtk_list_store_clear (demo->model);
	pgd_form_field_view_set_field (demo->field_view, NULL);

	page = poppler_document_get_page (demo->doc, demo->page);
	if (!page)
		return;

	timer = g_timer_new ();
	mapping = poppler_page_get_form_field_mapping (page);
	g_timer_stop (timer);

	n_fields = g_list_length (mapping);
	if (n_fields > 0) {
		gchar *str;
		
		str = g_strdup_printf ("<i>%d form fields found in %.4f seconds</i>",
				       n_fields, g_timer_elapsed (timer, NULL));
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), str);
		g_free (str);
	} else {
		gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No form fields found</i>");
	}

	g_timer_destroy (timer);

	for (l = mapping; l; l = g_list_next (l)) {
		PopplerFormFieldMapping *fmapping;
		GtkTreeIter              iter;
		gchar                   *x1, *y1, *x2, *y2;

		fmapping = (PopplerFormFieldMapping *)l->data;

		x1 = g_strdup_printf ("%.2f", fmapping->area.x1);
		y1 = g_strdup_printf ("%.2f", fmapping->area.y1);
		x2 = g_strdup_printf ("%.2f", fmapping->area.x2);
		y2 = g_strdup_printf ("%.2f", fmapping->area.y2);

		gtk_list_store_append (demo->model, &iter);
		gtk_list_store_set (demo->model, &iter,
				    FORMS_FIELD_TYPE_COLUMN, get_form_field_type (fmapping->field),
				    FORMS_ID_COLUMN, poppler_form_field_get_id (fmapping->field),
				    FORMS_READ_ONLY_COLUMN, poppler_form_field_is_read_only (fmapping->field),
				    FORMS_X1_COLUMN, x1, 
				    FORMS_Y1_COLUMN, y1,
				    FORMS_X2_COLUMN, x2,
				    FORMS_Y2_COLUMN, y2,
				    FORMS_FIELD_COLUMN, fmapping->field,
				    -1);
		g_free (x1);
		g_free (y1);
		g_free (x2);
		g_free (y2);
	}

	poppler_page_free_form_field_mapping (mapping);
	g_object_unref (page);
}

static void
pgd_forms_page_selector_value_changed (GtkSpinButton *spinbutton,
				       PgdFormsDemo  *demo)
{
	demo->page = (gint)gtk_spin_button_get_value (spinbutton) - 1;
}

static void
pgd_forms_selection_changed (GtkTreeSelection *treeselection,
			     PgdFormsDemo     *demo)
{
	GtkTreeModel *model;
	GtkTreeIter   iter;

	if (gtk_tree_selection_get_selected (treeselection, &model, &iter)) {
		PopplerFormField *field;

		gtk_tree_model_get (model, &iter,
				    FORMS_FIELD_COLUMN, &field,
				    -1);
		pgd_form_field_view_set_field (demo->field_view, field);
		g_object_unref (field);
	}
}

GtkWidget *
pgd_forms_create_widget (PopplerDocument *document)
{
	PgdFormsDemo     *demo;
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

	demo = g_new0 (PgdFormsDemo, 1);
	
	demo->doc = g_object_ref (document);
	
	n_pages = poppler_document_get_n_pages (document);

	vbox = gtk_vbox_new (FALSE, 12);

	hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Page:");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);

	page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
	g_signal_connect (G_OBJECT (page_selector), "value-changed",
			  G_CALLBACK (pgd_forms_page_selector_value_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (hbox), page_selector, FALSE, TRUE, 0);
	gtk_widget_show (page_selector);

	str = g_strdup_printf ("of %d", n_pages);
	label = gtk_label_new (str);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);
	g_free (str);

	button = gtk_button_new_with_label ("Get Forms Fields");
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_forms_get_form_fields),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);

	demo->timer_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No form fields found</i>");
	g_object_set (G_OBJECT (demo->timer_label), "xalign", 1.0, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), demo->timer_label, FALSE, TRUE, 0);
	gtk_widget_show (demo->timer_label);

	hpaned = gtk_hpaned_new ();

	demo->field_view = pgd_form_field_view_new ();

	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	
	demo->model = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING,
					  G_TYPE_INT, G_TYPE_BOOLEAN,
					  G_TYPE_STRING,
					  G_TYPE_STRING, G_TYPE_STRING,
					  G_TYPE_STRING, G_TYPE_OBJECT);
	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (demo->model));

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     0, "Form Field Type",
						     renderer,
						     "text", FORMS_FIELD_TYPE_COLUMN,
						     NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     1, "Form Field Id",
						     renderer,
						     "text", FORMS_ID_COLUMN,
						     NULL);
	
	renderer = gtk_cell_renderer_toggle_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     2, "Read Only",
						     renderer,
						     "active", FORMS_READ_ONLY_COLUMN,
						     NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     3, "X1",
						     renderer,
						     "text", FORMS_X1_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     4, "Y1",
						     renderer,
						     "text", FORMS_Y1_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     5, "X2",
						     renderer,
						     "text", FORMS_X2_COLUMN,
						     NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
						     6, "Y2",
						     renderer,
						     "text", FORMS_Y2_COLUMN,
						     NULL);
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (pgd_forms_selection_changed),
			  (gpointer)demo);

	gtk_container_add (GTK_CONTAINER (swindow), treeview);
	gtk_widget_show (treeview);

	gtk_paned_add1 (GTK_PANED (hpaned), swindow);
	gtk_widget_show (swindow);

	gtk_paned_add2 (GTK_PANED (hpaned), demo->field_view);
	gtk_widget_show (demo->field_view);

	gtk_paned_set_position (GTK_PANED (hpaned), 300);

	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
	gtk_widget_show (hpaned);

	g_object_weak_ref (G_OBJECT (vbox),
			   (GWeakNotify)pgd_forms_free,
			   demo);
	
	return vbox;
}
