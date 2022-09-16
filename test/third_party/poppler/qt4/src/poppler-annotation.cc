/* poppler-annotation.cc: qt interface to poppler
 * Copyright (C) 2006, 2009 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006, 2008, 2010 Pino Toscano <pino@kde.org>
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

// qt/kde includes
#include <QtXml/QDomElement>
#include <QtGui/QColor>

// local includes
#include "poppler-annotation.h"
#include "poppler-link.h"
#include "poppler-qt4.h"
#include "poppler-annotation-private.h"

// poppler includes
#include <Page.h>
#include <Annot.h>

namespace Poppler {

//BEGIN AnnotationUtils implementation
Annotation * AnnotationUtils::createAnnotation( const QDomElement & annElement )
{
    // safety check on annotation element
    if ( !annElement.hasAttribute( "type" ) )
        return 0;

    // build annotation of given type
    Annotation * annotation = 0;
    int typeNumber = annElement.attribute( "type" ).toInt();
    switch ( typeNumber )
    {
        case Annotation::AText:
            annotation = new TextAnnotation( annElement );
            break;
        case Annotation::ALine:
            annotation = new LineAnnotation( annElement );
            break;
        case Annotation::AGeom:
            annotation = new GeomAnnotation( annElement );
            break;
        case Annotation::AHighlight:
            annotation = new HighlightAnnotation( annElement );
            break;
        case Annotation::AStamp:
            annotation = new StampAnnotation( annElement );
            break;
        case Annotation::AInk:
            annotation = new InkAnnotation( annElement );
            break;
        case Annotation::ACaret:
            annotation = new CaretAnnotation( annElement );
            break;
    }

    // return created annotation
    return annotation;
}

void AnnotationUtils::storeAnnotation( const Annotation * ann, QDomElement & annElement,
    QDomDocument & document )
{
    // save annotation's type as element's attribute
    annElement.setAttribute( "type", (uint)ann->subType() );

    // append all annotation data as children of this node
    ann->store( annElement, document );
}

QDomElement AnnotationUtils::findChildElement( const QDomNode & parentNode,
    const QString & name )
{
    // loop through the whole children and return a 'name' named element
    QDomNode subNode = parentNode.firstChild();
    while( subNode.isElement() )
    {
        QDomElement element = subNode.toElement();
        if ( element.tagName() == name )
            return element;
        subNode = subNode.nextSibling();
    }
    // if the name can't be found, return a dummy null element
    return QDomElement();
}
//END AnnotationUtils implementation


//BEGIN Annotation implementation
AnnotationPrivate::AnnotationPrivate()
    : flags( 0 )
{
}

AnnotationPrivate::~AnnotationPrivate()
{
}


Annotation::Style::Style()
    : opacity( 1.0 ), width( 1.0 ), style( Solid ), xCorners( 0.0 ),
    yCorners( 0.0 ), marks( 3 ), spaces( 0 ), effect( NoEffect ),
    effectIntensity( 1.0 ) {}

Annotation::Window::Window()
    : flags( -1 ), width( 0 ), height( 0 ) {}

Annotation::Revision::Revision()
    : annotation( 0 ), scope( Reply ), type( None ) {}

Annotation::Annotation( AnnotationPrivate &dd )
    : d_ptr( &dd )
{
}

Annotation::~Annotation()
{
    Q_D( Annotation );
    // delete all children revisions
    QLinkedList< Annotation::Revision >::iterator it = d->revisions.begin(), end = d->revisions.end();
    for ( ; it != end; ++it )
        delete (*it).annotation;

    delete d_ptr;
}

Annotation::Annotation( AnnotationPrivate &dd, const QDomNode &annNode )
    : d_ptr( &dd )
{
    Q_D( Annotation );
    // get the [base] element of the annotation node
    QDomElement e = AnnotationUtils::findChildElement( annNode, "base" );
    if ( e.isNull() )
        return;

    // parse -contents- attributes
    if ( e.hasAttribute( "author" ) )
        d->author = e.attribute( "author" );
    if ( e.hasAttribute( "contents" ) )
        d->contents = e.attribute( "contents" );
    if ( e.hasAttribute( "uniqueName" ) )
        d->uniqueName = e.attribute( "uniqueName" );
    if ( e.hasAttribute( "modifyDate" ) )
        d->modDate = QDateTime::fromString( e.attribute( "modifyDate" ) );
    if ( e.hasAttribute( "creationDate" ) )
        d->creationDate = QDateTime::fromString( e.attribute( "creationDate" ) );

    // parse -other- attributes
    if ( e.hasAttribute( "flags" ) )
        d->flags = e.attribute( "flags" ).toInt();
    if ( e.hasAttribute( "color" ) )
        style.color = QColor( e.attribute( "color" ) );
    if ( e.hasAttribute( "opacity" ) )
        style.opacity = e.attribute( "opacity" ).toDouble();

    // parse -the-subnodes- (describing Style, Window, Revision(s) structures)
    // Note: all subnodes if present must be 'attributes complete'
    QDomNode eSubNode = e.firstChild();
    while ( eSubNode.isElement() )
    {
        QDomElement ee = eSubNode.toElement();
        eSubNode = eSubNode.nextSibling();

        // parse boundary
        if ( ee.tagName() == "boundary" )
        {
            d->boundary.setLeft(ee.attribute( "l" ).toDouble());
            d->boundary.setTop(ee.attribute( "t" ).toDouble());
            d->boundary.setRight(ee.attribute( "r" ).toDouble());
            d->boundary.setBottom(ee.attribute( "b" ).toDouble());
        }
        // parse penStyle if not default
        else if ( ee.tagName() == "penStyle" )
        {
            style.width = ee.attribute( "width" ).toDouble();
            style.style = (LineStyle)ee.attribute( "style" ).toInt();
            style.xCorners = ee.attribute( "xcr" ).toDouble();
            style.yCorners = ee.attribute( "ycr" ).toDouble();
            style.marks = ee.attribute( "marks" ).toInt();
            style.spaces = ee.attribute( "spaces" ).toInt();
        }
        // parse effectStyle if not default
        else if ( ee.tagName() == "penEffect" )
        {
            style.effect = (LineEffect)ee.attribute( "effect" ).toInt();
            style.effectIntensity = ee.attribute( "intensity" ).toDouble();
        }
        // parse window if present
        else if ( ee.tagName() == "window" )
        {
            window.flags = ee.attribute( "flags" ).toInt();
            window.topLeft.setX(ee.attribute( "top" ).toDouble());
            window.topLeft.setY(ee.attribute( "left" ).toDouble());
            window.width = ee.attribute( "width" ).toInt();
            window.height = ee.attribute( "height" ).toInt();
            window.title = ee.attribute( "title" );
            window.summary = ee.attribute( "summary" );
            // parse window subnodes
            QDomNode winNode = ee.firstChild();
            for ( ; winNode.isElement(); winNode = winNode.nextSibling() )
            {
                QDomElement winElement = winNode.toElement();
                if ( winElement.tagName() == "text" )
                    window.text = winElement.firstChild().toCDATASection().data();
            }
        }
    }

    // get the [revisions] element of the annotation node
    QDomNode revNode = annNode.firstChild();
    for ( ; revNode.isElement(); revNode = revNode.nextSibling() )
    {
        QDomElement revElement = revNode.toElement();
        if ( revElement.tagName() != "revision" )
            continue;

        // compile the Revision structure crating annotation
        Revision rev;
        rev.scope = (RevScope)revElement.attribute( "revScope" ).toInt();
        rev.type = (RevType)revElement.attribute( "revType" ).toInt();
        rev.annotation = AnnotationUtils::createAnnotation( revElement );

        // if annotation is valid, add revision to internal list
        if ( rev.annotation )
            d->revisions.append( rev );
    }
}

void Annotation::store( QDomNode & annNode, QDomDocument & document ) const
{
    Q_D( const Annotation );
    // create [base] element of the annotation node
    QDomElement e = document.createElement( "base" );
    annNode.appendChild( e );

    // store -contents- attributes
    if ( !d->author.isEmpty() )
        e.setAttribute( "author", d->author );
    if ( !d->contents.isEmpty() )
        e.setAttribute( "contents", d->contents );
    if ( !d->uniqueName.isEmpty() )
        e.setAttribute( "uniqueName", d->uniqueName );
    if ( d->modDate.isValid() )
        e.setAttribute( "modifyDate", d->modDate.toString() );
    if ( d->creationDate.isValid() )
        e.setAttribute( "creationDate", d->creationDate.toString() );

    // store -other- attributes
    if ( d->flags )
        e.setAttribute( "flags", d->flags );
    if ( style.color.isValid() && style.color != Qt::black )
        e.setAttribute( "color", style.color.name() );
    if ( style.opacity != 1.0 )
        e.setAttribute( "opacity", QString::number( style.opacity ) );

    // Sub-Node-1 - boundary
    QDomElement bE = document.createElement( "boundary" );
    e.appendChild( bE );
    bE.setAttribute( "l", QString::number( (double)d->boundary.left() ) );
    bE.setAttribute( "t", QString::number( (double)d->boundary.top() ) );
    bE.setAttribute( "r", QString::number( (double)d->boundary.right() ) );
    bE.setAttribute( "b", QString::number( (double)d->boundary.bottom() ) );

    // Sub-Node-2 - penStyle
    if ( style.width != 1 || style.style != Solid || style.xCorners != 0 ||
         style.yCorners != 0.0 || style.marks != 3 || style.spaces != 0 )
    {
        QDomElement psE = document.createElement( "penStyle" );
        e.appendChild( psE );
        psE.setAttribute( "width", QString::number( style.width ) );
        psE.setAttribute( "style", (int)style.style );
        psE.setAttribute( "xcr", QString::number( style.xCorners ) );
        psE.setAttribute( "ycr", QString::number( style.yCorners ) );
        psE.setAttribute( "marks", style.marks );
        psE.setAttribute( "spaces", style.spaces );
    }

    // Sub-Node-3 - penEffect
    if ( style.effect != NoEffect || style.effectIntensity != 1.0 )
    {
        QDomElement peE = document.createElement( "penEffect" );
        e.appendChild( peE );
        peE.setAttribute( "effect", (int)style.effect );
        peE.setAttribute( "intensity", QString::number( style.effectIntensity ) );
    }

    // Sub-Node-4 - window
    if ( window.flags != -1 || !window.title.isEmpty() ||
         !window.summary.isEmpty() || !window.text.isEmpty() )
    {
        QDomElement wE = document.createElement( "window" );
        e.appendChild( wE );
        wE.setAttribute( "flags", window.flags );
        wE.setAttribute( "top", QString::number( window.topLeft.x() ) );
        wE.setAttribute( "left", QString::number( window.topLeft.y() ) );
        wE.setAttribute( "width", window.width );
        wE.setAttribute( "height", window.height );
        wE.setAttribute( "title", window.title );
        wE.setAttribute( "summary", window.summary );
        // store window.text as a subnode, because we need escaped data
        if ( !window.text.isEmpty() )
        {
            QDomElement escapedText = document.createElement( "text" );
            wE.appendChild( escapedText );
            QDomCDATASection textCData = document.createCDATASection( window.text );
            escapedText.appendChild( textCData );
        }
    }

    // create [revision] element of the annotation node (if any)
    if ( d->revisions.isEmpty() )
        return;

    // add all revisions as children of revisions element
    QLinkedList< Revision >::const_iterator it = d->revisions.begin(), end = d->revisions.end();
    for ( ; it != end; ++it )
    {
        // create revision element
        const Revision & revision = *it;
        QDomElement r = document.createElement( "revision" );
        annNode.appendChild( r );
        // set element attributes
        r.setAttribute( "revScope", (int)revision.scope );
        r.setAttribute( "revType", (int)revision.type );
        // use revision as the annotation element, so fill it up
        AnnotationUtils::storeAnnotation( revision.annotation, r, document );
    }
}

QString Annotation::author() const
{
    Q_D( const Annotation );
    return d->author;
}

void Annotation::setAuthor( const QString &author )
{
    Q_D( Annotation );
    d->author = author;
}

QString Annotation::contents() const
{
    Q_D( const Annotation );
    return d->contents;
}

void Annotation::setContents( const QString &contents )
{
    Q_D( Annotation );
    d->contents = contents;
}

QString Annotation::uniqueName() const
{
    Q_D( const Annotation );
    return d->uniqueName;
}

void Annotation::setUniqueName( const QString &uniqueName )
{
    Q_D( Annotation );
    d->uniqueName = uniqueName;
}

QDateTime Annotation::modificationDate() const
{
    Q_D( const Annotation );
    return d->modDate;
}

void Annotation::setModificationDate( const QDateTime &date )
{
    Q_D( Annotation );
    d->modDate = date;
}

QDateTime Annotation::creationDate() const
{
    Q_D( const Annotation );
    return d->creationDate;
}

void Annotation::setCreationDate( const QDateTime &date )
{
    Q_D( Annotation );
    d->creationDate = date;
}

int Annotation::flags() const
{
    Q_D( const Annotation );
    return d->flags;
}

void Annotation::setFlags( int flags )
{
    Q_D( Annotation );
    d->flags = flags;
}

QRectF Annotation::boundary() const
{
    Q_D( const Annotation );
    return d->boundary;
}

void Annotation::setBoundary( const QRectF &boundary )
{
    Q_D( Annotation );
    d->boundary = boundary;
}

QLinkedList< Annotation::Revision >& Annotation::revisions()
{
    Q_D( Annotation );
    return d->revisions;
}

const QLinkedList< Annotation::Revision >& Annotation::revisions() const
{
    Q_D( const Annotation );
    return d->revisions;
}

//END AnnotationUtils implementation


/** TextAnnotation [Annotation] */
class TextAnnotationPrivate : public AnnotationPrivate
{
    public:
        TextAnnotationPrivate();

        // data fields
        TextAnnotation::TextType textType;
        QString textIcon;
        QFont textFont;
        int inplaceAlign; // 0:left, 1:center, 2:right
        QString inplaceText;  // overrides contents
        QPointF inplaceCallout[3];
        TextAnnotation::InplaceIntent inplaceIntent;
};

TextAnnotationPrivate::TextAnnotationPrivate()
    : AnnotationPrivate(), textType( TextAnnotation::Linked ),
    textIcon( "Note" ), inplaceAlign( 0 ),
    inplaceIntent( TextAnnotation::Unknown )
{
}

TextAnnotation::TextAnnotation()
    : Annotation( *new TextAnnotationPrivate() )
{}

TextAnnotation::TextAnnotation( const QDomNode & node )
    : Annotation( *new TextAnnotationPrivate, node )
{
    Q_D( TextAnnotation );

    // loop through the whole children looking for a 'text' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "text" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "type" ) )
            d->textType = (TextAnnotation::TextType)e.attribute( "type" ).toInt();
        if ( e.hasAttribute( "icon" ) )
            d->textIcon = e.attribute( "icon" );
        if ( e.hasAttribute( "font" ) )
            d->textFont.fromString( e.attribute( "font" ) );
        if ( e.hasAttribute( "align" ) )
            d->inplaceAlign = e.attribute( "align" ).toInt();
        if ( e.hasAttribute( "intent" ) )
            d->inplaceIntent = (TextAnnotation::InplaceIntent)e.attribute( "intent" ).toInt();

        // parse the subnodes
        QDomNode eSubNode = e.firstChild();
        while ( eSubNode.isElement() )
        {
            QDomElement ee = eSubNode.toElement();
            eSubNode = eSubNode.nextSibling();

            if ( ee.tagName() == "escapedText" )
            {
                d->inplaceText = ee.firstChild().toCDATASection().data();
            }
            else if ( ee.tagName() == "callout" )
            {
                d->inplaceCallout[0].setX(ee.attribute( "ax" ).toDouble());
                d->inplaceCallout[0].setY(ee.attribute( "ay" ).toDouble());
                d->inplaceCallout[1].setX(ee.attribute( "bx" ).toDouble());
                d->inplaceCallout[1].setY(ee.attribute( "by" ).toDouble());
                d->inplaceCallout[2].setX(ee.attribute( "cx" ).toDouble());
                d->inplaceCallout[2].setY(ee.attribute( "cy" ).toDouble());
            }
        }

        // loading complete
        break;
    }
}

TextAnnotation::~TextAnnotation()
{
}

void TextAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    Q_D( const TextAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [text] element
    QDomElement textElement = document.createElement( "text" );
    node.appendChild( textElement );

    // store the optional attributes
    if ( d->textType != Linked )
        textElement.setAttribute( "type", (int)d->textType );
    if ( d->textIcon != "Comment" )
        textElement.setAttribute( "icon", d->textIcon );
    if ( d->inplaceAlign )
        textElement.setAttribute( "align", d->inplaceAlign );
    if ( d->inplaceIntent != Unknown )
        textElement.setAttribute( "intent", (int)d->inplaceIntent );

    textElement.setAttribute( "font", d->textFont.toString() );

    // Sub-Node-1 - escapedText
    if ( !d->inplaceText.isEmpty() )
    {
        QDomElement escapedText = document.createElement( "escapedText" );
        textElement.appendChild( escapedText );
        QDomCDATASection textCData = document.createCDATASection( d->inplaceText );
        escapedText.appendChild( textCData );
    }

    // Sub-Node-2 - callout
    if ( d->inplaceCallout[0].x() != 0.0 )
    {
        QDomElement calloutElement = document.createElement( "callout" );
        textElement.appendChild( calloutElement );
        calloutElement.setAttribute( "ax", QString::number( d->inplaceCallout[0].x() ) );
        calloutElement.setAttribute( "ay", QString::number( d->inplaceCallout[0].y() ) );
        calloutElement.setAttribute( "bx", QString::number( d->inplaceCallout[1].x() ) );
        calloutElement.setAttribute( "by", QString::number( d->inplaceCallout[1].y() ) );
        calloutElement.setAttribute( "cx", QString::number( d->inplaceCallout[2].x() ) );
        calloutElement.setAttribute( "cy", QString::number( d->inplaceCallout[2].y() ) );
    }
}

Annotation::SubType TextAnnotation::subType() const
{
    return AText;
}

TextAnnotation::TextType TextAnnotation::textType() const
{
    Q_D( const TextAnnotation );
    return d->textType;
}

void TextAnnotation::setTextType( TextAnnotation::TextType type )
{
    Q_D( TextAnnotation );
    d->textType = type;
}

QString TextAnnotation::textIcon() const
{
    Q_D( const TextAnnotation );
    return d->textIcon;
}

void TextAnnotation::setTextIcon( const QString &icon )
{
    Q_D( TextAnnotation );
    d->textIcon = icon;
}

QFont TextAnnotation::textFont() const
{
    Q_D( const TextAnnotation );
    return d->textFont;
}

void TextAnnotation::setTextFont( const QFont &font )
{
    Q_D( TextAnnotation );
    d->textFont = font;
}

int TextAnnotation::inplaceAlign() const
{
    Q_D( const TextAnnotation );
    return d->inplaceAlign;
}

void TextAnnotation::setInplaceAlign( int align )
{
    Q_D( TextAnnotation );
    d->inplaceAlign = align;
}

QString TextAnnotation::inplaceText() const
{
    Q_D( const TextAnnotation );
    return d->inplaceText;
}

void TextAnnotation::setInplaceText( const QString &text )
{
    Q_D( TextAnnotation );
    d->inplaceText = text;
}

QPointF TextAnnotation::calloutPoint( int id ) const
{
    if ( id < 0 || id >= 3 )
        return QPointF();

    Q_D( const TextAnnotation );
    return d->inplaceCallout[id];
}

void TextAnnotation::setCalloutPoint( int id, const QPointF &point )
{
    if ( id < 0 || id >= 3 )
        return;

    Q_D( TextAnnotation );
    d->inplaceCallout[id] = point;
}

TextAnnotation::InplaceIntent TextAnnotation::inplaceIntent() const
{
    Q_D( const TextAnnotation );
    return d->inplaceIntent;
}

void TextAnnotation::setInplaceIntent( TextAnnotation::InplaceIntent intent )
{
    Q_D( TextAnnotation );
    d->inplaceIntent = intent;
}


/** LineAnnotation [Annotation] */
class LineAnnotationPrivate : public AnnotationPrivate
{
    public:
        LineAnnotationPrivate();

        // data fields (note uses border for rendering style)
        QLinkedList<QPointF> linePoints;
        LineAnnotation::TermStyle lineStartStyle;
        LineAnnotation::TermStyle lineEndStyle;
        bool lineClosed : 1;  // (if true draw close shape)
        bool lineShowCaption : 1;
        QColor lineInnerColor;
        double lineLeadingFwdPt;
        double lineLeadingBackPt;
        LineAnnotation::LineIntent lineIntent;
};

LineAnnotationPrivate::LineAnnotationPrivate()
    : AnnotationPrivate(), lineStartStyle( LineAnnotation::None ),
    lineEndStyle( LineAnnotation::None ), lineClosed( false ),
    lineShowCaption( false ), lineLeadingFwdPt( 0 ), lineLeadingBackPt( 0 ),
    lineIntent( LineAnnotation::Unknown )
{
}

LineAnnotation::LineAnnotation()
    : Annotation( *new LineAnnotationPrivate() )
{}

LineAnnotation::LineAnnotation( const QDomNode & node )
    : Annotation( *new LineAnnotationPrivate(), node )
{
    Q_D( LineAnnotation );

    // loop through the whole children looking for a 'line' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "line" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "startStyle" ) )
            d->lineStartStyle = (LineAnnotation::TermStyle)e.attribute( "startStyle" ).toInt();
        if ( e.hasAttribute( "endStyle" ) )
            d->lineEndStyle = (LineAnnotation::TermStyle)e.attribute( "endStyle" ).toInt();
        if ( e.hasAttribute( "closed" ) )
            d->lineClosed = e.attribute( "closed" ).toInt();
        if ( e.hasAttribute( "innerColor" ) )
            d->lineInnerColor = QColor( e.attribute( "innerColor" ) );
        if ( e.hasAttribute( "leadFwd" ) )
            d->lineLeadingFwdPt = e.attribute( "leadFwd" ).toDouble();
        if ( e.hasAttribute( "leadBack" ) )
            d->lineLeadingBackPt = e.attribute( "leadBack" ).toDouble();
        if ( e.hasAttribute( "showCaption" ) )
            d->lineShowCaption = e.attribute( "showCaption" ).toInt();
        if ( e.hasAttribute( "intent" ) )
            d->lineIntent = (LineAnnotation::LineIntent)e.attribute( "intent" ).toInt();

        // parse all 'point' subnodes
        QDomNode pointNode = e.firstChild();
        while ( pointNode.isElement() )
        {
            QDomElement pe = pointNode.toElement();
            pointNode = pointNode.nextSibling();

            if ( pe.tagName() != "point" )
                continue;

            QPointF p(pe.attribute( "x", "0.0" ).toDouble(), pe.attribute( "y", "0.0" ).toDouble());
            d->linePoints.append( p );
        }

        // loading complete
        break;
    }
}

LineAnnotation::~LineAnnotation()
{
}

void LineAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    Q_D( const LineAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [line] element
    QDomElement lineElement = document.createElement( "line" );
    node.appendChild( lineElement );

    // store the attributes
    if ( d->lineStartStyle != None )
        lineElement.setAttribute( "startStyle", (int)d->lineStartStyle );
    if ( d->lineEndStyle != None )
        lineElement.setAttribute( "endStyle", (int)d->lineEndStyle );
    if ( d->lineClosed )
        lineElement.setAttribute( "closed", d->lineClosed );
    if ( d->lineInnerColor.isValid() )
        lineElement.setAttribute( "innerColor", d->lineInnerColor.name() );
    if ( d->lineLeadingFwdPt != 0.0 )
        lineElement.setAttribute( "leadFwd", QString::number( d->lineLeadingFwdPt ) );
    if ( d->lineLeadingBackPt != 0.0 )
        lineElement.setAttribute( "leadBack", QString::number( d->lineLeadingBackPt ) );
    if ( d->lineShowCaption )
        lineElement.setAttribute( "showCaption", d->lineShowCaption );
    if ( d->lineIntent != Unknown )
        lineElement.setAttribute( "intent", d->lineIntent );

    // append the list of points
    int points = d->linePoints.count();
    if ( points > 1 )
    {
        QLinkedList<QPointF>::const_iterator it = d->linePoints.begin(), end = d->linePoints.end();
        while ( it != end )
        {
            const QPointF & p = *it;
            QDomElement pElement = document.createElement( "point" );
            lineElement.appendChild( pElement );
            pElement.setAttribute( "x", QString::number( p.x() ) );
            pElement.setAttribute( "y", QString::number( p.y() ) );
            ++it;
        }
    }
}

Annotation::SubType LineAnnotation::subType() const
{
    return ALine;
}

QLinkedList<QPointF> LineAnnotation::linePoints() const
{
    Q_D( const LineAnnotation );
    return d->linePoints;
}

void LineAnnotation::setLinePoints( const QLinkedList<QPointF> &points )
{
    Q_D( LineAnnotation );
    d->linePoints = points;
}

LineAnnotation::TermStyle LineAnnotation::lineStartStyle() const
{
    Q_D( const LineAnnotation );
    return d->lineStartStyle;
}

void LineAnnotation::setLineStartStyle( LineAnnotation::TermStyle style )
{
    Q_D( LineAnnotation );
    d->lineStartStyle = style;
}

LineAnnotation::TermStyle LineAnnotation::lineEndStyle() const
{
    Q_D( const LineAnnotation );
    return d->lineEndStyle;
}

void LineAnnotation::setLineEndStyle( LineAnnotation::TermStyle style )
{
    Q_D( LineAnnotation );
    d->lineEndStyle = style;
}

bool LineAnnotation::isLineClosed() const
{
    Q_D( const LineAnnotation );
    return d->lineClosed;
}

void LineAnnotation::setLineClosed( bool closed )
{
    Q_D( LineAnnotation );
    d->lineClosed = closed;
}

QColor LineAnnotation::lineInnerColor() const
{
    Q_D( const LineAnnotation );
    return d->lineInnerColor;
}

void LineAnnotation::setLineInnerColor( const QColor &color )
{
    Q_D( LineAnnotation );
    d->lineInnerColor = color;
}

double LineAnnotation::lineLeadingForwardPoint() const
{
    Q_D( const LineAnnotation );
    return d->lineLeadingFwdPt;
}

void LineAnnotation::setLineLeadingForwardPoint( double point )
{
    Q_D( LineAnnotation );
    d->lineLeadingFwdPt = point;
}

double LineAnnotation::lineLeadingBackPoint() const
{
    Q_D( const LineAnnotation );
    return d->lineLeadingBackPt;
}

void LineAnnotation::setLineLeadingBackPoint( double point )
{
    Q_D( LineAnnotation );
    d->lineLeadingBackPt = point;
}

bool LineAnnotation::lineShowCaption() const
{
    Q_D( const LineAnnotation );
    return d->lineShowCaption;
}

void LineAnnotation::setLineShowCaption( bool show )
{
    Q_D( LineAnnotation );
    d->lineShowCaption = show;
}

LineAnnotation::LineIntent LineAnnotation::lineIntent() const
{
    Q_D( const LineAnnotation );
    return d->lineIntent;
}

void LineAnnotation::setLineIntent( LineAnnotation::LineIntent intent )
{
    Q_D( LineAnnotation );
    d->lineIntent = intent;
}


/** GeomAnnotation [Annotation] */
class GeomAnnotationPrivate : public AnnotationPrivate
{
    public:
        GeomAnnotationPrivate();

        // data fields (note uses border for rendering style)
        GeomAnnotation::GeomType geomType;
        QColor geomInnerColor;
        int geomWidthPt;
};

GeomAnnotationPrivate::GeomAnnotationPrivate()
    : AnnotationPrivate(), geomType( GeomAnnotation::InscribedSquare ),
    geomWidthPt( 18 )
{
}

GeomAnnotation::GeomAnnotation()
    : Annotation( *new GeomAnnotationPrivate() )
{}

GeomAnnotation::GeomAnnotation( const QDomNode & node )
    : Annotation( *new GeomAnnotationPrivate(), node )
{
    Q_D( GeomAnnotation );

    // loop through the whole children looking for a 'geom' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "geom" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "type" ) )
            d->geomType = (GeomAnnotation::GeomType)e.attribute( "type" ).toInt();
        if ( e.hasAttribute( "color" ) )
            d->geomInnerColor = QColor( e.attribute( "color" ) );
        if ( e.hasAttribute( "width" ) )
            d->geomWidthPt = e.attribute( "width" ).toInt();

        // loading complete
        break;
    }
}

GeomAnnotation::~GeomAnnotation()
{
}

void GeomAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    Q_D( const GeomAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [geom] element
    QDomElement geomElement = document.createElement( "geom" );
    node.appendChild( geomElement );

    // append the optional attributes
    if ( d->geomType != InscribedSquare )
        geomElement.setAttribute( "type", (int)d->geomType );
    if ( d->geomInnerColor.isValid() )
        geomElement.setAttribute( "color", d->geomInnerColor.name() );
    if ( d->geomWidthPt != 18 )
        geomElement.setAttribute( "width", d->geomWidthPt );
}

Annotation::SubType GeomAnnotation::subType() const
{
    return AGeom;
}

GeomAnnotation::GeomType GeomAnnotation::geomType() const
{
    Q_D( const GeomAnnotation );
    return d->geomType;
}

void GeomAnnotation::setGeomType( GeomAnnotation::GeomType type )
{
    Q_D( GeomAnnotation );
    d->geomType = type;
}

QColor GeomAnnotation::geomInnerColor() const
{
    Q_D( const GeomAnnotation );
    return d->geomInnerColor;
}

void GeomAnnotation::setGeomInnerColor( const QColor &color )
{
    Q_D( GeomAnnotation );
    d->geomInnerColor = color;
}

int GeomAnnotation::geomPointWidth() const
{
    Q_D( const GeomAnnotation );
    return d->geomWidthPt;
}

void GeomAnnotation::setGeomPointWidth( int width )
{
    Q_D( GeomAnnotation );
    d->geomWidthPt = width;
}



/** HighlightAnnotation [Annotation] */
class HighlightAnnotationPrivate : public AnnotationPrivate
{
    public:
        HighlightAnnotationPrivate();

        // data fields
        HighlightAnnotation::HighlightType highlightType;
        QList< HighlightAnnotation::Quad > highlightQuads; // not empty
};

HighlightAnnotationPrivate::HighlightAnnotationPrivate()
    : AnnotationPrivate(), highlightType( HighlightAnnotation::Highlight )
{
}

HighlightAnnotation::HighlightAnnotation()
    : Annotation( *new HighlightAnnotationPrivate() )
{}

HighlightAnnotation::HighlightAnnotation( const QDomNode & node )
    : Annotation( *new HighlightAnnotationPrivate(), node )
{
    Q_D( HighlightAnnotation );

    // loop through the whole children looking for a 'hl' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "hl" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "type" ) )
            d->highlightType = (HighlightAnnotation::HighlightType)e.attribute( "type" ).toInt();

        // parse all 'quad' subnodes
        QDomNode quadNode = e.firstChild();
        for ( ; quadNode.isElement(); quadNode = quadNode.nextSibling() )
        {
            QDomElement qe = quadNode.toElement();
            if ( qe.tagName() != "quad" )
                continue;

            Quad q;
            q.points[0].setX(qe.attribute( "ax", "0.0" ).toDouble());
            q.points[0].setY(qe.attribute( "ay", "0.0" ).toDouble());
            q.points[1].setX(qe.attribute( "bx", "0.0" ).toDouble());
            q.points[1].setY(qe.attribute( "by", "0.0" ).toDouble());
            q.points[2].setX(qe.attribute( "cx", "0.0" ).toDouble());
            q.points[2].setY(qe.attribute( "cy", "0.0" ).toDouble());
            q.points[3].setX(qe.attribute( "dx", "0.0" ).toDouble());
            q.points[3].setY(qe.attribute( "dy", "0.0" ).toDouble());
            q.capStart = qe.hasAttribute( "start" );
            q.capEnd = qe.hasAttribute( "end" );
            q.feather = qe.attribute( "feather", "0.1" ).toDouble();
            d->highlightQuads.append( q );
        }

        // loading complete
        break;
    }
}

HighlightAnnotation::~HighlightAnnotation()
{
}

void HighlightAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    Q_D( const HighlightAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [hl] element
    QDomElement hlElement = document.createElement( "hl" );
    node.appendChild( hlElement );

    // append the optional attributes
    if ( d->highlightType != Highlight )
        hlElement.setAttribute( "type", (int)d->highlightType );
    if ( d->highlightQuads.count() < 1 )
        return;
    // append highlight quads, all children describe quads
    QList< HighlightAnnotation::Quad >::const_iterator it = d->highlightQuads.begin(), end = d->highlightQuads.end();
    for ( ; it != end; ++it )
    {
        QDomElement quadElement = document.createElement( "quad" );
        hlElement.appendChild( quadElement );
        const Quad & q = *it;
        quadElement.setAttribute( "ax", QString::number( q.points[0].x() ) );
        quadElement.setAttribute( "ay", QString::number( q.points[0].y() ) );
        quadElement.setAttribute( "bx", QString::number( q.points[1].x() ) );
        quadElement.setAttribute( "by", QString::number( q.points[1].y() ) );
        quadElement.setAttribute( "cx", QString::number( q.points[2].x() ) );
        quadElement.setAttribute( "cy", QString::number( q.points[2].y() ) );
        quadElement.setAttribute( "dx", QString::number( q.points[3].x() ) );
        quadElement.setAttribute( "dy", QString::number( q.points[3].y() ) );
        if ( q.capStart )
            quadElement.setAttribute( "start", 1 );
        if ( q.capEnd )
            quadElement.setAttribute( "end", 1 );
        quadElement.setAttribute( "feather", QString::number( q.feather ) );
    }
}

Annotation::SubType HighlightAnnotation::subType() const
{
    return AHighlight;
}

HighlightAnnotation::HighlightType HighlightAnnotation::highlightType() const
{
    Q_D( const HighlightAnnotation );
    return d->highlightType;
}

void HighlightAnnotation::setHighlightType( HighlightAnnotation::HighlightType type )
{
    Q_D( HighlightAnnotation );
    d->highlightType = type;
}

QList< HighlightAnnotation::Quad > HighlightAnnotation::highlightQuads() const
{
    Q_D( const HighlightAnnotation );
    return d->highlightQuads;
}

void HighlightAnnotation::setHighlightQuads( const QList< HighlightAnnotation::Quad > &quads )
{
    Q_D( HighlightAnnotation );
    d->highlightQuads = quads;
}


/** StampAnnotation [Annotation] */
class StampAnnotationPrivate : public AnnotationPrivate
{
    public:
        StampAnnotationPrivate();

        // data fields
        QString stampIconName;
};

StampAnnotationPrivate::StampAnnotationPrivate()
    : AnnotationPrivate(), stampIconName( "Draft" )
{
}

StampAnnotation::StampAnnotation()
    : Annotation( *new StampAnnotationPrivate() )
{}

StampAnnotation::StampAnnotation( const QDomNode & node )
    : Annotation( *new StampAnnotationPrivate(), node )
{
   Q_D( StampAnnotation );

    // loop through the whole children looking for a 'stamp' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "stamp" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "icon" ) )
            d->stampIconName = e.attribute( "icon" );

        // loading complete
        break;
    }
}

StampAnnotation::~StampAnnotation()
{
}

void StampAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
   Q_D( const StampAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [stamp] element
    QDomElement stampElement = document.createElement( "stamp" );
    node.appendChild( stampElement );

    // append the optional attributes
    if ( d->stampIconName != "Draft" )
        stampElement.setAttribute( "icon", d->stampIconName );
}

Annotation::SubType StampAnnotation::subType() const
{
    return AStamp;
}

QString StampAnnotation::stampIconName() const
{
    Q_D( const StampAnnotation );
    return d->stampIconName;
}

void StampAnnotation::setStampIconName( const QString &name )
{
    Q_D( StampAnnotation );
    d->stampIconName = name;
}

/** InkAnnotation [Annotation] */
class InkAnnotationPrivate : public AnnotationPrivate
{
    public:
        InkAnnotationPrivate();

        // data fields
        QList< QLinkedList<QPointF> > inkPaths;
};

InkAnnotationPrivate::InkAnnotationPrivate()
    : AnnotationPrivate()
{
}

InkAnnotation::InkAnnotation()
    : Annotation( *new InkAnnotationPrivate() )
{}

InkAnnotation::InkAnnotation( const QDomNode & node )
    : Annotation( *new InkAnnotationPrivate(), node )
{
    Q_D( InkAnnotation );

    // loop through the whole children looking for a 'ink' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "ink" )
            continue;

        // parse the 'path' subnodes
        QDomNode pathNode = e.firstChild();
        while ( pathNode.isElement() )
        {
            QDomElement pathElement = pathNode.toElement();
            pathNode = pathNode.nextSibling();

            if ( pathElement.tagName() != "path" )
                continue;

            // build each path parsing 'point' subnodes
            QLinkedList<QPointF> path;
            QDomNode pointNode = pathElement.firstChild();
            while ( pointNode.isElement() )
            {
                QDomElement pointElement = pointNode.toElement();
                pointNode = pointNode.nextSibling();

                if ( pointElement.tagName() != "point" )
                    continue;

                QPointF p(pointElement.attribute( "x", "0.0" ).toDouble(), pointElement.attribute( "y", "0.0" ).toDouble());
                path.append( p );
            }

            // add the path to the path list if it contains at least 2 nodes
            if ( path.count() >= 2 )
                d->inkPaths.append( path );
        }

        // loading complete
        break;
    }
}

InkAnnotation::~InkAnnotation()
{
}

void InkAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    Q_D( const InkAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [ink] element
    QDomElement inkElement = document.createElement( "ink" );
    node.appendChild( inkElement );

    // append the optional attributes
    if ( d->inkPaths.count() < 1 )
        return;
    QList< QLinkedList<QPointF> >::const_iterator pIt = d->inkPaths.begin(), pEnd = d->inkPaths.end();
    for ( ; pIt != pEnd; ++pIt )
    {
        QDomElement pathElement = document.createElement( "path" );
        inkElement.appendChild( pathElement );
        const QLinkedList<QPointF> & path = *pIt;
        QLinkedList<QPointF>::const_iterator iIt = path.begin(), iEnd = path.end();
        for ( ; iIt != iEnd; ++iIt )
        {
            const QPointF & point = *iIt;
            QDomElement pointElement = document.createElement( "point" );
            pathElement.appendChild( pointElement );
            pointElement.setAttribute( "x", QString::number( point.x() ) );
            pointElement.setAttribute( "y", QString::number( point.y() ) );
        }
    }
}

Annotation::SubType InkAnnotation::subType() const
{
    return AInk;
}

QList< QLinkedList<QPointF> > InkAnnotation::inkPaths() const
{
   Q_D( const InkAnnotation );
   return d->inkPaths;
}

void InkAnnotation::setInkPaths( const QList< QLinkedList<QPointF> > &paths )
{
   Q_D( InkAnnotation );
   d->inkPaths = paths;
}


/** LinkAnnotation [Annotation] */
class LinkAnnotationPrivate : public AnnotationPrivate
{
    public:
        LinkAnnotationPrivate();
        ~LinkAnnotationPrivate();

        // data fields
        Link * linkDestination;
        LinkAnnotation::HighlightMode linkHLMode;
        QPointF linkRegion[4];
};

LinkAnnotationPrivate::LinkAnnotationPrivate()
    : AnnotationPrivate(), linkDestination( 0 ), linkHLMode( LinkAnnotation::Invert )
{
}

LinkAnnotationPrivate::~LinkAnnotationPrivate()
{
    delete linkDestination;
}

LinkAnnotation::LinkAnnotation()
    : Annotation( *new LinkAnnotationPrivate() )
{}

LinkAnnotation::LinkAnnotation( const QDomNode & node )
    : Annotation( *new LinkAnnotationPrivate(), node )
{
    Q_D( LinkAnnotation );

    // loop through the whole children looking for a 'link' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "link" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "hlmode" ) )
            d->linkHLMode = (LinkAnnotation::HighlightMode)e.attribute( "hlmode" ).toInt();

        // parse all 'quad' subnodes
        QDomNode quadNode = e.firstChild();
        for ( ; quadNode.isElement(); quadNode = quadNode.nextSibling() )
        {
            QDomElement qe = quadNode.toElement();
            if ( qe.tagName() == "quad" )
            {
                d->linkRegion[0].setX(qe.attribute( "ax", "0.0" ).toDouble());
                d->linkRegion[0].setY(qe.attribute( "ay", "0.0" ).toDouble());
                d->linkRegion[1].setX(qe.attribute( "bx", "0.0" ).toDouble());
                d->linkRegion[1].setY(qe.attribute( "by", "0.0" ).toDouble());
                d->linkRegion[2].setX(qe.attribute( "cx", "0.0" ).toDouble());
                d->linkRegion[2].setY(qe.attribute( "cy", "0.0" ).toDouble());
                d->linkRegion[3].setX(qe.attribute( "dx", "0.0" ).toDouble());
                d->linkRegion[3].setY(qe.attribute( "dy", "0.0" ).toDouble());
            }
            else if ( qe.tagName() == "link" )
            {
                QString type = qe.attribute( "type" );
                if ( type == "GoTo" )
                {
                    Poppler::LinkGoto * go = new Poppler::LinkGoto( QRect(), qe.attribute( "filename" ), LinkDestination( qe.attribute( "destination" ) ) );
                    d->linkDestination = go;
                }
                else if ( type == "Exec" )
                {
                    Poppler::LinkExecute * exec = new Poppler::LinkExecute( QRect(), qe.attribute( "filename" ), qe.attribute( "parameters" ) );
                    d->linkDestination = exec;
                }
                else if ( type == "Browse" )
                {
                    Poppler::LinkBrowse * browse = new Poppler::LinkBrowse( QRect(), qe.attribute( "url" ) );
                    d->linkDestination = browse;
                }
                else if ( type == "Action" )
                {
                    Poppler::LinkAction::ActionType act;
                    QString actString = qe.attribute( "action" );
                    bool found = true;
                    if ( actString == "PageFirst" )
                        act = Poppler::LinkAction::PageFirst;
                    else if ( actString == "PagePrev" )
                        act = Poppler::LinkAction::PagePrev;
                    else if ( actString == "PageNext" )
                        act = Poppler::LinkAction::PageNext;
                    else if ( actString == "PageLast" )
                        act = Poppler::LinkAction::PageLast;
                    else if ( actString == "HistoryBack" )
                        act = Poppler::LinkAction::HistoryBack;
                    else if ( actString == "HistoryForward" )
                        act = Poppler::LinkAction::HistoryForward;
                    else if ( actString == "Quit" )
                        act = Poppler::LinkAction::Quit;
                    else if ( actString == "Presentation" )
                        act = Poppler::LinkAction::Presentation;
                    else if ( actString == "EndPresentation" )
                        act = Poppler::LinkAction::EndPresentation;
                    else if ( actString == "Find" )
                        act = Poppler::LinkAction::Find;
                    else if ( actString == "GoToPage" )
                        act = Poppler::LinkAction::GoToPage;
                    else if ( actString == "Close" )
                        act = Poppler::LinkAction::Close;
                    else if ( actString == "Print" )
                        act = Poppler::LinkAction::Print;
                    else
                        found = false;
                    if (found)
                    {
                        Poppler::LinkAction * action = new Poppler::LinkAction( QRect(), act );
                        d->linkDestination = action;
                    }
                }
#if 0
                else if ( type == "Movie" )
                {
                    Poppler::LinkMovie * movie = new Poppler::LinkMovie( QRect() );
                    d->linkDestination = movie;
                }
#endif
            }
        }

        // loading complete
        break;
    }
}

LinkAnnotation::~LinkAnnotation()
{
}

void LinkAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    Q_D( const LinkAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [hl] element
    QDomElement linkElement = document.createElement( "link" );
    node.appendChild( linkElement );

    // append the optional attributes
    if ( d->linkHLMode != Invert )
        linkElement.setAttribute( "hlmode", (int)d->linkHLMode );

    // saving region
    QDomElement quadElement = document.createElement( "quad" );
    linkElement.appendChild( quadElement );
    quadElement.setAttribute( "ax", QString::number( d->linkRegion[0].x() ) );
    quadElement.setAttribute( "ay", QString::number( d->linkRegion[0].y() ) );
    quadElement.setAttribute( "bx", QString::number( d->linkRegion[1].x() ) );
    quadElement.setAttribute( "by", QString::number( d->linkRegion[1].y() ) );
    quadElement.setAttribute( "cx", QString::number( d->linkRegion[2].x() ) );
    quadElement.setAttribute( "cy", QString::number( d->linkRegion[2].y() ) );
    quadElement.setAttribute( "dx", QString::number( d->linkRegion[3].x() ) );
    quadElement.setAttribute( "dy", QString::number( d->linkRegion[3].y() ) );

    // saving link
    QDomElement hyperlinkElement = document.createElement( "link" );
    linkElement.appendChild( hyperlinkElement );
    if ( d->linkDestination )
    {
        switch( d->linkDestination->linkType() )
        {
            case Poppler::Link::Goto:
            {
                Poppler::LinkGoto * go = static_cast< Poppler::LinkGoto * >( d->linkDestination );
                hyperlinkElement.setAttribute( "type", "GoTo" );
                hyperlinkElement.setAttribute( "filename", go->fileName() );
                hyperlinkElement.setAttribute( "destionation", go->destination().toString() );
                break;
            }
            case Poppler::Link::Execute:
            {
                Poppler::LinkExecute * exec = static_cast< Poppler::LinkExecute * >( d->linkDestination );
                hyperlinkElement.setAttribute( "type", "Exec" );
                hyperlinkElement.setAttribute( "filename", exec->fileName() );
                hyperlinkElement.setAttribute( "parameters", exec->parameters() );
                break;
            }
            case Poppler::Link::Browse:
            {
                Poppler::LinkBrowse * browse = static_cast< Poppler::LinkBrowse * >( d->linkDestination );
                hyperlinkElement.setAttribute( "type", "Browse" );
                hyperlinkElement.setAttribute( "url", browse->url() );
                break;
            }
            case Poppler::Link::Action:
            {
                Poppler::LinkAction * action = static_cast< Poppler::LinkAction * >( d->linkDestination );
                hyperlinkElement.setAttribute( "type", "Action" );
                switch ( action->actionType() )
                {
                    case Poppler::LinkAction::PageFirst:
                        hyperlinkElement.setAttribute( "action", "PageFirst" );
                        break;
                    case Poppler::LinkAction::PagePrev:
                        hyperlinkElement.setAttribute( "action", "PagePrev" );
                        break;
                    case Poppler::LinkAction::PageNext:
                        hyperlinkElement.setAttribute( "action", "PageNext" );
                        break;
                    case Poppler::LinkAction::PageLast:
                        hyperlinkElement.setAttribute( "action", "PageLast" );
                        break;
                    case Poppler::LinkAction::HistoryBack:
                        hyperlinkElement.setAttribute( "action", "HistoryBack" );
                        break;
                    case Poppler::LinkAction::HistoryForward:
                        hyperlinkElement.setAttribute( "action", "HistoryForward" );
                        break;
                    case Poppler::LinkAction::Quit:
                        hyperlinkElement.setAttribute( "action", "Quit" );
                        break;
                    case Poppler::LinkAction::Presentation:
                        hyperlinkElement.setAttribute( "action", "Presentation" );
                        break;
                    case Poppler::LinkAction::EndPresentation:
                        hyperlinkElement.setAttribute( "action", "EndPresentation" );
                        break;
                    case Poppler::LinkAction::Find:
                        hyperlinkElement.setAttribute( "action", "Find" );
                        break;
                    case Poppler::LinkAction::GoToPage:
                        hyperlinkElement.setAttribute( "action", "GoToPage" );
                        break;
                    case Poppler::LinkAction::Close:
                        hyperlinkElement.setAttribute( "action", "Close" );
                        break;
                    case Poppler::LinkAction::Print:
                        hyperlinkElement.setAttribute( "action", "Print" );
                        break;
                }
                break;
            }
            case Poppler::Link::Movie:
            {
                hyperlinkElement.setAttribute( "type", "Movie" );
                break;
            }
            case Poppler::Link::Sound:
            {
                // FIXME: implement me
                break;
            }
            case Poppler::Link::None:
                break;
        }
    }
}

Annotation::SubType LinkAnnotation::subType() const
{
    return ALink;
}

Link* LinkAnnotation::linkDestionation() const
{
    Q_D( const LinkAnnotation );
    return d->linkDestination;
}

void LinkAnnotation::setLinkDestination( Link *link )
{
    Q_D( LinkAnnotation );
    delete d->linkDestination;
    d->linkDestination = link;
}

LinkAnnotation::HighlightMode LinkAnnotation::linkHighlightMode() const
{
    Q_D( const LinkAnnotation );
    return d->linkHLMode;
}

void LinkAnnotation::setLinkHighlightMode( LinkAnnotation::HighlightMode mode )
{
    Q_D( LinkAnnotation );
    d->linkHLMode = mode;
}

QPointF LinkAnnotation::linkRegionPoint( int id ) const
{
    if ( id < 0 || id >= 4 )
        return QPointF();

    Q_D( const LinkAnnotation );
    return d->linkRegion[id];
}

void LinkAnnotation::setLinkRegionPoint( int id, const QPointF &point )
{
    if ( id < 0 || id >= 4 )
        return;

    Q_D( LinkAnnotation );
    d->linkRegion[id] = point;
}

/** CaretAnnotation [Annotation] */
class CaretAnnotationPrivate : public AnnotationPrivate
{
    public:
        CaretAnnotationPrivate();

        // data fields
        CaretAnnotation::CaretSymbol symbol;
};

static QString caretSymbolToString( CaretAnnotation::CaretSymbol symbol )
{
    switch ( symbol )
    {
        case CaretAnnotation::None:
            return QString::fromLatin1( "None" );
        case CaretAnnotation::P:
            return QString::fromLatin1( "P" );
    }
    return QString();
}

static CaretAnnotation::CaretSymbol caretSymbolFromString( const QString &symbol )
{
    if ( symbol == QLatin1String( "None" ) )
        return CaretAnnotation::None;
    else if ( symbol == QLatin1String( "P" ) )
        return CaretAnnotation::P;
    return CaretAnnotation::None;
}

CaretAnnotationPrivate::CaretAnnotationPrivate()
    : AnnotationPrivate(), symbol( CaretAnnotation::None )
{
}

CaretAnnotation::CaretAnnotation()
    : Annotation( *new CaretAnnotationPrivate() )
{
}

CaretAnnotation::CaretAnnotation( const QDomNode & node )
    : Annotation( *new CaretAnnotationPrivate(), node )
{
    Q_D( CaretAnnotation );

    // loop through the whole children looking for a 'caret' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "caret" )
            continue;

        // parse the attributes
        if ( e.hasAttribute( "symbol" ) )
            d->symbol = caretSymbolFromString( e.attribute( "symbol" ) );

        // loading complete
        break;
    }
}

CaretAnnotation::~CaretAnnotation()
{
}

void CaretAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    Q_D( const CaretAnnotation );

    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [caret] element
    QDomElement caretElement = document.createElement( "caret" );
    node.appendChild( caretElement );

    // append the optional attributes
    if ( d->symbol != CaretAnnotation::None )
        caretElement.setAttribute( "symbol", caretSymbolToString( d->symbol ) );
}

Annotation::SubType CaretAnnotation::subType() const
{
    return ACaret;
}

CaretAnnotation::CaretSymbol CaretAnnotation::caretSymbol() const
{
    Q_D( const CaretAnnotation );
    return d->symbol;
}

void CaretAnnotation::setCaretSymbol( CaretAnnotation::CaretSymbol symbol )
{
    Q_D( CaretAnnotation );
    d->symbol = symbol;
}

/** FileAttachmentAnnotation [Annotation] */
class FileAttachmentAnnotationPrivate : public AnnotationPrivate
{
    public:
        FileAttachmentAnnotationPrivate();
        ~FileAttachmentAnnotationPrivate();

        // data fields
        QString icon;
        EmbeddedFile *embfile;
};

FileAttachmentAnnotationPrivate::FileAttachmentAnnotationPrivate()
    : AnnotationPrivate(), icon( "PushPin" ), embfile( 0 )
{
}

FileAttachmentAnnotationPrivate::~FileAttachmentAnnotationPrivate()
{
    delete embfile;
}

FileAttachmentAnnotation::FileAttachmentAnnotation()
    : Annotation( *new FileAttachmentAnnotationPrivate() )
{
}

FileAttachmentAnnotation::FileAttachmentAnnotation( const QDomNode & node )
    : Annotation( *new FileAttachmentAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'fileattachment' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "fileattachment" )
            continue;

        // loading complete
        break;
    }
}

FileAttachmentAnnotation::~FileAttachmentAnnotation()
{
}

void FileAttachmentAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [fileattachment] element
    QDomElement fileAttachmentElement = document.createElement( "fileattachment" );
    node.appendChild( fileAttachmentElement );
}

Annotation::SubType FileAttachmentAnnotation::subType() const
{
    return AFileAttachment;
}

QString FileAttachmentAnnotation::fileIconName() const
{
    Q_D( const FileAttachmentAnnotation );
    return d->icon;
}

void FileAttachmentAnnotation::setFileIconName( const QString &icon )
{
    Q_D( FileAttachmentAnnotation );
    d->icon = icon;
}

EmbeddedFile* FileAttachmentAnnotation::embeddedFile() const
{
    Q_D( const FileAttachmentAnnotation );
    return d->embfile;
}

void FileAttachmentAnnotation::setEmbeddedFile( EmbeddedFile *ef )
{
    Q_D( FileAttachmentAnnotation );
    d->embfile = ef;
}

/** SoundAnnotation [Annotation] */
class SoundAnnotationPrivate : public AnnotationPrivate
{
    public:
        SoundAnnotationPrivate();
        ~SoundAnnotationPrivate();

        // data fields
        QString icon;
        SoundObject *sound;
};

SoundAnnotationPrivate::SoundAnnotationPrivate()
    : AnnotationPrivate(), icon( "Speaker" ), sound( 0 )
{
}

SoundAnnotationPrivate::~SoundAnnotationPrivate()
{
    delete sound;
}

SoundAnnotation::SoundAnnotation()
    : Annotation( *new SoundAnnotationPrivate() )
{
}

SoundAnnotation::SoundAnnotation( const QDomNode & node )
    : Annotation( *new SoundAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'sound' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "sound" )
            continue;

        // loading complete
        break;
    }
}

SoundAnnotation::~SoundAnnotation()
{
}

void SoundAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [sound] element
    QDomElement soundElement = document.createElement( "sound" );
    node.appendChild( soundElement );
}

Annotation::SubType SoundAnnotation::subType() const
{
    return ASound;
}

QString SoundAnnotation::soundIconName() const
{
    Q_D( const SoundAnnotation );
    return d->icon;
}

void SoundAnnotation::setSoundIconName( const QString &icon )
{
    Q_D( SoundAnnotation );
    d->icon = icon;
}

SoundObject* SoundAnnotation::sound() const
{
    Q_D( const SoundAnnotation );
    return d->sound;
}

void SoundAnnotation::setSound( SoundObject *s )
{
    Q_D( SoundAnnotation );
    d->sound = s;
}

/** MovieAnnotation [Annotation] */
class MovieAnnotationPrivate : public AnnotationPrivate
{
    public:
        MovieAnnotationPrivate();
        ~MovieAnnotationPrivate();

        // data fields
        MovieObject *movie;
        QString title;
};

MovieAnnotationPrivate::MovieAnnotationPrivate()
    : AnnotationPrivate(), movie( 0 )
{
}

MovieAnnotationPrivate::~MovieAnnotationPrivate()
{
    delete movie;
}

MovieAnnotation::MovieAnnotation()
    : Annotation( *new MovieAnnotationPrivate() )
{
}

MovieAnnotation::MovieAnnotation( const QDomNode & node )
    : Annotation( *new MovieAnnotationPrivate(), node )
{
    // loop through the whole children looking for a 'movie' element
    QDomNode subNode = node.firstChild();
    while( subNode.isElement() )
    {
        QDomElement e = subNode.toElement();
        subNode = subNode.nextSibling();
        if ( e.tagName() != "movie" )
            continue;

        // loading complete
        break;
    }
}

MovieAnnotation::~MovieAnnotation()
{
}

void MovieAnnotation::store( QDomNode & node, QDomDocument & document ) const
{
    // recurse to parent objects storing properties
    Annotation::store( node, document );

    // create [movie] element
    QDomElement movieElement = document.createElement( "movie" );
    node.appendChild( movieElement );
}

Annotation::SubType MovieAnnotation::subType() const
{
    return AMovie;
}

MovieObject* MovieAnnotation::movie() const
{
    Q_D( const MovieAnnotation );
    return d->movie;
}

void MovieAnnotation::setMovie( MovieObject *movie )
{
    Q_D( MovieAnnotation );
    d->movie = movie;
}

QString MovieAnnotation::movieTitle() const
{
    Q_D( const MovieAnnotation );
    return d->title;
}

void MovieAnnotation::setMovieTitle( const QString &title )
{
    Q_D( MovieAnnotation );
    d->title = title;
}

//BEGIN utility annotation functions
QColor convertAnnotColor( AnnotColor *color )
{
    if ( !color )
        return QColor();

    QColor newcolor;
    const double *color_data = color->getValues();
    switch ( color->getSpace() )
    {
        case AnnotColor::colorTransparent: // = 0,
            newcolor = Qt::transparent;
            break;
        case AnnotColor::colorGray: // = 1,
            newcolor.setRgbF( color_data[0], color_data[0], color_data[0] );
            break;
        case AnnotColor::colorRGB: // = 3,
            newcolor.setRgbF( color_data[0], color_data[1], color_data[2] );
            break;
        case AnnotColor::colorCMYK: // = 4
            newcolor.setCmykF( color_data[0], color_data[1], color_data[2], color_data[3] );
            break;
    }
    return newcolor;
}
//END utility annotation functions

}
