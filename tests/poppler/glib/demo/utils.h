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
#include <poppler.h>

#ifndef _UTILS_H_
#define _UTILS_H_

G_BEGIN_DECLS

void       pgd_table_add_property                    (GtkTable        *table,
						      const gchar     *markup,
						      const gchar     *value,
						      gint            *row);
void       pgd_table_add_property_with_value_widget  (GtkTable        *table,
						      const gchar     *markup,
						      GtkWidget      **value_widget,
						      const gchar     *value,
						      gint            *row);
void       pgd_table_add_property_with_custom_widget (GtkTable       *table,
						      const gchar    *markup,
						      GtkWidget      *widget,
						      gint           *row);
GtkWidget *pgd_action_view_new                       (PopplerDocument *document);
void       pgd_action_view_set_action                (GtkWidget       *action_view,
						      PopplerAction   *action);
gchar     *pgd_format_date                           (time_t           utime);
GtkWidget *pgd_movie_view_new                        (void);
void       pgd_movie_view_set_movie                  (GtkWidget       *movie_view,
						      PopplerMovie    *movie);

G_END_DECLS

#endif /* _UTILS_H_ */
