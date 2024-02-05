/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Tobias Koening <tokoe@kde.org>
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2005-2008, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2008, 2009, Pino Toscano <pino@kde.org>
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

FontInfo::FontInfo()
{
	m_data = new FontInfoData();
}

FontInfo::FontInfo( const FontInfoData &fid )
{
	m_data = new FontInfoData(fid);
}

FontInfo::FontInfo( const FontInfo &fi )
{
	m_data = new FontInfoData(*fi.m_data);
}

FontInfo::~FontInfo()
{
	delete m_data;
}

QString FontInfo::name() const
{
	return m_data->fontName;
}

QString FontInfo::file() const
{
	return m_data->fontFile;
}

bool FontInfo::isEmbedded() const
{
	return m_data->isEmbedded;
}

bool FontInfo::isSubset() const
{
	return m_data->isSubset;
}

FontInfo::Type FontInfo::type() const
{
	return m_data->type;
}

QString FontInfo::typeName() const
{
	switch (type()) {
	case unknown:
		return QObject::tr("unknown");
	case Type1:
		return QObject::tr("Type 1");
	case Type1C:
		return QObject::tr("Type 1C");
	case Type3:
		return QObject::tr("Type 3");
	case TrueType:
		return QObject::tr("TrueType");
	case CIDType0:
		return QObject::tr("CID Type 0");
	case CIDType0C:
		return QObject::tr("CID Type 0C");
	case CIDTrueType:
		return QObject::tr("CID TrueType");
	case Type1COT:
		return QObject::tr("Type 1C (OpenType)");
	case TrueTypeOT:
		return QObject::tr("TrueType (OpenType)");
	case CIDType0COT:
		return QObject::tr("CID Type 0C (OpenType)");
	case CIDTrueTypeOT:
		return QObject::tr("CID TrueType (OpenType)");
	}
	return QObject::tr("Bug: unexpected font type. Notify poppler mailing list!");
}

FontInfo& FontInfo::operator=( const FontInfo &fi )
{
	if (this == &fi)
		return *this;

	*m_data = *fi.m_data;
	return *this;
}


FontIterator::FontIterator( int startPage, DocumentData *dd )
	: d( new FontIteratorData( startPage, dd ) )
{
}

FontIterator::~FontIterator()
{
	delete d;
}

QList<FontInfo> FontIterator::next()
{
	++d->currentPage;

	QList<FontInfo> fonts;
	GooList *items = d->fontInfoScanner.scan( 1 );
	if ( !items )
		return fonts;
	for ( int i = 0; i < items->getLength(); ++i ) {
		fonts.append( FontInfo( FontInfoData( ( ::FontInfo* )items->get( i ) ) ) );
	}
	deleteGooList( items, ::FontInfo );
	return fonts;
}

bool FontIterator::hasNext() const
{
	return ( d->currentPage + 1 ) < d->totalPages;
}

int FontIterator::currentPage() const
{
	return d->currentPage;
}

}
