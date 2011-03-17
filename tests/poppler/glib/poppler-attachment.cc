/* poppler-attachment.cc: glib wrapper for poppler
 * Copyright (C) 2006, Red Hat, Inc.
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

#include "poppler.h"
#include "poppler-private.h"

/**
 * SECTION:poppler-attachment
 * @short_description: Attachments
 * @title: PopplerAttachment
 */

/* FIXME: We need to add gettext support sometime */
#define _(x) (x)

typedef struct _PopplerAttachmentPrivate PopplerAttachmentPrivate;
struct _PopplerAttachmentPrivate
{
  Object *obj_stream;
};

#define POPPLER_ATTACHMENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), POPPLER_TYPE_ATTACHMENT, PopplerAttachmentPrivate))

static void poppler_attachment_dispose (GObject *obj);
static void poppler_attachment_finalize (GObject *obj);

G_DEFINE_TYPE (PopplerAttachment, poppler_attachment, G_TYPE_OBJECT)

static void
poppler_attachment_init (PopplerAttachment *attachment)
{
}

static void
poppler_attachment_class_init (PopplerAttachmentClass *klass)
{
  G_OBJECT_CLASS (klass)->dispose = poppler_attachment_dispose;
  G_OBJECT_CLASS (klass)->finalize = poppler_attachment_finalize;
  g_type_class_add_private (klass, sizeof (PopplerAttachmentPrivate));
}

static void
poppler_attachment_dispose (GObject *obj)
{
  PopplerAttachmentPrivate *priv;

  priv = POPPLER_ATTACHMENT_GET_PRIVATE (obj);

  if (priv->obj_stream)
    {
      priv->obj_stream->free();
      delete priv->obj_stream;
      priv->obj_stream = NULL;
    }

  G_OBJECT_CLASS (poppler_attachment_parent_class)->dispose (obj);
}

static void
poppler_attachment_finalize (GObject *obj)
{
  PopplerAttachment *attachment;

  attachment = (PopplerAttachment *) obj;

  if (attachment->name)
    g_free (attachment->name);
  attachment->name = NULL;

  if (attachment->description)
    g_free (attachment->description);
  attachment->description = NULL;
  
  if (attachment->checksum)
    g_string_free (attachment->checksum, TRUE);
  attachment->checksum = NULL;
  
  G_OBJECT_CLASS (poppler_attachment_parent_class)->finalize (obj);
}

/* Public functions */

PopplerAttachment *
_poppler_attachment_new (EmbFile *emb_file)
{
  PopplerAttachment *attachment;
  PopplerAttachmentPrivate *priv;

  g_assert (emb_file != NULL);

  attachment = (PopplerAttachment *) g_object_new (POPPLER_TYPE_ATTACHMENT, NULL);
  priv = POPPLER_ATTACHMENT_GET_PRIVATE (attachment);

  if (emb_file->name ())
    attachment->name = _poppler_goo_string_to_utf8 (emb_file->name ());
  if (emb_file->description ())
    attachment->description = _poppler_goo_string_to_utf8 (emb_file->description ());

  attachment->size = emb_file->size ();
  
  _poppler_convert_pdf_date_to_gtime (emb_file->createDate (), (time_t *)&attachment->ctime);
  _poppler_convert_pdf_date_to_gtime (emb_file->modDate (), (time_t *)&attachment->mtime);

  if (emb_file->checksum ()->getLength () > 0)
	  attachment->checksum = g_string_new_len (emb_file->checksum ()->getCString (),
						   emb_file->checksum ()->getLength ());
  
  priv->obj_stream = new Object();
  emb_file->streamObject().copy(priv->obj_stream);

  return attachment;
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
		   _("Error writing to image file: %s"),
		   g_strerror (errno));
      return FALSE;
    }

  return TRUE;
}

/**
 * poppler_attachment_save:
 * @attachment: A #PopplerAttachment.
 * @filename: name of file to save
 * @error: (allow-none): return location for error, or %NULL.
 * 
 * Saves @attachment to a file indicated by @filename.  If @error is set, %FALSE
 * will be returned. Possible errors include those in the #G_FILE_ERROR domain
 * and whatever the save function generates.
 * 
 * Return value: %TRUE, if the file successfully saved
 **/
gboolean
poppler_attachment_save (PopplerAttachment  *attachment,
			 const char         *filename,
			 GError            **error)
{
  gboolean result;
  FILE *f;
  
  g_return_val_if_fail (POPPLER_IS_ATTACHMENT (attachment), FALSE);

  f = g_fopen (filename, "wb");

  if (f == NULL)
    {
      gchar *display_name = g_filename_display_name (filename);
      g_set_error (error,
		   G_FILE_ERROR,
		   g_file_error_from_errno (errno),
		   _("Failed to open '%s' for writing: %s"),
		   display_name,
		   g_strerror (errno));
      g_free (display_name);
      return FALSE;
    }

  result = poppler_attachment_save_to_callback (attachment, save_helper, f, error);

  if (fclose (f) < 0)
    {
      gchar *display_name = g_filename_display_name (filename);
      g_set_error (error,
		   G_FILE_ERROR,
		   g_file_error_from_errno (errno),
		   _("Failed to close '%s', all data may not have been saved: %s"),
		   display_name,
		   g_strerror (errno));
      g_free (display_name);
      return FALSE;
    }

  return TRUE;
}

#define BUF_SIZE 1024

/**
 * poppler_attachment_save_to_callback:
 * @attachment: A #PopplerAttachment.
 * @save_func: (scope call): a function that is called to save each block of data that the save routine generates.
 * @user_data: user data to pass to the save function.
 * @error: (allow-none): return location for error, or %NULL.
 * 
 * Saves @attachment by feeding the produced data to @save_func. Can be used
 * when you want to store the attachment to something other than a file, such as
 * an in-memory buffer or a socket. If @error is set, %FALSE will be
 * returned. Possible errors include those in the #G_FILE_ERROR domain and
 * whatever the save function generates.
 * 
 * Return value: %TRUE, if the save successfully completed
 **/
gboolean
poppler_attachment_save_to_callback (PopplerAttachment          *attachment,
				     PopplerAttachmentSaveFunc   save_func,
				     gpointer                    user_data,
				     GError                    **error)
{
  Stream *stream;
  gchar buf[BUF_SIZE]; 
  int i;
  gboolean eof_reached = FALSE;

  g_return_val_if_fail (POPPLER_IS_ATTACHMENT (attachment), FALSE);

  stream = POPPLER_ATTACHMENT_GET_PRIVATE (attachment)->obj_stream->getStream();
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
	    return FALSE;
	}
    }
  while (! eof_reached);


  return TRUE;
}
