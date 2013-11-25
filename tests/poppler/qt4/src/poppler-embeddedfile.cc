/* poppler-document.cc: qt interface to poppler
 * Copyright (C) 2005, 2008, 2009, 2012, 2013, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2008, 2011, Pino Toscano <pino@kde.org>
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
#include "FileSpec.h"

#include "poppler-private.h"
#include "poppler-embeddedfile-private.h"

namespace Poppler
{

EmbeddedFileData::EmbeddedFileData(FileSpec *fs)
	: filespec(fs)
{
}

EmbeddedFileData::~EmbeddedFileData()
{
	delete filespec;
}

EmbFile *EmbeddedFileData::embFile() const
{
	return filespec->isOk() ? filespec->getEmbeddedFile() : NULL;
}


EmbeddedFile::EmbeddedFile(EmbFile *embfile)
	: m_embeddedFile(0)
{
	assert(!"You must not use this private constructor!");
}

EmbeddedFile::EmbeddedFile(EmbeddedFileData &dd)
	: m_embeddedFile(&dd)
{
}

EmbeddedFile::~EmbeddedFile()
{
	delete m_embeddedFile;
}

QString EmbeddedFile::name() const
{
	GooString *goo = m_embeddedFile->filespec->getFileName();
	return goo ? UnicodeParsedString(goo) : QString();
}

QString EmbeddedFile::description() const
{
	GooString *goo = m_embeddedFile->filespec->getDescription();
	return goo ? UnicodeParsedString(goo) : QString();
}

int EmbeddedFile::size() const
{
	return m_embeddedFile->embFile() ? m_embeddedFile->embFile()->size() : -1;
}

QDateTime EmbeddedFile::modDate() const
{
	GooString *goo = m_embeddedFile->embFile() ? m_embeddedFile->embFile()->modDate() : NULL;
	return goo ? convertDate(goo->getCString()) : QDateTime();
}

QDateTime EmbeddedFile::createDate() const
{
	GooString *goo = m_embeddedFile->embFile() ? m_embeddedFile->embFile()->createDate() : NULL;
	return goo ? convertDate(goo->getCString()) : QDateTime();
}

QByteArray EmbeddedFile::checksum() const
{
	GooString *goo = m_embeddedFile->embFile() ? m_embeddedFile->embFile()->checksum() : NULL;
	return goo ? QByteArray::fromRawData(goo->getCString(), goo->getLength()) : QByteArray();
}

QString EmbeddedFile::mimeType() const
{
	GooString *goo = m_embeddedFile->embFile() ? m_embeddedFile->embFile()->mimeType() : NULL;
	return goo ? QString(goo->getCString()) : QString();
}

QByteArray EmbeddedFile::data()
{
	if (!isValid())
		return QByteArray();
	Stream *stream = m_embeddedFile->embFile() ? m_embeddedFile->embFile()->stream() : NULL;
	if (!stream)
		return QByteArray();
	
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
	return m_embeddedFile->filespec->isOk();
}

}
