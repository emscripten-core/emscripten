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
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "utils.h"

void
pgd_table_add_property_with_custom_widget (GtkTable    *table,
					   const gchar *markup,
					   GtkWidget   *widget,
					   gint        *row)
{
	GtkWidget *label;

	label = gtk_label_new (NULL);
	g_object_set (G_OBJECT (label), "xalign", 0.0, NULL);
	gtk_label_set_markup (GTK_LABEL (label), markup);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, *row, *row + 1,
			  GTK_FILL, GTK_FILL, 0, 0);
	gtk_widget_show (label);

	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, *row, *row + 1,
			  GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_widget_show (widget);

	*row += 1;
}

void
pgd_table_add_property_with_value_widget (GtkTable    *table,
					  const gchar *markup,
					  GtkWidget  **value_widget,
					  const gchar *value,
					  gint        *row)
{
	GtkWidget *label;

	*value_widget = label = gtk_label_new (value);
	g_object_set (G_OBJECT (label),
		      "xalign", 0.0,
		      "selectable", TRUE,
		      "ellipsize", PANGO_ELLIPSIZE_END,
		      NULL);
	pgd_table_add_property_with_custom_widget (table, markup, label, row);
}

void
pgd_table_add_property (GtkTable    *table,
			const gchar *markup,
			const gchar *value,
			gint        *row)
{
	GtkWidget *label;

	pgd_table_add_property_with_value_widget (table, markup, &label, value, row);
}

GtkWidget *
pgd_action_view_new (PopplerDocument *document)
{
	GtkWidget  *frame, *label;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Action Properties</b>");
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_widget_show (label);

	g_object_set_data (G_OBJECT (frame), "document", document);

	return frame;
}

static void
pgd_action_view_add_destination (GtkWidget   *action_view,
				 GtkTable    *table,
				 PopplerDest *dest,
				 gboolean     remote,
				 gint        *row)
{
	PopplerDocument *document;
	GEnumValue      *enum_value;
	gchar           *str;
	
	pgd_table_add_property (table, "<b>Type:</b>", "Destination", row);
	
	enum_value = g_enum_get_value ((GEnumClass *) g_type_class_ref (POPPLER_TYPE_DEST_TYPE), dest->type);
	pgd_table_add_property (table, "<b>Destination Type:</b>", enum_value->value_name, row);

	document = g_object_get_data (G_OBJECT (action_view), "document");
	
	if (dest->type != POPPLER_DEST_NAMED) {
		str = NULL;
		
		if (document && !remote) {
			PopplerPage *poppler_page;
			gchar       *page_label;
			
			poppler_page = poppler_document_get_page (document, MAX (0, dest->page_num - 1));
			
			g_object_get (G_OBJECT (poppler_page),
				      "label", &page_label,
				      NULL);
			if (page_label) {
				str = g_strdup_printf ("%d (%s)", dest->page_num, page_label);
				g_free (page_label);
			}
		}
		
		if (!str)
			str = g_strdup_printf ("%d", dest->page_num);
		pgd_table_add_property (table, "<b>Page:</b>", str, row);
		g_free (str);
		
		str = g_strdup_printf ("%.2f", dest->left);
		pgd_table_add_property (table, "<b>Left:</b>", str, row);
		g_free (str);
		
		str = g_strdup_printf ("%.2f", dest->right);
		pgd_table_add_property (table, "<b>Right:</b>", str, row);
		g_free (str);
		
		str = g_strdup_printf ("%.2f", dest->top);
		pgd_table_add_property (table, "<b>Top:</b>", str, row);
		g_free (str);
	
		str = g_strdup_printf ("%.2f", dest->bottom);
		pgd_table_add_property (table, "<b>Bottom:</b>", str, row);
		g_free (str);
	
		str = g_strdup_printf ("%.2f", dest->zoom);
		pgd_table_add_property (table, "<b>Zoom:</b>", str, row);
		g_free (str);
	} else {
		pgd_table_add_property (table, "<b>Named Dest:</b>", dest->named_dest, row);

		if (document && !remote) {
			PopplerDest *new_dest;

			new_dest = poppler_document_find_dest (document, dest->named_dest);
			if (new_dest) {
				GtkWidget *new_table, *alignment;
				gint       new_row = 0;

				alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
				gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
				
				new_table = gtk_table_new (8, 2, FALSE);
				gtk_table_set_col_spacings (GTK_TABLE (new_table), 6);
				gtk_table_set_row_spacings (GTK_TABLE (new_table), 6);
				gtk_table_attach_defaults (table, alignment, 0, 2, *row, *row + 1);
				gtk_widget_show (alignment);
				
				pgd_action_view_add_destination (action_view, GTK_TABLE (new_table),
								 new_dest, FALSE, &new_row);
				poppler_dest_free (new_dest);

				gtk_container_add (GTK_CONTAINER (alignment), new_table);
				gtk_widget_show (new_table);

				*row += 1;
			}
		}
	}
}

static const gchar *
get_movie_op (PopplerActionMovieOperation op)
{
	switch (op) {
	case POPPLER_ACTION_MOVIE_PLAY:
		return "Play";
	case POPPLER_ACTION_MOVIE_PAUSE:
		return "Pause";
	case POPPLER_ACTION_MOVIE_RESUME:
		return "Resume";
	case POPPLER_ACTION_MOVIE_STOP:
		return "Stop";
	}
	return NULL;
}

static void
free_tmp_file (GFile *tmp_file)
{

	g_file_delete (tmp_file, NULL, NULL);
	g_object_unref (tmp_file);
}

static gboolean
save_helper (const gchar  *buf,
	     gsize         count,
	     gpointer      data,
	     GError      **error)
{
	gint fd = GPOINTER_TO_INT (data);

	return write (fd, buf, count) == count;
}

static void
pgd_action_view_play_rendition (GtkWidget    *button,
				PopplerMedia *media)
{
	GFile *file = NULL;
	gchar *uri;

	if (poppler_media_is_embedded (media)) {
		gint   fd;
		gchar *tmp_filename = NULL;

		fd = g_file_open_tmp (NULL, &tmp_filename, NULL);
		if (fd != -1) {
			if (poppler_media_save_to_callback (media, save_helper, GINT_TO_POINTER (fd), NULL)) {
				file = g_file_new_for_path (tmp_filename);
				g_object_set_data_full (G_OBJECT (media),
							"tmp-file", g_object_ref (file),
							(GDestroyNotify)free_tmp_file);
			} else {
				g_free (tmp_filename);
			}
			close (fd);
		} else if (tmp_filename) {
			g_free (tmp_filename);
		}

	} else {
		const gchar *filename;

		filename = poppler_media_get_filename (media);
		if (g_path_is_absolute (filename)) {
			file = g_file_new_for_path (filename);
		} else if (g_strrstr (filename, "://")) {
			file = g_file_new_for_uri (filename);
		} else {
			gchar *cwd;
			gchar *path;

			// FIXME: relative to doc uri, not cwd
			cwd = g_get_current_dir ();
			path = g_build_filename (cwd, filename, NULL);
			g_free (cwd);

			file = g_file_new_for_path (path);
			g_free (path);
		}
	}

	if (file) {
		uri = g_file_get_uri (file);
		g_object_unref (file);
		if (uri) {
			gtk_show_uri (gtk_widget_get_screen (button),
				      uri, GDK_CURRENT_TIME, NULL);
			g_free (uri);
		}
	}
}

static void
pgd_action_view_do_action_layer (GtkWidget *button,
				 GList     *state_list)
{
	GList *l, *m;

	for (l = state_list; l; l = g_list_next (l)) {
		PopplerActionLayer *action_layer = (PopplerActionLayer *)l->data;

		for (m = action_layer->layers; m; m = g_list_next (m)) {
			PopplerLayer *layer = (PopplerLayer *)m->data;

			switch (action_layer->action) {
			case POPPLER_ACTION_LAYER_ON:
				poppler_layer_show (layer);
				break;
			case POPPLER_ACTION_LAYER_OFF:
				poppler_layer_hide (layer);
				break;
			case POPPLER_ACTION_LAYER_TOGGLE:
				if (poppler_layer_is_visible (layer))
					poppler_layer_hide (layer);
				else
					poppler_layer_show (layer);
				break;
			}
		}
	}
}

void
pgd_action_view_set_action (GtkWidget     *action_view,
			    PopplerAction *action)
{
	GtkWidget  *alignment;
	GtkWidget  *table;
	gint        row = 0;

	alignment = gtk_bin_get_child (GTK_BIN (action_view));
	if (alignment) {
		gtk_container_remove (GTK_CONTAINER (action_view), alignment);
	}
	
	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
	gtk_container_add (GTK_CONTAINER (action_view), alignment);
	gtk_widget_show (alignment);

	if (!action)
		return;

	table = gtk_table_new (10, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);

	pgd_table_add_property (GTK_TABLE (table), "<b>Title:</b>", action->any.title, &row);
	
	switch (action->type) {
	case POPPLER_ACTION_UNKNOWN:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Unknown", &row);
		break;
	case POPPLER_ACTION_NONE:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "None", &row);
		break;
	case POPPLER_ACTION_GOTO_DEST:
		pgd_action_view_add_destination (action_view, GTK_TABLE (table), action->goto_dest.dest, FALSE, &row);
		break;
	case POPPLER_ACTION_GOTO_REMOTE:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Remote Destination", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Filename:</b>", action->goto_remote.file_name, &row);
		pgd_action_view_add_destination (action_view, GTK_TABLE (table), action->goto_remote.dest, TRUE, &row);
		break;
	case POPPLER_ACTION_LAUNCH:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Launch", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Filename:</b>", action->launch.file_name, &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Params:</b>", action->launch.params, &row);
		break;
	case POPPLER_ACTION_URI:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "External URI", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>URI</b>", action->uri.uri, &row);
		break;
	case POPPLER_ACTION_NAMED:
		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Named Action", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Name:</b>", action->named.named_dest, &row);
		break;
	case POPPLER_ACTION_MOVIE: {
		GtkWidget *movie_view = pgd_movie_view_new ();

		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Movie", &row);
		pgd_table_add_property (GTK_TABLE (table), "<b>Operation:</b>", get_movie_op (action->movie.operation), &row);
		pgd_movie_view_set_movie (movie_view, action->movie.movie);
		pgd_table_add_property_with_custom_widget (GTK_TABLE (table), "<b>Movie:</b>", movie_view, &row);
	}
		break;
	case POPPLER_ACTION_RENDITION: {
		gchar *text;

		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "Rendition", &row);
		text = g_strdup_printf ("%d", action->rendition.op);
		pgd_table_add_property (GTK_TABLE (table), "<b>Operation:</b>", text, &row);
		g_free (text);
		if (action->rendition.media) {
			gboolean   embedded = poppler_media_is_embedded (action->rendition.media);
			GtkWidget *button;

			pgd_table_add_property (GTK_TABLE (table), "<b>Embedded:</b>", embedded ? "Yes": "No", &row);
			if (embedded) {
				const gchar *mime_type = poppler_media_get_mime_type (action->rendition.media);
				pgd_table_add_property (GTK_TABLE (table), "<b>Mime type:</b>",
							mime_type ? mime_type : "",
							&row);
			} else {
				pgd_table_add_property (GTK_TABLE (table), "<b>Filename:</b>",
							poppler_media_get_filename (action->rendition.media),
							&row);
			}

			button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);
			g_signal_connect (button, "clicked",
					  G_CALLBACK (pgd_action_view_play_rendition),
					  action->rendition.media);
			pgd_table_add_property_with_custom_widget (GTK_TABLE (table), NULL, button, &row);
			gtk_widget_show (button);
		}
	}
		break;
	case POPPLER_ACTION_OCG_STATE: {
		GList     *l;
		GtkWidget *button;

		pgd_table_add_property (GTK_TABLE (table), "<b>Type:</b>", "OCGState", &row);

		for (l = action->ocg_state.state_list; l; l = g_list_next (l)) {
			PopplerActionLayer *action_layer = (PopplerActionLayer *)l->data;
			gchar *text;
			gint   n_layers = g_list_length (action_layer->layers);

			switch (action_layer->action) {
			case POPPLER_ACTION_LAYER_ON:
				text = g_strdup_printf ("%d layers On", n_layers);
				break;
			case POPPLER_ACTION_LAYER_OFF:
				text = g_strdup_printf ("%d layers Off", n_layers);
				break;
			case POPPLER_ACTION_LAYER_TOGGLE:
				text = g_strdup_printf ("%d layers Toggle", n_layers);
				break;
			}
			pgd_table_add_property (GTK_TABLE (table), "<b>Action:</b>", text, &row);
			g_free (text);
		}

		button = gtk_button_new_with_label ("Do action");
		g_signal_connect (button, "clicked",
				  G_CALLBACK (pgd_action_view_do_action_layer),
				  action->ocg_state.state_list);
		pgd_table_add_property_with_custom_widget (GTK_TABLE (table), NULL, button, &row);
		gtk_widget_show (button);
	}
		break;
	default:
		g_assert_not_reached ();
	}

	gtk_container_add (GTK_CONTAINER (alignment), table);
	gtk_widget_show (table);
}

gchar *
pgd_format_date (time_t utime)
{
	time_t time = (time_t) utime;
	char s[256];
	const char *fmt_hack = "%c";
	size_t len;
#ifdef HAVE_LOCALTIME_R
	struct tm t;
	if (time == 0 || !localtime_r (&time, &t)) return NULL;
	len = strftime (s, sizeof (s), fmt_hack, &t);
#else
	struct tm *t;
	if (time == 0 || !(t = localtime (&time)) ) return NULL;
	len = strftime (s, sizeof (s), fmt_hack, t);
#endif

	if (len == 0 || s[0] == '\0') return NULL;

	return g_locale_to_utf8 (s, -1, NULL, NULL, NULL);
}

GtkWidget *
pgd_movie_view_new (void)
{
	GtkWidget  *frame, *label;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), "<b>Movie Properties</b>");
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_widget_show (label);

	return frame;
}

static void
pgd_movie_view_play_movie (GtkWidget    *button,
			   PopplerMovie *movie)
{
	const gchar *filename;
	GFile       *file;
	gchar       *uri;

	filename = poppler_movie_get_filename (movie);
	if (g_path_is_absolute (filename)) {
		file = g_file_new_for_path (filename);
	} else if (g_strrstr (filename, "://")) {
		file = g_file_new_for_uri (filename);
	} else {
		gchar *cwd;
		gchar *path;

		// FIXME: relative to doc uri, not cwd
		cwd = g_get_current_dir ();
		path = g_build_filename (cwd, filename, NULL);
		g_free (cwd);

		file = g_file_new_for_path (path);
		g_free (path);
	}

	uri = g_file_get_uri (file);
	g_object_unref (file);
	if (uri) {
		gtk_show_uri (gtk_widget_get_screen (button),
			      uri, GDK_CURRENT_TIME, NULL);
		g_free (uri);
	}
}

void
pgd_movie_view_set_movie (GtkWidget    *movie_view,
			  PopplerMovie *movie)
{
	GtkWidget  *alignment;
	GtkWidget  *table;
	GtkWidget  *button;
	gint        row = 0;

	alignment = gtk_bin_get_child (GTK_BIN (movie_view));
	if (alignment) {
		gtk_container_remove (GTK_CONTAINER (movie_view), alignment);
	}

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
	gtk_container_add (GTK_CONTAINER (movie_view), alignment);
	gtk_widget_show (alignment);

	if (!movie)
		return;

	table = gtk_table_new (10, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);

	pgd_table_add_property (GTK_TABLE (table), "<b>Filename:</b>", poppler_movie_get_filename (movie), &row);
	pgd_table_add_property (GTK_TABLE (table), "<b>Need Poster:</b>", poppler_movie_need_poster (movie) ? "Yes" : "No", &row);
	pgd_table_add_property (GTK_TABLE (table), "<b>Show Controls:</b>", poppler_movie_show_controls (movie) ? "Yes" : "No", &row);

	button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);
	g_signal_connect (button, "clicked",
			  G_CALLBACK (pgd_movie_view_play_movie),
			  movie);
	pgd_table_add_property_with_custom_widget (GTK_TABLE (table), NULL, button, &row);
	gtk_widget_show (button);

	gtk_container_add (GTK_CONTAINER (alignment), table);
	gtk_widget_show (table);
}
