/* poppler-movie.cc: glib interface to Movie
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

#include "poppler-movie.h"
#include "poppler-private.h"

/**
 * SECTION: poppler-movie
 * @short_description: Movies
 * @title: PopplerMovie
 */

typedef struct _PopplerMovieClass PopplerMovieClass;

struct _PopplerMovie
{
  GObject   parent_instance;

  gchar   *filename;
  gboolean need_poster;
  gboolean show_controls;
};

struct _PopplerMovieClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerMovie, poppler_movie, G_TYPE_OBJECT);

static void
poppler_movie_finalize (GObject *object)
{
  PopplerMovie* movie = POPPLER_MOVIE(object);

  if (movie->filename) {
    g_free (movie->filename);
    movie->filename = NULL;
  }

  G_OBJECT_CLASS (poppler_movie_parent_class)->finalize (object);
}

static void
poppler_movie_class_init (PopplerMovieClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_movie_finalize;
}

static void
poppler_movie_init (PopplerMovie *movie)
{
}

PopplerMovie *
_poppler_movie_new (Movie *poppler_movie)
{
  PopplerMovie *movie;

  g_assert (poppler_movie != NULL);

  movie = POPPLER_MOVIE (g_object_new (POPPLER_TYPE_MOVIE, NULL));

  movie->filename = g_strdup (poppler_movie->getFileName()->getCString());
  if (poppler_movie->getShowPoster()) {
    Object tmp;

    poppler_movie->getPoster(&tmp);
    movie->need_poster = (!tmp.isRef() && !tmp.isStream());
    tmp.free();
  }

  movie->show_controls = poppler_movie->getActivationParameters()->showControls;

  return movie;
}

/**
* poppler_movie_get_filename:
* @poppler_movie: a #PopplerMovie
*
* Returns the local filename identifying a self-describing movie file
*
* Return value: a local filename, return value is owned by #PopplerMovie and
*               should not be freed
*
* Since: 0.14
*/
const gchar *
poppler_movie_get_filename (PopplerMovie *poppler_movie)
{
  g_return_val_if_fail (POPPLER_IS_MOVIE (poppler_movie), NULL);

  return poppler_movie->filename;
}

/**
 * poppler_movie_need_poster:
 * @poppler_movie: a #PopplerMovie
 *
 * Returns whether a poster image representing the Movie
 * shall be displayed. The poster image must be retrieved
 * from the movie file.
 *
 * Return value: %TRUE if move needs a poster image, %FALSE otherwise
 *
 * Since: 0.14
 */
gboolean
poppler_movie_need_poster (PopplerMovie *poppler_movie)
{
  g_return_val_if_fail (POPPLER_IS_MOVIE (poppler_movie), FALSE);

  return poppler_movie->need_poster;
}

/**
 * poppler_movie_show_controls:
 * @poppler_movie: a #PopplerMovie
 *
 * Returns whether to display a movie controller bar while playing the movie
 *
 * Return value: %TRUE if controller bar should be displayed, %FALSE otherwise
 *
 * Since: 0.14
 */
gboolean
poppler_movie_show_controls (PopplerMovie *poppler_movie)
{
  g_return_val_if_fail (POPPLER_IS_MOVIE (poppler_movie), FALSE);

  return poppler_movie->show_controls;
}
