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

#include "permissions.h"

#include <poppler-qt4.h>

#include <QtGui/QListWidget>

PermissionsDock::PermissionsDock(QWidget *parent)
    : AbstractInfoDock(parent)
{
    m_table = new QListWidget(this);
    setWidget(m_table);
    setWindowTitle(tr("Permissions"));
    m_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

PermissionsDock::~PermissionsDock()
{
}

void PermissionsDock::fillInfo()
{
#define ADD_ROW(title, function) \
do { \
    QListWidgetItem *item = new QListWidgetItem(); \
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled); \
    item->setText(title); \
    item->setCheckState(document()->function() ? Qt::Checked : Qt::Unchecked); \
    m_table->addItem(item); \
} while (0)
    ADD_ROW("Print", okToPrint);
    ADD_ROW("PrintHiRes", okToPrintHighRes);
    ADD_ROW("Change", okToChange);
    ADD_ROW("Copy", okToCopy);
    ADD_ROW("Add Notes", okToAddNotes);
    ADD_ROW("Fill Forms", okToFillForm);
    ADD_ROW("Create Forms", okToCreateFormFields);
    ADD_ROW("Extract for accessibility", okToExtractForAccessibility);
    ADD_ROW("Assemble", okToAssemble);
#undef ADD_ROW
}

void PermissionsDock::documentClosed()
{
    m_table->clear();
    AbstractInfoDock::documentClosed();
}

#include "permissions.moc"
