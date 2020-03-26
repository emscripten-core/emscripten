/* poppler-link-extractor_p.h: qt interface to poppler
 * Copyright (C) 2007-2008, Pino Toscano <pino@kde.org>
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

#ifndef _POPPLER_LINK_EXTRACTOR_H_
#define _POPPLER_LINK_EXTRACTOR_H_

#include <Object.h>
#include <OutputDev.h>

#include <QtCore/QList>

namespace Poppler
{

class Link;
class PageData;

class LinkExtractorOutputDev : public OutputDev
{
  public:
    LinkExtractorOutputDev(PageData *data);
    virtual ~LinkExtractorOutputDev();

    // inherited from OutputDev
    virtual GBool upsideDown() { return gFalse; }
    virtual GBool useDrawChar() { return gFalse; }
    virtual GBool interpretType3Chars() { return gFalse; }
    virtual void processLink(::Link *link, Catalog *catalog);

    // our stuff
    QList< Link* > links();

  private:
    PageData *m_data;
    double m_pageCropWidth;
    double m_pageCropHeight;
    QList< Link* > m_links;
};

}

#endif
