/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2005-2013, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Stefan Kebekus <stefan.kebekus@math.uni-koeln.de>
 * Copyright (C) 2006-2011, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2009 Shawn Rutledge <shawn.t.rutledge@gmail.com>
 * Copyright (C) 2010, 2012, Guillermo Amaral <gamaral@kdab.com>
 * Copyright (C) 2010 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
 * Copyright (C) 2010 Matthias Fauconneau <matthias.fauconneau@gmail.com>
 * Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
 * Copyright (C) 2012 Tobias Koenig <tokoe@kdab.com>
 * Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2012 Adam Reichold <adamreichold@myopera.com>
 * Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
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
#include <Rendition.h>
#if defined(HAVE_SPLASH)
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#endif

#include "poppler-private.h"
#include "poppler-page-transition-private.h"
#include "poppler-page-private.h"
#include "poppler-link-extractor-private.h"
#include "poppler-annotation-private.h"
#include "poppler-form.h"
#include "poppler-media.h"

namespace Poppler {

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
    {
      ::LinkMovie *lm = (::LinkMovie *)a;

      const QString title = ( lm->hasAnnotTitle() ? UnicodeParsedString( lm->getAnnotTitle() ) : QString() );

      Ref reference;
      reference.num = reference.gen = -1;
      if ( lm->hasAnnotRef() )
        reference = *lm->getAnnotRef();

      LinkMovie::Operation operation = LinkMovie::Play;
      switch ( lm->getOperation() )
      {
        case ::LinkMovie::operationTypePlay:
          operation = LinkMovie::Play;
          break;
        case ::LinkMovie::operationTypePause:
          operation = LinkMovie::Pause;
          break;
        case ::LinkMovie::operationTypeResume:
          operation = LinkMovie::Resume;
          break;
        case ::LinkMovie::operationTypeStop:
          operation = LinkMovie::Stop;
          break;
      };

      popplerLink = new LinkMovie( linkArea, operation, title, reference );
    }
    break;

    case actionRendition:
    {
      ::LinkRendition *lrn = (::LinkRendition *)a;

      Ref reference;
      reference.num = reference.gen = -1;
      if ( lrn->hasScreenAnnot() )
        reference = lrn->getScreenAnnot();

      popplerLink = new LinkRendition( linkArea, lrn->getMedia()->copy(), lrn->getOperation(), UnicodeParsedString( lrn->getScript() ), reference );
    }
    break;

    case actionUnknown:
    break;
  }

  return popplerLink;
}

TextPage *PageData::prepareTextSearch(const QString &text, Page::SearchMode caseSensitive, Page::Rotation rotate, GBool *sCase, QVector<Unicode> *u)
{
  const QChar * str = text.unicode();
  const int len = text.length();
  u->resize(len);
  for (int i = 0; i < len; ++i) (*u)[i] = str[i].unicode();

  if (caseSensitive == Page::CaseSensitive) *sCase = gTrue;
  else *sCase = gFalse;

  const int rotation = (int)rotate * 90;

  // fetch ourselves a textpage
  TextOutputDev td(NULL, gTrue, 0, gFalse, gFalse);
  parentDoc->doc->displayPage( &td, index + 1, 72, 72, rotation, false, true, false,
    NULL, NULL, NULL, NULL, gTrue);
  TextPage *textPage=td.takeText();
  
  return textPage;
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
      SplashColor bgColor;
      GBool overprint = gFalse;
#if defined(SPLASH_CMYK)
      overprint = m_page->parentDoc->m_hints & Document::OverprintPreview ? gTrue : gFalse;
      if (overprint)
      {
        Guchar c, m, y, k;

        c = 255 - m_page->parentDoc->paperColor.blue();
        m = 255 - m_page->parentDoc->paperColor.red();
        y = 255 - m_page->parentDoc->paperColor.green();
        k = c;
        if (m < k) {
          k = m;
        }
        if (y < k) {
          k = y;
        }
        bgColor[0] = c - k;
        bgColor[1] = m - k;
        bgColor[2] = y - k;
        bgColor[3] = k;
        for (int i = 4; i < SPOT_NCOMPS + 4; i++) {
          bgColor[i] = 0;
        }
      }
      else
#endif
      {
        bgColor[0] = m_page->parentDoc->paperColor.blue();
        bgColor[1] = m_page->parentDoc->paperColor.green();
        bgColor[2] = m_page->parentDoc->paperColor.red();
      }
 
      GBool AA = m_page->parentDoc->m_hints & Document::TextAntialiasing ? gTrue : gFalse;
      SplashThinLineMode thinLineMode = splashThinLineDefault;
      if (m_page->parentDoc->m_hints & Document::ThinLineShape) thinLineMode = splashThinLineShape;
      if (m_page->parentDoc->m_hints & Document::ThinLineSolid) thinLineMode = splashThinLineSolid;

      SplashOutputDev * splash_output = new SplashOutputDev(
#if defined(SPLASH_CMYK)
                      (overprint) ? splashModeDeviceN8 : splashModeXBGR8,
#else
                      splashModeXBGR8,
#endif 
                      4, gFalse, bgColor, gTrue, AA, thinLineMode, overprint);

      splash_output->setVectorAntialias(m_page->parentDoc->m_hints & Document::Antialiasing ? gTrue : gFalse);
      splash_output->setFreeTypeHinting(m_page->parentDoc->m_hints & Document::TextHinting ? gTrue : gFalse, 
                                        m_page->parentDoc->m_hints & Document::TextSlightHinting ? gTrue : gFalse);

      splash_output->startDoc(m_page->parentDoc->doc);      

      m_page->parentDoc->doc->displayPageSlice(splash_output, m_page->index + 1, xres, yres,
                                               rotation, false, true, false, x, y, w, h,
                                               NULL, NULL, NULL, NULL, gTrue);

      SplashBitmap *bitmap = splash_output->getBitmap();
      int bw = bitmap->getWidth();
      int bh = bitmap->getHeight();

      if (bitmap->convertToXBGR())
      {
        SplashColorPtr dataPtr = bitmap->getDataPtr();

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
      }
      delete splash_output;
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
  output_dev = new TextOutputDev(0, gFalse, 0, rawOrder, gFalse);
  m_page->parentDoc->doc->displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      0, false, true, false, -1, -1, -1, -1,
      NULL, NULL, NULL, NULL, gTrue);
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
  GBool sCase;
  QVector<Unicode> u;
  TextPage *textPage = m_page->prepareTextSearch(text, caseSensitive, rotate, &sCase, &u);

  bool found = false;
  if (direction == FromTop)
    found = textPage->findText( u.data(), u.size(), 
            gTrue, gTrue, gFalse, gFalse, sCase, gFalse, gFalse, &sLeft, &sTop, &sRight, &sBottom );
  else if ( direction == NextResult )
    found = textPage->findText( u.data(), u.size(), 
            gFalse, gTrue, gTrue, gFalse, sCase, gFalse, gFalse, &sLeft, &sTop, &sRight, &sBottom );
  else if ( direction == PreviousResult )
    found = textPage->findText( u.data(), u.size(), 
            gFalse, gTrue, gTrue, gFalse, sCase, gTrue, gFalse, &sLeft, &sTop, &sRight, &sBottom );

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

QList<QRectF> Page::search(const QString &text, SearchMode caseSensitive, Rotation rotate) const
{
  GBool sCase;
  QVector<Unicode> u;
  TextPage *textPage = m_page->prepareTextSearch(text, caseSensitive, rotate, &sCase, &u);

  QList<QRectF> results;
  double sLeft = 0.0, sTop = 0.0, sRight = 0.0, sBottom = 0.0;
  
  while(textPage->findText( u.data(), u.size(), 
        gFalse, gTrue, gTrue, gFalse, sCase, gFalse, gFalse, &sLeft, &sTop, &sRight, &sBottom ))
  {
      QRectF result;
      
      result.setLeft(sLeft);
      result.setTop(sTop);
      result.setRight(sRight);
      result.setBottom(sBottom);
      
      results.append(result);
  }
  
  textPage->decRefCnt();

  return results;
}

QList<TextBox*> Page::textList(Rotation rotate) const
{
  TextOutputDev *output_dev;
  
  QList<TextBox*> output_list;
  
  output_dev = new TextOutputDev(0, gFalse, 0, gFalse, gFalse);
  
  int rotation = (int)rotate * 90;

  m_page->parentDoc->doc->displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      rotation, false, false, false, -1, -1, -1, -1,
      NULL, NULL, NULL, NULL, gTrue);

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
  return AnnotationPrivate::findAnnotations(m_page->page, m_page->parentDoc);
}

void Page::addAnnotation( const Annotation *ann )
{
  AnnotationPrivate::addAnnotationToPage(m_page->page, m_page->parentDoc, ann);
}

void Page::removeAnnotation( const Annotation *ann )
{
  AnnotationPrivate::removeAnnotationFromPage(m_page->page, ann);
}

QList<FormField*> Page::formFields() const
{
  QList<FormField*> fields;
  ::Page *p = m_page->page;
  ::FormPageWidgets * form = p->getFormWidgets();
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

  delete form;

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

  return UnicodeParsedString(&goo);
}


}
