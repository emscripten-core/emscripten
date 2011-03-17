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

#include "config.h"
#include "info.h"
#include "utils.h"

static void
pgd_info_add_permissions (GtkTable           *table,
			  PopplerPermissions  permissions,
			  gint               *row)
{
	GtkWidget *label, *hbox;
	GtkWidget *checkbox;

	label = gtk_label_new (NULL);
	g_object_set (G_OBJECT (label), "xalign", 0.0, NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Permissions:</b>");
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, *row, *row + 1,
			  GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (label);

	hbox = gtk_hbox_new (FALSE, 6);

	checkbox = gtk_check_button_new_with_label ("Print");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox),
				      (permissions & POPPLER_PERMISSIONS_OK_TO_PRINT));
	gtk_box_pack_start (GTK_BOX (hbox), checkbox, FALSE, TRUE, 0);
	gtk_widget_show (checkbox);
	
	checkbox = gtk_check_button_new_with_label ("Copy");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox),
				      (permissions & POPPLER_PERMISSIONS_OK_TO_COPY));
	gtk_box_pack_start (GTK_BOX (hbox), checkbox, FALSE, TRUE, 0);
	gtk_widget_show (checkbox);
	
	checkbox = gtk_check_button_new_with_label ("Modify");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox),
				      (permissions & POPPLER_PERMISSIONS_OK_TO_MODIFY));
	gtk_box_pack_start (GTK_BOX (hbox), checkbox, FALSE, TRUE, 0);
	gtk_widget_show (checkbox);
	
	checkbox = gtk_check_button_new_with_label ("Add notes");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox),
				      (permissions & POPPLER_PERMISSIONS_OK_TO_ADD_NOTES));
	gtk_box_pack_start (GTK_BOX (hbox), checkbox, FALSE, TRUE, 0);
	gtk_widget_show (checkbox);

	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, *row, *row + 1,
			  GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (hbox);
	
	*row += 1;
}

static void
pgd_info_add_metadata (GtkTable    *table,
		       const gchar *metadata,
		       gint        *row)
{
	GtkWidget     *label;
	GtkWidget     *textview, *swindow;
	GtkTextBuffer *buffer;

	label = gtk_label_new (NULL);
	g_object_set (G_OBJECT (label), "xalign", 0.0, NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Metadata:</b>");
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
	if (metadata)
		gtk_text_buffer_set_text (buffer, metadata, -1);

	gtk_container_add (GTK_CONTAINER (swindow), textview);
	gtk_widget_show (textview);
	
	gtk_table_attach (GTK_TABLE (table), swindow, 1, 2, *row, *row + 1,
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_widget_show (swindow); 
	
	*row += 1;
}

GtkWidget *
pgd_info_create_widget (PopplerDocument *document)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *frame, *alignment, *table;
	gchar     *str;
	gchar     *title, *format, *author, *subject;
	gchar     *keywords, *creator, *producer;
	gchar     *metadata;
	gchar     *perm_id;
	gchar     *up_id;
	gboolean   linearized;
	GTime      creation_date, mod_date;
	GEnumValue *enum_value;
	PopplerBackend backend;
	PopplerPageLayout layout;
	PopplerPageMode mode;
	PopplerPermissions permissions;
	PopplerViewerPreferences view_prefs;
	gint row = 0;

	g_object_get (document,
		      "title", &title,
		      "format", &format,
		      "author", &author,
		      "subject", &subject,
		      "keywords", &keywords,
		      "creation-date", &creation_date,
		      "mod-date", &mod_date,
		      "creator", &creator,
		      "producer", &producer,
		      "linearized", &linearized,
		      "page-mode", &mode,
		      "page-layout", &layout,
		      "permissions", &permissions,
		      "viewer-preferences", &view_prefs,
		      "metadata", &metadata,
		      NULL);
	
	vbox = gtk_vbox_new (FALSE, 12);

	backend = poppler_get_backend ();
	enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_BACKEND), backend);
	str = g_strdup_printf ("<span weight='bold' size='larger'>Poppler %s (%s)</span>",
			       poppler_get_version (), enum_value->value_name);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), str);
	g_free (str);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 12);
	gtk_widget_show (label);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Document properties</b>");
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_widget_show (label);
	
	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_widget_show (alignment);

	table = gtk_table_new (14, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);

	pgd_table_add_property (GTK_TABLE (table), "<b>Format:</b>", format, &row);
	g_free (format);
	
	pgd_table_add_property (GTK_TABLE (table), "<b>Title:</b>", title, &row);
	g_free (title);
	
	pgd_table_add_property (GTK_TABLE (table), "<b>Author:</b>", author, &row);
	g_free (author);
	
	pgd_table_add_property (GTK_TABLE (table), "<b>Subject:</b>", subject, &row);
	g_free (subject);
	
	pgd_table_add_property (GTK_TABLE (table), "<b>Keywords:</b>", keywords, &row);
	g_free (keywords);
	
	pgd_table_add_property (GTK_TABLE (table), "<b>Creator:</b>", creator, &row);
	g_free (creator);
	
	pgd_table_add_property (GTK_TABLE (table), "<b>Producer:</b>", producer, &row);
	g_free (producer);
	
	pgd_table_add_property (GTK_TABLE (table), "<b>Linearized:</b>", linearized ? "Yes" : "No", &row);

	str = pgd_format_date (creation_date);
	pgd_table_add_property (GTK_TABLE (table), "<b>Creation Date:</b>", str, &row);
	g_free (str);

	str = pgd_format_date (mod_date);
	pgd_table_add_property (GTK_TABLE (table), "<b>Modification Date:</b>", str, &row);
	g_free (str);

	enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_MODE), mode);
	pgd_table_add_property (GTK_TABLE (table), "<b>Page Mode:</b>", enum_value->value_name, &row);

	enum_value = g_enum_get_value ((GEnumClass *) g_type_class_peek (POPPLER_TYPE_PAGE_LAYOUT), layout);
	pgd_table_add_property (GTK_TABLE (table), "<b>Page Layout:</b>", enum_value->value_name, &row);

	if (poppler_document_get_id (document, &perm_id, &up_id)) {
		str = g_strndup (perm_id, 32);
		g_free (perm_id);
		pgd_table_add_property (GTK_TABLE (table), "<b>Permanent ID:</b>", str, &row);
		g_free (str);
		str = g_strndup (up_id, 32);
		g_free (up_id);
		pgd_table_add_property (GTK_TABLE (table), "<b>Update ID:</b>", str, &row);
		g_free (str);
	}

	pgd_info_add_permissions (GTK_TABLE (table), permissions, &row);

	pgd_info_add_metadata (GTK_TABLE (table), metadata, &row);
	g_free (metadata);

	/* TODO: view_prefs */

	gtk_container_add (GTK_CONTAINER (alignment), table);
	gtk_widget_show (table);

	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
	gtk_widget_show (frame);

	return vbox;
}
