/*
 * Copyright (C) 2009, Shawn Rutledge <shawn.t.rutledge@gmail.com>
 * Copyright (C) 2009, Pino Toscano <pino@kde.org>
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

#include "thumbnails.h"

#include <poppler-qt4.h>

#include <QtGui/QListWidget>

static const int PageRole = Qt::UserRole + 1;

ThumbnailsDock::ThumbnailsDock(QWidget *parent)
    : AbstractInfoDock(parent)
{
    m_list = new QListWidget(this);
    setWidget(m_list);
    setWindowTitle(tr("Thumbnails"));
    m_list->setViewMode(QListView::ListMode);
    m_list->setMovement(QListView::Static);
    m_list->setVerticalScrollMode(QListView::ScrollPerPixel);
    connect(m_list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(slotItemActivated(QListWidgetItem*)));
}

ThumbnailsDock::~ThumbnailsDock()
{
}

void ThumbnailsDock::fillInfo()
{
    const int num = document()->numPages();
    QSize maxSize;
    for (int i = 0; i < num; ++i) {
        const Poppler::Page *page = document()->page(i);
        const QImage image = page->thumbnail();
        if (!image.isNull()) {
            QListWidgetItem *item = new QListWidgetItem();
            item->setText(QString::number(i + 1));
            item->setData(Qt::DecorationRole, QPixmap::fromImage(image));
            item->setData(PageRole, i);
            m_list->addItem(item);
            maxSize.setWidth(qMax(maxSize.width(), image.width()));
            maxSize.setHeight(qMax(maxSize.height(), image.height()));
        }
        delete page;
    }
    if (num > 0) {
        m_list->setGridSize(maxSize);
        m_list->setIconSize(maxSize);
    }
}

void ThumbnailsDock::documentClosed()
{
    m_list->clear();
    AbstractInfoDock::documentClosed();
}

void ThumbnailsDock::slotItemActivated(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    setPage(item->data(PageRole).toInt());
}

#include "thumbnails.moc"
