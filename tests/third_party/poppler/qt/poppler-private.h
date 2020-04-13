/* poppler-private.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005-2008, 2010 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006, Kristian Høgsberg <krh@bitplanet.net>
 * Copyright (C) 2006, Wilfried Huss <Wilfried.Huss@gmx.at>
 * Copyright (C) 2007, Pino Toscano <pino@kde.org>
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

#include <qdom.h>

#include <config.h>
#include <Object.h>
#include <PDFDoc.h>
#include <FontInfo.h>
#if defined(HAVE_SPLASH)
#include <SplashOutputDev.h>
#else
class SplashOutputDev;
#endif

namespace Poppler {

class Document;
class DocumentData;
class PageTransition;

QString unicodeToQString(Unicode* u, int len);

QString UnicodeParsedString(GooString *s1);

GooString *QStringToGooString(const QString &s);

class LinkDestinationData {
  public:
     LinkDestinationData( LinkDest *l, GooString *nd, Poppler::DocumentData *pdfdoc ) : ld(l), namedDest(nd), doc(pdfdoc)
     {
     }
	
     LinkDest *ld;
     GooString *namedDest;
     Poppler::DocumentData *doc;
};

class DocumentData {
  public:
    DocumentData(GooString *filePath, GooString *password) : doc(filePath,password), m_fontInfoScanner(0), m_outputDev(0) {}

    ~DocumentData()
    {
#if defined(HAVE_SPLASH)
        delete m_outputDev;
#endif
        delete m_fontInfoScanner;
    }

    SplashOutputDev *getOutputDev()
    {
#if defined(HAVE_SPLASH)
        if (!m_outputDev)
        {
            SplashColor white;
            white[0] = 255;
            white[1] = 255;
            white[2] = 255;
            m_outputDev = new SplashOutputDev(splashModeXBGR8, 4, gFalse, white);
            m_outputDev->startDoc(doc.getXRef());
        }
#endif
        return m_outputDev;
    }

    void addTocChildren( QDomDocument * docSyn, QDomNode * parent, GooList * items );

  class PDFDoc doc;
  bool locked;
  FontInfoScanner *m_fontInfoScanner;
  SplashOutputDev *m_outputDev;
};

class PageData {
  public:
  const Document *doc;
  int index;
  PageTransition *transition;
  ::Page *page;
};

}
