/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Tobias Koening <tokoe@kde.org>
 * Copyright (C) 2005-2007, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005-2006, Stefan Kebekus <stefan.kebekus@math.uni-koeln.de>
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

#ifndef __POPPLER_QT_H__
#define __POPPLER_QT_H__

#include "poppler-link-qt3.h"
#include "poppler-page-transition.h"

#include <qcstring.h>
#include <qdatetime.h>
#include <qdom.h>
#include <qpixmap.h>

namespace Poppler {

class Document;
class Page;

/* A rectangle on a page, with coordinates in PDF points. */
class Rectangle
{
  public:
    Rectangle(double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0) : 
      m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}
    bool isNull() const { return m_x1 == 0 && m_y1 == 0 && m_x2 == 0 && m_y2 == 0; }
  
    double m_x1;
    double m_y1;
    double m_x2;
    double m_y2;
};

class TextBox
{
 public:
    TextBox(const QString& text, const Rectangle &bBox) :
    m_text(text), m_bBox(bBox) {};

    QString getText() const { return m_text; };
    Rectangle getBoundingBox() const { return m_bBox; };

  private:
    QString m_text;
    Rectangle m_bBox;
};


/**
  Container class for information about a font within a PDF document
*/
class FontInfoData;
class FontInfo {
public:
  enum Type {
    unknown,
    Type1,
    Type1C,
    Type1COT,
    Type3,
    TrueType,
    TrueTypeOT,
    CIDType0,
    CIDType0C,
    CIDType0COT,
    CIDTrueType,
    CIDTrueTypeOT
  };

  /**
    Create a new font information container
  */
  FontInfo( const QString &fontName, const bool isEmbedded,
            const bool isSubset, Type type );

  FontInfo();
  
  FontInfo( const FontInfo &fi );

  ~FontInfo();

  /**
    The name of the font. Can be QString::null if the font has no name
  */
  const QString &name() const;

  /**
    Whether the font is embedded in the file, or not

    \return true if the font is embedded
  */
  bool isEmbedded() const;

  /**
    Whether the font provided is only a subset of the full
    font or not. This only has meaning if the font is embedded.

    \return true if the font is only a subset
  */
  bool isSubset() const;

  /**
    The type of font encoding
  */
  Type type() const;

  const QString &typeName() const;

private:
  FontInfoData *data;
};

class PageData;
class Page {
  friend class Document;
  public:
    ~Page();
    void renderToPixmap(QPixmap **q, int x, int y, int w, int h, double xres, double yres, bool doLinks = false) const;

    /**
      This is a convenience function that is equivalent to
      renderToPixmap() with xres and yres set to 72.0. We keep it
      only for binary compatibility

      \sa renderToImage()
     */
    void renderToPixmap(QPixmap **q, int x, int y, int w, int h, bool doLinks = false) const;

    /**
      \brief Render the page to a QImage using the Splash renderer

     This method can be used to render the page to a QImage. It
     uses the "Splash" rendering engine.

     \param xres horizontal resolution of the graphics device,
     in dots per inch (defaults to 72 dpi)

     \param yres vertical resolution of the graphics device, in
     dots per inch (defaults to 72 dpi)

     \returns a QImage of the page.

     \sa renderToPixmap()
    */
    QImage renderToImage(double xres = 72.0, double yres = 72.0, bool doLinks = false) const;

    /**
     * Returns the size of the page in points
     **/
    QSize pageSize() const;

    /**
    * Returns the text that is inside the Rectangle r
    * If r is a null Rectangle all text of the page is given
    **/
    QString getText(const Rectangle &r) const;

    QValueList<TextBox*> textList() const;

    /**
    * Returns the transition of this page
    **/
    PageTransition *getTransition() const;

    enum Orientation {
      Landscape,
      Portrait,
      Seascape,
      UpsideDown
    };

    /**
    *  The orientation of the page
    **/
    Orientation orientation() const;
    
    /**
      Gets the links of the page once it has been rendered if doLinks was true
    */
    QValueList<Link*> links() const;

  private:
    Page(const Document *doc, int index);
    PageData *data;
};

class DocumentData;

class Document {
  friend class Page;
  
public:
  enum PageMode {
    UseNone,
    UseOutlines,
    UseThumbs,
    FullScreen,
    UseOC
  };
  
  static Document *load(const QString & filePath);
  
  Page *getPage(int index) const;
  
  int getNumPages() const;
  
  PageMode getPageMode() const;
  
  bool unlock(const QCString &password);
  
  bool isLocked() const;
  
  QDateTime getDate( const QString & data ) const;
  QString getInfo( const QString & data ) const;
  bool isEncrypted() const;
  bool isLinearized() const;
  bool okToPrint() const;
  bool okToChange() const;
  bool okToCopy() const;
  bool okToAddNotes() const;
  double getPDFVersion() const;
  /**
    The version of the PDF specification that the document
    conforms to

    \param major an optional pointer to a variable where store the
    "major" number of the version
    \param minor an optional pointer to a variable where store the
    "minor" number of the version

    \since 0.12
  */
  void getPdfVersion(int *major, int *minor) const;

  bool print(const QString &fileName, QValueList<int> pageList, double hDPI, double vDPI, int rotate);

  // If you are using QPrinter you can get paper size doing
  // QPrinter dummy(QPrinter::PrinterResolution);
  // dummy.setFullPage(true);
  // dummy.setPageSize(thePageSizeYouWant);
  // QPaintDeviceMetrics metrics(&dummy);
  // int width = metrics.width();
  // int height = metrics.height();
  bool print(const QString &fileName, QValueList<int> pageList, double hDPI, double vDPI, int rotate, int paperWidth, int paperHeight);

  /**
    The fonts within the PDF document.

    \note this can take a very long time to run with a large
    document. You may wish to use the call below if you have more
    than say 20 pages
  */
  QValueList<FontInfo> fonts() const;

  /**
    \overload

    \param numPages the number of pages to scan
    \param fontList pointer to the list where the font information
    should be placed

    \return false if the end of the document has been reached
  */
  bool scanForFonts( int numPages, QValueList<FontInfo> *fontList ) const;

  /**
    Gets the TOC of the Document, it is application responsabiliy to delete
    it when no longer needed

    * In the tree the tag name is the 'screen' name of the entry. A tag can have
    * attributes. Here follows the list of tag attributes with meaning:
    * - Destination: A string description of the referred destination
    * - DestinationName: A 'named reference' to the viewport that must be converted
    *      using linkDestination( *destination_name* )
    * - ExternalFileName: A link to a external filename

     \returns NULL if the Document does not have TOC
  */
  QDomDocument *toc() const;

  LinkDestination *linkDestination( const QString &name );

  ~Document();
  
private:
  DocumentData *data;
  Document(DocumentData *dataA);
};

}
#endif
