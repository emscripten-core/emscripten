/* poppler-base-converter.cc: qt interface to poppler
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

#include "poppler-qt4.h"

#include "poppler-converter-private.h"

#include <QtCore/QFile>

namespace Poppler {

BaseConverterPrivate::BaseConverterPrivate()
	: document(0), iodev(0), ownIodev(true)
{
}

BaseConverterPrivate::~BaseConverterPrivate()
{
}

QIODevice* BaseConverterPrivate::openDevice()
{
	if (!iodev)
	{
		Q_ASSERT(!outputFileName.isEmpty());
		QFile *f = new QFile(outputFileName);
		iodev = f;
		ownIodev = true;
	}
	Q_ASSERT(iodev);
	if (!iodev->isOpen())
	{
		if (!iodev->open(QIODevice::WriteOnly))
		{
			if (ownIodev)
			{
				delete iodev;
				iodev = 0;
			}
			else
			{
				return 0;
			}
		}
	}
	return iodev;
}

void BaseConverterPrivate::closeDevice()
{
	if (ownIodev)
	{
		iodev->close();
		delete iodev;
		iodev = 0;
	}
}


BaseConverter::BaseConverter(BaseConverterPrivate &dd)
	: d_ptr(&dd)
{
}

BaseConverter::~BaseConverter()
{
	delete d_ptr;
}

void BaseConverter::setOutputFileName(const QString &outputFileName)
{
	Q_D(BaseConverter);
	d->outputFileName = outputFileName;
}

void BaseConverter::setOutputDevice(QIODevice *device)
{
	Q_D(BaseConverter);
	d->iodev = device;
	d->ownIodev = false;
}

BaseConverter::Error BaseConverter::lastError() const
{
	Q_D(const BaseConverter);
	return d->lastError;
}

}
