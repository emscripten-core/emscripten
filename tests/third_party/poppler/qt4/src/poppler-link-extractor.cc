/* poppler-link-extractor_p.h: qt interface to poppler
 * Copyright (C) 2007-2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, Albert Astals Cid <aacid@kde.org>
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

#include "poppler-link-extractor-private.h"

#include <GfxState.h>
#include <Link.h>
#include <Object.h>
#include <Page.h>

#include "poppler-qt4.h"
#include "poppler-page-private.h"

namespace Poppler
{

LinkExtractorOutputDev::LinkExtractorOutputDev(PageData *data)
  : m_data(data)
{
  Q_ASSERT(m_data);
  ::Page *popplerPage = m_data->page;
  m_pageCropWidth = popplerPage->getCropWidth();
  m_pageCropHeight = popplerPage->getCropHeight();
  if (popplerPage->getRotate() == 90 || popplerPage->getRotate() == 270)
    qSwap(m_pageCropWidth, m_pageCropHeight);
  GfxState gfxState(72.0, 72.0, popplerPage->getCropBox(), popplerPage->getRotate(), gTrue);
  setDefaultCTM(gfxState.getCTM());
}

LinkExtractorOutputDev::~LinkExtractorOutputDev()
{
  qDeleteAll(m_links);
}

void LinkExtractorOutputDev::processLink(::Link *link, Catalog *catalog)
{
  if (!link->isOk())
    return;

  double left, top, right, bottom;
  int leftAux, topAux, rightAux, bottomAux;
  link->getRect(&left, &top, &right, &bottom);
  QRectF linkArea;

  cvtUserToDev(left, top, &leftAux, &topAux);
  cvtUserToDev(right, bottom, &rightAux, &bottomAux);
  linkArea.setLeft((double)leftAux / m_pageCropWidth);
  linkArea.setTop((double)topAux / m_pageCropHeight);
  linkArea.setRight((double)rightAux / m_pageCropWidth);
  linkArea.setBottom((double)bottomAux / m_pageCropHeight);

  Link *popplerLink = m_data->convertLinkActionToLink(link->getAction(), linkArea);
  if (popplerLink)
  {
    m_links.append(popplerLink);
  }
  OutputDev::processLink(link, catalog);
}

QList< Link* > LinkExtractorOutputDev::links()
{
  QList< Link* > ret = m_links;
  m_links.clear();
  return ret;
}

}
