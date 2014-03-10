/* poppler-movie.h: glib interface to Movie
 *
 * Copyright (C) 2010 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2008 Hugo Mercier <hmercier31[@]gmail.com>
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

#ifndef __POPPLER_MOVIE_H__
#define __POPPLER_MOVIE_H__

#include <glib-object.h>
#include "poppler.h"

G_BEGIN_DECLS

#define POPPLER_TYPE_MOVIE                   (poppler_movie_get_type ())
#define POPPLER_MOVIE(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_MOVIE, PopplerMovie))
#define POPPLER_IS_MOVIE(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_MOVIE))


GType        poppler_movie_get_type      (void) G_GNUC_CONST;
const gchar *poppler_movie_get_filename  (PopplerMovie *poppler_movie);
gboolean     poppler_movie_need_poster   (PopplerMovie *poppler_movie);
gboolean     poppler_movie_show_controls (PopplerMovie *poppler_movie);

G_END_DECLS

#endif /* __POPPLER_MOVIE_H__ */

