/* poppler-converter-private.h: Qt4 interface to poppler
 * Copyright (C) 2007, 2009, Albert Astals Cid <aacid@kde.org>
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

#ifndef POPPLER_QT4_CONVERTER_PRIVATE_H
#define POPPLER_QT4_CONVERTER_PRIVATE_H

#include <QtCore/QString>

class QIODevice;

namespace Poppler {

class DocumentData;

class BaseConverterPrivate
{
	public:
		BaseConverterPrivate();
		virtual ~BaseConverterPrivate();

		QIODevice* openDevice();
		void closeDevice();

		DocumentData *document;
		QString outputFileName;
		QIODevice *iodev;
		bool ownIodev : 1;
		BaseConverter::Error lastError;
};

}

#endif
