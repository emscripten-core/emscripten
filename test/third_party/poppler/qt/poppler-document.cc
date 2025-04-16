/* poppler-document.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005-2009, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006, Stefan Kebekus <stefan.kebekus@math.uni-koeln.de>
 * Copyright (C) 2006, Wilfried Huss <Wilfried.Huss@gmx.at>
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

#include <poppler-qt.h>
#include <qfile.h>
#include <GlobalParams.h>
#include <Outline.h>
#include <PDFDoc.h>
#include <PSOutputDev.h>
#include <Catalog.h>
#include <ErrorCodes.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#include <DateInfo.h>
#include "poppler-private.h"

namespace Poppler {

Document *Document::load(const QString &filePath)
{
  if (!globalParams) {
    globalParams = new GlobalParams();
  }

  DocumentData *doc = new DocumentData(new GooString(QFile::encodeName(filePath)), NULL);
  Document *pdoc;
  if (doc->doc.isOk() || doc->doc.getErrorCode() == errEncrypted) {
    pdoc = new Document(doc);
    if (doc->doc.getErrorCode() == errEncrypted)
      pdoc->data->locked = true;
    else
      pdoc->data->locked = false;
    pdoc->data->m_fontInfoScanner = new FontInfoScanner(&(doc->doc));
    return pdoc;
  }
  else
    return NULL;
}

Document::Document(DocumentData *dataA)
{
  data = dataA;
}

Document::~Document()
{
  delete data;
}

bool Document::isLocked() const
{
  return data->locked;
}

bool Document::unlock(const QCString &password)
{
  if (data->locked) {
    /* racier then it needs to be */
    GooString *filename = new GooString(data->doc.getFileName());
    GooString *pwd = new GooString(password.data());
    DocumentData *doc2 = new DocumentData(filename, pwd);
    delete pwd;
    if (!doc2->doc.isOk()) {
      delete doc2;
    } else {
      delete data;
      data = doc2;
      data->locked = false;
      data->m_fontInfoScanner = new FontInfoScanner(&(data->doc));
    }
  }
  return data->locked;
}

Document::PageMode Document::getPageMode(void) const
{
  switch (data->doc.getCatalog()->getPageMode()) {
    case Catalog::pageModeNone:
      return UseNone;
    case Catalog::pageModeOutlines:
      return UseOutlines;
    case Catalog::pageModeThumbs:
      return UseThumbs;
    case Catalog::pageModeFullScreen:
      return FullScreen;
    case Catalog::pageModeOC:
      return UseOC;
    default:
      return UseNone;
  }
}

int Document::getNumPages() const
{
  return data->doc.getNumPages();
}

Page *Document::getPage(int index) const
{
  Page *p = new Page(this, index);
  if (p->data->page == NULL) {
    delete p;
    return NULL;
  }

  return p;
}

QValueList<FontInfo> Document::fonts() const
{
  QValueList<FontInfo> ourList;
  scanForFonts(getNumPages(), &ourList);
  return ourList;
}

bool Document::scanForFonts( int numPages, QValueList<FontInfo> *fontList ) const
{
  GooList *items = data->m_fontInfoScanner->scan( numPages );

  if ( NULL == items )
    return false;

  for ( int i = 0; i < items->getLength(); ++i ) {
    QString fontName;
    if (((::FontInfo*)items->get(i))->getName())
      fontName = ((::FontInfo*)items->get(i))->getName()->getCString();

    FontInfo font(fontName,
                  ((::FontInfo*)items->get(i))->getEmbedded(),
                  ((::FontInfo*)items->get(i))->getSubset(),
                  (Poppler::FontInfo::Type)((::FontInfo*)items->get(i))->getType());
    fontList->append(font);
  }
  deleteGooList(items, ::FontInfo);
  return true;
}

/* borrowed from kpdf */
QString Document::getInfo( const QString & type ) const
{
  // [Albert] Code adapted from pdfinfo.cc on xpdf
  Object info;
  if ( data->locked )
    return NULL;

  data->doc.getDocInfo( &info );
  if ( !info.isDict() )
    return NULL;

  QString result;
  Object obj;
  GooString *s1;
  GBool isUnicode;
  Unicode u;
  int i;
  Dict *infoDict = info.getDict();

  if ( infoDict->lookup( (char*)type.latin1(), &obj )->isString() )
  {
    s1 = obj.getString();
    if ( ( s1->getChar(0) & 0xff ) == 0xfe && ( s1->getChar(1) & 0xff ) == 0xff )
    {
      isUnicode = gTrue;
      i = 2;
    }
    else
    {
      isUnicode = gFalse;
      i = 0;
    }
    while ( i < obj.getString()->getLength() )
    {
      if ( isUnicode )
      {
	u = ( ( s1->getChar(i) & 0xff ) << 8 ) | ( s1->getChar(i+1) & 0xff );
	i += 2;
      }
      else
      {
	u = s1->getChar(i) & 0xff;
	++i;
      }
      result += unicodeToQString( &u, 1 );
    }
    obj.free();
    info.free();
    return result;
  }
  obj.free();
  info.free();
  return NULL;
}

/* borrowed from kpdf */
QDateTime Document::getDate( const QString & type ) const
{
  // [Albert] Code adapted from pdfinfo.cc on xpdf
  if ( data->locked )
    return QDateTime();

  Object info;
  data->doc.getDocInfo( &info );
  if ( !info.isDict() ) {
    info.free();
    return QDateTime();
  }

  Object obj;
  int year, mon, day, hour, min, sec, tz_hour, tz_minute;
  char tz;
  Dict *infoDict = info.getDict();
  QString result;

  if ( infoDict->lookup( (char*)type.latin1(), &obj )->isString() )
  {
    QString s = UnicodeParsedString(obj.getString());
    // TODO do something with the timezone information
    if ( parseDateString( s.latin1(), &year, &mon, &day, &hour, &min, &sec, &tz, &tz_hour, &tz_minute ) )
    {
      QDate d( year, mon, day );  //CHECK: it was mon-1, Jan->0 (??)
      QTime t( hour, min, sec );
      if ( d.isValid() && t.isValid() ) {
	obj.free();
	info.free();
	return QDateTime( d, t );
      }
    }
  }
  obj.free();
  info.free();
  return QDateTime();
}

bool Document::isEncrypted() const
{
  return data->doc.isEncrypted();
}

bool Document::isLinearized() const
{
  return data->doc.isLinearized();
}

bool Document::okToPrint() const
{
  return data->doc.okToPrint();
}

bool Document::okToChange() const
{
  return data->doc.okToChange();
}

bool Document::okToCopy() const
{
  return data->doc.okToCopy();
}

bool Document::okToAddNotes() const
{
  return data->doc.okToAddNotes();
}

double Document::getPDFVersion() const
{
  return data->doc.getPDFMajorVersion () + data->doc.getPDFMinorVersion() / 10.0;
}

void Document::getPdfVersion(int *major, int *minor) const
{
  if (major)
    *major = data->doc.getPDFMajorVersion();
  if (minor)
    *minor = data->doc.getPDFMinorVersion();
}

QDomDocument *Document::toc() const
{
  Outline * outline = data->doc.getOutline();
  if ( !outline )
    return NULL;

  GooList * items = outline->getItems();
  if ( !items || items->getLength() < 1 )
    return NULL;

  QDomDocument *toc = new QDomDocument();
  if ( items->getLength() > 0 )
    data->addTocChildren( toc, toc, items );

  return toc;
}

LinkDestination *Document::linkDestination( const QString &name )
{
  GooString * namedDest = QStringToGooString( name );
  LinkDestinationData ldd(NULL, namedDest, data);
  LinkDestination *ld = new LinkDestination(ldd);
  delete namedDest;
  return ld;
}

bool Document::print(const QString &fileName, QValueList<int> pageList, double hDPI, double vDPI, int rotate)
{
  return print(fileName, pageList, hDPI, vDPI, rotate, -1, -1);
}

bool Document::print(const QString &file, QValueList<int> pageList, double hDPI, double vDPI, int rotate, int paperWidth, int paperHeight)
{
  PSOutputDev *psOut = new PSOutputDev(file.latin1(), &(data->doc), data->doc.getXRef(), data->doc.getCatalog(), NULL, 1, data->doc.getNumPages(), psModePS, paperWidth, paperHeight);
  
  if (psOut->isOk()) {
    QValueList<int>::iterator it;
    for (it = pageList.begin(); it != pageList.end(); ++it )
      data->doc.displayPage(psOut, *it, hDPI, vDPI, rotate, gFalse, gTrue, gTrue);
    
    delete psOut;
    return true;
  } else {
    delete psOut;
    return false;
  }
}

}
