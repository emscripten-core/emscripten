/* poppler-private.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2006, 2011 by Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2008, 2010, 2011 by Pino Toscano <pino@kde.org>
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

#include "poppler-private.h"

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QVariant>

#include <Link.h>
#include <Outline.h>
#include <UnicodeMap.h>

namespace Poppler {

namespace Debug {

    void qDebugDebugFunction(const QString &message, const QVariant & /*closure*/)
    {
        qDebug() << message;
    }

    PopplerDebugFunc debugFunction = qDebugDebugFunction;
    QVariant debugClosure;

}

    static UnicodeMap *utf8Map = 0;

    void setDebugErrorFunction(PopplerDebugFunc function, const QVariant &closure)
    {
        Debug::debugFunction = function ? function : Debug::qDebugDebugFunction;
        Debug::debugClosure = closure;
    }

    void qt4ErrorFunction(int pos, char *msg, va_list args)
    {
        QString emsg;
        char buffer[1024]; // should be big enough

        if (pos >= 0)
        {
            emsg = QString::fromLatin1("Error (%1): ").arg(pos);
        }
        else
        {
            emsg = QString::fromLatin1("Error: ");
        }
        qvsnprintf(buffer, sizeof(buffer) - 1, msg, args);
        emsg += QString::fromAscii(buffer);
        (*Debug::debugFunction)(emsg, Debug::debugClosure);
    }

    QString unicodeToQString(Unicode* u, int len) {
        if (!utf8Map)
        {
                GooString enc("UTF-8");
                utf8Map = globalParams->getUnicodeMap(&enc);
                utf8Map->incRefCnt();
        }

        // ignore the last character if it is 0x0
        if ((len > 0) && (u[len - 1] == 0))
        {
            --len;
        }

        GooString convertedStr;
        for (int i = 0; i < len; ++i)
        {
            char buf[8];
            const int n = utf8Map->mapUnicode(u[i], buf, sizeof(buf));
            convertedStr.append(buf, n);
        }

        return QString::fromUtf8(convertedStr.getCString(), convertedStr.getLength());
    }

    QString UnicodeParsedString(GooString *s1) {
        if ( !s1 || s1->getLength() == 0 )
            return QString();

        GBool isUnicode;
        int i;
        Unicode u;
        QString result;
        if ( ( s1->getChar(0) & 0xff ) == 0xfe && ( s1->getLength() > 1 && ( s1->getChar(1) & 0xff ) == 0xff ) )
        {
            isUnicode = gTrue;
            i = 2;
            result.reserve( ( s1->getLength() - 2 ) / 2 );
        }
        else
        {
            isUnicode = gFalse;
            i = 0;
            result.reserve( s1->getLength() );
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
            result += QChar( u );
        }
        return result;
    }

    GooString *QStringToUnicodeGooString(const QString &s) {
        int len = s.length() * 2 + 2;
        char *cstring = (char *)gmallocn(len, sizeof(char));
        cstring[0] = 0xfe;
        cstring[1] = 0xff;
        for (int i = 0; i < s.length(); ++i)
        {
            cstring[2+i*2] = s.at(i).row();
            cstring[3+i*2] = s.at(i).cell();
        }
        GooString *ret = new GooString(cstring, len);
        gfree(cstring);
        return ret;
    }

    GooString *QStringToGooString(const QString &s) {
        int len = s.length();
        char *cstring = (char *)gmallocn(s.length(), sizeof(char));
        for (int i = 0; i < len; ++i)
            cstring[i] = s.at(i).unicode();
        GooString *ret = new GooString(cstring, len);
        gfree(cstring);
        return ret;
    }

    void linkActionToTocItem( ::LinkAction * a, DocumentData * doc, QDomElement * e )
    {
        if ( !a || !e )
            return;

        switch ( a->getKind() )
        {
            case actionGoTo:
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
                    e->setAttribute( "DestinationName", aux );
                    delete[] charArray;
                }
                else if ( destination && destination->isOk() )
                {
                    LinkDestinationData ldd(destination, NULL, doc, false);
                    e->setAttribute( "Destination", LinkDestination(ldd).toString() );
                }
                break;
            }
            case actionGoToR:
            {
                // page number is contained/referenced in a LinkGoToR
                LinkGoToR * g = static_cast< LinkGoToR * >( a );
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
                    e->setAttribute( "DestinationName", aux );
                    delete[] charArray;
                }
                else if ( destination && destination->isOk() )
                {
                    LinkDestinationData ldd(destination, NULL, doc, g->getFileName() != 0);
                    e->setAttribute( "Destination", LinkDestination(ldd).toString() );
                }
                e->setAttribute( "ExternalFileName", g->getFileName()->getCString() );
                break;
            }
            case actionURI:
            {
                LinkURI * u = static_cast< LinkURI * >( a );
                e->setAttribute( "DestinationURI", u->getURI()->getCString() );
            }
            default: ;
        }
    }
    
    DocumentData::~DocumentData()
    {
        qDeleteAll(m_embeddedFiles);
        delete (OptContentModel *)m_optContentModel;
        delete doc;
        delete m_outputDev;
        delete m_fontInfoIterator;
    
        count --;
        if ( count == 0 )
        {
            utf8Map = 0;
            delete globalParams;
        }
      }
    
    void DocumentData::init(GooString *ownerPassword, GooString *userPassword)
    {
        m_fontInfoIterator = 0;
        m_backend = Document::SplashBackend;
        m_outputDev = 0;
        paperColor = Qt::white;
        m_hints = 0;
        m_optContentModel = 0;
        // It might be more appropriate to delete these in PDFDoc
        delete ownerPassword;
        delete userPassword;
      
        if ( count == 0 )
        {
            utf8Map = 0;
            globalParams = new GlobalParams();
            setErrorFunction(qt4ErrorFunction);
        }
        count ++;
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
            linkActionToTocItem( a, this, &item );

            item.setAttribute( "Open", QVariant( (bool)outlineItem->isOpen() ).toString() );

            // 3. recursively descend over children
            outlineItem->open();
            GooList * children = outlineItem->getKids();
            if ( children )
                addTocChildren( docSyn, &item, children );
        }
    }

}
