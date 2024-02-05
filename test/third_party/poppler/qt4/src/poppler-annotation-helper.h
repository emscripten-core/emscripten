/* poppler-annotation-helper.h: qt interface to poppler
 * Copyright (C) 2006, 2008, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Adapting code from
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

#include <QtCore/QDebug>

class QColor;

class AnnotColor;

namespace Poppler {

class XPDFReader
{
    public:
        // find named symbol and parse it
        static inline void lookupName( Dict *, char *, QString & dest );
        static inline void lookupString( Dict *, char *, QString & dest );
        static inline void lookupBool( Dict *, char *, bool & dest );
        static inline void lookupInt( Dict *, char *, int & dest );
        static inline void lookupNum( Dict *, char *, double & dest );
        static inline int lookupNumArray( Dict *, char *, double * dest, int len );
        static inline void lookupColor( Dict *, char *, QColor & color );
        static inline void lookupIntRef( Dict *, char *, int & dest );
        static inline void lookupDate( Dict *, char *, QDateTime & dest );
        // transform from user coords to normalized ones using the matrix M
        static inline void transform( double * M, double x, double y, QPointF &res );
};

void XPDFReader::lookupName( Dict * dict, char * type, QString & dest )
{
    Object nameObj;
    dict->lookup( type, &nameObj );
    if ( nameObj.isNull() )
        return;
    if ( nameObj.isName() )
        dest = nameObj.getName();
    else
        qDebug() << type << " is not Name." << endl;
    nameObj.free();
}

void XPDFReader::lookupString( Dict * dict, char * type, QString & dest )
{
    Object stringObj;
    dict->lookup( type, &stringObj );
    if ( stringObj.isNull() )
        return;
    if ( stringObj.isString() )
        dest = stringObj.getString()->getCString();
    else
        qDebug() << type << " is not String." << endl;
    stringObj.free();
}

void XPDFReader::lookupBool( Dict * dict, char * type, bool & dest )
{
    Object boolObj;
    dict->lookup( type, &boolObj );
    if ( boolObj.isNull() )
        return;
    if ( boolObj.isBool() )
        dest = boolObj.getBool() == gTrue;
    else
        qDebug() << type << " is not Bool." << endl;
    boolObj.free();
}

void XPDFReader::lookupInt( Dict * dict, char * type, int & dest )
{
    Object intObj;
    dict->lookup( type, &intObj );
    if ( intObj.isNull() )
        return;
    if ( intObj.isInt() )
        dest = intObj.getInt();
    else
        qDebug() << type << " is not Int." << endl;
    intObj.free();
}

void XPDFReader::lookupNum( Dict * dict, char * type, double & dest )
{
    Object numObj;
    dict->lookup( type, &numObj );
    if ( numObj.isNull() )
        return;
    if ( numObj.isNum() )
        dest = numObj.getNum();
    else
        qDebug() << type << " is not Num." << endl;
    numObj.free();
}

int XPDFReader::lookupNumArray( Dict * dict, char * type, double * dest, int len )
{
    Object arrObj;
    dict->lookup( type, &arrObj );
    if ( arrObj.isNull() )
        return 0;
    Object numObj;
    if ( arrObj.isArray() )
    {
        len = qMin( len, arrObj.arrayGetLength() );
        for ( int i = 0; i < len; i++ )
        {
            dest[i] = arrObj.arrayGet( i, &numObj )->getNum();
            numObj.free();
        }
    }
    else
    {
        len = 0;
        qDebug() << type << "is not Array." << endl;
    }
    arrObj.free();
    return len;
}

void XPDFReader::lookupColor( Dict * dict, char * type, QColor & dest )
{
    double c[3];
    if ( XPDFReader::lookupNumArray( dict, type, c, 3 ) == 3 )
        dest = QColor( (int)(c[0]*255.0), (int)(c[1]*255.0), (int)(c[2]*255.0));
}

void XPDFReader::lookupIntRef( Dict * dict, char * type, int & dest )
{
    Object refObj;
    dict->lookupNF( type, &refObj );
    if ( refObj.isNull() )
        return;
    if ( refObj.isRef() )
        dest = refObj.getRefNum();
    else
        qDebug() << type << " is not Ref." << endl;
    refObj.free();
}

void XPDFReader::lookupDate( Dict * dict, char * type, QDateTime & dest )
{
    Object dateObj;
    dict->lookup( type, &dateObj );
    if ( dateObj.isNull() )
        return;
    if ( dateObj.isString() )
    {
        dest = convertDate( dateObj.getString()->getCString() );
    }
    else
        qDebug() << type << " is not Date" << endl;
    dateObj.free();
}

void XPDFReader::transform( double * M, double x, double y, QPointF &res )
{
    res.setX( M[0] * x + M[2] * y + M[4] );
    res.setY( M[1] * x + M[3] * y + M[5] );
}

/** @short Helper classes for CROSSDEPS resolving and DS conversion. */
struct ResolveRevision
{
    int           prevAnnotationID; // ID of the annotation to be reparended
    int           nextAnnotationID; // (only needed for speeding up resolving)
    Annotation *  nextAnnotation;   // annotation that will act as parent
    Annotation::RevScope nextScope; // scope of revision (Reply)
    Annotation::RevType  nextType;  // type of revision (None)
};

struct ResolveWindow
{
    AnnotPopup *  popup;            // the (maybe shared) window
    Annotation *  annotation;       // annotation having the popup window
};

struct PostProcessText              // this handles a special pdf case conversion
{
    Annotation *  textAnnotation;   // a popup text annotation (not FreeText)
    bool          opened;           // pdf property to convert to window flags
};

struct PopupWindow
{
    Annotation *  dummyAnnotation;  // window properties (in pdf as Annotation)
    bool          shown;            // converted to Annotation::Hidden flag
};

QColor convertAnnotColor( AnnotColor *color );

}
