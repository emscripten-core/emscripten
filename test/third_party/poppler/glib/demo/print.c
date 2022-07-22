/*
 * Copyright (C) 2009 Carlos Garcia Campos  <carlosgc@gnome.org>
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
#include "print.h"

typedef enum {
        PRINT_DOCUMENT,
        PRINT_DOCUMENT_MARKUPS,
        PRINT_DOCUMENT_STAMPS
} PgdPrintOptions;

typedef struct {
	PopplerDocument *doc;
        GtkWidget       *options_combo;
        PgdPrintOptions  options;
} PgdPrintDemo;

#define PGD_PRINT_OPTIONS "pgd-print-options"

static void
pgd_print_free (PgdPrintDemo *demo)
{
	if (!demo)
		return;

	if (demo->doc) {
		g_object_unref (demo->doc);
		demo->doc = NULL;
	}

	g_free (demo);
}

static void
pgd_print_begin_print (GtkPrintOperation *op,
		       GtkPrintContext   *context,
		       PgdPrintDemo      *demo)
{
	gtk_print_operation_set_n_pages (op, poppler_document_get_n_pages (demo->doc));
}

static void
pgd_print_draw_page (GtkPrintOperation *op,
		     GtkPrintContext   *context,
		     gint               page_nr,
		     PgdPrintDemo      *demo)
{
	PopplerPage      *page;
	cairo_t          *cr;
        GtkPrintSettings *settings;
        PgdPrintOptions   options;
        PopplerPrintFlags flags = 0;

	page = poppler_document_get_page (demo->doc, page_nr);
	if (!page)
		return;

        settings = gtk_print_operation_get_print_settings (op);
        /* Workaround for gtk+ bug, we need to save the options ourselves */
        options = demo->options;
#if 0
        options = gtk_print_settings_get_int_with_default (settings,
                                                           PGD_PRINT_OPTIONS,
                                                           PRINT_DOCUMENT_MARKUPS);
#endif
        switch (options) {
        case PRINT_DOCUMENT:
                flags |= POPPLER_PRINT_DOCUMENT;
                break;
        case PRINT_DOCUMENT_MARKUPS:
                flags |= POPPLER_PRINT_MARKUP_ANNOTS;
                break;
        case PRINT_DOCUMENT_STAMPS:
                flags |= POPPLER_PRINT_STAMP_ANNOTS_ONLY;
                break;
        default:
                g_assert_not_reached ();
        }

	cr = gtk_print_context_get_cairo_context (context);
	poppler_page_render_for_printing_with_options (page, cr, flags);
	g_object_unref (page);
}

static GObject *
pgd_print_create_custom_widget (GtkPrintOperation *op,
                                PgdPrintDemo      *demo)
{
        GtkWidget        *hbox;
        GtkWidget        *label, *combo;
        GtkPrintSettings *settings;
        PgdPrintOptions   options;

        settings = gtk_print_operation_get_print_settings (op);
        options = gtk_print_settings_get_int_with_default (settings,
                                                           PGD_PRINT_OPTIONS,
                                                           PRINT_DOCUMENT_MARKUPS);

        hbox = gtk_hbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);

        label = gtk_label_new ("Print: ");
        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_widget_show (label);

        combo = gtk_combo_box_new_text ();
        demo->options_combo = combo;
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Document");
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Document and markup");
        gtk_combo_box_append_text (GTK_COMBO_BOX (combo), "Document and stamps");
        gtk_combo_box_set_active (GTK_COMBO_BOX (combo), options);
        gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
        gtk_widget_show (combo);

        return G_OBJECT (hbox);
}

static void
pgd_print_custom_widget_apply (GtkPrintOperation *op,
                               GtkWidget         *widget,
                               PgdPrintDemo      *demo)
{
        GtkPrintSettings *settings;
        PgdPrintOptions   options;

        settings = gtk_print_operation_get_print_settings (op);
        options = gtk_combo_box_get_active (GTK_COMBO_BOX (demo->options_combo));
        /* Workaround for gtk+ bug, we need to save the options ourselves */
        demo->options = options;
        gtk_print_settings_set_int (settings, PGD_PRINT_OPTIONS, options);
}

static void
pgd_print_print (GtkWidget    *button,
		 PgdPrintDemo *demo)
{
	GtkPrintOperation *op;
	GError            *error = NULL;

	op = gtk_print_operation_new ();
        gtk_print_operation_set_custom_tab_label (op, "PDF Options");
	g_signal_connect (op, "begin-print",
			  G_CALLBACK (pgd_print_begin_print),
			  demo);
	g_signal_connect (op, "draw-page",
			  G_CALLBACK (pgd_print_draw_page),
			  demo);
        g_signal_connect (op, "create_custom_widget",
                          G_CALLBACK (pgd_print_create_custom_widget),
                          demo);
        g_signal_connect (op, "custom_widget_apply",
                          G_CALLBACK (pgd_print_custom_widget_apply),
                          demo);
	gtk_print_operation_run (op,
				 GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				 GTK_WINDOW (gtk_widget_get_toplevel (button)),
				 &error);
	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (GTK_WINDOW (gtk_widget_get_toplevel (button)),
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 "%s", error->message);
		g_error_free (error);

		g_signal_connect (dialog, "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);

		gtk_widget_show (dialog);
	}
	g_object_unref (op);
}

GtkWidget *
pgd_print_create_widget (PopplerDocument *document)
{
	PgdPrintDemo *demo;
	GtkWidget    *vbox;
	GtkWidget    *hbox;
	GtkWidget    *button;

	demo = g_new0 (PgdPrintDemo, 1);

	demo->doc = g_object_ref (document);

	vbox = gtk_vbox_new (FALSE, 12);

	hbox = gtk_hbox_new (FALSE, 6);

	button = gtk_button_new_with_label ("Print...");
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_print_print),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show (hbox);

	g_object_weak_ref (G_OBJECT (vbox),
			   (GWeakNotify)pgd_print_free,
			   (gpointer)demo);

	return vbox;
}
