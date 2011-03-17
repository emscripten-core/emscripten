/* poppler-ps-converter.cc: qt interface to poppler
 * Copyright (C) 2007, 2009, 2010, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
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
#include "poppler-converter-private.h"

#include "PSOutputDev.h"

static void outputToQIODevice(void *stream, char *data, int len)
{
	static_cast<QIODevice*>(stream)->write(data, len);
}

namespace Poppler {

class PSConverterPrivate : public BaseConverterPrivate
{
	public:
		PSConverterPrivate();

		QList<int> pageList;
		QString title;
		double hDPI;
		double vDPI;
		int rotate;
		int paperWidth;
		int paperHeight;
		int marginRight;
		int marginBottom;
		int marginLeft;
		int marginTop;
		PSConverter::PSOptions opts;
		void (* pageConvertedCallback)(int page, void *payload);
		void *pageConvertedPayload;
};

PSConverterPrivate::PSConverterPrivate()
	: BaseConverterPrivate(),
	hDPI(72), vDPI(72), rotate(0), paperWidth(-1), paperHeight(-1),
	marginRight(0), marginBottom(0), marginLeft(0), marginTop(0),
	opts(PSConverter::Printing), pageConvertedCallback(0),
	pageConvertedPayload(0)
{
}


PSConverter::PSConverter(DocumentData *document)
	: BaseConverter(*new PSConverterPrivate())
{
	Q_D(PSConverter);
	d->document = document;
}

PSConverter::~PSConverter()
{
}

void PSConverter::setPageList(const QList<int> &pageList)
{
	Q_D(PSConverter);
	d->pageList = pageList;
}

void PSConverter::setTitle(const QString &title)
{
	Q_D(PSConverter);
	d->title = title;
}

void PSConverter::setHDPI(double hDPI)
{
	Q_D(PSConverter);
	d->hDPI = hDPI;
}

void PSConverter::setVDPI(double vDPI)
{
	Q_D(PSConverter);
	d->vDPI = vDPI;
}

void PSConverter::setRotate(int rotate)
{
	Q_D(PSConverter);
	d->rotate = rotate;
}

void PSConverter::setPaperWidth(int paperWidth)
{
	Q_D(PSConverter);
	d->paperWidth = paperWidth;
}

void PSConverter::setPaperHeight(int paperHeight)
{
	Q_D(PSConverter);
	d->paperHeight = paperHeight;
}

void PSConverter::setRightMargin(int marginRight)
{
	Q_D(PSConverter);
	d->marginRight = marginRight;
}

void PSConverter::setBottomMargin(int marginBottom)
{
	Q_D(PSConverter);
	d->marginBottom = marginBottom;
}

void PSConverter::setLeftMargin(int marginLeft)
{
	Q_D(PSConverter);
	d->marginLeft = marginLeft;
}

void PSConverter::setTopMargin(int marginTop)
{
	Q_D(PSConverter);
	d->marginTop = marginTop;
}

void PSConverter::setStrictMargins(bool strictMargins)
{
	Q_D(PSConverter);
	if (strictMargins)
		d->opts |= StrictMargins;
	else
		d->opts &= ~StrictMargins;
}

void PSConverter::setForceRasterize(bool forceRasterize)
{
	Q_D(PSConverter);
	if (forceRasterize)
		d->opts |= ForceRasterization;
	else
		d->opts &= ~ForceRasterization;
}

void PSConverter::setPSOptions(PSConverter::PSOptions options)
{
	Q_D(PSConverter);
	d->opts = options;
}

PSConverter::PSOptions PSConverter::psOptions() const
{
	Q_D(const PSConverter);
	return d->opts;
}

void PSConverter::setPageConvertedCallback(void (* callback)(int page, void *payload), void *payload)
{
	Q_D(PSConverter);
	d->pageConvertedCallback = callback;
	d->pageConvertedPayload = payload;
}

bool PSConverter::convert()
{
	Q_D(PSConverter);
	d->lastError = NoError;

	Q_ASSERT(!d->pageList.isEmpty());
	Q_ASSERT(d->paperWidth != -1);
	Q_ASSERT(d->paperHeight != -1);
	
	if (d->document->locked)
	{
		d->lastError = FileLockedError;
		return false;
	}
	
	QIODevice *dev = d->openDevice();
	if (!dev)
	{
		d->lastError = OpenOutputError;
		return false;
	}

	QByteArray pstitle8Bit = d->title.toLocal8Bit();
	char* pstitlechar;
	if (!d->title.isEmpty()) pstitlechar = pstitle8Bit.data();
	else pstitlechar = 0;
	
	PSOutputDev *psOut = new PSOutputDev(outputToQIODevice, dev,
	                                     pstitlechar,
	                                     d->document->doc,
	                                     d->document->doc->getXRef(),
	                                     d->document->doc->getCatalog(),
	                                     1,
	                                     d->document->doc->getNumPages(),
	                                     psModePS,
	                                     d->paperWidth,
	                                     d->paperHeight,
	                                     gFalse,
	                                     d->marginLeft,
	                                     d->marginBottom,
	                                     d->paperWidth - d->marginRight,
	                                     d->paperHeight - d->marginTop,
	                                     (d->opts & ForceRasterization));
	
	if (d->opts & StrictMargins)
	{
		double xScale = ((double)d->paperWidth - (double)d->marginLeft - (double)d->marginRight) / (double)d->paperWidth;
		double yScale = ((double)d->paperHeight - (double)d->marginBottom - (double)d->marginTop) / (double)d->paperHeight;
		psOut->setScale(xScale, yScale);
	}
	
	if (psOut->isOk())
	{
		GBool isPrinting = (d->opts & Printing) ? gTrue : gFalse;
		foreach(int page, d->pageList)
		{
			d->document->doc->displayPage(psOut, page, d->hDPI, d->vDPI, d->rotate, gFalse, gTrue, isPrinting);
			if (d->pageConvertedCallback)
				(*d->pageConvertedCallback)(page, d->pageConvertedPayload);
		}
		delete psOut;
		d->closeDevice();
		return true;
	}
	else
	{
		delete psOut;
		d->closeDevice();
		return false;
	}
}

}
