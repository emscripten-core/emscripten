/*
 * Copyright (C) 2008 Inigo Martinez <inigomartinez@gmail.com>
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

#include "annots.h"
#include "utils.h"

enum {
    ANNOTS_X1_COLUMN,
    ANNOTS_Y1_COLUMN,
    ANNOTS_X2_COLUMN,
    ANNOTS_Y2_COLUMN,
    ANNOTS_TYPE_COLUMN,
    ANNOTS_COLOR_COLUMN,
    ANNOTS_FLAG_INVISIBLE_COLUMN,
    ANNOTS_FLAG_HIDDEN_COLUMN,
    ANNOTS_FLAG_PRINT_COLUMN,
    ANNOTS_COLUMN,
    N_COLUMNS
};

typedef struct {
    PopplerDocument *doc;
    PopplerPage     *page;

    GtkListStore    *model;
    GtkWidget       *annot_view;
    GtkWidget       *timer_label;

    gint             num_page;
} PgdAnnotsDemo;

static void
pgd_annots_free (PgdAnnotsDemo *demo)
{
    if (!demo)
        return;

    if (demo->doc) {
        g_object_unref (demo->doc);
        demo->doc = NULL;
    }

    if (demo->page) {
        g_object_unref (demo->page);
        demo->page = NULL;
    }

    if (demo->model) {
        g_object_unref (demo->model);
        demo->model = NULL;
    }

    g_free (demo);
}

static GtkWidget *
pgd_annot_view_new (void)
{
    GtkWidget  *frame, *label;

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), "<b>Annot Properties</b>");
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_widget_show (label);

    return frame;
}

const gchar *
get_annot_type (PopplerAnnot *poppler_annot)
{
    switch (poppler_annot_get_annot_type (poppler_annot))
    {
      case POPPLER_ANNOT_TEXT:
        return "Text";
      case POPPLER_ANNOT_LINK:
        return "Link";
      case POPPLER_ANNOT_FREE_TEXT:
        return "Free Text";
      case POPPLER_ANNOT_LINE:
        return "Line";
      case POPPLER_ANNOT_SQUARE:
        return "Square";
      case POPPLER_ANNOT_CIRCLE:
        return "Circle";
      case POPPLER_ANNOT_POLYGON:
        return "Polygon";
      case POPPLER_ANNOT_POLY_LINE:
        return "Poly Line";
      case POPPLER_ANNOT_HIGHLIGHT:
        return "Highlight";
      case POPPLER_ANNOT_UNDERLINE:
        return "Underline";
      case POPPLER_ANNOT_SQUIGGLY:
        return "Squiggly";
      case POPPLER_ANNOT_STRIKE_OUT:
        return "Strike Out";
      case POPPLER_ANNOT_STAMP:
        return "Stamp";
      case POPPLER_ANNOT_CARET:
        return "Caret";
      case POPPLER_ANNOT_INK:
        return "Ink";
      case POPPLER_ANNOT_POPUP:
        return "Popup";
      case POPPLER_ANNOT_FILE_ATTACHMENT:
        return "File Attachment";
      case POPPLER_ANNOT_SOUND:
        return "Sound";
      case POPPLER_ANNOT_MOVIE:
        return "Movie";
      case POPPLER_ANNOT_WIDGET:
        return "Widget";
      case POPPLER_ANNOT_SCREEN:
        return "Screen";
      case POPPLER_ANNOT_PRINTER_MARK:
        return "Printer Mark";
      case POPPLER_ANNOT_TRAP_NET:
        return "Trap Net";
      case POPPLER_ANNOT_WATERMARK:
        return "Watermark";
      case POPPLER_ANNOT_3D:
        return "3D";
      default:
        break;
  }

  return "Unknown";
}

GdkPixbuf *
get_annot_color (PopplerAnnot *poppler_annot)
{
    PopplerColor *poppler_color;

    if ((poppler_color = poppler_annot_get_color (poppler_annot))) {
        GdkPixbuf *pixbuf;
	gint rowstride, num, x;
	guchar *pixels;

        pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
                                 FALSE, 8,
                                 64, 16);

	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
            
	num = gdk_pixbuf_get_width (pixbuf) *
                gdk_pixbuf_get_height (pixbuf);

	for (x = 0; x < num; x++) {
          pixels[0] = poppler_color->red;
	  pixels[1] = poppler_color->green;
	  pixels[2] = poppler_color->blue;
	  pixels += 3;
	}

        g_free (poppler_color);

        return pixbuf;
    }

    return NULL;
}

gchar *
get_markup_date (PopplerAnnotMarkup *poppler_annot)
{
    GDate *date;
    struct tm t;
    time_t timet;

    date = poppler_annot_markup_get_date (poppler_annot);
    if (!date)
	    return NULL;

    g_date_to_struct_tm (date, &t);
    g_date_free (date);

    timet = mktime (&t);
    return timet == (time_t) - 1 ? NULL : pgd_format_date (timet);
}

const gchar *
get_markup_reply_to (PopplerAnnotMarkup *poppler_annot)
{
    switch (poppler_annot_markup_get_reply_to (poppler_annot))
    {
      case POPPLER_ANNOT_MARKUP_REPLY_TYPE_R:
        return "Type R";
      case POPPLER_ANNOT_MARKUP_REPLY_TYPE_GROUP:
        return "Type Group";
      default:
        break;
    }

  return "Unknown";
}

const gchar *
get_markup_external_data (PopplerAnnotMarkup *poppler_annot)
{
    switch (poppler_annot_markup_get_external_data (poppler_annot))
    {
      case POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_3D:
        return "Markup 3D";
      default:
        break;
    }

  return "Unknown";
}

const gchar *
get_text_state (PopplerAnnotText *poppler_annot)
{
    switch (poppler_annot_text_get_state (poppler_annot))
    {
      case POPPLER_ANNOT_TEXT_STATE_MARKED:
        return "Marked";
      case POPPLER_ANNOT_TEXT_STATE_UNMARKED:
        return "Unmarked";
      case POPPLER_ANNOT_TEXT_STATE_ACCEPTED:
        return "Accepted";
      case POPPLER_ANNOT_TEXT_STATE_REJECTED:
        return "Rejected";
      case POPPLER_ANNOT_TEXT_STATE_CANCELLED:
        return "Cancelled";
      case POPPLER_ANNOT_TEXT_STATE_COMPLETED:
        return "Completed";
      case POPPLER_ANNOT_TEXT_STATE_NONE:
        return "None";
      case POPPLER_ANNOT_TEXT_STATE_UNKNOWN:
        return "Unknown";
      default:
        break;
    }

  return "Unknown";
}

const gchar *
get_free_text_quadding (PopplerAnnotFreeText *poppler_annot)
{
    switch (poppler_annot_free_text_get_quadding (poppler_annot))
    {
      case POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED:
        return "Left Justified";
      case POPPLER_ANNOT_FREE_TEXT_QUADDING_CENTERED:
        return "Centered";
      case POPPLER_ANNOT_FREE_TEXT_QUADDING_RIGHT_JUSTIFIED:
        return "Right Justified";
      default:
        break;
    }

  return "Unknown";
}

gchar *
get_free_text_callout_line (PopplerAnnotFreeText *poppler_annot)
{
    PopplerAnnotCalloutLine *callout;
    gdouble x1, y1, x2, y2;
    gchar *text;
    
    if ((callout = poppler_annot_free_text_get_callout_line (poppler_annot))) {
        text = g_strdup_printf ("%f,%f,%f,%f", callout->x1,
                                               callout->y1,
                                               callout->x2,
                                               callout->y2);
        if (callout->multiline)
            text = g_strdup_printf ("%s,%f,%f", text,
                                                callout->x3,
                                                callout->y3);

        return text;
    }
    
    return NULL;
}

static void
pgd_annot_view_set_annot_markup (GtkWidget          *table,
                                 PopplerAnnotMarkup *markup,
                                 gint               *row)
{
    gchar *text;
    PopplerRectangle rect;

    text = poppler_annot_markup_get_label (markup);
    pgd_table_add_property (GTK_TABLE (table), "<b>Label:</b>", text, row);
    g_free (text);

    if (poppler_annot_markup_has_popup (markup)) {
	    pgd_table_add_property (GTK_TABLE (table), "<b>Popup is open:</b>",
				    poppler_annot_markup_get_popup_is_open (markup) ? "Yes" : "No", row);

	    poppler_annot_markup_get_popup_rectangle (markup, &rect);
	    text = g_strdup_printf ("X1: %.2f, Y1: %.2f, X2: %.2f, Y2: %.2f",
				    rect.x1, rect.y1, rect.x2, rect.y2);
	    pgd_table_add_property (GTK_TABLE (table), "<b>Popup Rectangle:</b>", text, row);
	    g_free (text);
    }

    text = g_strdup_printf ("%f", poppler_annot_markup_get_opacity (markup));
    pgd_table_add_property (GTK_TABLE (table), "<b>Opacity:</b>", text, row);
    g_free (text);

    text = get_markup_date (markup);
    pgd_table_add_property (GTK_TABLE (table), "<b>Date:</b>", text, row);
    g_free (text);

    text = poppler_annot_markup_get_subject (markup);
    pgd_table_add_property (GTK_TABLE (table), "<b>Subject:</b>", text, row);
    g_free (text);

    pgd_table_add_property (GTK_TABLE (table), "<b>Reply To:</b>", get_markup_reply_to (markup), row);

    pgd_table_add_property (GTK_TABLE (table), "<b>External Data:</b>", get_markup_external_data (markup), row);
}

static void
pgd_annot_view_set_annot_text (GtkWidget        *table,
                               PopplerAnnotText *annot,
                               gint             *row)
{
    gchar *text;

    pgd_table_add_property (GTK_TABLE (table), "<b>Is open:</b>",
                            poppler_annot_text_get_is_open (annot) ? "Yes" : "No", row);

    text = poppler_annot_text_get_icon (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Icon:</b>", text, row);
    g_free (text);

    pgd_table_add_property (GTK_TABLE (table), "<b>State:</b>", get_text_state (annot), row);
}

static void
pgd_annot_view_set_annot_free_text (GtkWidget            *table,
                                    PopplerAnnotFreeText *annot,
                                    gint                 *row)
{
    gchar *text;

    pgd_table_add_property (GTK_TABLE (table), "<b>Quadding:</b>", get_free_text_quadding (annot), row);

    text = get_free_text_callout_line (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Callout:</b>", text, row);
    g_free (text);
}

static void
pgd_annots_file_attachment_save_dialog_response (GtkFileChooser    *file_chooser,
						 gint               response,
						 PopplerAttachment *attachment)
{
    gchar  *filename;
    GError *error = NULL;

    if (response != GTK_RESPONSE_ACCEPT) {
        g_object_unref (attachment);
	gtk_widget_destroy (GTK_WIDGET (file_chooser));
	return;
    }

    filename = gtk_file_chooser_get_filename (file_chooser);
    if (!poppler_attachment_save (attachment, filename, &error)) {
        g_warning ("%s", error->message);
	g_error_free (error);
    }
    g_free (filename);
    g_object_unref (attachment);
    gtk_widget_destroy (GTK_WIDGET (file_chooser));
}

static void
pgd_annot_save_file_attachment_button_clicked (GtkButton                  *button,
					       PopplerAnnotFileAttachment *annot)
{
    GtkWidget         *file_chooser;
    PopplerAttachment *attachment;

    attachment = poppler_annot_file_attachment_get_attachment (annot);
    if (!attachment)
        return;

    file_chooser = gtk_file_chooser_dialog_new ("Save attachment",
						GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (button))),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
						NULL);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (file_chooser), attachment->name);
    g_signal_connect (G_OBJECT (file_chooser), "response",
		      G_CALLBACK (pgd_annots_file_attachment_save_dialog_response),
		      (gpointer) attachment);
    gtk_widget_show (file_chooser);
}

static void
pgd_annot_view_set_annot_file_attachment (GtkWidget                  *table,
					  PopplerAnnotFileAttachment *annot,
					  gint                       *row)
{
    GtkWidget *button;
    gchar *text;

    text = poppler_annot_file_attachment_get_name (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Attachment Name:</b>", text, row);
    g_free (text);

    button = gtk_button_new_with_label ("Save Attachment");
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (pgd_annot_save_file_attachment_button_clicked),
		      (gpointer)annot);
    pgd_table_add_property_with_custom_widget (GTK_TABLE (table), "<b>File Attachment:</b>", button, row);
    gtk_widget_show (button);

}

static void
pgd_annot_view_set_annot_movie (GtkWidget         *table,
				PopplerAnnotMovie *annot,
				gint              *row)
{
    GtkWidget *movie_view;
    gchar *text;

    text = poppler_annot_movie_get_title (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Movie Title:</b>", text, row);
    g_free (text);

    movie_view = pgd_movie_view_new ();
    pgd_movie_view_set_movie (movie_view, poppler_annot_movie_get_movie (annot));
    pgd_table_add_property_with_custom_widget (GTK_TABLE (table), "<b>Movie:</b>", movie_view, row);
    gtk_widget_show (movie_view);
}

static void
pgd_annot_view_set_annot_screen (GtkWidget          *table,
				 PopplerAnnotScreen *annot,
				 gint               *row)
{
    GtkWidget *action_view;

    action_view = pgd_action_view_new (NULL);
    pgd_action_view_set_action (action_view, poppler_annot_screen_get_action (annot));
    pgd_table_add_property_with_custom_widget (GTK_TABLE (table), "<b>Action:</b>", action_view, row);
    gtk_widget_show (action_view);
}

static void
pgd_annot_view_set_annot (GtkWidget    *annot_view,
                          PopplerAnnot *annot)
{
    GtkWidget  *alignment;
    GtkWidget  *table;
    GEnumValue *enum_value;
    gint        row = 0;
    gchar      *text, *warning;
    time_t      timet;

    alignment = gtk_bin_get_child (GTK_BIN (annot_view));
    if (alignment) {
        gtk_container_remove (GTK_CONTAINER (annot_view), alignment);
    }

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
    gtk_container_add (GTK_CONTAINER (annot_view), alignment);
    gtk_widget_show (alignment);

    if (!annot)
        return;

    table = gtk_table_new (10, 2, FALSE);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);

    text = poppler_annot_get_contents (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Contents:</b>", text, &row);
    g_free (text);

    text = poppler_annot_get_name (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Name:</b>", text, &row);
    g_free (text);

    text = poppler_annot_get_modified (annot);
    if (poppler_date_parse (text, &timet)) {
	    g_free (text);
	    text = pgd_format_date (timet);
    }
    pgd_table_add_property (GTK_TABLE (table), "<b>Modified:</b>", text, &row);
    g_free (text);

    text = g_strdup_printf ("%d", poppler_annot_get_flags (annot));
    pgd_table_add_property (GTK_TABLE (table), "<b>Flags:</b>", text, &row);
    g_free (text);

    text = g_strdup_printf ("%d", poppler_annot_get_page_index (annot));
    pgd_table_add_property (GTK_TABLE (table), "<b>Page:</b>", text, &row);
    g_free (text);

    if (POPPLER_IS_ANNOT_MARKUP (annot))
        pgd_annot_view_set_annot_markup (table, POPPLER_ANNOT_MARKUP (annot), &row);

    switch (poppler_annot_get_annot_type (annot))
    {
        case POPPLER_ANNOT_TEXT:
          pgd_annot_view_set_annot_text (table, POPPLER_ANNOT_TEXT (annot), &row);
          break;
        case POPPLER_ANNOT_FREE_TEXT:
          pgd_annot_view_set_annot_free_text (table, POPPLER_ANNOT_FREE_TEXT (annot), &row);
          break;
        case POPPLER_ANNOT_FILE_ATTACHMENT:
	  pgd_annot_view_set_annot_file_attachment (table, POPPLER_ANNOT_FILE_ATTACHMENT (annot), &row);
	  break;
        case POPPLER_ANNOT_MOVIE:
	  pgd_annot_view_set_annot_movie (table, POPPLER_ANNOT_MOVIE (annot), &row);
	  break;
        case POPPLER_ANNOT_SCREEN:
	  pgd_annot_view_set_annot_screen (table, POPPLER_ANNOT_SCREEN (annot), &row);
	  break;
        default:
          break;
    }

    gtk_container_add (GTK_CONTAINER (alignment), table);
    gtk_widget_show (table);
}

static void
pgd_annots_get_annots (GtkWidget     *button,
                       PgdAnnotsDemo *demo)
{
    GList       *mapping, *l;
    gint         n_fields;
    GTimer      *timer;

    gtk_list_store_clear (demo->model);
    pgd_annot_view_set_annot (demo->annot_view, NULL);

    if (demo->page) {
        g_object_unref (demo->page);
        demo->page = NULL;
    }

    demo->page = poppler_document_get_page (demo->doc, demo->num_page);
    if (!demo->page)
        return;

    timer = g_timer_new ();
    mapping = poppler_page_get_annot_mapping (demo->page);
    g_timer_stop (timer);

    n_fields = g_list_length (mapping);
    if (n_fields > 0) {
        gchar *str;

        str = g_strdup_printf ("<i>%d annots found in %.4f seconds</i>",
                               n_fields, g_timer_elapsed (timer, NULL));
        gtk_label_set_markup (GTK_LABEL (demo->timer_label), str);
        g_free (str);
    } else {
        gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No annots found</i>");
    }

    g_timer_destroy (timer);

    for (l = mapping; l; l = g_list_next (l)) {
        PopplerAnnotMapping *amapping;
        GtkTreeIter          iter;
        gchar               *x1, *y1, *x2, *y2;
        GdkPixbuf           *pixbuf;
	PopplerAnnotFlag     flags;

        amapping = (PopplerAnnotMapping *) l->data;

        x1 = g_strdup_printf ("%.2f", amapping->area.x1);
        y1 = g_strdup_printf ("%.2f", amapping->area.y1);
        x2 = g_strdup_printf ("%.2f", amapping->area.x2);
        y2 = g_strdup_printf ("%.2f", amapping->area.y2);

        pixbuf = get_annot_color (amapping->annot);
	flags = poppler_annot_get_flags (amapping->annot);

        gtk_list_store_append (demo->model, &iter);
        gtk_list_store_set (demo->model, &iter,
                            ANNOTS_X1_COLUMN, x1,
                            ANNOTS_Y1_COLUMN, y1,
                            ANNOTS_X2_COLUMN, x2,
                            ANNOTS_Y2_COLUMN, y2,
                            ANNOTS_TYPE_COLUMN, get_annot_type (amapping->annot),
                            ANNOTS_COLOR_COLUMN, pixbuf,
			    ANNOTS_FLAG_INVISIBLE_COLUMN, (flags & POPPLER_ANNOT_FLAG_INVISIBLE),
			    ANNOTS_FLAG_HIDDEN_COLUMN, (flags & POPPLER_ANNOT_FLAG_HIDDEN),
			    ANNOTS_FLAG_PRINT_COLUMN, (flags & POPPLER_ANNOT_FLAG_PRINT),
                            ANNOTS_COLUMN, amapping->annot,
                           -1);

        if (pixbuf)
            g_object_unref (pixbuf);

        g_free (x1);
        g_free (y1);
        g_free (x2);
        g_free (y2);
    }

    poppler_page_free_annot_mapping (mapping);
}

static void
pgd_annots_page_selector_value_changed (GtkSpinButton *spinbutton,
                                        PgdAnnotsDemo *demo)
{
    demo->num_page = (gint) gtk_spin_button_get_value (spinbutton) - 1;
}

static void
pgd_annots_selection_changed (GtkTreeSelection *treeselection,
                              PgdAnnotsDemo    *demo)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;

    if (gtk_tree_selection_get_selected (treeselection, &model, &iter)) {
        PopplerAnnot *annot;

        gtk_tree_model_get (model, &iter,
                            ANNOTS_COLUMN, &annot,
                           -1);
        pgd_annot_view_set_annot (demo->annot_view, annot);
        g_object_unref (annot);
    }
}

static void
pgd_annots_add_annot (GtkWidget     *button,
		      PgdAnnotsDemo *demo)
{
    GtkWidget   *hbox, *vbox;
    GtkWidget   *type_selector;
    GtkWidget   *label;
    GtkWidget   *rect_hbox;
    GtkWidget   *rect_x1, *rect_y1, *rect_x2, *rect_y2;
    GtkWidget   *dialog;
    PopplerPage *page;
    gdouble      width, height;
    PopplerAnnot *annot;
    PopplerRectangle rect;

    page = poppler_document_get_page (demo->doc, demo->num_page);
    if (!page)
	    return;
    poppler_page_get_size (page, &width, &height);

    dialog = gtk_dialog_new_with_buttons ("Add new annotation",
					  GTK_WINDOW (gtk_widget_get_toplevel (button)),
					  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					  "Add annotation", GTK_RESPONSE_ACCEPT,
					  NULL);

    vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    type_selector = gtk_combo_box_new_text ();
    gtk_combo_box_append_text (GTK_COMBO_BOX (type_selector), "POPPLER_ANNOT_UNKNOWN");
    gtk_combo_box_append_text (GTK_COMBO_BOX (type_selector), "POPPLER_ANNOT_TEXT");
    gtk_combo_box_set_active (GTK_COMBO_BOX (type_selector), 1);
    gtk_box_pack_start (GTK_BOX (vbox), type_selector, TRUE, TRUE, 0);
    gtk_widget_show (type_selector);

    hbox = gtk_hbox_new (FALSE, 6);

    rect_hbox = gtk_hbox_new (FALSE, 6);

    label = gtk_label_new ("x1:");
    gtk_box_pack_start (GTK_BOX (rect_hbox), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    rect_x1 = gtk_spin_button_new_with_range (0, width, 1.0);
    gtk_box_pack_start (GTK_BOX (rect_hbox), rect_x1, TRUE, TRUE, 0);
    gtk_widget_show (rect_x1);

    gtk_box_pack_start (GTK_BOX (hbox), rect_hbox, FALSE, TRUE, 0);
    gtk_widget_show (rect_hbox);

    rect_hbox = gtk_hbox_new (FALSE, 6);

    label = gtk_label_new ("x2:");
    gtk_box_pack_start (GTK_BOX (rect_hbox), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    rect_x2 = gtk_spin_button_new_with_range (0, width, 1.0);
    gtk_box_pack_start (GTK_BOX (rect_hbox), rect_x2, TRUE, TRUE, 0);
    gtk_widget_show (rect_x2);

    gtk_box_pack_start (GTK_BOX (hbox), rect_hbox, FALSE, TRUE, 0);
    gtk_widget_show (rect_hbox);

    rect_hbox = gtk_hbox_new (FALSE, 6);

    label = gtk_label_new ("y1:");
    gtk_box_pack_start (GTK_BOX (rect_hbox), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    rect_y1 = gtk_spin_button_new_with_range (0, height, 1.0);
    gtk_box_pack_start (GTK_BOX (rect_hbox), rect_y1, TRUE, TRUE, 0);
    gtk_widget_show (rect_y1);

    gtk_box_pack_start (GTK_BOX (hbox), rect_hbox, FALSE, TRUE, 0);
    gtk_widget_show (rect_hbox);

    rect_hbox = gtk_hbox_new (FALSE, 6);

    label = gtk_label_new ("y2:");
    gtk_box_pack_start (GTK_BOX (rect_hbox), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    rect_y2 = gtk_spin_button_new_with_range (0, height, 1.0);
    gtk_box_pack_start (GTK_BOX (rect_hbox), rect_y2, TRUE, TRUE, 0);
    gtk_widget_show (rect_y2);

    gtk_box_pack_start (GTK_BOX (hbox), rect_hbox, FALSE, TRUE, 0);
    gtk_widget_show (rect_hbox);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
    gtk_widget_show (hbox);

    gtk_dialog_run (GTK_DIALOG (dialog));

    rect.x1 = gtk_spin_button_get_value (GTK_SPIN_BUTTON (rect_x1));
    rect.x2 = gtk_spin_button_get_value (GTK_SPIN_BUTTON (rect_x2));
    rect.y1 = height - gtk_spin_button_get_value (GTK_SPIN_BUTTON (rect_y2));
    rect.y2 = height - gtk_spin_button_get_value (GTK_SPIN_BUTTON (rect_y1));
    annot = poppler_annot_text_new (demo->doc, &rect);
    poppler_page_add_annot (page, annot);

    g_object_unref (page);

    gtk_widget_destroy (dialog);
}

GtkWidget *
pgd_annots_create_widget (PopplerDocument *document)
{
    PgdAnnotsDemo    *demo;
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

    demo = g_new0 (PgdAnnotsDemo, 1);

    demo->doc = g_object_ref (document);

    n_pages = poppler_document_get_n_pages (document);

    vbox = gtk_vbox_new (FALSE, 12);

    hbox = gtk_hbox_new (FALSE, 6);

    label = gtk_label_new ("Page:");
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
    gtk_widget_show (label);

    page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
    g_signal_connect (G_OBJECT (page_selector), "value-changed",
                      G_CALLBACK (pgd_annots_page_selector_value_changed),
                      (gpointer) demo);
    gtk_box_pack_start (GTK_BOX (hbox), page_selector, FALSE, TRUE, 0);
    gtk_widget_show (page_selector);

    str = g_strdup_printf ("of %d", n_pages);
    label = gtk_label_new (str);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
    gtk_widget_show (label);
    g_free (str);

    button = gtk_button_new_with_label ("Get Annots");
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (pgd_annots_get_annots),
                      (gpointer) demo);
    gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_widget_show (button);

    button = gtk_button_new_with_label ("Add Annot");
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (pgd_annots_add_annot),
		      (gpointer) demo);
    gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_widget_show (button);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show (hbox);

    demo->timer_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No annots found</i>");
    g_object_set (G_OBJECT (demo->timer_label), "xalign", 1.0, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), demo->timer_label, FALSE, TRUE, 0);
    gtk_widget_show (demo->timer_label);

    hpaned = gtk_hpaned_new ();

    demo->annot_view = pgd_annot_view_new ();

    swindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    demo->model = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING,
                                      G_TYPE_STRING, G_TYPE_STRING,
                                      G_TYPE_STRING, G_TYPE_STRING,
                                      GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN,
				      G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
				      G_TYPE_OBJECT);
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (demo->model));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_X1_COLUMN, "X1",
                                                 renderer,
                                                 "text", ANNOTS_X1_COLUMN,
                                                 NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_Y1_COLUMN, "Y1",
                                                 renderer,
                                                 "text", ANNOTS_Y1_COLUMN,
                                                 NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_X2_COLUMN, "X2",
                                                 renderer,
                                                 "text", ANNOTS_X2_COLUMN,
                                                 NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_Y2_COLUMN, "Y2",
                                                 renderer,
                                                 "text", ANNOTS_Y2_COLUMN,
                                                 NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_TYPE_COLUMN, "Type",
                                                 renderer,
                                                 "text", ANNOTS_TYPE_COLUMN,
                                                 NULL);

    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_COLOR_COLUMN, "Color",
                                                 renderer,
                                                 "pixbuf", ANNOTS_COLOR_COLUMN,
                                                 NULL);

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_FLAG_INVISIBLE_COLUMN, "Invisible",
                                                 renderer,
                                                 "active", ANNOTS_FLAG_INVISIBLE_COLUMN,
                                                 NULL);

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_FLAG_HIDDEN_COLUMN, "Hidden",
                                                 renderer,
                                                 "active", ANNOTS_FLAG_HIDDEN_COLUMN,
                                                 NULL);

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 ANNOTS_FLAG_PRINT_COLUMN, "Print",
                                                 renderer,
                                                 "active", ANNOTS_FLAG_PRINT_COLUMN,
                                                 NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (pgd_annots_selection_changed),
                      (gpointer) demo);

    gtk_container_add (GTK_CONTAINER (swindow), treeview);
    gtk_widget_show (treeview);

    gtk_paned_add1 (GTK_PANED (hpaned), swindow);
    gtk_widget_show (swindow);

    gtk_paned_add2 (GTK_PANED (hpaned), demo->annot_view);
    gtk_widget_show (demo->annot_view);

    gtk_paned_set_position (GTK_PANED (hpaned), 300);

    gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
    gtk_widget_show (hpaned);

    g_object_weak_ref (G_OBJECT (vbox),
                       (GWeakNotify)pgd_annots_free,
                       demo);

    return vbox;
}
