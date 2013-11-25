/* poppler-media.h: qt interface to poppler
 * Copyright (C) 2012 Guillermo A. Amaral B. <gamaral@kde.org>
 * Copyright (C) 2012, 2013 Albert Astals Cid <aacid@kde.org>
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

#ifndef __POPPLER_MEDIARENDITION_H__
#define __POPPLER_MEDIARENDITION_H__

#include "poppler-export.h"

#include <QtCore/QSize>
#include <QtCore/QString>

class MediaRendition;
class QIODevice;

namespace Poppler
{
  class MediaRenditionPrivate;

  /**
    Qt wrapper for MediaRendition.

    \since 0.20
   */
  class POPPLER_QT5_EXPORT MediaRendition {
   public:
    /**
      Constructs a MediaRendition. Takes ownership of the passed rendition
     */
    MediaRendition(::MediaRendition *rendition);
    ~MediaRendition();

    /**
      Check if wrapper is holding a valid rendition object.
     */
    bool isValid() const;

    /**
      Returns content type.
     */
    QString contentType() const;

    /**
      Returns file name.
     */
    QString fileName() const;

    /**
      Returns true if media is embedded.
     */
    bool isEmbedded() const;

    /**
      Returns data buffer.
     */
    QByteArray data() const;

    /**
      Convenience accessor for auto-play parameter.
     */
    bool autoPlay() const;

    /**
      Convenience accessor for show controls parameter.
     */
    bool showControls() const;

    /**
      Convenience accessor for repeat count parameter.
     */
    float repeatCount() const;

    /**
      Convenience accessor for size parameter.
     */
    QSize size() const;

   private:
    Q_DECLARE_PRIVATE( MediaRendition )
    MediaRenditionPrivate *d_ptr;
    Q_DISABLE_COPY( MediaRendition )
  };
}

#endif /* __POPPLER_MEDIARENDITION_H__ */
