/* poppler-attachment.h: glib interface to poppler
 * Copyright (C) 2004, Red Hat, Inc.
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

#ifndef __POPPLER_ATTACHMENT_H__
#define __POPPLER_ATTACHMENT_H__

#include <glib.h>
#include <glib-object.h>

#include "poppler.h"

G_BEGIN_DECLS


#define POPPLER_TYPE_ATTACHMENT             (poppler_attachment_get_type ())
#define POPPLER_ATTACHMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ATTACHMENT, PopplerAttachment))
#define POPPLER_IS_ATTACHMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ATTACHMENT))


/**
 * PopplerAttachmentSaveFunc:
 * @buf: buffer containing bytes to be written.
 * @count: number of bytes in @buf.
 * @data: user data passed to poppler_attachment_save_to_callback()
 * @error: GError to set on error, or NULL
 *
 * Specifies the type of the function passed to
 * poppler_attachment_save_to_callback().  It is called once for each block of
 * bytes that is "written" by poppler_attachment_save_to_callback().  If
 * successful it should return %TRUE.  If an error occurs it should set
 * @error and return %FALSE, in which case poppler_attachment_save_to_callback()
 * will fail with the same error.
 *
 * @Returns: %TRUE if successful, %FALSE (with @error set) if failed.
 */
typedef gboolean (*PopplerAttachmentSaveFunc) (const gchar  *buf,
					       gsize         count,
					       gpointer      data,
					       GError      **error);

struct _PopplerAttachment
{
  GObject parent;

  gchar *name;
  gchar *description;
  gsize size;
  GTime mtime;
  GTime ctime;
  GString *checksum;
};

typedef struct _PopplerAttachmentClass
{
  GObjectClass parent_class;
} PopplerAttachmentClass;


GType     poppler_attachment_get_type         (void) G_GNUC_CONST;
gboolean  poppler_attachment_save             (PopplerAttachment          *attachment,
					       const char                 *filename,
					       GError                    **error);
gboolean  poppler_attachment_save_to_callback (PopplerAttachment          *attachment,
					       PopplerAttachmentSaveFunc   save_func,
					       gpointer                    user_data,
					       GError                    **error);


G_END_DECLS

#endif /* __POPPLER_ATTACHMENT_H__ */
