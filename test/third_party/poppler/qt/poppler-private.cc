/* poppler-private.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005-2008, Albert Astals Cid <aacid@kde.org>
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

#include "poppler-private.h"
#include "poppler-link-qt3.h"

#include <qstring.h>

#include <Outline.h>
#include <Link.h>

namespace Poppler {

/* borrowed from kpdf */
QString unicodeToQString(Unicode* u, int len)
{
    QString ret;
    ret.setLength(len);
    QChar* qch = (QChar*) ret.unicode();
    for (;len;--len)
      *qch++ = (QChar) *u++;
    return ret;
}

QString UnicodeParsedString(GooString *s1)
{
    GBool isUnicode;
    int i;
    Unicode u;
    QString result;
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
    while ( i < s1->getLength() )
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
    return result;
}

GooString *QStringToGooString(const QString &s)
{
    int len = s.length();
    char *cstring = (char *)gmallocn(s.length(), sizeof(char));
    for (int i = 0; i < len; ++i)
      cstring[i] = s.at(i).unicode();
    GooString *ret = new GooString(cstring, len);
    gfree(cstring);
    return ret;
}


void DocumentData::addTocChildren( QDomDocument * docSyn, QDomNode * parent, GooList * items )
{
    int numItems = items->getLength();
    for ( int i = 0; i < numItems; ++i )
    {
        // iterate over every object in 'items'
        OutlineItem * outlineItem = (OutlineItem *)items->get( i );

        // 1. create element using outlineItem's title as tagName
        QString name;
        Unicode * uniChar = outlineItem->getTitle();
        int titleLength = outlineItem->getTitleLength();
        name = unicodeToQString(uniChar, titleLength);
        if ( name.isEmpty() )
            continue;

        QDomElement item = docSyn->createElement( name );
        parent->appendChild( item );

        // 2. find the page the link refers to
        ::LinkAction * a = outlineItem->getAction();
        if ( a && ( a->getKind() == actionGoTo || a->getKind() == actionGoToR ) )
        {
            // page number is contained/referenced in a LinkGoTo
            LinkGoTo * g = static_cast< LinkGoTo * >( a );
            LinkDest * destination = g->getDest();
            if ( !destination && g->getNamedDest() )
            {
                // no 'destination' but an internal 'named reference'. we could
                // get the destination for the page now, but it's VERY time consuming,
                // so better storing the reference and provide the viewport on demand
                GooString *s = g->getNamedDest();
                QChar *charArray = new QChar[s->getLength()];
                for (int i = 0; i < s->getLength(); ++i) charArray[i] = QChar(s->getCString()[i]);
                    QString aux(charArray, s->getLength());
                    item.setAttribute( "DestinationName", aux );
                    delete[] charArray;
                }
                else if ( destination && destination->isOk() )
                {
                    LinkDestinationData ldd(destination, NULL, this);
                    item.setAttribute( "Destination", LinkDestination(ldd).toString() );
                }
                if ( a->getKind() == actionGoToR )
                {
                    LinkGoToR * g2 = static_cast< LinkGoToR * >( a );
                    item.setAttribute( "ExternalFileName", g2->getFileName()->getCString() );
                }
            }

        // 3. recursively descend over children
        outlineItem->open();
        GooList * children = outlineItem->getKids();
        if ( children )
            addTocChildren( docSyn, &item, children );
    }
}

}
