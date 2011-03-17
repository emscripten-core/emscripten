/* poppler-annot.h: glib interface to poppler
 *
 * Copyright (C) 2007 Inigo Martinez <inigomartinez@gmail.com>
 * Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
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

#ifndef __POPPLER_ANNOT_H__
#define __POPPLER_ANNOT_H__

#include <glib-object.h>
#include "poppler.h"

G_BEGIN_DECLS

#define POPPLER_TYPE_ANNOT                   (poppler_annot_get_type ())
#define POPPLER_ANNOT(obj)                   (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ANNOT, PopplerAnnot))
#define POPPLER_IS_ANNOT(obj)                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ANNOT))

#define POPPLER_TYPE_ANNOT_MARKUP            (poppler_annot_markup_get_type ())
#define POPPLER_ANNOT_MARKUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ANNOT_MARKUP, PopplerAnnotMarkup))
#define POPPLER_IS_ANNOT_MARKUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ANNOT_MARKUP))

#define POPPLER_TYPE_ANNOT_TEXT              (poppler_annot_text_get_type ())
#define POPPLER_ANNOT_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ANNOT_TEXT, PopplerAnnotText))
#define POPPLER_IS_ANNOT_TEXT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ANNOT_TEXT))

#define POPPLER_TYPE_ANNOT_FREE_TEXT         (poppler_annot_free_text_get_type ())
#define POPPLER_ANNOT_FREE_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ANNOT_FREE_TEXT, PopplerAnnotFreeText))
#define POPPLER_IS_ANNOT_FREE_TEXT(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ANNOT_FREE_TEXT))

#define POPPLER_TYPE_ANNOT_FILE_ATTACHMENT   (poppler_annot_file_attachment_get_type ())
#define POPPLER_ANNOT_FILE_ATTACHMENT(obj)   (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ANNOT_MARKUP, PopplerAnnotFileAttachment))
#define POPPLER_IS_ANNOT_FILE_ATTACHMENT(obj)(G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ANNOT_FILE_ATTACHMENT))

#define POPPLER_TYPE_ANNOT_MOVIE             (poppler_annot_movie_get_type ())
#define POPPLER_ANNOT_MOVIE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ANNOT_MOVIE, PopplerAnnotMovie))
#define POPPLER_IS_ANNOT_MOVIE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ANNOT_MOVIE))

#define POPPLER_TYPE_ANNOT_SCREEN            (poppler_annot_screen_get_type ())
#define POPPLER_ANNOT_SCREEN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ANNOT_SCREEN, PopplerAnnotScreen))
#define POPPLER_IS_ANNOT_SCREEN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ANNOT_SCREEN))

#define POPPLER_TYPE_ANNOT_CALLOUT_LINE      (poppler_annot_callout_line_get_type ())


typedef enum
{
  POPPLER_ANNOT_UNKNOWN,
  POPPLER_ANNOT_TEXT,
  POPPLER_ANNOT_LINK,
  POPPLER_ANNOT_FREE_TEXT,
  POPPLER_ANNOT_LINE,
  POPPLER_ANNOT_SQUARE,
  POPPLER_ANNOT_CIRCLE,
  POPPLER_ANNOT_POLYGON,
  POPPLER_ANNOT_POLY_LINE,
  POPPLER_ANNOT_HIGHLIGHT,
  POPPLER_ANNOT_UNDERLINE,
  POPPLER_ANNOT_SQUIGGLY,
  POPPLER_ANNOT_STRIKE_OUT,
  POPPLER_ANNOT_STAMP,
  POPPLER_ANNOT_CARET,
  POPPLER_ANNOT_INK,
  POPPLER_ANNOT_POPUP,
  POPPLER_ANNOT_FILE_ATTACHMENT,
  POPPLER_ANNOT_SOUND,
  POPPLER_ANNOT_MOVIE,
  POPPLER_ANNOT_WIDGET,
  POPPLER_ANNOT_SCREEN,
  POPPLER_ANNOT_PRINTER_MARK,
  POPPLER_ANNOT_TRAP_NET,
  POPPLER_ANNOT_WATERMARK,
  POPPLER_ANNOT_3D
} PopplerAnnotType;

typedef enum /*< flags >*/
{
  POPPLER_ANNOT_FLAG_UNKNOWN = 0,
  POPPLER_ANNOT_FLAG_INVISIBLE = 1 << 0,
  POPPLER_ANNOT_FLAG_HIDDEN = 1 << 1,
  POPPLER_ANNOT_FLAG_PRINT = 1 << 2,
  POPPLER_ANNOT_FLAG_NO_ZOOM = 1 << 3,
  POPPLER_ANNOT_FLAG_NO_ROTATE = 1 << 4,
  POPPLER_ANNOT_FLAG_NO_VIEW = 1 << 5,
  POPPLER_ANNOT_FLAG_READ_ONLY = 1 << 6,
  POPPLER_ANNOT_FLAG_LOCKED = 1 << 7,
  POPPLER_ANNOT_FLAG_TOGGLE_NO_VIEW = 1 << 8,
  POPPLER_ANNOT_FLAG_LOCKED_CONTENTS = 1 << 9
} PopplerAnnotFlag;

typedef enum
{
  POPPLER_ANNOT_MARKUP_REPLY_TYPE_R,
  POPPLER_ANNOT_MARKUP_REPLY_TYPE_GROUP
} PopplerAnnotMarkupReplyType;

typedef enum
{
  POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_3D,
  POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_UNKNOWN
} PopplerAnnotExternalDataType;

#define POPPLER_ANNOT_TEXT_ICON_NOTE          "Note"
#define POPPLER_ANNOT_TEXT_ICON_COMMENT       "Comment"
#define POPPLER_ANNOT_TEXT_ICON_KEY           "Key"
#define POPPLER_ANNOT_TEXT_ICON_HELP          "Help"
#define POPPLER_ANNOT_TEXT_ICON_NEW_PARAGRAPH "NewParagraph"
#define POPPLER_ANNOT_TEXT_ICON_PARAGRAPH     "Paragraph"
#define POPPLER_ANNOT_TEXT_ICON_INSERT        "Insert"
#define POPPLER_ANNOT_TEXT_ICON_CROSS         "Cross"
#define POPPLER_ANNOT_TEXT_ICON_CIRCLE        "Circle"

typedef enum
{
  POPPLER_ANNOT_TEXT_STATE_MARKED,
  POPPLER_ANNOT_TEXT_STATE_UNMARKED,
  POPPLER_ANNOT_TEXT_STATE_ACCEPTED,
  POPPLER_ANNOT_TEXT_STATE_REJECTED,
  POPPLER_ANNOT_TEXT_STATE_CANCELLED,
  POPPLER_ANNOT_TEXT_STATE_COMPLETED,
  POPPLER_ANNOT_TEXT_STATE_NONE,
  POPPLER_ANNOT_TEXT_STATE_UNKNOWN
} PopplerAnnotTextState;

typedef enum
{
  POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED,
  POPPLER_ANNOT_FREE_TEXT_QUADDING_CENTERED,
  POPPLER_ANNOT_FREE_TEXT_QUADDING_RIGHT_JUSTIFIED
} PopplerAnnotFreeTextQuadding;

struct _PopplerAnnotCalloutLine
{
  gboolean multiline;
  gdouble  x1;
  gdouble  y1;
  gdouble  x2;
  gdouble  y2;
  gdouble  x3;
  gdouble  y3;
};

GType                         poppler_annot_get_type                           (void) G_GNUC_CONST;
PopplerAnnotType              poppler_annot_get_annot_type                     (PopplerAnnot *poppler_annot);
gchar                        *poppler_annot_get_contents                       (PopplerAnnot *poppler_annot);
void                          poppler_annot_set_contents                       (PopplerAnnot *poppler_annot,
										const gchar  *contents);
gchar                        *poppler_annot_get_name                           (PopplerAnnot *poppler_annot);
gchar                        *poppler_annot_get_modified                       (PopplerAnnot *poppler_annot);
PopplerAnnotFlag              poppler_annot_get_flags                          (PopplerAnnot *poppler_annot);
PopplerColor                 *poppler_annot_get_color                          (PopplerAnnot *poppler_annot);
void                          poppler_annot_set_color                          (PopplerAnnot *poppler_annot,
										PopplerColor *poppler_color);
gint                          poppler_annot_get_page_index                     (PopplerAnnot *poppler_annot);

/* PopplerAnnotMarkup */
GType                         poppler_annot_markup_get_type                    (void) G_GNUC_CONST;
gchar                        *poppler_annot_markup_get_label                   (PopplerAnnotMarkup *poppler_annot);
void                          poppler_annot_markup_set_label                   (PopplerAnnotMarkup *poppler_annot,
										const gchar        *label);
gboolean                      poppler_annot_markup_has_popup                   (PopplerAnnotMarkup *poppler_annot);
void                          poppler_annot_markup_set_popup                   (PopplerAnnotMarkup *poppler_annot,
										PopplerRectangle   *popup_rect);
gboolean                      poppler_annot_markup_get_popup_is_open           (PopplerAnnotMarkup *poppler_annot);
void                          poppler_annot_markup_set_popup_is_open           (PopplerAnnotMarkup *poppler_annot,
										gboolean            is_open);
gboolean                      poppler_annot_markup_get_popup_rectangle         (PopplerAnnotMarkup *poppler_annot,
										PopplerRectangle   *poppler_rect);
gdouble                       poppler_annot_markup_get_opacity                 (PopplerAnnotMarkup *poppler_annot);
void                          poppler_annot_markup_set_opacity                 (PopplerAnnotMarkup *poppler_annot,
										gdouble             opacity);
GDate                        *poppler_annot_markup_get_date                    (PopplerAnnotMarkup *poppler_annot);
gchar                        *poppler_annot_markup_get_subject                 (PopplerAnnotMarkup *poppler_annot);
PopplerAnnotMarkupReplyType   poppler_annot_markup_get_reply_to                (PopplerAnnotMarkup *poppler_annot);
PopplerAnnotExternalDataType  poppler_annot_markup_get_external_data           (PopplerAnnotMarkup *poppler_annot);

/* PopplerAnnotText */
GType                         poppler_annot_text_get_type                      (void) G_GNUC_CONST;
PopplerAnnot                 *poppler_annot_text_new                           (PopplerDocument  *doc,
										PopplerRectangle *rect);
gboolean                      poppler_annot_text_get_is_open                   (PopplerAnnotText *poppler_annot);
void                          poppler_annot_text_set_is_open                   (PopplerAnnotText *poppler_annot,
										gboolean          is_open);
gchar                        *poppler_annot_text_get_icon                      (PopplerAnnotText *poppler_annot);
void                          poppler_annot_text_set_icon                      (PopplerAnnotText *poppler_annot,
										const gchar      *icon);
PopplerAnnotTextState         poppler_annot_text_get_state                     (PopplerAnnotText *poppler_annot);

/* PopplerAnnotFreeText */
GType                         poppler_annot_free_text_get_type                 (void) G_GNUC_CONST;
PopplerAnnotFreeTextQuadding  poppler_annot_free_text_get_quadding             (PopplerAnnotFreeText *poppler_annot);
PopplerAnnotCalloutLine      *poppler_annot_free_text_get_callout_line         (PopplerAnnotFreeText *poppler_annot);

/* PopplerAnnotFileAttachment */
GType                         poppler_annot_file_attachment_get_type           (void) G_GNUC_CONST;
PopplerAttachment            *poppler_annot_file_attachment_get_attachment     (PopplerAnnotFileAttachment *poppler_annot);
gchar                        *poppler_annot_file_attachment_get_name           (PopplerAnnotFileAttachment *poppler_annot);

/* PopplerAnnotMovie */
GType                         poppler_annot_movie_get_type                     (void) G_GNUC_CONST;
gchar                        *poppler_annot_movie_get_title                    (PopplerAnnotMovie *poppler_annot);
PopplerMovie                 *poppler_annot_movie_get_movie                    (PopplerAnnotMovie *poppler_annot);

/* PopplerAnnotScreen */
GType                         poppler_annot_screen_get_type                    (void) G_GNUC_CONST;
PopplerAction                *poppler_annot_screen_get_action                  (PopplerAnnotScreen *poppler_annot);

/* PopplerCalloutLine */
GType                         poppler_annot_callout_line_get_type              (void) G_GNUC_CONST;
PopplerAnnotCalloutLine      *poppler_annot_callout_line_new                   (void);
PopplerAnnotCalloutLine      *poppler_annot_callout_line_copy                  (PopplerAnnotCalloutLine *callout);
void                          poppler_annot_callout_line_free                  (PopplerAnnotCalloutLine *callout);

G_END_DECLS

#endif /* __POPPLER_ANNOT_H__ */
