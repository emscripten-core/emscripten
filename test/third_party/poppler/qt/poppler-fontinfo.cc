/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Albert Astals Cid <aacid@kde.org>
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

#include "poppler-qt.h"

namespace Poppler {

class FontInfoData
{
	public:
		QString fontName;
		bool isEmbedded;
		bool isSubset;
		FontInfo::Type type;
};

FontInfo::FontInfo( const QString &fontName, const bool isEmbedded, const bool isSubset, Type type )
{
	data = new FontInfoData();
	data->fontName = fontName;
	data->isEmbedded = isEmbedded;
	data->isSubset = isSubset;
	data->type = type;
}

FontInfo::FontInfo( const FontInfo &fi )
{
	data = new FontInfoData();
	data->fontName = fi.data->fontName;
	data->isEmbedded = fi.data->isEmbedded;
	data->isSubset = fi.data->isSubset;
	data->type = fi.data->type;
}

FontInfo::FontInfo()
{
	data = new FontInfoData();
	data->isEmbedded = false;
	data->isSubset = false;
	data->type = unknown;
}

FontInfo::~FontInfo()
{
	delete data;
}

const QString &FontInfo::name() const
{
	return data->fontName;
}

bool FontInfo::isEmbedded() const
{
	return data->isEmbedded;
}

bool FontInfo::isSubset() const
{
	return data->isSubset;
}

FontInfo::Type FontInfo::type() const
{
	return data->type;
}

}
