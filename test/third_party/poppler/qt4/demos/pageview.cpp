/*
 * Copyright (C) 2008-2009, Pino Toscano <pino@kde.org>
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

#include "pageview.h"

#include <poppler-qt4.h>

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>

PageView::PageView(QWidget *parent)
    : QScrollArea(parent)
    , m_zoom(1.0)
    , m_dpiX(QApplication::desktop()->physicalDpiX())
    , m_dpiY(QApplication::desktop()->physicalDpiY())
{
    m_imageLabel = new QLabel(this);
    m_imageLabel->resize(0, 0);
    setWidget(m_imageLabel);
}

PageView::~PageView()
{
}

void PageView::documentLoaded()
{
}

void PageView::documentClosed()
{
    m_imageLabel->clear();
    m_imageLabel->resize(0, 0);
}

void PageView::pageChanged(int page)
{
    Poppler::Page *popplerPage = document()->page(page);
    const double resX = m_dpiX * m_zoom;
    const double resY = m_dpiY * m_zoom;
    QImage image = popplerPage->renderToImage(resX, resY);
    if (!image.isNull()) {
        m_imageLabel->resize(image.size());
        m_imageLabel->setPixmap(QPixmap::fromImage(image));
    } else {
        m_imageLabel->resize(0, 0);
        m_imageLabel->setPixmap(QPixmap());
    }
    delete popplerPage;
}

void PageView::slotZoomChanged(qreal value)
{
    m_zoom = value;
    if (!document()) {
        return;
    }
    reloadPage();
}

#include "pageview.moc"
