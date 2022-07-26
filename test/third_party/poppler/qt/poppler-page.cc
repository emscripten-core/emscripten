/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005-2006, 2010 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Tobias Koening <tokoe@kde.org>
 * Copyright (C) 2005, Stefan Kebekus <stefan.kebekus@math.uni-koeln.de>
 * Copyright (C) 2006, Wilfried Huss <Wilfried.Huss@gmx.at>
 * Copyright (C) 2006, Jerry Epplin <jepplin@globalvelocity.com>
 * Copyright (C) 2007, 2010, Pino Toscano <pino@kde.org>
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
#include <qimage.h>
#include <config.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Catalog.h>
#include <ErrorCodes.h>
#include <TextOutputDev.h>
#include <Link.h>
#if defined(HAVE_SPLASH)
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#endif

#include "poppler-private.h"
#include "poppler-page-transition-private.h"

namespace Poppler {

Page::Page(const Document *doc, int index) {
  data = new PageData();
  data->index = index;
  data->doc = doc;
  data->transition = 0;
  data->page = doc->data->doc.getPage(data->index + 1);
}

Page::~Page()
{
  delete data->transition;
  delete data;
}

void Page::renderToPixmap(QPixmap **q, int x, int y, int w, int h, bool doLinks) const
{
  renderToPixmap(q, x, y, w, h, 72.0, 72.0, doLinks);
}

void Page::renderToPixmap(QPixmap **q, int x, int y, int w, int h, double xres, double yres, bool doLinks) const
{
  QImage img = renderToImage(xres, yres, doLinks);
  *q = new QPixmap( img );
}

QImage Page::renderToImage(double xres, double yres, bool doLinks) const
{
#if defined(HAVE_SPLASH)
  SplashOutputDev *output_dev;
  SplashBitmap *bitmap;
  SplashColorPtr color_ptr;
  output_dev = data->doc->data->getOutputDev();

  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, xres, yres,
      0, false, true, false, -1, -1, -1, -1);
  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();
  int bw = output_dev->getBitmap()->getWidth();
  int bh = output_dev->getBitmap()->getHeight();
  SplashColorPtr dataPtr = output_dev->getBitmap()->getDataPtr();
  
  if (QImage::BigEndian == QImage::systemByteOrder())
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
  QImage img( dataPtr, bw, bh, 32, 0, 0, QImage::IgnoreEndian );
  img = img.copy();
  // unload underlying xpdf bitmap
  output_dev->startPage( 0, NULL );

  return img;
#else
  (void)xres;
  (void)xres;
  (void)doLinks;

  return QImage();
#endif
}

QString Page::getText(const Rectangle &r) const
{
  TextOutputDev *output_dev;
  GooString *s;
  PDFRectangle *rect;
  QString result;
  ::Page *p;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);
  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, 72, 72,
      0, false, false, false, -1, -1, -1, -1);
  p = data->page;
  if (r.isNull())
  {
    rect = p->getCropBox();
    s = output_dev->getText(rect->x1, rect->y1, rect->x2, rect->y2);
  }
  else
  {
    double height, y1, y2;
    height = p->getCropHeight();
    y1 = height - r.m_y2;
    y2 = height - r.m_y1;
    s = output_dev->getText(r.m_x1, y1, r.m_x2, y2);
  }

  result = QString::fromUtf8(s->getCString());

  delete output_dev;
  delete s;
  return result;
}

QValueList<TextBox*> Page::textList() const
{
  TextOutputDev *output_dev;
  
  QValueList<TextBox*> output_list;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);

  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, 72, 72,
      0, false, false, false, -1, -1, -1, -1);

  TextWordList *word_list = output_dev->makeWordList();
  
  if (!word_list) {
    delete output_dev;
    return output_list;
  }
  
  for (int i = 0; i < word_list->getLength(); i++) {
    TextWord *word = word_list->get(i);
    GooString *word_str = word->getText();
    QString string = QString::fromUtf8(word_str->getCString());
    delete word_str;
    double xMin, yMin, xMax, yMax;
    word->getBBox(&xMin, &yMin, &xMax, &yMax);
    
    TextBox* text_box = new TextBox(string, Rectangle(xMin, yMin, xMax, yMax));
    
    output_list.append(text_box);
  }
  
  delete word_list;
  delete output_dev;
  
  return output_list;
}

PageTransition *Page::getTransition() const
{
  if (!data->transition) 
  {
    Object o;
    PageTransitionParams params;
    params.dictObj = data->page->getTrans(&o);
    data->transition = new PageTransition(params);
    o.free();
  }
  return data->transition;
}

QSize Page::pageSize() const
{
  ::Page *p;

  p = data->page;
  if ( ( Page::Landscape == orientation() ) || (Page::Seascape == orientation() ) ) {
    return QSize( (int)p->getCropHeight(), (int)p->getCropWidth() );
  } else {
    return QSize( (int)p->getCropWidth(), (int)p->getCropHeight() );
  }
}

Page::Orientation Page::orientation() const
{
  ::Page *p = data->page;

  int rotation = p->getRotate();
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

QValueList<Link*> Page::links() const
{
  QValueList<Link*> popplerLinks;

#if defined(HAVE_SPLASH)
  Links *xpdfLinks = data->doc->data->doc.getLinks(data->index + 1);
  for (int i = 0; i < xpdfLinks->getNumLinks(); ++i)
  {
    ::Link *xpdfLink = xpdfLinks->getLink(i);
    
    double left, top, right, bottom;
    int leftAux, topAux, rightAux, bottomAux;
    xpdfLink->getRect( &left, &top, &right, &bottom );
    QRect linkArea;
    
    data->doc->data->m_outputDev->cvtUserToDev( left, top, &leftAux, &topAux );
    data->doc->data->m_outputDev->cvtUserToDev( right, bottom, &rightAux, &bottomAux );
    linkArea.setLeft(leftAux);
    linkArea.setTop(topAux);
    linkArea.setRight(rightAux);
    linkArea.setBottom(bottomAux);

    if (!xpdfLink->isOk()) continue;

    Link *popplerLink = NULL;
    ::LinkAction *a = xpdfLink->getAction();
    if ( a )
    {
      switch ( a->getKind() )
      {
        case actionGoTo:
        {
          LinkGoTo * g = (LinkGoTo *) a;
          // create link: no ext file, namedDest, object pointer
          popplerLink = new LinkGoto( linkArea, QString::null, LinkDestination( LinkDestinationData(g->getDest(), g->getNamedDest(), data->doc->data ) ) );
        }
        break;

        case actionGoToR:
        {
          LinkGoToR * g = (LinkGoToR *) a;
          // copy link file
          const QString fileName = UnicodeParsedString( g->getFileName() );
          // ceate link: fileName, namedDest, object pointer
          popplerLink = new LinkGoto( linkArea, fileName, LinkDestination( LinkDestinationData(g->getDest(), g->getNamedDest(), data->doc->data ) ) );
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

        case actionMovie:
        case actionSound:
        case actionRendition:
        case actionJavaScript:
        case actionOCGState:
        break;

        case actionUnknown:
        break;
      }
    }
    
    if (popplerLink)
    {
      popplerLinks.append(popplerLink);
    }
  }

  delete xpdfLinks;
#endif
  
  return popplerLinks;
}

}
