/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2006-2008, Albert Astals Cid <aacid@kde.org>
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

#include "poppler-qt4.h"
#include "poppler-private.h"

namespace Poppler {

TextBox::TextBox(const QString& text, const QRectF &bBox)
{
	m_data = new TextBoxData();
	m_data->text = text;
	m_data->bBox = bBox;
}

TextBox::~TextBox()
{
	delete m_data;
}

QString TextBox::text() const
{
	return m_data->text;
}

QRectF TextBox::boundingBox() const
{
	return m_data->bBox;
}

TextBox *TextBox::nextWord() const
{
	return m_data->nextWord;
}

QRectF TextBox::charBoundingBox(int i) const
{
	return m_data->charBBoxes.value(i);
}

bool TextBox::hasSpaceAfter() const
{
	return m_data->hasSpaceAfter;
}

}
