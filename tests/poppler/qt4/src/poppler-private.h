/* poppler-private.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, 2008, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2006-2009, 2011, 2012 by Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2007-2009, 2011 by Pino Toscano <pino@kde.org>
 * Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
 * Copyright (C) 2011 Hib Eris <hib@hiberis.nl>
 * Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
 * Copyright (C) 2013 Julien Nabet <serval2412@yahoo.fr>
 * Inspired on code by
 * Copyright (C) 2004 by Albert Astals Cid <tsdgeos@terra.es>
 * Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

#ifndef _POPPLER_PRIVATE_H_
#define _POPPLER_PRIVATE_H_

#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QVector>

#include <config.h>
#include <GfxState.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <FontInfo.h>
#include <OutputDev.h>
#include <Error.h>
#if defined(HAVE_SPLASH)
#include <SplashOutputDev.h>
#endif

#include "poppler-qt4.h"
#include "poppler-embeddedfile-private.h"

class LinkDest;
class FormWidget;

namespace Poppler {

    /* borrowed from kpdf */
    QString unicodeToQString(Unicode* u, int len);

    QString UnicodeParsedString(GooString *s1);

    GooString *QStringToUnicodeGooString(const QString &s);

    GooString *QStringToGooString(const QString &s);

    void qt4ErrorFunction(int pos, char *msg, va_list args);

    class LinkDestinationData
    {
        public:
            LinkDestinationData( LinkDest *l, GooString *nd, Poppler::DocumentData *pdfdoc, bool external )
             : ld(l), namedDest(nd), doc(pdfdoc), externalDest(external)
            {
            }

            LinkDest *ld;
            GooString *namedDest;
            Poppler::DocumentData *doc;
            bool externalDest;
    };

    class DocumentData {
    public:
	DocumentData(const QString &filePath, GooString *ownerPassword, GooString *userPassword)
	    {
		init();
		m_filePath = filePath;	

#if defined(_WIN32)
		wchar_t *fileName = new WCHAR[filePath.length()];
		int length = filePath.toWCharArray(fileName); 
		doc = new PDFDoc(fileName, length, ownerPassword, userPassword);
		delete[] fileName;
#else
		GooString *fileName = new GooString(QFile::encodeName(filePath));
		doc = new PDFDoc(fileName, ownerPassword, userPassword);
#endif

		delete ownerPassword;
		delete userPassword;
	    }
	
	DocumentData(const QByteArray &data, GooString *ownerPassword, GooString *userPassword)
	    {
		Object obj;
		fileContents = data;
		obj.initNull();
		MemStream *str = new MemStream((char*)fileContents.data(), 0, fileContents.length(), &obj);
		init();
		doc = new PDFDoc(str, ownerPassword, userPassword);
		delete ownerPassword;
		delete userPassword;
	    }
	
	void init();
	
	~DocumentData();
	
	void addTocChildren( QDomDocument * docSyn, QDomNode * parent, GooList * items );
	
	void setPaperColor(const QColor &color)
	{
		paperColor = color;
	}
	
	void fillMembers()
	{
		m_fontInfoIterator = new FontIterator(0, this);
		int numEmb = doc->getCatalog()->numEmbeddedFiles();
		if (!(0 == numEmb)) {
			// we have some embedded documents, build the list
			for (int yalv = 0; yalv < numEmb; ++yalv) {
				FileSpec *fs = doc->getCatalog()->embeddedFile(yalv);
				m_embeddedFiles.append(new EmbeddedFile(*new EmbeddedFileData(fs)));
			}
		}
	}
	
	static Document *checkDocument(DocumentData *doc);

	PDFDoc *doc;
	QString m_filePath;
	QByteArray fileContents;
	bool locked;
	FontIterator *m_fontInfoIterator;
	Document::RenderBackend m_backend;
	QList<EmbeddedFile*> m_embeddedFiles;
	QPointer<OptContentModel> m_optContentModel;
	QColor paperColor;
	int m_hints;
	static int count;
    };

    class FontInfoData
    {
	public:
		FontInfoData()
		{
			isEmbedded = false;
			isSubset = false;
			type = FontInfo::unknown;
		}
		
		FontInfoData( const FontInfoData &fid )
		{
			fontName = fid.fontName;
			fontFile = fid.fontFile;
			isEmbedded = fid.isEmbedded;
			isSubset = fid.isSubset;
			type = fid.type;
			embRef = fid.embRef;
		}
		
		FontInfoData( ::FontInfo* fi )
		{
			if (fi->getName()) fontName = fi->getName()->getCString();
			if (fi->getFile()) fontFile = fi->getFile()->getCString();
			isEmbedded = fi->getEmbedded();
			isSubset = fi->getSubset();
			type = (Poppler::FontInfo::Type)fi->getType();
			embRef = fi->getEmbRef();
		}

		QString fontName;
		QString fontFile;
		bool isEmbedded : 1;
		bool isSubset : 1;
		FontInfo::Type type;
		Ref embRef;
    };

    class FontIteratorData
    {
	public:
		FontIteratorData( int startPage, DocumentData *dd )
		  : fontInfoScanner( dd->doc, startPage )
		  , totalPages( dd->doc->getNumPages() )
		  , currentPage( qMax( startPage, 0 ) - 1 )
		{
		}

		~FontIteratorData()
		{
		}

		FontInfoScanner fontInfoScanner;
		int totalPages;
		int currentPage;
    };

    class TextBoxData
    {
	public:
		TextBoxData()
		  : nextWord(0), hasSpaceAfter(false)
		{
		}

		QString text;
		QRectF bBox;
		TextBox *nextWord;
		QVector<QRectF> charBBoxes; // the boundingRect of each character
		bool hasSpaceAfter;
    };

    class FormFieldData
    {
	public:
		FormFieldData(DocumentData *_doc, ::Page *p, ::FormWidget *w) :
		doc(_doc), page(p), fm(w)
		{
		}

		DocumentData *doc;
		::Page *page;
		::FormWidget *fm;
		QRectF box;
    };

}

#endif
