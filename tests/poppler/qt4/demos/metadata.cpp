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

#include "metadata.h"

#include <poppler-qt4.h>

#include <QtGui/QTextEdit>

MetadataDock::MetadataDock(QWidget *parent)
    : AbstractInfoDock(parent)
{
    m_edit = new QTextEdit(this);
    setWidget(m_edit);
    setWindowTitle(tr("Metadata"));
    m_edit->setAcceptRichText(false);
    m_edit->setReadOnly(true);
}

MetadataDock::~MetadataDock()
{
}

void MetadataDock::fillInfo()
{
    m_edit->setPlainText(document()->metadata());
}

void MetadataDock::documentClosed()
{
    m_edit->clear();
    AbstractInfoDock::documentClosed();
}

#include "metadata.moc"
