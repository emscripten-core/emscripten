/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2005-2010, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Stefan Kebekus <stefan.kebekus@math.uni-koeln.de>
 * Copyright (C) 2006-2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2009 Shawn Rutledge <shawn.t.rutledge@gmail.com>
 * Copyright (C) 2010, Guillermo Amaral <gamaral@kdab.com>
 * Copyright (C) 2010 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
 * Copyright (C) 2010 Matthias Fauconneau <matthias.fauconneau@gmail.com>
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

#include <poppler-qt4.h>

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QVarLengthArray>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include <config.h>
#include <PDFDoc.h>
#include <Catalog.h>
#include <Form.h>
#include <ErrorCodes.h>
#include <TextOutputDev.h>
#include <Annot.h>
#include <Link.h>
#include <ArthurOutputDev.h>
#if defined(HAVE_SPLASH)
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#endif

#include "poppler-private.h"
#include "poppler-page-transition-private.h"
#include "poppler-page-private.h"
#include "poppler-link-extractor-private.h"
#include "poppler-annotation-helper.h"
#include "poppler-annotation-private.h"
#include "poppler-form.h"

namespace Poppler {

class DummyAnnotation : public Annotation
{
    public:
        DummyAnnotation()
            : Annotation( *new AnnotationPrivate() )
        {
        }

        virtual SubType subType() const { return A_BASE; }
};

Link* PageData::convertLinkActionToLink(::LinkAction * a, const QRectF &linkArea)
{
    return convertLinkActionToLink(a, parentDoc, linkArea);
}

Link* PageData::convertLinkActionToLink(::LinkAction * a, DocumentData *parentDoc, const QRectF &linkArea)
{
  if ( !a )
    return NULL;

  Link * popplerLink = NULL;
  switch ( a->getKind() )
  {
    case actionGoTo:
    {
      LinkGoTo * g = (LinkGoTo *) a;
      const LinkDestinationData ldd( g->getDest(), g->getNamedDest(), parentDoc, false );
      // create link: no ext file, namedDest, object pointer
      popplerLink = new LinkGoto( linkArea, QString::null, LinkDestination( ldd ) );
    }
    break;

    case actionGoToR:
    {
      LinkGoToR * g = (LinkGoToR *) a;
      // copy link file
      const QString fileName = UnicodeParsedString( g->getFileName() );
      const LinkDestinationData ldd( g->getDest(), g->getNamedDest(), parentDoc, !fileName.isEmpty() );
      // ceate link: fileName, namedDest, object pointer
      popplerLink = new LinkGoto( linkArea, fileName, LinkDestination( ldd ) );
    }
    break;

    case actionLaunch:
    {
      LinkLaunch * e = (LinkLaunch *)a;
      GooString * p = e->getParams();
      popplerLink = new LinkExecute( linkArea, e->getFileName()->getCString(), p ? p->getCString() : 0 );
    }
    break;

    case actionNamed:
    {
      const char * name = ((LinkNamed *)a)->getName()->getCString();
      if ( !strcmp( name, "NextPage" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::PageNext );
      else if ( !strcmp( name, "PrevPage" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::PagePrev );
      else if ( !strcmp( name, "FirstPage" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::PageFirst );
      else if ( !strcmp( name, "LastPage" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::PageLast );
      else if ( !strcmp( name, "GoBack" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::HistoryBack );
      else if ( !strcmp( name, "GoForward" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::HistoryForward );
      else if ( !strcmp( name, "Quit" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::Quit );
      else if ( !strcmp( name, "GoToPage" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::GoToPage );
      else if ( !strcmp( name, "Find" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::Find );
      else if ( !strcmp( name, "FullScreen" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::Presentation );
      else if ( !strcmp( name, "Print" ) )
        popplerLink = new LinkAction( linkArea, LinkAction::Print );
      else if ( !strcmp( name, "Close" ) )
      {
        // acroread closes the document always, doesnt care whether 
        // its presentation mode or not
        // popplerLink = new LinkAction( linkArea, LinkAction::EndPresentation );
        popplerLink = new LinkAction( linkArea, LinkAction::Close );
      }
      else
      {
        // TODO
      }
    }
    break;

    case actionURI:
    {
      popplerLink = new LinkBrowse( linkArea, ((LinkURI *)a)->getURI()->getCString() );
    }
    break;

    case actionSound:
    {
      ::LinkSound *ls = (::LinkSound *)a;
      popplerLink = new LinkSound( linkArea, ls->getVolume(), ls->getSynchronous(), ls->getRepeat(), ls->getMix(), new SoundObject( ls->getSound() ) );
    }
    break;

    case actionJavaScript:
    {
      ::LinkJavaScript *ljs = (::LinkJavaScript *)a;
      popplerLink = new LinkJavaScript( linkArea, UnicodeParsedString(ljs->getScript()) );
    }
    break;

    case actionMovie:
/*      TODO this (Movie link)
          m_type = Movie;
          LinkMovie * m = (LinkMovie *) a;
          // copy Movie parameters (2 IDs and a const char *)
          Ref * r = m->getAnnotRef();
          m_refNum = r->num;
          m_refGen = r->gen;
          copyString( m_uri, m->getTitle()->getCString() );
*/  break;

    case actionUnknown:
    break;
  }

  return popplerLink;
}


Page::Page(DocumentData *doc, int index) {
  m_page = new PageData();
  m_page->index = index;
  m_page->parentDoc = doc;
  m_page->page = doc->doc->getPage(m_page->index + 1);
  m_page->transition = 0;
}

Page::~Page()
{
  delete m_page->transition;
  delete m_page;
}

QImage Page::renderToImage(double xres, double yres, int x, int y, int w, int h, Rotation rotate) const
{
  int rotation = (int)rotate * 90;
  QImage img;
  switch(m_page->parentDoc->m_backend)
  {
    case Poppler::Document::SplashBackend:
    {
#if defined(HAVE_SPLASH)
      SplashOutputDev *splash_output = static_cast<SplashOutputDev *>(m_page->parentDoc->getOutputDev());

      m_page->parentDoc->doc->displayPageSlice(splash_output, m_page->index + 1, xres, yres,
						 rotation, false, true, false, x, y, w, h);

      SplashBitmap *bitmap = splash_output->getBitmap();
      int bw = bitmap->getWidth();
      int bh = bitmap->getHeight();

      SplashColorPtr dataPtr = splash_output->getBitmap()->getDataPtr();

      if (QSysInfo::BigEndian == QSysInfo::ByteOrder)
      {
        uchar c;
        int count = bw * bh * 4;
        for (int k = 0; k < count; k += 4)
        {
          c = dataPtr[k];
          dataPtr[k] = dataPtr[k+3];
          dataPtr[k+3] = c;

          c = dataPtr[k+1];
          dataPtr[k+1] = dataPtr[k+2];
          dataPtr[k+2] = c;
        }
      }

      // construct a qimage SHARING the raw bitmap data in memory
      QImage tmpimg( dataPtr, bw, bh, QImage::Format_ARGB32 );
      img = tmpimg.copy();
      // unload underlying xpdf bitmap
      splash_output->startPage( 0, NULL );
#endif
      break;
    }
    case Poppler::Document::ArthurBackend:
    {
      QSize size = pageSize();
      QImage tmpimg(w == -1 ? qRound( size.width() * xres / 72.0 ) : w, h == -1 ? qRound( size.height() * yres / 72.0 ) : h, QImage::Format_ARGB32);

      QPainter painter(&tmpimg);
      renderToPainter(&painter, xres, yres, x, y, w, h, rotate, DontSaveAndRestore);
      painter.end();
      img = tmpimg;
      break;
    }
  }

  return img;
}

bool Page::renderToPainter(QPainter* painter, double xres, double yres, int x, int y, int w, int h, Rotation rotate, PainterFlags flags) const
{
  if (!painter)
    return false;

  switch(m_page->parentDoc->m_backend)
  {
    case Poppler::Document::SplashBackend:
      return false;
    case Poppler::Document::ArthurBackend:
    {
      const bool savePainter = !(flags & DontSaveAndRestore);
      if (savePainter)
         painter->save();
      if (m_page->parentDoc->m_hints & Document::Antialiasing)
          painter->setRenderHint(QPainter::Antialiasing);
      if (m_page->parentDoc->m_hints & Document::TextAntialiasing)
          painter->setRenderHint(QPainter::TextAntialiasing);
      painter->translate(x == -1 ? 0 : -x, y == -1 ? 0 : -y);
      ArthurOutputDev arthur_output(painter);
      arthur_output.startDoc(m_page->parentDoc->doc->getXRef());
      m_page->parentDoc->doc->displayPageSlice(&arthur_output,
                                               m_page->index + 1,
                                               xres,
                                               yres,
                                               (int)rotate * 90,
                                               false,
                                               true,
                                               false,
                                               x,
                                               y,
                                               w,
                                               h);
      if (savePainter)
         painter->restore();
      return true;
    }
  }
  return false;
}

QImage Page::thumbnail() const
{
  unsigned char* data = 0;
  int w = 0;
  int h = 0;
  int rowstride = 0;
  GBool r = m_page->page->loadThumb(&data, &w, &h, &rowstride);
  QImage ret;
  if (r)
  {
    // first construct a temporary image with the data got,
    // then force a copy of it so we can free the raw thumbnail data
    ret = QImage(data, w, h, rowstride, QImage::Format_RGB888).copy();
    gfree(data);
  }
  return ret;
}

QString Page::text(const QRectF &r, TextLayout textLayout) const
{
  TextOutputDev *output_dev;
  GooString *s;
  PDFRectangle *rect;
  QString result;
  
  const GBool rawOrder = textLayout == RawOrderLayout;
  output_dev = new TextOutputDev(0, gFalse, rawOrder, gFalse);
  m_page->parentDoc->doc->displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      0, false, true, false, -1, -1, -1, -1);
  if (r.isNull())
  {
    rect = m_page->page->getCropBox();
    s = output_dev->getText(rect->x1, rect->y1, rect->x2, rect->y2);
  }
  else
  {
    s = output_dev->getText(r.left(), r.top(), r.right(), r.bottom());
  }

  result = QString::fromUtf8(s->getCString());

  delete output_dev;
  delete s;
  return result;
}

QString Page::text(const QRectF &r) const
{
  return text(r, PhysicalLayout);
}

bool Page::search(const QString &text, double &sLeft, double &sTop, double &sRight, double &sBottom, SearchDirection direction, SearchMode caseSensitive, Rotation rotate) const
{
  const QChar * str = text.unicode();
  int len = text.length();
  QVector<Unicode> u(len);
  for (int i = 0; i < len; ++i) u[i] = str[i].unicode();

  GBool sCase;
  if (caseSensitive == CaseSensitive) sCase = gTrue;
  else sCase = gFalse;

  bool found = false;

  int rotation = (int)rotate * 90;

  // fetch ourselves a textpage
  TextOutputDev td(NULL, gTrue, gFalse, gFalse);
  m_page->parentDoc->doc->displayPage( &td, m_page->index + 1, 72, 72, rotation, false, true, false );
  TextPage *textPage=td.takeText();

  if (direction == FromTop)
    found = textPage->findText( u.data(), len, 
            gTrue, gTrue, gFalse, gFalse, sCase, gFalse, &sLeft, &sTop, &sRight, &sBottom );
  else if ( direction == NextResult )
    found = textPage->findText( u.data(), len, 
            gFalse, gTrue, gTrue, gFalse, sCase, gFalse, &sLeft, &sTop, &sRight, &sBottom );
  else if ( direction == PreviousResult )
    found = textPage->findText( u.data(), len, 
            gFalse, gTrue, gTrue, gFalse, sCase, gTrue, &sLeft, &sTop, &sRight, &sBottom );

  textPage->decRefCnt();

  return found;
}

bool Page::search(const QString &text, QRectF &rect, SearchDirection direction, SearchMode caseSensitive, Rotation rotate) const
{
  double sLeft, sTop, sRight, sBottom;
  sLeft = rect.left();
  sTop = rect.top();
  sRight = rect.right();
  sBottom = rect.bottom();

  bool found = search(text, sLeft, sTop, sRight, sBottom, direction, caseSensitive, rotate);

  rect.setLeft( sLeft );
  rect.setTop( sTop );
  rect.setRight( sRight );
  rect.setBottom( sBottom );

  return found;
}

QList<TextBox*> Page::textList(Rotation rotate) const
{
  TextOutputDev *output_dev;
  
  QList<TextBox*> output_list;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);
  
  int rotation = (int)rotate * 90;

  m_page->parentDoc->doc->displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      rotation, false, false, false, -1, -1, -1, -1);

  TextWordList *word_list = output_dev->makeWordList();
  
  if (!word_list) {
    delete output_dev;
    return output_list;
  }
  
  QHash<TextWord *, TextBox*> wordBoxMap;
  
  for (int i = 0; i < word_list->getLength(); i++) {
    TextWord *word = word_list->get(i);
    GooString *gooWord = word->getText();
    QString string = QString::fromUtf8(gooWord->getCString());
    delete gooWord;
    double xMin, yMin, xMax, yMax;
    word->getBBox(&xMin, &yMin, &xMax, &yMax);
    
    TextBox* text_box = new TextBox(string, QRectF(xMin, yMin, xMax-xMin, yMax-yMin));
    text_box->m_data->hasSpaceAfter = word->hasSpaceAfter() == gTrue;
    text_box->m_data->charBBoxes.reserve(word->getLength());
    for (int j = 0; j < word->getLength(); ++j)
    {
        word->getCharBBox(j, &xMin, &yMin, &xMax, &yMax);
        text_box->m_data->charBBoxes.append(QRectF(xMin, yMin, xMax-xMin, yMax-yMin));
    }
    
    wordBoxMap.insert(word, text_box);
    
    output_list.append(text_box);
  }
  
  for (int i = 0; i < word_list->getLength(); i++) {
    TextWord *word = word_list->get(i);
    TextBox* text_box = wordBoxMap.value(word);
    text_box->m_data->nextWord = wordBoxMap.value(word->nextWord());
  }
  
  delete word_list;
  delete output_dev;
  
  return output_list;
}

PageTransition *Page::transition() const
{
  if (!m_page->transition) {
    Object o;
    PageTransitionParams params;
    params.dictObj = m_page->page->getTrans(&o);
    if (params.dictObj->isDict()) m_page->transition = new PageTransition(params);
    o.free();
  }
  return m_page->transition;
}

Link *Page::action( PageAction act ) const
{
  if ( act == Page::Opening || act == Page::Closing )
  {
    Object o;
    m_page->page->getActions(&o);
    if (!o.isDict())
    {
      o.free();
      return 0;
    }
    Dict *dict = o.getDict();
    Object o2;
    const char *key = act == Page::Opening ? "O" : "C";
    dict->lookup((char*)key, &o2);
    ::LinkAction *lact = ::LinkAction::parseAction(&o2, m_page->parentDoc->doc->getCatalog()->getBaseURI() );
    o2.free();
    o.free();
    Link *popplerLink = NULL;
    if (lact != NULL)
    {
      popplerLink = m_page->convertLinkActionToLink(lact, QRectF());
      delete lact;
    }
    return popplerLink;
  }
  return 0;
}

QSizeF Page::pageSizeF() const
{
  Page::Orientation orient = orientation();
  if ( ( Page::Landscape == orient ) || (Page::Seascape == orient ) ) {
      return QSizeF( m_page->page->getCropHeight(), m_page->page->getCropWidth() );
  } else {
    return QSizeF( m_page->page->getCropWidth(), m_page->page->getCropHeight() );
  }
}

QSize Page::pageSize() const
{
  return pageSizeF().toSize();
}

Page::Orientation Page::orientation() const
{
  const int rotation = m_page->page->getRotate();
  switch (rotation) {
  case 90:
    return Page::Landscape;
    break;
  case 180:
    return Page::UpsideDown;
    break;
  case 270:
    return Page::Seascape;
    break;
  default:
    return Page::Portrait;
  }
}

void Page::defaultCTM(double *CTM, double dpiX, double dpiY, int rotate, bool upsideDown)
{
  m_page->page->getDefaultCTM(CTM, dpiX, dpiY, rotate, gFalse, upsideDown);
}

QList<Link*> Page::links() const
{
  LinkExtractorOutputDev link_dev(m_page);
  m_page->parentDoc->doc->processLinks(&link_dev, m_page->index + 1);
  QList<Link*> popplerLinks = link_dev.links();

  return popplerLinks;
}

QList<Annotation*> Page::annotations() const
{
    ::Page *pdfPage = m_page->page;
    Annots* annots = pdfPage->getAnnots(m_page->parentDoc->doc->getCatalog());
    const uint numAnnotations = annots->getNumAnnots();
    if ( numAnnotations == 0 )
    {
        delete annots;
        return QList<Annotation*>();
    }

    // ID to Annotation/PopupWindow maps
    QMap< int, Annotation * > annotationsMap;
    QHash< AnnotPopup *, PopupWindow * > popupsMap;
    // lists of Windows and Revisions that needs resolution
    QLinkedList< ResolveRevision > resolveRevList;
    QLinkedList< ResolveWindow > resolvePopList;
    QLinkedList< PostProcessText > ppTextList;

    // build a normalized transform matrix for this page at 100% scale
    GfxState * gfxState = new GfxState( 72.0, 72.0, pdfPage->getCropBox(), pdfPage->getRotate(), gTrue );
    double * gfxCTM = gfxState->getCTM();
    double MTX[6];
    for ( int i = 0; i < 6; i+=2 )
    {
        MTX[i] = gfxCTM[i] / pdfPage->getCropWidth();
        MTX[i+1] = gfxCTM[i+1] / pdfPage->getCropHeight();
    }
    delete gfxState;

    /** 1 - PARSE ALL ANNOTATIONS AND POPUPS FROM THE PAGE */
    for ( uint j = 0; j < numAnnotations; j++ )
    {
        // get the j-th annotation
        Annot * ann = annots->getAnnot( j );
        if ( !ann )
        {
            qDebug() << "Annot" << j << "is null.";
            continue;
        }

        Annotation * annotation = 0;
        int annotID = ann->getId();
        AnnotMarkup * markupann = dynamic_cast< AnnotMarkup * >( ann );
        bool addToPage = true;      // Popup annots are added to custom queue

        /** 1.1. GET Subtype */
        Annot::AnnotSubtype subType = ann->getType();

        /** 1.2. CREATE Annotation / PopupWindow and PARSE specific params */
        switch ( subType )
        {
            case Annot::typeText:
            {
                // parse TextAnnotation params
                AnnotText * textann = static_cast< AnnotText * >( ann );
                TextAnnotation * t = new TextAnnotation();
                annotation = t;
                // -> textType
                t->setTextType( TextAnnotation::Linked );
                // -> textIcon
                t->setTextIcon( QString::fromLatin1( textann->getIcon()->getCString() ) );
                // request for postprocessing window geometry
                PostProcessText request;
                request.textAnnotation = t;
                request.opened = textann->getOpen();
                ppTextList.append( request );
                break;
            }
            case Annot::typeFreeText:
            {
                // parse TextAnnotation params
                AnnotFreeText * textann = static_cast< AnnotFreeText * >( ann );
                TextAnnotation * t = new TextAnnotation();
                annotation = t;
                // -> textType
                t->setTextType( TextAnnotation::InPlace );
#if 0
                // -> textFont
                QString textFormat;
                XPDFReader::lookupString( annotDict, "DA", textFormat );
                // TODO, fill t->textFont using textFormat if not empty
#endif
                // -> inplaceAlign
                t->setInplaceAlign( textann->getQuadding() );
                // -> inplaceText (simple)
                QString tmpstring;
                GooString *styleString = textann->getStyleString();
                if ( styleString )
                    tmpstring = UnicodeParsedString( styleString );
#if 0
                // -> inplaceText (complex override)
                XPDFReader::lookupString( annotDict, "RC", tmpstring );
#endif
                t->setInplaceText( tmpstring );
                // -> inplaceCallout
                AnnotCalloutLine *callout = textann->getCalloutLine();
                if ( callout )
                {
                    QPointF tmppoint;
                    XPDFReader::transform( MTX, callout->getX1(), callout->getY1(), tmppoint );
                    t->setCalloutPoint( 0, tmppoint );
                    XPDFReader::transform( MTX, callout->getX2(), callout->getY2(), tmppoint );
                    t->setCalloutPoint( 1, tmppoint );
                    if ( AnnotCalloutMultiLine * callout_v6 = dynamic_cast< AnnotCalloutMultiLine * >( callout ) )
                    {
                        XPDFReader::transform( MTX, callout_v6->getX3(), callout_v6->getY3(), tmppoint );
                        t->setCalloutPoint( 2, tmppoint );
                    }
                }
                // -> inplaceIntent
                t->setInplaceIntent( (TextAnnotation::InplaceIntent)textann->getIntent() );
                AnnotBorderEffect *border_effect = textann->getBorderEffect();
                if ( border_effect )
                {
                    // -> style.effect
                    t->style.effect = (Annotation::LineEffect)border_effect->getEffectType();
                    // -> style.effectIntensity
                    t->style.effectIntensity = (int)border_effect->getIntensity();
                }
                break;
            }
            case Annot::typeLine:
            {
                // parse LineAnnotation params
                AnnotLine * lineann = static_cast< AnnotLine * >( ann );
                LineAnnotation * l = new LineAnnotation();
                annotation = l;

                // -> linePoints
                QLinkedList<QPointF> linePoints;
                QPointF p;
                XPDFReader::transform( MTX, lineann->getX1(), lineann->getY1(), p );
                linePoints.push_back( p );
                XPDFReader::transform( MTX, lineann->getX2(), lineann->getY2(), p );
                linePoints.push_back( p );
                l->setLinePoints( linePoints );
                // -> lineStartStyle
                l->setLineStartStyle( (LineAnnotation::TermStyle)lineann->getStartStyle() );
                // -> lineEndStyle
                l->setLineEndStyle( (LineAnnotation::TermStyle)lineann->getEndStyle() );
                // -> lineClosed
                l->setLineClosed( false );
                // -> lineInnerColor
                l->setLineInnerColor( convertAnnotColor( lineann->getInteriorColor() ) );
                // -> lineLeadingFwdPt
                l->setLineLeadingForwardPoint( lineann->getLeaderLineLength() );
                // -> lineLeadingBackPt
                l->setLineLeadingBackPoint( lineann->getLeaderLineExtension() );
                // -> lineShowCaption
                l->setLineShowCaption( lineann->getCaption() );
                // -> lineIntent
                l->setLineIntent( (LineAnnotation::LineIntent)( lineann->getIntent() + 1 ) );
                break;
            }
            case Annot::typePolygon:
            case Annot::typePolyLine:
            {
                // parse LineAnnotation params
                AnnotPolygon * polyann = static_cast< AnnotPolygon * >( ann );
                LineAnnotation * l = new LineAnnotation();
                annotation = l;

                // -> polyPoints
                QLinkedList<QPointF> polyPoints;
                AnnotPath * vertices = polyann->getVertices();
                for ( int i = 0; i < vertices->getCoordsLength(); ++i )
                {
                    QPointF p;
                    XPDFReader::transform( MTX, vertices->getX( i ), vertices->getY( i ), p );
                    polyPoints.push_back( p );
                }
                l->setLinePoints( polyPoints );
                // -> polyStartStyle
                l->setLineStartStyle( (LineAnnotation::TermStyle)polyann->getStartStyle() );
                // -> polyEndStyle
                l->setLineEndStyle( (LineAnnotation::TermStyle)polyann->getEndStyle() );
                // -> polyClosed
                l->setLineClosed( subType == Annot::typePolygon );
                // -> polyInnerColor
                l->setLineInnerColor( convertAnnotColor( polyann->getInteriorColor() ) );
                // -> polyIntent
                switch ( polyann->getIntent() )
                {
                    case AnnotPolygon::polygonCloud:
                        l->setLineIntent( LineAnnotation::PolygonCloud );
                        break;
                    case AnnotPolygon::polylineDimension:
                    case AnnotPolygon::polygonDimension:
                        l->setLineIntent( LineAnnotation::Dimension );
                        break;
                }
                break;
            }
            case Annot::typeSquare:
            case Annot::typeCircle:
            {
                // parse GeomAnnotation params
                AnnotGeometry * geomann = static_cast< AnnotGeometry * >( ann );
                GeomAnnotation * g = new GeomAnnotation();
                annotation = g;

                // -> highlightType
                if ( subType == Annot::typeSquare )
                    g->setGeomType( GeomAnnotation::InscribedSquare );
                else if ( subType == Annot::typeCircle )
                    g->setGeomType( GeomAnnotation::InscribedCircle );

                // -> geomInnerColor
                g->setGeomInnerColor( convertAnnotColor( geomann->getInteriorColor() ) );

                AnnotBorderEffect *border_effect = geomann->getBorderEffect();
                if ( border_effect )
                {
                    // -> style.effect
                    g->style.effect = (Annotation::LineEffect)border_effect->getEffectType();
                    // -> style.effectIntensity
                    g->style.effectIntensity = (int)border_effect->getIntensity();
                }

                // TODO RD

                break;
            }
            case Annot::typeHighlight:
            case Annot::typeUnderline:
            case Annot::typeSquiggly:
            case Annot::typeStrikeOut:
            {
                AnnotTextMarkup * hlann = static_cast< AnnotTextMarkup * >( ann );
                HighlightAnnotation * h = new HighlightAnnotation();
                annotation = h;

                // -> highlightType
                if ( subType == Annot::typeHighlight )
                    h->setHighlightType( HighlightAnnotation::Highlight );
                else if ( subType == Annot::typeUnderline )
                    h->setHighlightType( HighlightAnnotation::Underline );
                else if ( subType == Annot::typeSquiggly )
                    h->setHighlightType( HighlightAnnotation::Squiggly );
                else if ( subType == Annot::typeStrikeOut )
                    h->setHighlightType( HighlightAnnotation::StrikeOut );

                // -> highlightQuads
                AnnotQuadrilaterals *hlquads = hlann->getQuadrilaterals();
                if ( !hlquads || !hlquads->getQuadrilateralsLength() )
                {
                    qDebug() << "Not enough quads for a Highlight annotation.";
                    delete annotation;
                    continue;
                }
                QList< HighlightAnnotation::Quad > quads;
                const int quadsCount = hlquads->getQuadrilateralsLength();
                for ( int q = 0; q < quadsCount; ++q )
                {
                    HighlightAnnotation::Quad quad;
                    XPDFReader::transform( MTX, hlquads->getX1( q ), hlquads->getY1( q ), quad.points[ 0 ] );
                    XPDFReader::transform( MTX, hlquads->getX2( q ), hlquads->getY2( q ), quad.points[ 1 ] );
                    XPDFReader::transform( MTX, hlquads->getX3( q ), hlquads->getY3( q ), quad.points[ 2 ] );
                    XPDFReader::transform( MTX, hlquads->getX4( q ), hlquads->getY4( q ), quad.points[ 3 ] );
                    // ### PDF1.6 specs says that point are in ccw order, but in fact
                    // points 3 and 4 are swapped in every PDF around!
                    QPointF tmpPoint = quad.points[ 2 ];
                    quad.points[ 2 ] = quad.points[ 3 ];
                    quad.points[ 3 ] = tmpPoint;
                    // initialize other properties and append quad
                    quad.capStart = true;       // unlinked quads are always capped
                    quad.capEnd = true;         // unlinked quads are always capped
                    quad.feather = 0.1;         // default feather
                    quads.append( quad );
                }
                h->setHighlightQuads( quads );
                break;
            }
            case Annot::typeStamp:
            {
                AnnotStamp * stampann = static_cast< AnnotStamp * >( ann );
                StampAnnotation * s = new StampAnnotation();
                annotation = s;
                // -> stampIcon
                s->setStampIconName( QString::fromLatin1( stampann->getIcon()->getCString() ) );
                break;
            }
            case Annot::typeInk:
            {
                // parse InkAnnotation params
                AnnotInk * inkann = static_cast< AnnotInk * >( ann );
                InkAnnotation * k = new InkAnnotation();
                annotation = k;

                // -> inkPaths
                AnnotPath ** paths = inkann->getInkList();
                if ( !paths || !inkann->getInkListLength() )
                {
                    qDebug() << "InkList not present for ink annot";
                    delete annotation;
                    continue;
                }
                QList< QLinkedList<QPointF> > inkPaths;
                const int pathsNumber = inkann->getInkListLength();
                for ( int m = 0; m < pathsNumber; m++ )
                {
                    // transform each path in a list of normalized points ..
                    QLinkedList<QPointF> localList;
                    AnnotPath * path = paths[ m ];
                    const int pointsNumber = path ? path->getCoordsLength() : 0;
                    for ( int n = 0; n < pointsNumber; ++n )
                    {
                        // get the x,y numbers for current point
                        double x = path->getX( n );
                        double y = path->getY( n );
                        // add normalized point to localList
                        QPointF np;
                        XPDFReader::transform( MTX, x, y, np );
                        localList.push_back( np );
                    }
                    // ..and add it to the annotation
                    inkPaths.push_back( localList );
                }
                k->setInkPaths( inkPaths );
                break;
            }
            case Annot::typePopup:
            {
                AnnotPopup * popupann = static_cast< AnnotPopup * >( ann );

                // create PopupWindow and add it to the popupsMap
                PopupWindow * popup = new PopupWindow();
                popupsMap[ popupann ] = popup;
                addToPage = false;

                // get window specific properties if any
                popup->shown = popupann->getOpen();
                // no need to parse parent annotation id
                //XPDFReader::lookupIntRef( annotDict, "Parent", popup->... );

                // use the 'dummy annotation' for getting other parameters
                popup->dummyAnnotation = new DummyAnnotation();
                annotation = popup->dummyAnnotation;

                break;
            }
            case Annot::typeLink:
            {
                // parse Link params
                AnnotLink * linkann = static_cast< AnnotLink * >( ann );
                LinkAnnotation * l = new LinkAnnotation();
                annotation = l;

                // -> hlMode
                l->setLinkHighlightMode( (LinkAnnotation::HighlightMode)linkann->getLinkEffect() );

                // -> link region
                // TODO

                // reading link action
                if ( !linkann->getDest()->isNull() )
                {
                    ::LinkAction *act = ::LinkAction::parseDest( linkann->getDest() );
                    Link * popplerLink = m_page->convertLinkActionToLink( act, QRectF() );
                    delete act;
                    if ( popplerLink )
                    {
                        l->setLinkDestination( popplerLink );
                    }
                }

                break;
            }
            case Annot::typeCaret:
            {
                // parse CaretAnnotation params
                AnnotCaret * caretann = static_cast< AnnotCaret * >( ann );
                CaretAnnotation * c = new CaretAnnotation();
                annotation = c;

                // -> caretSymbol
                c->setCaretSymbol( (CaretAnnotation::CaretSymbol)caretann->getSymbol() );

                // TODO RD

                break;
            }
            case Annot::typeFileAttachment:
            {
                AnnotFileAttachment * attachann = static_cast< AnnotFileAttachment * >( ann );
                FileAttachmentAnnotation * f = new FileAttachmentAnnotation();
                annotation = f;
                // -> fileIcon
                f->setFileIconName( QString::fromLatin1( attachann->getName()->getCString() ) );
                // -> embeddedFile
                EmbFile *embfile = new EmbFile( attachann->getFile(), attachann->getContents() );
                f->setEmbeddedFile( new EmbeddedFile( embfile ) );
                break;
            }
            case Annot::typeSound:
            {
                AnnotSound * soundann = static_cast< AnnotSound * >( ann );
                SoundAnnotation * s = new SoundAnnotation();
                annotation = s;

                // -> soundIcon
                s->setSoundIconName( QString::fromLatin1( soundann->getName()->getCString() ) );
                // -> sound
                s->setSound( new SoundObject( soundann->getSound() ) );

                break;
            }
            case Annot::typeMovie:
            {
                AnnotMovie * movieann = static_cast< AnnotMovie * >( ann );
                MovieAnnotation * m = new MovieAnnotation();
                annotation = m;

                // -> movie
                MovieObject *movie = new MovieObject( movieann );
                m->setMovie( movie );
                // -> movieTitle
                GooString * movietitle = movieann->getTitle();
                if ( movietitle )
                {
                    m->setMovieTitle( QString::fromLatin1( movietitle->getCString() ) );
                }

                break;
            }
            // special case for ignoring unknwon annotations
            case Annot::typeUnknown:
                continue;
            default:
            {
#define CASE_FOR_TYPE( thetype ) \
                    case Annot::type ## thetype: \
                        type = #thetype ; \
                        break;
                QByteArray type;
                switch ( subType )
                {
                    CASE_FOR_TYPE( Widget )
                    CASE_FOR_TYPE( Screen )
                    CASE_FOR_TYPE( PrinterMark )
                    CASE_FOR_TYPE( TrapNet )
                    CASE_FOR_TYPE( Watermark )
                    CASE_FOR_TYPE( 3D )
                    default: type = QByteArray::number( subType );
                }
                // MISSING: Widget, Screen, PrinterMark, TrapNet, Watermark, 3D
                qDebug() << "Annotation" << type.constData() << "not supported.";
                continue;
#undef CASE_FOR_TYPE
            }
        }

        /** 1.3. PARSE common parameters */
        // -> boundary
        PDFRectangle *boundary = ann->getRect();
        // transform annotation rect to uniform coords
        QPointF topLeft, bottomRight;
        XPDFReader::transform( MTX, boundary->x1, boundary->y1, topLeft );
        XPDFReader::transform( MTX, boundary->x2, boundary->y2, bottomRight );
        QRectF boundaryRect;
        boundaryRect.setTopLeft(topLeft);
        boundaryRect.setBottomRight(bottomRight);
        if ( boundaryRect.left() > boundaryRect.right() )
        {
            double aux = boundaryRect.left();
            boundaryRect.setLeft( boundaryRect.right() );
            boundaryRect.setRight(aux);
        }
        if ( boundaryRect.top() > boundaryRect.bottom() )
        {
            double aux = boundaryRect.top();
            boundaryRect.setTop( boundaryRect.bottom() );
            boundaryRect.setBottom(aux);
           //annotation->rUnscaledWidth = (r[2] > r[0]) ? r[2] - r[0] : r[0] - r[2];
           //annotation->rUnscaledHeight = (r[3] > r[1]) ? r[3] - r[1] : r[1] - r[3];
        }
        annotation->setBoundary( boundaryRect );
        // -> contents
        annotation->setContents( UnicodeParsedString( ann->getContents() ) );
        // -> uniqueName
        annotation->setUniqueName( UnicodeParsedString( ann->getName() ) );
        // -> modifyDate (and -> creationDate)
        GooString *modDate = ann->getModified();
        if ( modDate )
        {
            annotation->setModificationDate( convertDate( modDate->getCString() ) );
        }
        if ( annotation->creationDate().isNull() && !annotation->modificationDate().isNull() )
            annotation->setCreationDate( annotation->modificationDate() );
        // -> flags: set the external attribute since it's embedded on file
        int annoflags = 0;
        annoflags |= Annotation::External;
        // -> flags
        int flags = ann->getFlags();
        if ( flags & Annot::flagHidden )
            annoflags |= Annotation::Hidden;
        if ( flags & Annot::flagNoZoom )
            annoflags |= Annotation::FixedSize;
        if ( flags & Annot::flagNoRotate )
            annoflags |= Annotation::FixedRotation;
        if ( !( flags & Annot::flagPrint ) )
            annoflags |= Annotation::DenyPrint;
        if ( flags & Annot::flagReadOnly )
            annoflags |= (Annotation::DenyWrite | Annotation::DenyDelete);
        if ( flags & Annot::flagLocked )
            annoflags |= Annotation::DenyDelete;
        if ( flags & Annot::flagToggleNoView )
            annoflags |= Annotation::ToggleHidingOnMouse;
        annotation->setFlags( annoflags );
        // -> style
        AnnotBorder *border = ann->getBorder();
        if ( border )
        {
            if ( border->getType() == AnnotBorder::typeArray )
            {
                AnnotBorderArray * border_array = static_cast< AnnotBorderArray * >( border );
                // -> style.xCorners
                annotation->style.xCorners = border_array->getHorizontalCorner();
                // -> style.yCorners
                annotation->style.yCorners = border_array->getVerticalCorner();
            }
            // -> style.width
            annotation->style.width = border->getWidth();
            // -> style.style
            annotation->style.style = (Annotation::LineStyle)( 1 << border->getStyle() );
#if 0
            // -> style.marks and style.spaces
            // TODO
            Object dashArray;
            bsObj.getDict()->lookup( "D", &dashArray );
            if ( dashArray.isArray() )
            {
                int dashMarks = 3;
                int dashSpaces = 0;
                Object intObj;
                dashArray.arrayGet( 0, &intObj );
                if ( intObj.isInt() )
                    dashMarks = intObj.getInt();
                intObj.free();
                dashArray.arrayGet( 1, &intObj );
                if ( intObj.isInt() )
                    dashSpaces = intObj.getInt();
                intObj.free();
                annotation->style.marks = dashMarks;
                annotation->style.spaces = dashSpaces;
            }
#endif
        }
        // -> style.color
        annotation->style.color = convertAnnotColor( ann->getColor() );

        /** 1.4. PARSE markup { common, Popup, Revision } parameters */
        if ( markupann )
        {
            // -> creationDate
            GooString *createDate = markupann->getDate();
            if ( createDate )
            {
                annotation->setCreationDate( convertDate( createDate->getCString() ) );
            }
            // -> style.opacity
            annotation->style.opacity = markupann->getOpacity();
            // -> window.title and author
            annotation->window.title = UnicodeParsedString( markupann->getLabel() );
            annotation->setAuthor( annotation->window.title );
            // -> window.summary
            annotation->window.summary = UnicodeParsedString( markupann->getSubject() );
#if 0
            // -> window.text
            // TODO
            XPDFReader::lookupString( annotDict, "RC", annotation->window.text );
#endif

            // if a popup is referenced, schedule for resolving it later
            AnnotPopup *popup = markupann->getPopup();
            if ( popup )
            {
                ResolveWindow request;
                request.popup = popup;
                request.annotation = annotation;
                resolvePopList.append( request );
            }

            // if an older version is referenced, schedule for reparenting
            int parentID = markupann->getInReplyToID();
            if ( parentID )
            {
                ResolveRevision request;
                request.nextAnnotation = annotation;
                request.nextAnnotationID = annotID;
                request.prevAnnotationID = parentID;

                // -> request.nextScope
                request.nextScope = Annotation::Reply;
                switch ( markupann->getReplyTo() )
                {
                    case AnnotMarkup::replyTypeR:
                        request.nextScope = Annotation::Reply;
                        break;
                    case AnnotMarkup::replyTypeGroup:
                        request.nextScope = Annotation::Group;
                        break;
                }

                // -> revision.type (StateModel is deduced from type, not parsed)
                request.nextType = Annotation::None;
                if ( subType == Annot::typeText )
                {
                    AnnotText * textann = static_cast< AnnotText * >( ann );
                    switch ( textann->getState() )
                    {
                        case AnnotText::stateMarked:
                            request.nextType = Annotation::Marked;
                            break;
                        case AnnotText::stateUnmarked:
                            request.nextType = Annotation::Unmarked;
                            break;
                        case AnnotText::stateAccepted:
                            request.nextType = Annotation::Accepted;
                            break;
                        case AnnotText::stateRejected:
                            request.nextType = Annotation::Rejected;
                            break;
                        case AnnotText::stateCancelled:
                            request.nextType = Annotation::Cancelled;
                            break;
                        case AnnotText::stateCompleted:
                            request.nextType = Annotation::Completed;
                            break;
                        case AnnotText::stateNone:
                        case AnnotText::stateUnknown:
                            ;
                    }
                }

                // schedule for later reparenting
                resolveRevList.append( request );
            }
        }

        /** 1.5. ADD ANNOTATION to the annotationsMap  */
        if ( addToPage )
        {
            if ( annotationsMap.contains( annotID ) )
                qDebug() << "Clash for annotations with ID:" << annotID;
            annotationsMap[ annotID ] = annotation;
        }
    } // end Annotation/PopupWindow parsing loop

    /** 2 - RESOLVE POPUPS (popup.* -> annotation.window) */
    if ( !resolvePopList.isEmpty() && !popupsMap.isEmpty() )
    {
        QLinkedList< ResolveWindow >::iterator it = resolvePopList.begin(),
                                              end = resolvePopList.end();
        for ( ; it != end; ++it )
        {
            const ResolveWindow & request = *it;
            if ( !popupsMap.contains( request.popup ) )
                // warn aboud problems in popup resolving logic
                qDebug() << "Cannot resolve popup"
                          << request.popup << ".";
            else
            {
                // set annotation's window properties taking ones from popup
                PopupWindow * pop = popupsMap[ request.popup ];
                Annotation * pa = pop->dummyAnnotation;
                Annotation::Window & w = request.annotation->window;

                // transfer properties to Annotation's window
                w.flags = pa->flags() & (Annotation::Hidden |
                    Annotation::FixedSize | Annotation::FixedRotation);
                if ( !pop->shown )
                    w.flags |= Annotation::Hidden;
                w.topLeft.setX(pa->boundary().left());
                w.topLeft.setY(pa->boundary().top());
                w.width = (int)( pa->boundary().right() - pa->boundary().left() );
                w.height = (int)( pa->boundary().bottom() - pa->boundary().top() );
            }
        }

        // clear data
        QHash< AnnotPopup *, PopupWindow * >::Iterator dIt = popupsMap.begin(), dEnd = popupsMap.end();
        for ( ; dIt != dEnd; ++dIt )
        {
            PopupWindow * p = dIt.value();
            delete p->dummyAnnotation;
            delete p;
        }
    }

    /** 3 - RESOLVE REVISIONS (parent.revisions.append( children )) */
    if ( !resolveRevList.isEmpty() )
    {
        // append children to parents
        QVarLengthArray< int > excludeIDs( resolveRevList.count() );   // can't even reach this size
        int excludeIndex = 0;                       // index in excludeIDs array
        QLinkedList< ResolveRevision >::iterator it = resolveRevList.begin(), end = resolveRevList.end();
        for ( ; it != end; ++it )
        {
            const ResolveRevision & request = *it;
            int parentID = request.prevAnnotationID;
            if ( !annotationsMap.contains( parentID ) )
                // warn about problems in reparenting logic
                qDebug() << "Cannot reparent annotation to"
                          << parentID << ".";
            else
            {
                // compile and add a Revision structure to the parent annotation
                Annotation::Revision childRevision;
                childRevision.annotation = request.nextAnnotation;
                childRevision.scope = request.nextScope;
                childRevision.type = request.nextType;
                annotationsMap[ parentID ]->revisions().append( childRevision );
                // exclude child annotation from being rooted in page
                excludeIDs[ excludeIndex++ ] = request.nextAnnotationID;
            }
        }

        // prevent children from being attached to page as roots
        for ( int i = 0; i < excludeIndex; i++ )
            annotationsMap.remove( excludeIDs[ i ] );
    }

    /** 4 - POSTPROCESS TextAnnotations (when window geom is embedded) */
    if ( !ppTextList.isEmpty() )
    {
        QLinkedList< PostProcessText >::const_iterator it = ppTextList.begin(), end = ppTextList.end();
        for ( ; it != end; ++it )
        {
            const PostProcessText & request = *it;
            Annotation::Window & window = request.textAnnotation->window;
            // if not present, 'create' the window in-place over the annotation
            if ( window.flags == -1 )
            {
                window.flags = 0;
                QRectF geom = request.textAnnotation->boundary();
                // initialize window geometry to annotation's one
                window.width = (int)( geom.right() - geom.left() );
                window.height = (int)( geom.bottom() - geom.top() );
                window.topLeft.setX( geom.left() > 0.0 ? geom.left() : 0.0 );
                window.topLeft.setY( geom.top() > 0.0 ? geom.top() : 0.0 );
            }
            // (pdf) if text is not 'opened', force window hiding. if the window
            // was parsed from popup, the flag should already be set
            if ( !request.opened && window.flags != -1 )
                window.flags |= Annotation::Hidden;
        }
    }

    delete annots;
    /** 5 - finally RETURN ANNOTATIONS */
    return annotationsMap.values();
}

QList<FormField*> Page::formFields() const
{
  QList<FormField*> fields;
  ::Page *p = m_page->page;
  ::FormPageWidgets * form = p->getPageWidgets();
  int formcount = form->getNumWidgets();
  for (int i = 0; i < formcount; ++i)
  {
    ::FormWidget *fm = form->getWidget(i);
    FormField * ff = NULL;
    switch (fm->getType())
    {
      case formButton:
      {
        ff = new FormFieldButton(m_page->parentDoc, p, static_cast<FormWidgetButton*>(fm));
      }
      break;

      case formText:
      {
        ff = new FormFieldText(m_page->parentDoc, p, static_cast<FormWidgetText*>(fm));
      }
      break;

      case formChoice:
      {
        ff = new FormFieldChoice(m_page->parentDoc, p, static_cast<FormWidgetChoice*>(fm));
      }
      break;

      default: ;
    }

    if (ff)
      fields.append(ff);
  }

  return fields;
}

double Page::duration() const
{
  return m_page->page->getDuration();
}

QString Page::label() const
{
  GooString goo;
  if (!m_page->parentDoc->doc->getCatalog()->indexToLabel(m_page->index, &goo))
    return QString();

  return QString(goo.getCString());
}


}
