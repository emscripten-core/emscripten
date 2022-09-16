/* poppler-document.cc: qt interface to poppler
 * Copyright (C) 2005, 2008, 2009, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
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

#include <QtCore/QString>
#include <QtCore/QDateTime>

#include "Object.h"
#include "Stream.h"
#include "Catalog.h"

#include "poppler-private.h"

namespace Poppler
{

class EmbeddedFileData
{
public:
	EmbFile *embfile;
};

EmbeddedFile::EmbeddedFile(EmbFile *embfile)
{
	m_embeddedFile = new EmbeddedFileData();
	m_embeddedFile->embfile = embfile;
}

EmbeddedFile::~EmbeddedFile()
{
	delete m_embeddedFile->embfile;
	delete m_embeddedFile;
}

QString EmbeddedFile::name() const
{
	return QString(m_embeddedFile->embfile->name()->getCString());
}

QString EmbeddedFile::description() const
{
	return UnicodeParsedString(m_embeddedFile->embfile->description());
}

int EmbeddedFile::size() const
{
	return m_embeddedFile->embfile->size();
}

QDateTime EmbeddedFile::modDate() const
{
	return convertDate(m_embeddedFile->embfile->modDate()->getCString());
}

QDateTime EmbeddedFile::createDate() const
{
	return convertDate(m_embeddedFile->embfile->createDate()->getCString());
}

QByteArray EmbeddedFile::checksum() const
{
	GooString *goo_checksum = m_embeddedFile->embfile->checksum();
	return QByteArray::fromRawData(goo_checksum->getCString(), goo_checksum->getLength());
}

QString EmbeddedFile::mimeType() const
{
	return QString(m_embeddedFile->embfile->mimeType()->getCString());
}

QByteArray EmbeddedFile::data()
{
	if (!isValid())
		return QByteArray();
	
	Object obj;
	Stream *stream = m_embeddedFile->embfile->streamObject().getStream();
	stream->reset();
	int dataLen = 0;
	QByteArray fileArray;
	int i;
	while ( (i = stream->getChar()) != EOF) {
		fileArray[dataLen] = (char)i;
		++dataLen;
	}
	fileArray.resize(dataLen);
	return fileArray;
}

bool EmbeddedFile::isValid() const
{
	return m_embeddedFile->embfile->isOk();
}

}
