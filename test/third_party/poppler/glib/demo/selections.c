/*
 * Copyright (C) 2010 Carlos Garcia Campos  <carlosgc@gnome.org>
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
#include <cairo.h>

#include "selections.h"

typedef struct {
	PopplerDocument      *doc;

	/* Properties */
	gint                  page_index;
	gdouble               scale;

	GtkWidget            *swindow;
	GtkWidget            *darea;
	GtkWidget            *fg_color_button;
	GtkWidget            *bg_color_button;

	PopplerPage          *page;
	cairo_surface_t      *surface;

	GdkPoint              start;
	GdkPoint              stop;
	PopplerRectangle      doc_area;
	cairo_surface_t      *selection_surface;
	PopplerSelectionStyle style;
	PopplerColor          glyph_color;
	PopplerColor          background_color;
	guint                 selections_idle;
	cairo_region_t       *selection_region;
	cairo_region_t       *selected_region;
	GdkCursorType         cursor;
	gchar                *selected_text;
} PgdSelectionsDemo;

static void
pgd_selections_clear_selections (PgdSelectionsDemo *demo)
{
	demo->start.x = -1;

	if (demo->selection_surface) {
		cairo_surface_destroy (demo->selection_surface);
		demo->selection_surface = NULL;
	}

	if (demo->selection_region) {
		cairo_region_destroy (demo->selection_region);
		demo->selection_region = NULL;
	}

	if (demo->selected_text) {
		g_free (demo->selected_text);
		demo->selected_text = NULL;
	}

	if (demo->selected_region) {
		cairo_region_destroy (demo->selected_region);
		demo->selected_region = NULL;
	}
}

static void
pgd_selections_free (PgdSelectionsDemo *demo)
{
	if (!demo)
		return;

	if (demo->selections_idle > 0) {
		g_source_remove (demo->selections_idle);
		demo->selections_idle = 0;
	}

	if (demo->doc) {
		g_object_unref (demo->doc);
		demo->doc = NULL;
	}

	if (demo->page) {
		g_object_unref (demo->page);
		demo->page = NULL;
	}

	if (demo->surface) {
		cairo_surface_destroy (demo->surface);
		demo->surface = NULL;
	}

	pgd_selections_clear_selections (demo);

	g_free (demo);
}

static void
pgd_selections_update_selection_region (PgdSelectionsDemo *demo)
{
	PopplerRectangle area = { 0, 0, 0, 0 };

	if (demo->selection_region)
		cairo_region_destroy (demo->selection_region);

	poppler_page_get_size (demo->page, &area.x2, &area.y2);
	demo->selection_region = poppler_page_get_selected_region (demo->page,
                                                                   1.0,
                                                                   POPPLER_SELECTION_GLYPH,
                                                                   &area);
}

static void
pgd_selections_update_seleted_text (PgdSelectionsDemo *demo)
{
	GList *region;
	gchar *text;

	if (demo->selected_region)
		cairo_region_destroy (demo->selected_region);
	demo->selected_region = poppler_page_get_selected_region (demo->page,
                                                                  1.0,
                                                                  demo->style,
                                                                  &demo->doc_area);
	if (demo->selected_text)
		g_free (demo->selected_text);
	demo->selected_text = NULL;

	text = poppler_page_get_selected_text (demo->page,
					       demo->style,
					       &demo->doc_area);
	if (text) {
		demo->selected_text = g_utf8_normalize (text, -1, G_NORMALIZE_NFKC);
		g_free (text);
	}
}

static void
pgd_selections_update_cursor (PgdSelectionsDemo *demo,
			      GdkCursorType      cursor_type)
{
	GdkWindow *window = gtk_widget_get_window (demo->darea);
	GdkCursor *cursor = NULL;

	if (cursor_type == demo->cursor)
		return;

	if (cursor_type != GDK_LAST_CURSOR) {
		cursor = gdk_cursor_new_for_display (gtk_widget_get_display (demo->darea),
						     cursor_type);
	}

	demo->cursor = cursor_type;

	gdk_window_set_cursor (window, cursor);
	gdk_flush ();
	if (cursor)
		gdk_cursor_unref (cursor);
}

static gboolean
pgd_selections_render_selections (PgdSelectionsDemo *demo)
{
	PopplerRectangle doc_area;
	gdouble page_width, page_height;
	cairo_t *cr;

	if (!demo->page || demo->start.x == -1) {
		demo->selections_idle = 0;

		return FALSE;
	}

	poppler_page_get_size (demo->page, &page_width, &page_height);
	page_width *= demo->scale;
	page_height *= demo->scale;

	doc_area.x1 = demo->start.x / demo->scale;
	doc_area.y1 = demo->start.y / demo->scale;
	doc_area.x2 = demo->stop.x / demo->scale;
	doc_area.y2 = demo->stop.y / demo->scale;

	if (demo->selection_surface)
		cairo_surface_destroy (demo->selection_surface);
	demo->selection_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
							      page_width, page_height);
	cr = cairo_create (demo->selection_surface);
	if (demo->scale != 1.0)
		cairo_scale (cr, demo->scale, demo->scale);
	poppler_page_render_selection (demo->page, cr,
				       &doc_area, &demo->doc_area,
				       demo->style,
				       &demo->glyph_color,
				       &demo->background_color);
	cairo_destroy (cr);

	demo->doc_area = doc_area;
	gtk_widget_queue_draw (demo->darea);

	demo->selections_idle = 0;

	return FALSE;
}

static gboolean
pgd_selections_drawing_area_expose (GtkWidget         *area,
				    GdkEventExpose    *event,
				    PgdSelectionsDemo *demo)
{
	cairo_t *cr;

	if (!demo->surface)
		return FALSE;

	gdk_window_clear (gtk_widget_get_window (area));

	cr = gdk_cairo_create (gtk_widget_get_window (area));

	cairo_save (cr);
	cairo_set_source_surface (cr, demo->surface, 0, 0);
	cairo_paint (cr);
	cairo_restore (cr);

	if (demo->selection_surface) {
		cairo_set_source_surface (cr, demo->selection_surface, 0, 0);
		cairo_paint (cr);
	}

	cairo_destroy (cr);

	return TRUE;
}

static gboolean
pgd_selections_drawing_area_button_press (GtkWidget         *area,
					  GdkEventButton    *event,
					  PgdSelectionsDemo *demo)
{
	if (!demo->page)
		return FALSE;

	if (event->button != 1)
		return FALSE;

	demo->start.x = event->x;
	demo->start.y = event->y;
	demo->stop = demo->start;

	switch (event->type) {
	case GDK_2BUTTON_PRESS:
		demo->style = POPPLER_SELECTION_WORD;
		break;
	case GDK_3BUTTON_PRESS:
		demo->style = POPPLER_SELECTION_LINE;
		break;
	default:
		demo->style = POPPLER_SELECTION_GLYPH;
	}

	pgd_selections_render_selections (demo);

	return TRUE;
}

static gboolean
pgd_selections_drawing_area_motion_notify (GtkWidget         *area,
					   GdkEventMotion    *event,
					   PgdSelectionsDemo *demo)
{
	if (!demo->page)
		return FALSE;

	if (demo->start.x != -1) {
		demo->stop.x = event->x;
		demo->stop.y = event->y;
		if (demo->selections_idle == 0) {
			demo->selections_idle =
				g_idle_add ((GSourceFunc)pgd_selections_render_selections,
					    demo);
		}
	} else {
		gboolean over_text;

		over_text = cairo_region_contains_point (demo->selection_region,
                                                         event->x / demo->scale,
                                                         event->y / demo->scale);
		pgd_selections_update_cursor (demo, over_text ? GDK_XTERM : GDK_LAST_CURSOR);
	}

	return TRUE;
}

static gboolean
pgd_selections_drawing_area_button_release (GtkWidget         *area,
					    GdkEventButton    *event,
					    PgdSelectionsDemo *demo)
{
	if (!demo->page)
		return FALSE;

	if (event->button != 1)
		return FALSE;

	if (demo->start.x != -1)
		pgd_selections_update_seleted_text (demo);

	demo->start.x = -1;

	if (demo->selections_idle > 0) {
		g_source_remove (demo->selections_idle);
		demo->selections_idle = 0;
	}

	return TRUE;
}

static void
pgd_selections_drawing_area_realize (GtkWidget         *area,
				     PgdSelectionsDemo *demo)
{
	GtkStyle *style = gtk_widget_get_style (area);

	gtk_widget_add_events (area,
			       GDK_POINTER_MOTION_HINT_MASK |
			       GDK_BUTTON1_MOTION_MASK |
			       GDK_BUTTON_PRESS_MASK |
			       GDK_BUTTON_RELEASE_MASK);
	g_object_set (area, "has-tooltip", TRUE, NULL);

	gtk_color_button_set_color (GTK_COLOR_BUTTON (demo->fg_color_button),
				    &style->text[GTK_STATE_SELECTED]);
	gtk_color_button_set_color (GTK_COLOR_BUTTON (demo->bg_color_button),
				    &style->base[GTK_STATE_SELECTED]);
}

static gboolean
pgd_selections_drawing_area_query_tooltip (GtkWidget         *area,
					   gint               x,
					   gint               y,
					   gboolean           keyboard_mode,
					   GtkTooltip        *tooltip,
					   PgdSelectionsDemo *demo)
{
	gboolean over_selection;

	if (!demo->selected_text)
		return FALSE;

	over_selection = cairo_region_contains_point (demo->selected_region,
                                                      x / demo->scale,
                                                      y / demo->scale);

	if (over_selection) {
		GdkRectangle selection_area;

		cairo_region_get_extents (demo->selected_region,
                                          (cairo_rectangle_int_t *)&selection_area);
		selection_area.x *= demo->scale;
		selection_area.y *= demo->scale;
		selection_area.width *= demo->scale;
		selection_area.height *= demo->scale;

		gtk_tooltip_set_text (tooltip, demo->selected_text);
		gtk_tooltip_set_tip_area (tooltip, &selection_area);

		return TRUE;
	}

	return FALSE;
}

static void
pgd_selections_render (GtkButton         *button,
		       PgdSelectionsDemo *demo)
{
	gdouble  page_width, page_height;
	cairo_t *cr;

	if (!demo->page)
		demo->page = poppler_document_get_page (demo->doc, demo->page_index);

	if (!demo->page)
		return;

	pgd_selections_clear_selections (demo);
	pgd_selections_update_selection_region (demo);

	if (demo->surface)
		cairo_surface_destroy (demo->surface);
	demo->surface = NULL;

	poppler_page_get_size (demo->page, &page_width, &page_height);
	page_width *= demo->scale;
	page_height *= demo->scale;

	demo->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
						    page_width, page_height);
	cr = cairo_create (demo->surface);

	cairo_save (cr);

	if (demo->scale != 1.0)
		cairo_scale (cr, demo->scale, demo->scale);

	poppler_page_render (demo->page, cr);
	cairo_restore (cr);

	cairo_set_operator (cr, CAIRO_OPERATOR_DEST_OVER);
	cairo_set_source_rgb (cr, 1., 1., 1.);
	cairo_paint (cr);

	cairo_destroy (cr);

	gtk_widget_set_size_request (demo->darea, page_width, page_height);
	gtk_widget_queue_draw (demo->darea);
}

static void
pgd_selections_page_selector_value_changed (GtkSpinButton     *spinbutton,
					    PgdSelectionsDemo *demo)
{
	demo->page_index = (gint)gtk_spin_button_get_value (spinbutton) - 1;
	if (demo->page)
		g_object_unref (demo->page);
	demo->page = NULL;
}

static void
pgd_selections_scale_selector_value_changed (GtkSpinButton     *spinbutton,
					     PgdSelectionsDemo *demo)
{
	demo->scale = gtk_spin_button_get_value (spinbutton);
}

static void
pgd_selections_fg_color_changed (GtkColorButton    *button,
				 GParamSpec        *pspec,
				 PgdSelectionsDemo *demo)
{
	GdkColor color;

	gtk_color_button_get_color (GTK_COLOR_BUTTON (button), &color);
	demo->glyph_color.red = color.red;
	demo->glyph_color.green = color.green;
	demo->glyph_color.blue = color.blue;
}

static void
pgd_selections_bg_color_changed (GtkColorButton    *button,
				 GParamSpec        *pspec,
				 PgdSelectionsDemo *demo)
{
	GdkColor color;

	gtk_color_button_get_color (GTK_COLOR_BUTTON (button), &color);
	demo->background_color.red = color.red;
	demo->background_color.green = color.green;
	demo->background_color.blue = color.blue;
}

GtkWidget *
pgd_selections_properties_selector_create (PgdSelectionsDemo *demo)
{
	GtkWidget *hbox, *vbox;
	GtkWidget *label;
	GtkWidget *page_hbox, *page_selector;
	GtkWidget *scale_hbox, *scale_selector;
	GtkWidget *rotate_hbox, *rotate_selector;
	GtkWidget *color_hbox;
	GtkWidget *button;
	gint       n_pages;
	gchar     *str;

	n_pages = poppler_document_get_n_pages (demo->doc);

	vbox = gtk_vbox_new (FALSE, 6);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	page_hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Page:");
	gtk_box_pack_start (GTK_BOX (page_hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
	g_signal_connect (G_OBJECT (page_selector), "value-changed",
			  G_CALLBACK (pgd_selections_page_selector_value_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (page_hbox), page_selector, TRUE, TRUE, 0);
	gtk_widget_show (page_selector);

	str = g_strdup_printf ("of %d", n_pages);
	label = gtk_label_new (str);
	gtk_box_pack_start (GTK_BOX (page_hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);
	g_free (str);

	gtk_box_pack_start (GTK_BOX (hbox), page_hbox, FALSE, TRUE, 0);
	gtk_widget_show (page_hbox);

	scale_hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Scale:");
	gtk_box_pack_start (GTK_BOX (scale_hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	scale_selector = gtk_spin_button_new_with_range (0, 10.0, 0.1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (scale_selector), 1.0);
	g_signal_connect (G_OBJECT (scale_selector), "value-changed",
			  G_CALLBACK (pgd_selections_scale_selector_value_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (scale_hbox), scale_selector, TRUE, TRUE, 0);
	gtk_widget_show (scale_selector);

	gtk_box_pack_start (GTK_BOX (hbox), scale_hbox, FALSE, TRUE, 0);
	gtk_widget_show (scale_hbox);

	rotate_hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Rotate:");
	gtk_box_pack_start (GTK_BOX (rotate_hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	rotate_selector = gtk_combo_box_new_text ();
	gtk_combo_box_append_text (GTK_COMBO_BOX (rotate_selector), "0");
	gtk_combo_box_append_text (GTK_COMBO_BOX (rotate_selector), "90");
	gtk_combo_box_append_text (GTK_COMBO_BOX (rotate_selector), "180");
	gtk_combo_box_append_text (GTK_COMBO_BOX (rotate_selector), "270");
	gtk_combo_box_set_active (GTK_COMBO_BOX (rotate_selector), 0);
#if 0
	g_signal_connect (G_OBJECT (rotate_selector), "changed",
			  G_CALLBACK (pgd_selections_rotate_selector_changed),
			  (gpointer)demo);
#endif
	gtk_box_pack_start (GTK_BOX (rotate_hbox), rotate_selector, TRUE, TRUE, 0);
	gtk_widget_show (rotate_selector);

	gtk_box_pack_start (GTK_BOX (hbox), rotate_hbox, FALSE, TRUE, 0);
	gtk_widget_show (rotate_hbox);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	color_hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Foreground Color:");
	gtk_box_pack_start (GTK_BOX (color_hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	demo->fg_color_button = gtk_color_button_new ();
	g_signal_connect (demo->fg_color_button, "notify::color",
			  G_CALLBACK (pgd_selections_fg_color_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (color_hbox), demo->fg_color_button, TRUE, TRUE, 0);
	gtk_widget_show (demo->fg_color_button);

	gtk_box_pack_start (GTK_BOX (hbox), color_hbox, FALSE, TRUE, 0);
	gtk_widget_show (color_hbox);

	color_hbox = gtk_hbox_new (FALSE, 6);

	label = gtk_label_new ("Background Color:");
	gtk_box_pack_start (GTK_BOX (color_hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	demo->bg_color_button = gtk_color_button_new ();
	g_signal_connect (demo->bg_color_button, "notify::color",
			  G_CALLBACK (pgd_selections_bg_color_changed),
			  (gpointer)demo);
	gtk_box_pack_start (GTK_BOX (color_hbox), demo->bg_color_button, TRUE, TRUE, 0);
	gtk_widget_show (demo->bg_color_button);

	gtk_box_pack_start (GTK_BOX (hbox), color_hbox, FALSE, TRUE, 0);
	gtk_widget_show (color_hbox);

	button = gtk_button_new_with_label ("Render");
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (pgd_selections_render),
			  (gpointer)demo);
	gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, TRUE, 0);
	gtk_widget_show (button);

	return vbox;
}

GtkWidget *
pgd_selections_create_widget (PopplerDocument *document)
{
	PgdSelectionsDemo *demo;
	GtkWidget         *vbox, *hbox;

	demo = g_new0 (PgdSelectionsDemo, 1);

	demo->doc = g_object_ref (document);
	demo->scale = 1.0;
	demo->cursor = GDK_LAST_CURSOR;

	pgd_selections_clear_selections (demo);

	vbox = gtk_vbox_new (FALSE, 6);

	hbox = pgd_selections_properties_selector_create (demo);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 6);
	gtk_widget_show (hbox);

	demo->darea = gtk_drawing_area_new ();
	g_signal_connect (demo->darea, "realize",
			  G_CALLBACK (pgd_selections_drawing_area_realize),
			  (gpointer)demo);
	g_signal_connect (demo->darea, "expose_event",
			  G_CALLBACK (pgd_selections_drawing_area_expose),
			  (gpointer)demo);
	g_signal_connect (demo->darea, "button_press_event",
			  G_CALLBACK (pgd_selections_drawing_area_button_press),
			  (gpointer)demo);
	g_signal_connect (demo->darea, "motion_notify_event",
			  G_CALLBACK (pgd_selections_drawing_area_motion_notify),
			  (gpointer)demo);
	g_signal_connect (demo->darea, "button_release_event",
			  G_CALLBACK (pgd_selections_drawing_area_button_release),
			  (gpointer)demo);
	g_signal_connect (demo->darea, "query_tooltip",
			  G_CALLBACK (pgd_selections_drawing_area_query_tooltip),
			  (gpointer)demo);
	demo->swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (demo->swindow),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (demo->swindow),
					       demo->darea);
	gtk_widget_show (demo->darea);

	gtk_box_pack_start (GTK_BOX (vbox), demo->swindow, TRUE, TRUE, 0);
	gtk_widget_show (demo->swindow);

	g_object_weak_ref (G_OBJECT (demo->swindow),
			   (GWeakNotify)pgd_selections_free,
			   (gpointer)demo);

	return vbox;
}
