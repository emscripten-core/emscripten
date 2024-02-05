/*
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
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

#include "info.h"

#include <poppler-qt4.h>

#include <QtGui/QTableWidget>

InfoDock::InfoDock(QWidget *parent)
    : AbstractInfoDock(parent)
{
    m_table = new QTableWidget(this);
    setWidget(m_table);
    setWindowTitle(tr("Information"));
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Value"));
    m_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

InfoDock::~InfoDock()
{
}

void InfoDock::fillInfo()
{
    QStringList keys = document()->infoKeys();
    m_table->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Value"));
    m_table->setRowCount(keys.count());
    QStringList dateKeys;
    dateKeys << QString::fromLatin1("CreationDate");
    dateKeys << QString::fromLatin1("ModDate");
    int i = 0;
    Q_FOREACH(const QString &date, dateKeys) {
        const int id = keys.indexOf(date);
        if (id != -1) {
            m_table->setItem(i, 0, new QTableWidgetItem(date));
            m_table->setItem(i, 1, new QTableWidgetItem(document()->date(date).toString(Qt::SystemLocaleDate)));
            ++i;
            keys.removeAt(id);
        }
    }
    Q_FOREACH(const QString &key, keys) {
        m_table->setItem(i, 0, new QTableWidgetItem(key));
        m_table->setItem(i, 1, new QTableWidgetItem(document()->info(key)));
        ++i;
    }
}

void InfoDock::documentClosed()
{
    m_table->clear();
    m_table->setRowCount(0);
    AbstractInfoDock::documentClosed();
}

#include "info.moc"
