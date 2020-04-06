/* poppler-media.cc: glib interface to MediaRendition
 *
 * Copyright (C) 2010 Carlos Garcia Campos <carlosgc@gnome.org>
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

#include <errno.h>
#include <glib/gstdio.h>

#include "poppler-media.h"
#include "poppler-private.h"

/**
 * SECTION: poppler-media
 * @short_description: Media
 * @title: PopplerMedia
 */

typedef struct _PopplerMediaClass PopplerMediaClass;

struct _PopplerMedia
{
  GObject   parent_instance;

  gchar  *filename;

  gchar  *mime_type;
  Stream *stream;
};

struct _PopplerMediaClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (PopplerMedia, poppler_media, G_TYPE_OBJECT);

static void
poppler_media_finalize (GObject *object)
{
  PopplerMedia *media = POPPLER_MEDIA(object);

  if (media->filename) {
    g_free (media->filename);
    media->filename = NULL;
  }

  if (media->mime_type) {
    g_free (media->mime_type);
    media->mime_type = NULL;
  }

  if (media->stream) {
    media->stream->decRef();
    media->stream = NULL;
  }

  G_OBJECT_CLASS (poppler_media_parent_class)->finalize (object);
}

static void
poppler_media_class_init (PopplerMediaClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = poppler_media_finalize;
}

static void
poppler_media_init (PopplerMedia *media)
{
}

PopplerMedia *
_poppler_media_new (MediaRendition *poppler_media)
{
  PopplerMedia *media;

  g_assert (poppler_media != NULL);

  media = POPPLER_MEDIA (g_object_new (POPPLER_TYPE_MEDIA, NULL));

  if (poppler_media->getIsEmbedded()) {
    GooString* mime_type;

    media->stream = poppler_media->getEmbbededStream();
    mime_type = poppler_media->getContentType();
    if (mime_type)
      media->mime_type = g_strdup (mime_type->getCString());
  } else {
    media->filename = g_strdup (poppler_media->getFileName()->getCString());
  }

  return media;
}

/**
* poppler_media_get_filename:
* @poppler_media: a #PopplerMedia
*
* Returns the media clip filename, in case of non-embedded media. filename might be
* a local relative or absolute path or a URI
*
* Return value: a filename, return value is owned by #PopplerMedia and should not be freed
*
* Since: 0.14
*/
const gchar *
poppler_media_get_filename (PopplerMedia *poppler_media)
{
  g_return_val_if_fail (POPPLER_IS_MEDIA (poppler_media), NULL);
  g_return_val_if_fail (poppler_media->stream == NULL, NULL);

  return poppler_media->filename;
}

/**
 * poppler_media_is_embedded:
 * @poppler_media: a #PopplerMedia
 *
 * Whether the media clip is embedded in the PDF. If the result is %TRUE, the embedded stream
 * can be saved with poppler_media_save() or poppler_media_save_to_callback() function.
 * If the result is %FALSE, the media clip filename can be retrieved with
 * poppler_media_get_file_name() function.
 *
 * Return value: %TRUE if media clip is embedded, %FALSE otherwise
 *
 * Since: 0.14
 */
gboolean
poppler_media_is_embedded (PopplerMedia *poppler_media)
{
  g_return_val_if_fail (POPPLER_IS_MEDIA (poppler_media), FALSE);

  return poppler_media->stream != NULL;
}

/**
 * poppler_media_get_mime_type:
 * @poppler_media: a #PopplerMedia
 *
 * Returns the media clip mime-type
 *
 * Return value: the mime-type, return value is owned by #PopplerMedia and should not be freed
 *
 * Since: 0.14
 */
const gchar *
poppler_media_get_mime_type (PopplerMedia *poppler_media)
{
  g_return_val_if_fail (POPPLER_IS_MEDIA (poppler_media), NULL);

  return poppler_media->mime_type;
}

static gboolean
save_helper (const gchar  *buf,
	     gsize         count,
	     gpointer      data,
	     GError      **error)
{
  FILE *f = (FILE *) data;
  gsize n;

  n = fwrite (buf, 1, count, f);
  if (n != count)
    {
      g_set_error (error,
		   G_FILE_ERROR,
		   g_file_error_from_errno (errno),
		   "Error writing to media file: %s",
		   g_strerror (errno));
      return FALSE;
    }

  return TRUE;
}

/**
 * poppler_media_save:
 * @poppler_media: a #PopplerMedia
 * @filename: name of file to save
 * @error: (allow-none): return location for error, or %NULL.
 *
 * Saves embedded stream of @poppler_media to a file indicated by @filename.
 * If @error is set, %FALSE will be returned.
 * Possible errors include those in the #G_FILE_ERROR domain
 * and whatever the save function generates.
 *
 * Return value: %TRUE, if the file successfully saved
 *
 * Since: 0.14
 */
gboolean
poppler_media_save (PopplerMedia *poppler_media,
		    const char   *filename,
		    GError      **error)
{
  gboolean result;
  FILE *f;

  g_return_val_if_fail (POPPLER_IS_MEDIA (poppler_media), FALSE);
  g_return_val_if_fail (poppler_media->stream != NULL, FALSE);

  f = g_fopen (filename, "wb");

  if (f == NULL)
    {
      gchar *display_name = g_filename_display_name (filename);
      g_set_error (error,
		   G_FILE_ERROR,
		   g_file_error_from_errno (errno),
		   "Failed to open '%s' for writing: %s",
		   display_name,
		   g_strerror (errno));
      g_free (display_name);
      return FALSE;
    }

  result = poppler_media_save_to_callback (poppler_media, save_helper, f, error);

  if (fclose (f) < 0)
    {
      gchar *display_name = g_filename_display_name (filename);
      g_set_error (error,
		   G_FILE_ERROR,
		   g_file_error_from_errno (errno),
		   "Failed to close '%s', all data may not have been saved: %s",
		   display_name,
		   g_strerror (errno));
      g_free (display_name);
      return FALSE;
    }

  return result;
}

#define BUF_SIZE 1024

/**
 * poppler_media_save_to_callback:
 * @poppler_media: a #PopplerMedia
 * @save_func: (scope call): a function that is called to save each block of data that the save routine generates.
 * @user_data: user data to pass to the save function.
 * @error: (allow-none): return location for error, or %NULL.
 *
 * Saves embedded stream of @poppler_media by feeding the produced data to @save_func. Can be used
 * when you want to store the media clip stream to something other than a file, such as
 * an in-memory buffer or a socket. If @error is set, %FALSE will be
 * returned. Possible errors include those in the #G_FILE_ERROR domain and
 * whatever the save function generates.
 *
 * Return value: %TRUE, if the save successfully completed
 *
 * Since: 0.14
 */
gboolean
poppler_media_save_to_callback (PopplerMedia        *poppler_media,
				PopplerMediaSaveFunc save_func,
				gpointer             user_data,
				GError             **error)
{
  Stream *stream;
  gchar buf[BUF_SIZE];
  int i;
  gboolean eof_reached = FALSE;

  g_return_val_if_fail (POPPLER_IS_MEDIA (poppler_media), FALSE);
  g_return_val_if_fail (poppler_media->stream != NULL, FALSE);

  stream = poppler_media->stream;
  stream->reset();

  do
    {
      int data;

      for (i = 0; i < BUF_SIZE; i++)
	{
	  data = stream->getChar ();
	  if (data == EOF)
	    {
	      eof_reached = TRUE;
	      break;
	    }
	  buf[i] = data;
	}

      if (i > 0)
	{
	  if (! (save_func) (buf, i, user_data, error))
	    {
	      stream->close ();
	      return FALSE;
	    }
	}
    }
  while (! eof_reached);

  stream->close ();

  return TRUE;
}
